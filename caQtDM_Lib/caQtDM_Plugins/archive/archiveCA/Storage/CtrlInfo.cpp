// Base
#include <cvtFast.h>
// Tools
#include <GenericException.h>
#include <string2cp.h>
#include <Conversions.h>
// Storage
#include "CtrlInfo.h"
#include "RawValue.h"
#include "DataFile.h"

CtrlInfo::CtrlInfo()
{
    size_t additional_buffer = 10;

    _infobuf.reserve(sizeof(CtrlInfoData) + additional_buffer);
    CtrlInfoData *info = _infobuf.mem();
    info->type = Invalid;
    //sizeof (DbrCount) + sizeof(DbrType);
    info->size = 2 + 2;
}

CtrlInfo::CtrlInfo(const CtrlInfo &rhs)
{
    const CtrlInfoData *rhs_info = rhs._infobuf.mem();
    _infobuf.reserve(rhs_info->size);
    CtrlInfoData *info = _infobuf.mem();
    memcpy(info, rhs_info, rhs_info->size);
}

CtrlInfo & CtrlInfo::operator = (const CtrlInfo &rhs)
{
    const CtrlInfoData *rhs_info = rhs._infobuf.mem();
    _infobuf.reserve(rhs_info->size);
    CtrlInfoData *info = _infobuf.mem();
    memcpy(info, rhs_info, rhs_info->size);
    return *this;
}

bool CtrlInfo::operator == (const CtrlInfo &rhs) const
{
    const CtrlInfoData *rhs_info = rhs._infobuf.mem();
    const CtrlInfoData *info = _infobuf.mem();
    // memcmp over the full info should start by comparing
    // the size & type fields and work in any case,
    // but valgrind complained about use of uninitialized
    // memory, so now it's explicit:
    return info->size == rhs_info->size &&
        info->type == rhs_info->type &&
        memcmp(&info->value, &rhs_info->value,
               rhs_info->size - 2*sizeof(uint16_t)) == 0;
}

CtrlInfo::Type CtrlInfo::getType() const
{   return (CtrlInfo::Type) (_infobuf.mem()->type);}

int32_t CtrlInfo::getPrecision() const
{
    if (getType() == Numeric)
        return _infobuf.mem()->value.analog.prec;
    return 0;
}

const char *CtrlInfo::getUnits() const
{
    if (getType() == Numeric)
        return _infobuf.mem()->value.analog.units;
    return "";
}

float CtrlInfo::getDisplayHigh() const
{   return _infobuf.mem()->value.analog.disp_high; }

float CtrlInfo::getDisplayLow() const
{   return _infobuf.mem()->value.analog.disp_low; }

float CtrlInfo::getHighAlarm() const
{   return _infobuf.mem()->value.analog.high_alarm; }

float CtrlInfo::getHighWarning() const
{   return _infobuf.mem()->value.analog.high_warn; }

float CtrlInfo::getLowWarning() const
{   return _infobuf.mem()->value.analog.low_warn; }

float CtrlInfo::getLowAlarm() const
{   return _infobuf.mem()->value.analog.low_alarm; }

size_t CtrlInfo::getNumStates() const
{
    if (getType() == Enumerated)
        return _infobuf.mem()->value.index.num_states;
    return 0;
}

void CtrlInfo::setNumeric(
    int32_t prec, const stdString &units,
    float disp_low, float disp_high,
    float low_alarm, float low_warn, float high_warn, float high_alarm)
{
    size_t len = units.length();
    size_t size = sizeof(CtrlInfoData) + len;
    _infobuf.reserve(size);
    CtrlInfoData *info = _infobuf.mem();

    info->type = Numeric;
    info->size = size;
    info->value.analog.disp_high  = disp_high;
    info->value.analog.disp_low   = disp_low;
    info->value.analog.low_warn   = low_warn;
    info->value.analog.low_alarm  = low_alarm;
    info->value.analog.high_warn  = high_warn;
    info->value.analog.high_alarm = high_alarm;
    info->value.analog.prec       = prec;
    string2cp (info->value.analog.units, units, len+1);
}

void CtrlInfo::setEnumerated(size_t num_states, char *strings[])
{
    size_t i, len = 0;
    for (i=0; i<num_states; i++) // calling strlen twice for each string...
        len += strlen(strings[i]) + 1;

    allocEnumerated (num_states, len);
    for (i=0; i<num_states; i++)
        setEnumeratedString(i, strings[i]);
}

void CtrlInfo::allocEnumerated(size_t num_states, size_t string_len)
{
    // actually this is too big...
    size_t size = sizeof(CtrlInfoData) + string_len;
    _infobuf.reserve(size);
    CtrlInfoData *info = _infobuf.mem();

    info->type = Enumerated;
    info->size = size;
    info->value.index.num_states = num_states;
    char *enum_string = info->value.index.state_strings;
    *enum_string = '\0';
}

