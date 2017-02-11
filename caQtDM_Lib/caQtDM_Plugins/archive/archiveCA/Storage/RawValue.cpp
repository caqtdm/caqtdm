// --------------------------------------------------------
// $Id: RawValue.cpp,v 1.8 2012/07/04 09:08:46 jud Exp $
//
// Please refer to NOTICE.txt,
// included as part of this distribution,
// for legal information.
//
// Kay-Uwe Kasemir, kasemir@lanl.gov
// --------------------------------------------------------

// System
#include <stdlib.h>
#include <math.h>
// Base
#include <alarm.h>
#include <alarmString.h>
// Tools
#include "ArrayTools.h"
#include "epicsTimeHelper.h"
#include "MsgLogger.h"
#include "Conversions.h"
// Storage
#include "RawValue.h"
#include "CtrlInfo.h"
#include "DataFile.h"

RawValue::Data * RawValue::allocate(DbrType type, DbrCount count, size_t num)
{
    size_t s = getSize(type, count);
    Data *data = (Data *) calloc(num, s);
    if (!data)
        throw GenericException(__FILE__, __LINE__,
                               "Cannot allocate %zu bytes for RawValue(%u, %u, %zu)",
                               s, type, count, num);
    return data;
}

void RawValue::free(Data *value)
{
    if (value)
        ::free(value);
}

size_t RawValue::getSize(DbrType type, DbrCount count)
{
    if (type > LAST_BUFFER_TYPE)
        throw GenericException(__FILE__, __LINE__,
                               "Invalid type code %zu",
                               (size_t) type);
    // Need to make the buffer size be a properly structure aligned number
    // Not sure why, but this is the way the first chan_arch did it.
    size_t buf_size = dbr_size_n(type, count);
    if (buf_size % 8)
        buf_size += 8 - (buf_size % 8);

    return buf_size;
}

bool RawValue::hasSameValue(DbrType type, DbrCount count, size_t size,
                            const Data *lhs, const Data *rhs)
{
    size_t offset;

    // Different status or severity?
    if (lhs->status != rhs->status  ||
        lhs->severity != rhs->severity)
        return false;
 
    // Skip the time stamp and pads, compare the value
    switch (type)
    {
    case DBR_TIME_STRING: offset = offsetof (dbr_time_string, value); break;
    case DBR_TIME_SHORT:  offset = offsetof (dbr_time_short, value);  break;
    case DBR_TIME_FLOAT:  offset = offsetof (dbr_time_float, value);  break;
    case DBR_TIME_ENUM:   offset = offsetof (dbr_time_enum, value);   break;
    case DBR_TIME_CHAR:   offset = offsetof (dbr_time_char, value);   break;
    case DBR_TIME_LONG:   offset = offsetof (dbr_time_long, value);   break;
    case DBR_TIME_DOUBLE: offset = offsetof (dbr_time_double, value); break;
    default:
        LOG_MSG("RawValue::hasSameValue: cannot decode type %d\n", type);
        return false;
    }

    return memcmp(((const char *)lhs) + offset,
                  ((const char *)rhs) + offset, size - offset) == 0;
}

bool RawValue::hasSameValue(DbrType type, DbrCount count,
                            const Data *lhs, const Data *rhs)
{
    size_t size = getSize(type, count);
    return hasSameValue(type, count, size, lhs, rhs);
}

void RawValue::getStatus(const Data *value, stdString &result)
{
    char buf[200];

    short severity = short(value->severity & 0xfff);
    switch (severity)
    {
    case NO_ALARM:
        result = '\0';
        return;
    // Archiver specials:
    case ARCH_EST_REPEAT:
        sprintf(buf, "Est_Repeat %d", (int)value->status);
        result = buf;
        return;
    case ARCH_REPEAT:
        sprintf(buf, "Repeat %d", (int)value->status);
        result = buf;
        return;
    case ARCH_DISCONNECT:
        result = "Disconnected";
        return;
    case ARCH_STOPPED:
        result = "Archive_Off";
        return;
    case ARCH_DISABLED:
        result = "Archive_Disabled";
        return;
    }

    if (severity < (short)SIZEOF_ARRAY(alarmSeverityString)  &&
        (short)value->status < (short)SIZEOF_ARRAY(alarmStatusString))
    {
        result = alarmSeverityString[severity];
        result += " ";
        result += alarmStatusString[value->status];
    }
    else
    {
        sprintf(buf, "%d %d", severity, value->status);
        result = buf;
    }
}

