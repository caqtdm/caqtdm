// -*- c++ -*-

// Tools
#include <stdString.h>
#include <AutoPtr.h>
#include <NoCopy.h>
// Storage
#include <CtrlInfo.h>
#include <RawValue.h>
// Index
#include <Index.h>

/// \addtogroup Storage
/// @{

/// Writes data to storage.
///
/// The data writer interfaces between a series of
/// RawValue values and the Index/DataFile.
///
class DataWriter
{
public:
    /// Create a writer for the given index.
    ///
    /// @param channel_name: name of the channel
    /// @param ctrl_info:    meta information for the channel
    /// @param dbr_type:     the dbr_time_xxx type
    /// @param dbr_count:    array size
    /// @param period:       estimated periodicity of the channel
    /// @param num_samples:  estimated number of samples
    ///                      (helps w/ buffer allocation)
    ///
    /// @exception GenericException on error.
    DataWriter(Index &index,
               const stdString &channel_name,
               const CtrlInfo &ctrl_info,
               DbrType dbr_type,
               DbrCount dbr_count,
               double period,
               size_t num_samples);

    /// Destructor.
    ///
    /// <b>Note:</b> Since one might use another DataWriter
    ///              to add to the same set of data files,
    ///              the DataWriter does not clear the DataFile cache.
    ///              Call DataFile::close_all() when done!
    ~DataWriter();

    /// Returns the last time stamp in the archive.
    ///
    /// This allows you to avoid the back-in-time error
    /// by checking before adding.
    /// The result is a null time stamp in case
    /// there's nothing in the archive, yet.
    epicsTime getLastStamp();

    /// Add a value.
    ///
    /// @return Returns true if the sample was added,
    ///         false if the sample goes back-in-time
    ///         and is thus ignored.
    /// @exception GenericException on error.
    bool add(const RawValue::Data *data);

    /// Data file size limit.
    static FileOffset file_size_limit;

    /// Base name of data files.
    /// If not set, the date and time is used.
    static stdString data_file_name_base;
    
private:
    PROHIBIT_DEFAULT_COPY(DataWriter);
    Index &index;
    stdString directory;
    AutoPtr<RTree> tree;
    const stdString channel_name;
    const CtrlInfo &ctrl_info;
    DbrType dbr_type;
    DbrCount dbr_count;
    double period;
    size_t raw_value_size;

    void makeDataFileName(int serial, stdString &name);
    DataFile *createNewDataFile(size_t headroom);

    // Sets next_buffer_size to at least 'start',
    // so that buffers get bigger and bigger up to
    // some limit.
    void calc_next_buffer_size(size_t start);
    size_t next_buffer_size;

    AutoPtr<class DataHeader> header;
    size_t available;
    MemoryBuffer<dbr_time_string> cvt_buffer;

    void addNewHeader(bool new_ctrl_info);
};

/// @}
