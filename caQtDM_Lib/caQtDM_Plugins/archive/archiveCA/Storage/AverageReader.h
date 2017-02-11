// -*- c++ -*-

#ifndef __AVERAGE_READER_H__
#define __AVERAGE_READER_H__

#include "RawDataReader.h"

/// \addtogroup Storage
/// @{

/// Reads data from storage, averaging over the raw samples.
///
/// The AverageReader is an implementaion of a DataReader
/// that returns the average value of the raw values within
/// each 'bin' of 'delta' seconds on the time axis.
///
/// Assume the averaging delta is 10.0 seconds.
/// In order to get nice-looking time stamps like
/// <code>01:30:00, 01:30:10, 01:30:10, ...</code>,
/// the averaging reader creates bins ending at
/// <code>01:30:05, 01:30:15, 01:30:15, ...</code>.
///
/// The averaged data from 01:30:05 to just before 01:30:15
/// is then time-stamped 01:30:10.
class AverageReader : public DataReader
{
public:
    /// Create a reader for an index.
    ///
    /// @param delta The averaging time interval in seconds.
    AverageReader(Index &index, double delta);

    const RawValue::Data *find(const stdString &channel_name,
                               const epicsTime *start);

    const RawValue::Data *next();

    const RawValue::Data *get() const;

    DbrType getType() const;

    DbrCount getCount() const;

    const CtrlInfo &getInfo() const;

    bool changedType();

    bool changedInfo();

protected:
    RawDataReader reader;
    double delta;

    // Current value of reader
    const RawValue::Data *reader_data;

    // Current value of this AverageReader
    epicsTime end_of_bin;
    DbrType type;
    DbrCount count;
    CtrlInfo info;
    bool type_changed;
    bool ctrl_info_changed;
    RawValueAutoPtr data;
};

/// @}

#endif