bool RawValue::isAboveZero(DbrType type, const Data *value)
{
    switch (type)
    {
        case DBR_TIME_STRING:
            return ((dbr_time_string *)value)->value[0] != '\0';
        case DBR_TIME_ENUM:
            return ((dbr_time_enum *)value)->value > 0;
        case DBR_TIME_CHAR:
            return ((dbr_time_char *)value)->value > 0;
        case DBR_TIME_SHORT:
            return ((dbr_time_short *)value)->value > 0;
        case DBR_TIME_LONG:
            return ((dbr_time_long *)value)->value > 0;
        case DBR_TIME_FLOAT:
            return ((dbr_time_float *)value)->value > (float)0.0;
        case DBR_TIME_DOUBLE:
            return ((dbr_time_double *)value)->value > (double)0.0;
    }
    return false;
}

bool RawValue::parseStatus(const stdString &text, short &stat, short &sevr)
{
    if (text.empty())
    {
        stat = sevr = 0;
        return true;
    }
    if (!strncmp(text.c_str(), "Est_Repeat ", 11))
    {
        sevr = ARCH_EST_REPEAT;
        stat = atoi(text.c_str()+11);
        return true;
    }
    if (!strncmp(text.c_str(), "Repeat ", 7))
    {
        sevr = ARCH_REPEAT;
        stat = atoi(text.c_str()+7);
        return true;
    }
    if (!strcmp(text.c_str(), "Disconnected"))
    {
        sevr = ARCH_DISCONNECT;
        stat = 0;
        return true;
    }
    if (!strcmp(text.c_str(), "Archive_Off"))
    {
        sevr = ARCH_STOPPED;
        stat = 0;
        return true;
    }
    if (!strcmp(text.c_str(), "Archive_Disabled"))
    {
        sevr = ARCH_DISABLED;
        stat = 0;
        return true;
    }
    short i, j;
    for (i=0; i<(short)SIZEOF_ARRAY(alarmSeverityString); ++i)
    {
        if (!strncmp(text.c_str(), alarmSeverityString[i],
                     strlen(alarmSeverityString[i])))
        {
            sevr = i;
            stdString status = text.substr(strlen(alarmSeverityString[i]));

            for (j=0; j<(short)SIZEOF_ARRAY(alarmStatusString); ++j)
            {
                if (status.find(alarmStatusString[j]) != stdString::npos)
                {
                    stat = j;
                    return true;
                }
            }
            return false;
        }
    }

    return false;
}

void RawValue::getTime(const Data *value, stdString &time)
{
    if (value->stamp.nsec >= 1000000000L)
        throw  GenericException(__FILE__, __LINE__,
        "invalid time stamp with %zu secs, %zu nsecs.",
        (size_t)value->stamp.secPastEpoch, (size_t)value->stamp.nsec);
    epicsTime et(value->stamp);
    epicsTime2string(et, time);
}

bool RawValue::getDouble(DbrType type, DbrCount count,
                         const Data *value, double &d, int i)
{
    if (isInfo(value)) // done
    {
        d = 0.0;
        return true;
    }
    if (i >= count)
        return false;
    switch (type)
    {
        case DBR_TIME_ENUM:
        {
            const dbr_enum_t *v = &((const dbr_time_enum *)value)->value;
            d = v[i];
            return true;
        }
        case DBR_TIME_SHORT:
        {
            const dbr_short_t *v = &((const dbr_time_short *)value)->value;
            d = v[i];
            return true;
        }
        case DBR_TIME_LONG:
        {
            const dbr_long_t *v = &((const dbr_time_long *)value)->value;
            d = v[i];
            return true;
        }
        case DBR_TIME_FLOAT:
        {
            const dbr_float_t *v = &((const dbr_time_float *)value)->value;
            d = v[i];
            return true;
        }
        case DBR_TIME_DOUBLE:
        {
            const double *v = & ((const dbr_time_double *)value)->value;
            d = v[i];
            return true;
        }
    }
    d = 0.0;
    return false;
}

