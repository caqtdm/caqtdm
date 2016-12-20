// -*- c++ -*-

// Tools
#include <ToolsConfig.h>
#include <AutoPtr.h>
// Storage
#include <ReaderFactory.h>
// Index
#include <Index.h>

/// \addtogroup Storage
/// @{

/// Reads data from storage, formatted as spreadsheet.

/// Based on several DataReader classes, which read a single
/// channel, the SpreadsheetReader reads multiple channels,
/// stepping through the values in time such that one can
/// use them for spreadsheet-type output, one point in time
/// per line.
class SpreadsheetReader
{
  public:
    /// Create the SpreadsheetReader
    SpreadsheetReader(Index &index,
                      ReaderFactory::How how = ReaderFactory::Raw,
                      double delta=0.0);

    virtual ~SpreadsheetReader();
    
    /// Locate initial values.
    ///
    /// Position the reader on-or-before start time
    /// for all channels.
    /// @param channel_names: List of channel names.
    /// @param start: start time or 0 for first value
    /// @return Returns true if <i>any</i> of the channels was found.
    ///         It's a severe error to invoke any of the following
    ///         after find() returns false.
    /// @exception GenericException on internal errors.
    /// @see found()
    virtual bool find(const stdVector<stdString> &channel_names,
                      const epicsTime *start = 0);

    /// Was this channel found?
    ///
    /// Only valid after find().
    /// @return Returns true if this channel was found.
    /// @see find().
    bool found(size_t i) const
    {
        return info[i];
    }

    /// Time stamp for the current slice of data
    virtual const epicsTime &getTime() const;
    
    /// Number of entries in the following arrays.

    /// Should match the size of the channel_names array passed to find().
    /// It is a severe error to invoke getName(), getValue() etc.
    /// with an index outside of 0...getNum()-1.
    virtual size_t getNum() const;

    /// Returns name of channel i=0...getNum()-1.
    virtual const stdString &getName(size_t i) const;

    /// Returns value of channel i=0...getNum()-1.
    ///
    /// The result might be 0 in case a channel
    /// does not have a valid value for the current
    /// time slice.
    virtual const RawValue::Data *get(size_t i) const;

    /// The dbr_time_xxx type.
    /// @exception GenericException when channel was not found.
    /// @see found()
    virtual DbrType getType(size_t i) const;
    
    /// array size.
    /// @exception GenericException when channel was not found.
    /// @see found()
    virtual DbrCount getCount(size_t i) const;
    
    /// The meta information for the channel.
    /// @exception GenericException when channel was not found.
    /// @see found()
    virtual const CtrlInfo &getInfo(size_t i) const;
    
    /// Get the next time slice.
    ///
    /// @return Returns true if any of the values are valid.
    bool next();  
  
protected:
    Index              &index;
    ReaderFactory::How how;
    double             delta;
    
    // Number of array entries for the following stuff that's non-scalar
    size_t num;

    // One reader per channel (also has a copy of the channel names)
    AutoArrayPtr< AutoPtr<DataReader> > reader;

    // Current data for each reader.
    // This often already points at the 'next' value.
    AutoArrayPtr< const RawValue::Data *> read_data;
    
    // The current time slice
    epicsTime time;

    // Copies of the current control infos
    AutoArrayPtr< AutoPtr<CtrlInfo> > info;

    // Type/count for following value
    AutoArrayPtr<DbrType> type;
    AutoArrayPtr<DbrCount> count;
    
    // The current values, i.e. copy of the reader's value
    // for the current time slice, or 0.
    AutoArrayPtr<RawValueAutoPtr> value;
private:
    PROHIBIT_DEFAULT_COPY(SpreadsheetReader);
};

/// @}

