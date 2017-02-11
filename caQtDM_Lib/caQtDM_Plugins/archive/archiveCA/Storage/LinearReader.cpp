// Tools
#include "MsgLogger.h"
// Storage
#include "LinearReader.h"

// #define DEBUG_LINREAD

LinearReader::LinearReader(Index &index, double delta)
        : AverageReader(index, delta)
{
}

const RawValue::Data *LinearReader::find(
    const stdString &channel_name, const epicsTime *start)
{
    this->channel_name = channel_name;
    reader_data = reader.find(channel_name, start);
    if (!reader_data)
        return 0;
    if (start)
        end_of_bin = roundTimeUp(*start, delta);
    else
        end_of_bin =
            roundTimeUp(RawValue::getTime(reader_data), delta);
    return next();
}

const RawValue::Data *LinearReader::next()
{
    if (!reader_data)
        return 0;
#ifdef DEBUG_LINREAD
    stdString txt;
    printf("Next time slot: %s\n", epicsTimeTxt(end_of_bin, txt));
#endif
    // To interpolate onto the time slot, we need the last value before
    // and the first sample after the time slot.
    bool anything = false;
    epicsTime t0, t1;
    double d0, d1;
    if (RawValue::getTime(reader_data) > end_of_bin)
    {   // Continue where the data is, skip bins that have nothing anyway
        end_of_bin = roundTimeUp(RawValue::getTime(reader_data), delta);
#ifdef DEBUG_LINREAD
        printf("Adjusted: %s\n", epicsTimeTxt(end_of_bin, txt));
#endif
    }
    while (reader_data  &&  RawValue::getTime(reader_data) < end_of_bin)
    {
#ifdef DEBUG_LINREAD
        printf("Raw: ");
        RawValue::show(stdout, reader.getType(), reader.getCount(),
                       reader_data, &reader.getInfo());
#endif
        // copy reader_data before calling reader.next()
        if (reader.changedInfo())
        {
            info = reader.getInfo();
            ctrl_info_changed = true;
        }        
        if (data==0 || type!=reader.getType() || count!=reader.getCount())
        {
            type = reader.getType();
            count = reader.getCount();
            data = RawValue::allocate(type, count, 1);
            type_changed = true;
        }
        RawValue::copy(type, count, data, reader_data);
        // Advance reader.
        reader_data = reader.next();
        if (count==1  &&  !RawValue::isInfo(data) &&
            RawValue::getDouble(type, count, data, d0))
        {
            t0 = RawValue::getTime(data);
            anything = true;
        }
        else   // This sample can't be interpolated; return as is.
            return data;
    }
    if (anything)
    {
        if (reader_data && !RawValue::isInfo(reader_data) &&
            reader.getCount()==1 &&
            RawValue::getDouble(reader.getType(), reader.getCount(),
                                reader_data, d1))
        {   // Have good pre- and post end_of_bin sample: Interpolate.
#ifdef DEBUG_LINREAD
            printf("Interpolating %g .. %g onto time slot\n", d0, d1);
#endif
            t1 = RawValue::getTime(reader_data);
            double dT = t1-t0;
            if (dT > 0)
                RawValue::setDouble(type, count, data,
                                    d0 + (d1-d0)*((end_of_bin-t0)/dT));
            else // Use average if there's no time between t0..t1?
                RawValue::setDouble(type, count, data, (d0 + d1)/2);
            RawValue::setTime(data, end_of_bin);
        }
        // Else: Have only pre-end_of_bin sample: Return as is
    }
    // Else: nothing pre-end_of_bin. See if we find anything in following slot
    end_of_bin += delta;
    if (anything)
        return data;
    return next();
}