bool RawValue::getLong(DbrType type, DbrCount count,
                       const Data *value, long &l, int i)
{
    if (isInfo(value)) // done
    {
        l = 0;
        return true;
    }
    if (i >= count)
        return false;
    switch (type)
    {
        case DBR_TIME_ENUM:
        {
            const dbr_enum_t *v = &((const dbr_time_enum *)value)->value;
            l = v[i];
            return true;
        }
        case DBR_TIME_SHORT:
        {
            const dbr_short_t *v = &((const dbr_time_short *)value)->value;
            l = v[i];
            return true;
        }
        case DBR_TIME_LONG:
        {
            const dbr_long_t *v = &((const dbr_time_long *)value)->value;
            l = v[i];
            return true;
        }
        case DBR_TIME_FLOAT:
        {
            const dbr_float_t *v = &((const dbr_time_float *)value)->value;
            l = (long)v[i];
            return true;
        }
        case DBR_TIME_DOUBLE:
        {
            const double *v = & ((const dbr_time_double *)value)->value;
            l = (long)v[i];
            return true;
        }
    }
    l = 0;
    return false;
}

bool RawValue::setDouble(DbrType type, DbrCount count,
                         Data *value, double d)
{
    if (isInfo(value)) // done    
        return false;
    if (count != 1)
        return false;
    switch (type)
    {
        case DBR_TIME_SHORT:
        {
            ((dbr_time_short *)value)->value = (dbr_short_t)d;
            return true;
        }
        case DBR_TIME_LONG:
        {
            ((dbr_time_long *)value)->value = (dbr_long_t)d;
            return true;
        }
        case DBR_TIME_FLOAT:
        {
            ((dbr_time_float *)value)->value = (dbr_float_t)d;
            return true;
        }
        case DBR_TIME_DOUBLE:
        {
            ((dbr_time_double *)value)->value = d;
            return true;
        }
    }
    return false;
}

size_t RawValue::formatDouble(double value, NumberFormat format, int prec,
                              char *buffer, size_t max_len)
{
    double absVal, mantissa;
    bool minus;
    int  n, exponent = 0;
    if (max_len <= 0)
        return 0;
    switch (format)
    {
        case DEFAULT:
            return snprintf(buffer, max_len, "%.*g", prec, value);
        case DECIMAL:
            return snprintf(buffer, max_len, "%.*f", prec, value);
        case ENGINEERING:
            // Mostly stolen from dm2k's updateMonitors.c,
            // Mark Andersion, Frederick Vong.
            mantissa = absVal = fabs(value);
            minus = value < 0.0;
            if (absVal < 1.0)
            {
                if (absVal > 1.0e-307) // MIN double is around 1e-308
                {
                    do
                    {
                        mantissa *= 1000.0;
                        exponent -= 3;
                    }
                    while (mantissa < 1.0);
                }
            }
            else
            {   // absVal >= 1.0
                while (mantissa >= 1000.)
                {
                    mantissa *= 0.001; // mult. usually faster than dividing/
                    exponent += 3;
                }
            }
            if (minus)
            {
                *(buffer++) = '-';
                --max_len;
            }
            n = snprintf(buffer, max_len, "%.*f", prec, mantissa);
            if (n <= 0)
                return 0;
            buffer += n;
            max_len -= n;
            if (max_len < 5)
                return 0;
            *(buffer++) = 'e';
            if (exponent >= 0)
                *(buffer++) = '+';	// want e+00 for consistency
            else
            {
                *(buffer++) = '-';
                exponent = -exponent;
            }
            *(buffer++) = '0' + exponent/10;
            *(buffer++) = '0' + exponent%10;
            *(buffer++) = '\0';
            return n+4+(minus?1:0);
        case EXPONENTIAL:
            return snprintf(buffer, max_len, "%.*e", prec, value);
    }
    return 0;
}

