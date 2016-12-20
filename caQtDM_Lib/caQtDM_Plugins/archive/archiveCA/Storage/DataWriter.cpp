// DataWriter.cpp

// Tools
#include <MsgLogger.h>
#include <Filename.h>
#include <epicsTimeHelper.h>
// Storage
#include "DataFile.h"
#include "DataWriter.h"
#include "RTree.h"

// #define DEBUG_DATA_WRITER

FileOffset DataWriter::file_size_limit = 100*1024*1024; // 100MB Default.

stdString DataWriter::data_file_name_base;

DataWriter::DataWriter(Index &index,
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
    next_buffer_size(0),
    available(0)
{
    DataFile *datafile = 0;
    try
    {
        // Size of next buffer should at least hold num_samples
        calc_next_buffer_size(num_samples);

        // Find or add appropriate data buffer
        tree = index.addChannel(channel_name, directory);
        RTree::Datablock block;
        RTree::Node node(tree->getM(), true);
        int idx;
        if (tree->getLastDatablock(node, idx, block))        
        {   // - There is a data file and buffer
            datafile = DataFile::reference(directory, block.data_filename, true);
            header = datafile->getHeader(block.data_offset);
            datafile->release(); // now ref'ed by header
            datafile = 0;
            // See if anything has changed
            CtrlInfo prev_ctrl_info;
            prev_ctrl_info.read(header->datafile,
                                header->data.ctrl_info_offset);
            if (prev_ctrl_info != ctrl_info)
                // Add new header and new ctrl_info
                addNewHeader(true);
            else if (header->data.dbr_type != dbr_type  ||
                     header->data.dbr_count != dbr_count)
                // Add new header, use existing ctrl_info
                addNewHeader(false);
            else
            {   // All fine, just check if we're already in bigger league
                size_t capacity = header->capacity();
                if (capacity > num_samples)
                    calc_next_buffer_size(capacity);
            }
        }
        else // New data file, add the initial header
            addNewHeader(true);
        available = header->available();
    }
    catch (GenericException &e)
    {
        tree = 0;
        if (datafile)
            datafile->release();
        throw GenericException(__FILE__, __LINE__,
                               "Channel '%s':\n%s",
                               channel_name.c_str(),
                               e.what());
    }
}
    
DataWriter::~DataWriter()
{
    try
    {   // Update index
        if (header)
        {   
            header->write();
            if (tree)
            {
                if (!tree->updateLastDatablock(
                        header->data.begin_time, header->data.end_time,
                        header->offset, header->datafile->getBasename()))
                {
                    LOG_MSG("~DataWriter: updateLastDatablock '%s' %s @ 0x%lX was a NOP\n",
                            channel_name.c_str(),
                            header->datafile->getBasename().c_str(),
                            (unsigned long)header->offset);
                }
                tree = 0;
            }
            header = 0;
        }
    }
    catch (GenericException &e)
    {
        LOG_MSG("Exception in %s (%zu):\n%s\n",
                __FILE__, (size_t) __LINE__, e.what());
    }
    catch (...)
    {
        LOG_MSG("Unknown Exception in %s (%zu)\n\n",
                __FILE__, (size_t) __LINE__);
    }
}

epicsTime DataWriter::getLastStamp()
{
    if (header)
        return epicsTime(header->data.end_time);
    return nullTime;
}

bool DataWriter::add(const RawValue::Data *data)
{
    LOG_ASSERT(header);
    epicsTime data_stamp = RawValue::getTime(data);
    if (data_stamp < header->data.end_time)
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
    if (header->data.num_samples == 1) // first entry?
        header->data.begin_time = data_stamp;
    header->data.end_time = data_stamp;
    // Note: we didn't write the header nor update the index,
    // that'll happen when we close the DataWriter!
    return true;
}


void DataWriter::makeDataFileName(int serial, stdString &name)
{
    int  len;
    char buffer[30];    

    if (data_file_name_base.length() > 0)
    {
	name = data_file_name_base;
        if (serial > 0)
        {
            len = snprintf(buffer, sizeof(buffer), "-%d", serial);
            if (len >= (int)sizeof(buffer))
                len = sizeof(buffer)-1;
            name.append(buffer, len);
        }
        return;
    }
    // Else: Create name based on  "<today>[-serial]"
    int year, month, day, hour, min, sec;
    unsigned long nano;
    epicsTime now = epicsTime::getCurrent();    
    epicsTime2vals(now, year, month, day, hour, min, sec, nano);
    if (serial > 0)
        len = snprintf(buffer, sizeof(buffer),
                       "%04d%02d%02d-%d", year, month, day, serial);
    else
        len = snprintf(buffer,sizeof(buffer),
                       "%04d%02d%02d", year, month, day);
    if (len >= (int)sizeof(buffer))
        len = sizeof(buffer)-1;
    name.assign(buffer, len);
}

// Create new DataFile that's below file_size_limit in size.
DataFile *DataWriter::createNewDataFile(size_t headroom)
{
    DataFile *datafile = 0;
    int serial=0;
    stdString data_file_name;
    try
    {   // Keep opening existing data files until we find
        // one below the file limit.
        // We might have to create a new one.
        while (true)
        {
            makeDataFileName(serial, data_file_name);
            datafile = DataFile::reference(directory,
                                           data_file_name, true);
            FileOffset file_size = datafile->getSize();
            if (file_size+headroom < file_size_limit)
                return datafile;
            if (datafile->is_new_file)
            {
                LOG_MSG ("Warning: %s: "
                         "Cannot create a new data file within file size limit\n"
                         "type %d, count %d, %zu samples, file limit: %d bytes.\n",
                         channel_name.c_str(),
                         dbr_type, dbr_count, next_buffer_size, file_size_limit);
                return datafile; // Use anyway
            }
            // Try the next one.
            ++serial;
            datafile->release();
            datafile = 0;
        }
    }
    catch (GenericException &e)
    {
        if (datafile)
        {
            datafile->release();
            datafile = 0;
        }
        throw GenericException(__FILE__, __LINE__,
                               "Reference new datafile '%s':\n%s",
                               data_file_name.c_str(), e.what());
    }
    throw GenericException(__FILE__, __LINE__,
                           "createNewDataFile(%zu) failed", headroom);
    return 0;
}

void DataWriter::calc_next_buffer_size(size_t start)
{
    if (start < 64)
        next_buffer_size = 64;
    else if (start >= 4096)
        next_buffer_size = 4096;
    else
    {   // We want the next power of 2:
        int log2 = 0;
        size_t req = start;
        while (req > 0)
        {
            req >>= 1;
            ++log2;
        }
        next_buffer_size = 1 << log2;
    }

#ifdef DEBUG_DATA_WRITER
    LOG_MSG("calc_next_buffer_size: %10zu  -> %10zu\n",
            start, next_buffer_size);
#endif
}

// Add a new header because
// - there's none
// - data type or ctrl_info changed
// - the existing data buffer is full.
// Might switch to new DataFile.
// Will write ctrl_info out if new_ctrl_info is set,
// otherwise the new header tries to point to the existing ctrl_info.
void DataWriter::addNewHeader(bool new_ctrl_info)
{
    FileOffset ctrl_info_offset;
    AutoPtr<DataHeader> new_header;
    {
        bool       new_datafile = false;    // Need to use new DataFile?
        DataFile  *datafile = 0;
        size_t     headroom = 0;
        try
        {
            if (!header)
                new_datafile = true;            // Yes, because there's none.
            else
            {   // Check how big the current data file would get
                FileOffset file_size = header->datafile->getSize();
                headroom = header->datafile->getHeaderSize(channel_name,
                                                           dbr_type, dbr_count,
                                                           next_buffer_size);
                if (new_ctrl_info)
                    headroom += ctrl_info.getSize();
                if (file_size+headroom > file_size_limit) // Yes: reached max. size.
                    new_datafile = true;
            }
            if (new_datafile)
            {
#               ifdef DEBUG_DATA_WRITER
                LOG_MSG("DataWriter::addNewHeader: New Datafile\n");
#               endif
                datafile = createNewDataFile(headroom);
                new_ctrl_info = true;
            }
            else
            {
#               ifdef DEBUG_DATA_WRITER
                LOG_MSG("DataWriter::addNewHeader: adding to '%s'\n",
                        header->datafile->getFilename().c_str());
#               endif
                datafile = header->datafile->reference();
            }
            if (new_ctrl_info)
                datafile->addCtrlInfo(ctrl_info, ctrl_info_offset);
            else // use existing one
                ctrl_info_offset = header->data.ctrl_info_offset;
            new_header = datafile->addHeader(channel_name, dbr_type, dbr_count,
                                             period, next_buffer_size);
            datafile->release(); // now ref'ed by new_header
        }
        catch (GenericException &e)
        {
            if (datafile)
            {
                datafile->release();
                datafile = 0;
            }
            throw GenericException(__FILE__, __LINE__,
                               "Channel '%s':\n%s",
                               channel_name.c_str(), e.what());
        }
    }
    LOG_ASSERT(new_header);
    new_header->data.ctrl_info_offset = ctrl_info_offset;
    if (header)
    {   // Link old header to new one.
        header->set_next(new_header->datafile->getBasename(),
                         new_header->offset);
        header->write();
        // back from new to old.
        new_header->set_prev(header->datafile->getBasename(),
                             header->offset);        
        // Update index entry for the old header.
        if (tree) // Ignore result since block might already be in index
            tree->updateLastDatablock(
                    header->data.begin_time, header->data.end_time,
                    header->offset, header->datafile->getBasename());
    }
    // Switch to new_header (AutoPtr, might del. current header).
    header = new_header;
    // Calc buffer size for the following header (not the current one).
    calc_next_buffer_size(next_buffer_size);
    // new header will be added to index when it's closed.
}
 
