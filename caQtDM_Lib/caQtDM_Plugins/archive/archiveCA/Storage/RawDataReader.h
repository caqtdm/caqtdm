// -*- c++ -*-

#ifndef __RAW_DATA_READER_H__
#define __RAW_DATA_READER_H__

// Tools
#include <ToolsConfig.h>
#include <AutoPtr.h>
// Storage
#include "DataReader.h"

/// \addtogroup Storage
/// @{

/// An implementation of the DataReader for the raw data.
class RawDataReader : public DataReader
{
public:
    RawDataReader(Index &index);
    virtual ~RawDataReader();
    virtual const RawValue::Data *find(const stdString &channel_name,
                                       const epicsTime *start);
    virtual const RawValue::Data *next();
    virtual const RawValue::Data *get() const;
    virtual DbrType getType() const;
    virtual DbrCount getCount() const;
    virtual const CtrlInfo &getInfo() const;
    virtual bool changedType();
    virtual bool changedInfo();
private:
    Index                &index;
    stdString            directory;
    AutoPtr<RTree>       tree;
    AutoPtr<RTree::Node> node;// used to iterate
    int                  rec_idx; 
    bool                 valid_datablock; // are node/idx on valid datablock? 
    RTree::Datablock     datablock; // the current datablock

    DbrType dbr_type;
    DbrCount dbr_count;
    CtrlInfo ctrl_info;
    bool type_changed;
    bool ctrl_info_changed;    
    double period;    

    RawValueAutoPtr data;
    size_t raw_value_size;
    AutoPtr<class DataHeader> header;
    size_t val_idx; // current index in data buffer

    void getHeader(const stdString &dirname, const stdString &basename,
                   FileOffset offset);
    const RawValue::Data *findSample(const epicsTime &start);
};

/// @}

#endif