void RawValue::getValueString(stdString &text,
                              DbrType type, DbrCount count, const Data *value,
                              const class CtrlInfo *info,
                              NumberFormat format,
                              int prec)
{
    int i;
    text.assign(0,0);
    if (isInfo(value)) // done    
        return;
    char line[100];
    if (prec < 0  && info)
        prec = info->getPrecision();
    switch (type)
    {
    case DBR_TIME_STRING:
        text = ((dbr_time_string *)value)->value;
        return;
    case DBR_TIME_ENUM:
        i = ((dbr_time_enum *)value)->value;
        if (info)
            info->getState(i, text);
        else
        {
            sprintf(line, "%d", i);
            text = line;
        }
        return;
    case DBR_TIME_CHAR:
        {
            text.reserve(4*count);
            dbr_char_t *val = &((dbr_time_char *)value)->value;
            for (i=0; i<count; ++i)
            {
                if (i==0)
                    sprintf(line, "%d", (int)*val);
                else
                    sprintf(line, "\t%d", (int)*val);
                text += line;
                ++val;
            }
            return;
        }
    case DBR_TIME_SHORT:
        {
            text.reserve(6*count);
            dbr_short_t *val = &((dbr_time_short *)value)->value;
            for (i=0; i<count; ++i)
            {
                if (i==0)
                    sprintf(line, "%d", (int)*val);
                else
                    sprintf(line, "\t%d", (int)*val);
                text += line;
                ++val;
            }
            return;
        }
    case DBR_TIME_LONG:
        {
            text.reserve(8*count);
            dbr_long_t *val = &((dbr_time_long *)value)->value;
            for (i=0; i<count; ++i)
            {
                if (i==0)
                    sprintf(line, "%ld", (long)*val);
                else
                    sprintf(line, "\t%ld", (long)*val);
                text += line;
                ++val;
            }
            return;
        }
    case DBR_TIME_FLOAT:
        {
            text.reserve((7+prec)*count);
            dbr_float_t *val = &((dbr_time_float *)value)->value;
            for (i=0; i<count; ++i)
            {
                if (i==0)
                    formatDouble(*val, format, prec, line, sizeof(line));
                else
                {
                    line[0] = '\t';
                    formatDouble(*val, format, prec, &line[1], sizeof(line)-1);
                }
                text += line;
                ++val;
            }            
            return;
        }
    case DBR_TIME_DOUBLE:
        {
            text.reserve((7+prec)*count);
            dbr_double_t *val = &((dbr_time_double *)value)->value;
            for (i=0; i<count; ++i)
            {
                if (i==0)
                    formatDouble(*val, format, prec, line, sizeof(line));
                else
                {
                    line[0] = '\t';
                    formatDouble(*val, format, prec, &line[1], sizeof(line)-1);
                }
                text += line;
                ++val;
            }
            return;
        }
    }
    text = "<cannot decode>";
}

void RawValue::toString(stdString &text, DbrType type, DbrCount count, const Data *value,
                               const class CtrlInfo *info)
{
    stdString t, s, v;
    epicsTime2string(getTime(value), t);
    getStatus(value, s);
    getValueString(v, type, count, value, info);

    text.reserve(t.length() + v.length() + s.length() + 2);
    text = t;
    if (isInfo(value))
        text += "\t#N/A";
    else
    {
        text += '\t';
        text += v;
    }
    if (s.length() > 0)
    {
        text += '\t';
        text += s;
    }
}

void RawValue::show(FILE *file,
    DbrType type, DbrCount count, const Data *value,
    const class CtrlInfo *info)
{
    if (!value)
    {
        fprintf(file, "<null-value>\n");
        return;
    }
    stdString time, stat, txt;
    getTime(value, time);
    getStatus(value, stat);
    if (isInfo(value))
    {   // done
        fprintf(file, "%s\t%s\n", time.c_str(), stat.c_str());
        return;
    }
    getValueString(txt, type, count, value, info);
    fprintf(file, "%s\t%s\t%s\n",
            time.c_str(), txt.c_str(), stat.c_str());
}   

