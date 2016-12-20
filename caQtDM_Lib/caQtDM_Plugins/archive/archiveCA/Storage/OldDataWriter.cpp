
// Tools
#include "MsgLogger.h"
#include "Filename.h"
// Storage
#include "DataFile.h"
#include "OldDataWriter.h"

// TODO: Switch to new data file after
// time limit or file size limit

static stdString makeDataFileName()
{
    int year, month, day, hour, min, sec;
    unsigned long nano;
    char buffer[80];
    
    epicsTime now = epicsTime::getCurrent();    
    //    if (getSecsPerFile() == SECS_PER_MONTH)
    //{
    epicsTime2vals(now, year, month, day, hour, min, sec, nano);
    sprintf(buffer, "%04d%02d%02d", year, month, day);
    return stdString(buffer);
    //}
    //now = roundTimeDown(now, _secs_per_file);
    //epicsTime2vals(now, year, month, day, hour, min, sec, nano);
    //sprintf(buffer, "%04d%02d%02d-%02d%02d%02d", year, month, day, hour, min, sec);
    //return stdString(buffer);
}

OldDataWriter::OldDataWriter(OldDirectoryFile &index,
                             const stdString &channel_name,
                             const CtrlInfo &ctrl_info,
                             DbrType dbr_type,
                             DbrCount dbr_count,
                             double period,
                             size_t num_samples)
  : index(index),
    channel_name(channel_name),
    ctrl_info(ctrl_info),
    dbr_type(dbr_type),
    dbr_count(dbr_count),
    period(period),
    raw_value_size(RawValue::getSize(dbr_type, dbr_count)),
    available(0)
{
    DataFile *datafile = 0;
    try
    {
        // Size of next buffer should at least hold num_samples
        calc_next_buffer_size(num_samples);
        // Find or add channel name
        dfi = index.find(channel_name);
        if (!dfi.isValid())
            dfi = index.add(channel_name);
        if (!dfi.isValid())
        {
            LOG_MSG ("OldDataWriter: Cannot add '%s' to index\n",
                     channel_name.c_str());
            return;
        }
        // Find or add appropriate data buffer
        if (!Filename::isValid(dfi.entry.data.last_file))
        {   // - There is no datafile, no buffer
            // Create data file
            stdString data_file_name = makeDataFileName();
            datafile = DataFile::reference(index.getDirname(),
                                           data_file_name, true);
            // add CtrlInfo
            FileOffset ctrl_info_offset;
            datafile->addCtrlInfo(ctrl_info, ctrl_info_offset);
            // add first header
            header = datafile->addHeader(channel_name, dbr_type, dbr_count,
                                         period, next_buffer_size);
            datafile->release(); // now ref'ed by header
            datafile = 0;
            header->data.ctrl_info_offset = ctrl_info_offset;
            // Upp the buffer size
            calc_next_buffer_size(next_buffer_size);
            // Will add to index when we release the buffer
        }
        else
        {   // - There is a data file and buffer
            datafile = DataFile::reference(index.getDirname(),
                                           dfi.entry.data.last_file, true);
            header = datafile->getHeader(dfi.entry.data.last_offset);
            datafile->release(); // now ref'ed by header
            datafile = 0;
            // See if anything has changed
            CtrlInfo prev_ctrl_info;
            prev_ctrl_info.read(header->datafile,
                                header->data.ctrl_info_offset);
            if (prev_ctrl_info != ctrl_info)
            {   // Add new header because info has changed
                addNewHeader(true);
            }
            else if (header->data.dbr_type != dbr_type  ||
                     header->data.dbr_count != dbr_count)
            {   // Add new header because type has changed
                addNewHeader(false);
            }
            else
            {   // All fine, just check if we're already in bigger league
                size_t capacity = header->capacity();
                if (capacity > num_samples)
                    calc_next_buffer_size(capacity);
            }
        }   
        available = header->available();
    }
    catch (GenericException &e)
    {
        if (datafile)
        {
            datafile->release();
            datafile = 0;
        }
        throw e;
    }
}
        