// Must be called after allocEnumerated()
// AND must be called in sequence,
// i.e. setEnumeratedString (0, ..
//      setEnumeratedString (1, ..
void CtrlInfo::setEnumeratedString(size_t state, const char *string)
{
    CtrlInfoData *info = _infobuf.mem();
    if (info->type != Enumerated  ||
        state >= (size_t)info->value.index.num_states)
        return;

    char *enum_string = info->value.index.state_strings;
    size_t i;
    for (i=0; i<state; i++) // find this state string...
        enum_string += strlen(enum_string) + 1;
    strcpy(enum_string, string);
}

// After allocEnumerated() and a sequence of setEnumeratedString ()
// calls, this method recalcs the total size
// and checks if the buffer is sufficient (Debug version only)
void CtrlInfo::calcEnumeratedSize ()
{
    size_t i, len, total=sizeof(CtrlInfoData);
    CtrlInfoData *info = _infobuf.mem();
    char *enum_string = info->value.index.state_strings;
    for (i=0; i<(size_t)info->value.index.num_states; i++)
    {
        len = strlen(enum_string) + 1;
        enum_string += len;
        total += len;
    }

    info->size = total;
    LOG_ASSERT(total <= _infobuf.capacity());
}

void CtrlInfo::formatDouble(double value, stdString &result) const
{
    if (getType() != Numeric)
    {
        result = "<enumerated>";
        return;
    }
    char buf[200];
    if (cvtDoubleToString(value, buf, getPrecision()) >= 200)
        result = "<too long>";
    else
        result = buf;
}

const char *CtrlInfo::getState(size_t state, size_t &len) const
{
    if (getType() != Enumerated)
        return 0;

    const CtrlInfoData *info = _infobuf.mem();
    const char *enum_string = info->value.index.state_strings;
    size_t i=0;

    do
    {
        len = strlen(enum_string);
        if (i == state)
            return enum_string;
        enum_string += len + 1;
        ++i;
    }
    while (i < (size_t)info->value.index.num_states);
    len = 0;
    
    return 0;
}

void CtrlInfo::getState(size_t state, stdString &result) const
{
    size_t len;
    const char *text = getState(state, len);
    if (text)
    {
        result.assign(text, len);
        return;
    }

    char buffer[80];
    sprintf(buffer, "<Undef: %u>", (unsigned int)state);
    result = buffer;
}

bool CtrlInfo::parseState(const char *text,
                         const char **next, size_t &state) const
{
    const char *state_text;
    size_t  i, len;

    for (i=0; i<getNumStates(); ++i)
    {
        state_text = getState(i, len);
        if (! state_text)
        {
            LOG_MSG("CtrlInfo::parseState: missing state %zu", i);
            return false;
        }
        if (!strncmp(text, state_text, len))
        {
            state = i;
            if (next)
                *next = text + len;
            return true;
        }
    }
    return false;
}

// Read CtrlInfo for Binary format
//
// Especially for the original engine,
// the CtrlInfo on disk can be damaged.
// Special case of a CtrlInfo that's "too small":
// For enumerated types, an empty info is assumed.
// For other types, the current info is maintained
// so that the reader can decide to ignore the problem.
// In other cases, the type is set to Invalid
void CtrlInfo::read(DataFile *datafile, FileOffset offset)
{
    // read size field only
    uint16_t size;
    if (fseek(datafile->file, offset, SEEK_SET) != 0 ||
        (FileOffset) ftell(datafile->file) != offset ||
        fread(&size, sizeof size, 1, datafile->file) != 1)
    {
        _infobuf.mem()->type = Invalid;
        throw GenericException(__FILE__, __LINE__,
                               "Datafile %s: Cannot read size of CtrlInfo "
                               " @ 0x%lX\n",
                               datafile->getBasename().c_str(),
                               (unsigned long)offset);
    }
    SHORTFromDisk(size);
    if (size < offsetof(CtrlInfoData, value) + sizeof(EnumeratedInfo))
    {
        if (getType() == Enumerated)
        {
            LOG_MSG("CtrlInfo too small: %d, "
                    "forcing to empty enum for compatibility\n", size);
            setEnumerated (0, 0);
            return;
        }
        // keep current values for _infobuf!
        throw GenericException(__FILE__, __LINE__,
                               "Datafile %s: Incomplete CtrlInfo @ 0x%lX\n",
                               datafile->getBasename().c_str(),
                               (unsigned long)offset);
    }
    _infobuf.reserve(size+1); // +1 for possible unit string hack, see below
    CtrlInfoData *info = _infobuf.mem();
    info->size = size;
    if (info->size > _infobuf.capacity())
    {
        info->type = Invalid;
        throw GenericException(__FILE__, __LINE__,
                               "Datafile %s: CtrlInfo @ 0x%lX is too big\n",
                               datafile->getBasename().c_str(),
                               (unsigned long)offset);
    }
    // read remainder of CtrlInfo:
    if (fread(((char *)info) + sizeof size,
              info->size - sizeof size, 1, datafile->file) != 1)
    {
        info->type = Invalid;
        throw GenericException(__FILE__, __LINE__,
                               "Datafile %s: Cannot read remainder of "
                               "CtrlInfo @ 0x%lX\n",
                               datafile->getBasename().c_str(),
                               (unsigned long)offset);
    }
    // convert rest from disk format
    SHORTFromDisk(info->type);
    switch (info->type)
    {
        case Numeric:
            FloatFromDisk(info->value.analog.disp_high);
            FloatFromDisk(info->value.analog.disp_low);
            FloatFromDisk(info->value.analog.low_warn);
            FloatFromDisk(info->value.analog.low_alarm);
            FloatFromDisk(info->value.analog.high_warn);
            FloatFromDisk(info->value.analog.high_alarm);
            LONGFromDisk (info->value.analog.prec);
            {
                // Hack: some old archives are written with nonterminated
                // unit strings:
                int i, end;
                end = info->size - offsetof(CtrlInfoData, value.analog.units);
                for (i=0; i<end; ++i)
                    if (info->value.analog.units[i] == '\0')
                        return; // OK, string is terminated
                ++info->size; // include string terminator
                info->value.analog.units[end] = '\0';
            }
            break;
        case Enumerated:
            SHORTFromDisk(info->value.index.num_states);
            break;
        default:
            info->type = Invalid;
            throw GenericException(__FILE__, __LINE__,
                                   "Datafile %s: "
                                   "CtrlInfo @ 0x%lX has invalid  type %d, "
                                   "size %d\n",
                                   datafile->getBasename().c_str(),
                                   (unsigned long)offset, info->type,
                                   info->size);
    }
}