void RawValue::read(DbrType type, DbrCount count, size_t size, Data *value,
                    DataFile *datafile, FileOffset offset)
{
    if (fseek(datafile->file, offset, SEEK_SET) != 0 ||
        (FileOffset) ftell(datafile->file) != offset   ||
        fread(value, size, 1, datafile->file) != 1)
        throw GenericException(__FILE__, __LINE__,
                               "Data read error in '%s' @ 0x%08lX",
                               datafile->getFilename().c_str(),
                               (unsigned long)offset);
    
    SHORTFromDisk(value->status);
    SHORTFromDisk(value->severity);
    epicsTimeStampFromDisk(value->stamp);
    if (value->stamp.nsec >= 1000000000L)
    {
        size_t nsec = value->stamp.nsec;
        value->stamp.nsec = 0;
        epicsTime t = value->stamp;
        stdString txt;
        epicsTime2string(t, txt);
        // Similar to RTree.cpp, it's probably best to live
        // with the data as is and patch it, because otherwise
        // the data is lost.
        // Unfortunately, we can't show a channel name
        // or more detail at this level:
        LOG_MSG("RawValue::readRawValue::read patching "
                "time stamp with invalid nsecs %zu: %s\n",
                nsec, txt.c_str());
    }

    // nasty: cannot use inheritance in lightweight RawValue,
    // so we have to switch on the type here:
    switch (type)
    {
    case DBR_TIME_CHAR:
    case DBR_TIME_STRING:
        break;

        // The following might generate "unused variable 'data'"
        // warnings on systems where no byte swapping is required.
#define FROM_DISK(DBR, TYP, TIMETYP, MACRO)                                  \
    case DBR:                                                                \
        {                                                                    \
            TYP *data __attribute__ ((unused)) = & ((TIMETYP *)value)->value;\
            for (size_t i=0; i<count; ++i)  MACRO(data[i]);                  \
        }                                                                    \
        break;
        FROM_DISK(DBR_TIME_DOUBLE,dbr_double_t,dbr_time_double, DoubleFromDisk)
        FROM_DISK(DBR_TIME_FLOAT, dbr_float_t, dbr_time_float,  FloatFromDisk)
        FROM_DISK(DBR_TIME_SHORT, dbr_short_t, dbr_time_short,  SHORTFromDisk)
        FROM_DISK(DBR_TIME_ENUM,  dbr_enum_t,  dbr_time_enum,   USHORTFromDisk)
        FROM_DISK(DBR_TIME_LONG,  dbr_long_t,  dbr_time_long,   LONGFromDisk)
    default:
        throw GenericException(__FILE__, __LINE__,
                               "Data with unknown DBR_xx %d in '%s' @ 0x%08lX",
                               type, datafile->getFilename().c_str(),
                               (unsigned long)offset);
#undef FROM_DISK
    }
}

void RawValue::write(DbrType type, DbrCount count, size_t size,
                     const Data *value,
                     MemoryBuffer<dbr_time_string> &cvt_buffer,
                     DataFile *datafile, FileOffset offset)
{
    LOG_ASSERT(datafile->is_writable());
    cvt_buffer.reserve(size);
    dbr_time_string *buffer = cvt_buffer.mem();

    memcpy(buffer, value, size);
    SHORTToDisk(buffer->status);
    SHORTToDisk(buffer->severity);

    if (buffer->stamp.nsec >= 1000000000L)
        throw  GenericException(__FILE__, __LINE__,
        "invalid time stamp with %zu secs, %zu nsecs.",
        (size_t)buffer->stamp.secPastEpoch, (size_t)buffer->stamp.nsec);
    epicsTimeStampToDisk(buffer->stamp);

    switch (type)
    {
#define TO_DISK(DBR, TYP, TIMETYP, CVT_MACRO)                                 \
    case DBR:                                                                 \
        {                                                                     \
            TYP *data __attribute__ ((unused)) = & ((TIMETYP *)buffer)->value;\
            for (size_t i=0; i<count; ++i)  CVT_MACRO (data[i]);              \
        }                                                                     \
        break;

    case DBR_TIME_CHAR:
    case DBR_TIME_STRING:
        // no conversion necessary
        break;
        TO_DISK(DBR_TIME_DOUBLE, dbr_double_t, dbr_time_double, DoubleToDisk)
        TO_DISK(DBR_TIME_FLOAT,  dbr_float_t,  dbr_time_float,  FloatToDisk)
        TO_DISK(DBR_TIME_SHORT,  dbr_short_t,  dbr_time_short,  SHORTToDisk)
        TO_DISK(DBR_TIME_ENUM,   dbr_enum_t,   dbr_time_enum,   USHORTToDisk)
        TO_DISK(DBR_TIME_LONG,   dbr_long_t,   dbr_time_long,   LONGToDisk)
    default:
        throw GenericException(__FILE__, __LINE__,
                               "Data with unknown DBR_xx %d in '%s' @ 0x%08lX",
                               type, datafile->getFilename().c_str(),
                               (unsigned long)offset);
#undef TO_DISK
    }

    if (fseek(datafile->file, offset, SEEK_SET) != 0  ||
        (FileOffset)ftell(datafile->file) != offset)
        throw GenericException(__FILE__, __LINE__,
                               "Data seek error in '%s' @ 0x%08lX",
                               datafile->getFilename().c_str(),
                               (unsigned long)offset);
    size_t written = fwrite(buffer, 1, size, datafile->file);
    if (written != size)
        throw GenericException(__FILE__, __LINE__,
                               "Data write error in '%s' @ 0x%08lX, %zu != %zu",
                               datafile->getFilename().c_str(),
                               (unsigned long)offset, written, size);
}

