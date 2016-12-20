// -*- c++ -*-

#ifndef __PLOT_READER_H__
#define __PLOT_READER_H__

#include "RawDataReader.h"

/** \ingroup Storage
 * Reads data from storage, modified for plotting.
 *
 * The PlotReader is an implementaion of a DataReader
 * that returns data in a format suitable for plotting.
 * 
 * Beginning at the requested start time, the raw samples
 * within the next bin of 'delta' seconds are investigated.
 * Returned is then the
 * - initial,
 * - minimum,
 * - maximum,
 * - and final value
 * within the bin.
 *
 * Then the next bin is investigated.
 *
 * Note that there is no indication which value we're currently
 * returning: The first call to find() will investigate the current
 * bin and return the inital value. The following call to next() will
 * return the minumum within the bin, then the maximum is returned and so on.
 *
 * This is meant to feed a plotting tool, with the intention
 * of showing an envelope of the raw data, resulting in data reduction when
 * zooming out.
 */
class PlotReader : public DataReader
{
public:
    /** Create a reader for an index.
     *
     * delta == 0 causes it to behave like the RawDataReader.
     * @param delta The bin size in seconds.
     */
    PlotReader(Index &index, double delta);
    const RawValue::Data *find(const stdString &channel_name,
                               const epicsTime *start);
    const RawValue::Data *next();
    const RawValue::Data *get() const;
    DbrType getType() const;
    DbrCount getCount() const;
    const CtrlInfo &getInfo() const;
    bool changedType();
    bool changedInfo();
private:
    RawDataReader reader;
    double delta;

    // Current value of reader
    const RawValue::Data *reader_data;

    // Value of this PlotReader
    epicsTime end_of_bin;
    unsigned long N;
    DbrType type;
    DbrCount count;
    CtrlInfo info;
    bool type_changed;
    bool ctrl_info_changed;
    bool have_initial_final;
    RawValueAutoPtr initial, final;
    const RawValue::Data *current;
    bool have_mini_maxi;
    double mini, maxi;
    // State machine for next()
    enum { s_dunno, s_gotit, s_ini, s_min, s_max, s_fin } state;

    const RawValue::Data *fill_bin();
};

#endif