OldDataWriter::~OldDataWriter()
{
    // Update index
    if (header)
    {
        if (dfi.isValid())
        {
            header->data.dir_offset = dfi.entry.offset;
            header->write();
            // if this is the first buffer, update the index's start
            if (dfi.entry.data.first_offset == INVALID_OFFSET ||
                !Filename::isValid(dfi.entry.data.first_file))
            {
                dfi.entry.setFirst(header->datafile->getBasename(),
                                   header->offset);
                dfi.entry.data.first_save_time = header->data.begin_time;
            }
            // Always update the index's end
            dfi.entry.setLast(header->datafile->getBasename(), header->offset);
            dfi.entry.data.last_save_time = header->data.end_time;
            dfi.save();
        }
        else
        {
            LOG_MSG("OldDataWriter(%s): Cannot update index, it's invalid\n",
                    channel_name.c_str());
        }
        header = 0;
    }
}

bool OldDataWriter::add(const RawValue::Data *data)
{
    if (!header) // In here, we should always have a header
        return false;
    if (available <= 0) // though it might be full
    {
        addNewHeader(false);
        available = header->available();
    }
    // Add the value
    available -= 1;
    FileOffset offset = header->offset
        + sizeof(DataHeader::DataHeaderData)
        + header->data.curr_offset;
    RawValue::write(dbr_type, dbr_count,
                    raw_value_size, data,
                    cvt_buffer,
                    header->datafile, offset);
    // Update the header
    header->data.curr_offset += raw_value_size;
    header->data.num_samples += 1;
    header->data.buf_free    -= raw_value_size;
    epicsTime time = RawValue::getTime(data);
    if (header->data.num_samples == 1) // first entry?
        header->data.begin_time = time;
    header->data.end_time = time;
    // Note: we didn't write the header nor the dfi,
    // that'll happen when we close the OldDataWriter!
    return true;
}

void OldDataWriter::calc_next_buffer_size(size_t start)
{
    // We want the next power of 2:
    int log2 = 0;
    while (start > 0)
    {
        start >>= 1;
        ++log2;
    }
    if (log2 < 6) // minumum: 2^6 == 64
        log2 = 6;
    if (log2 > 12) // maximum: 2^12 = 4096
        log2 = 12;
    next_buffer_size = 1 << log2;
}

// Helper: Assuming that we have
// a valid header, we add a new one,
// because the existing one is full or
// has the wrong ctrl_info
//
// Will write ctrl_info out if new_ctrl_info is set,
// otherwise the new header points to the existin ctrl_info
void OldDataWriter::addNewHeader(bool new_ctrl_info)
{
    LOG_ASSERT(header != 0);
    FileOffset ctrl_info_offset;
    if (new_ctrl_info)
        header->datafile->addCtrlInfo(ctrl_info, ctrl_info_offset);
    else // use existing one
        ctrl_info_offset = header->data.ctrl_info_offset;
        
    DataHeader *new_header =
        header->datafile->addHeader(channel_name, dbr_type, dbr_count, period,
                                    next_buffer_size);
    new_header->data.ctrl_info_offset = ctrl_info_offset;
    // Link old header to new one
    header->set_next(new_header->datafile->getBasename(),
                     new_header->offset);
    header->write();
    // back from new to old
    new_header->set_prev(header->datafile->getBasename(),
                         header->offset);        
    // Update index's start if this was the first header
    if (dfi.entry.data.first_offset == INVALID_OFFSET ||
        !Filename::isValid(dfi.entry.data.first_file))
    {
        dfi.entry.setFirst(header->datafile->getBasename(),
                           header->offset);
        dfi.entry.data.first_save_time = header->data.begin_time;
    }        
    // Switch to new header
    header = new_header;
    // Upp the buffer size
    calc_next_buffer_size(next_buffer_size);
}
    