// Write CtrlInfo to file.
void CtrlInfo::write(DataFile *datafile, FileOffset offset) const
{   // Attention:
    // copy holds only the fixed CtrlInfo portion,  not enum strings etc.!
    const CtrlInfoData *info = _infobuf.mem();
    CtrlInfoData copy = *info;
    size_t converted;
    
    switch (copy.type) // convert to disk format
    {
        case Numeric:
            FloatToDisk(copy.value.analog.disp_high);
            FloatToDisk(copy.value.analog.disp_low);
            FloatToDisk(copy.value.analog.low_warn);
            FloatToDisk(copy.value.analog.low_alarm);
            FloatToDisk(copy.value.analog.high_warn);
            FloatToDisk(copy.value.analog.high_alarm);
            LONGToDisk (copy.value.analog.prec);
            converted = offsetof (CtrlInfoData, value) + sizeof (NumericInfo);
            break;
        case Enumerated:
            SHORTToDisk (copy.value.index.num_states);
            converted = offsetof (CtrlInfoData, value)
                + sizeof (EnumeratedInfo);
            break;
        default:
            throw GenericException(__FILE__, __LINE__,
                                   "Datafile %s: CtrlInfo for 0x%lX has "
                                   "invalid type %d, size %d\n",
                                   datafile->getBasename().c_str(),
                                   (unsigned long)offset, info->type,
                                   info->size);
    }
    SHORTToDisk(copy.size);
    SHORTToDisk(copy.type);
    if (fseek(datafile->file, offset, SEEK_SET) != 0 ||
        (FileOffset) ftell(datafile->file) != offset ||
        fwrite(&copy, converted, 1, datafile->file) != 1)
        throw GenericException(__FILE__, __LINE__,
                               "Datafile %s: Cannot write CtrlInfo @ 0x%lX\n",
                               datafile->getBasename().c_str(),
                               (unsigned long)offset);
    // only the common, minimal CtrlInfoData portion was converted,
    // the remaining strings are written from 'this'
    if (info->size > converted &&
        fwrite(((char *)info) + converted,
               info->size - converted, 1, datafile->file) != 1)
        throw GenericException(__FILE__, __LINE__,
                               "Datafile %s: Cannot write rest of "
                               "CtrlInfo @ 0x%lX\n",
                               datafile->getBasename().c_str(),
                               (unsigned long)offset);
}

void CtrlInfo::show(FILE *f) const
{
    if (getType() == Numeric)
    {
        fprintf(f, "CtrlInfo: Numeric\n");
        fprintf(f, "Display : %g ... %g\n", getDisplayLow(), getDisplayHigh());
        fprintf(f, "Alarm   : %g ... %g\n", getLowAlarm(), getHighAlarm());
        fprintf(f, "Warning : %g ... %g\n", getLowWarning(), getHighWarning());
        fprintf(f, "Prec    : %ld '%s'\n", (long)getPrecision(), getUnits());
    }
    else if (getType() == Enumerated)
    {
        fprintf(f, "CtrlInfo: Enumerated\n");
        fprintf(f, "States:\n");
        size_t i, len;
        for (i=0; i<getNumStates(); ++i)
        {
            fprintf(f, "\tstate='%s'\n", getState(i, len));
        }
    }
    else
        fprintf(f, "CtrlInfo: Unknown\n");
}

