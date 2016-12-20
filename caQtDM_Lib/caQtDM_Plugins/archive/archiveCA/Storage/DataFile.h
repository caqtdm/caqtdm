// -*- c++ -*-
#if !defined(_DATAFILE_H_)
#define _DATAFILE_H_

// System
#include <stdio.h>
// Tools
#include <Filename.h>
#include <AutoPtr.h>
#include <AutoFilePtr.h>
// Storage
#include <RawValue.h>

/// \addtogroup Storage
/// @{

/// Binary Data File.
///
/// The DataFile class handles access to the binary data files.
/// One important feature is reference counting and caching.
/// When the ArchiveEngine adds samples, it is very likely
/// to add samples for several channels to the same collection
/// of data files.
/// - When referencing a data file for the first time, the file gets opened.
/// - When releasing it, the data file stays open.
/// - During a write cycle, it is likely that at least some of the channels
///   reference the same files, and voila: The file is already open.
/// - Finally, clear_cache() should be called to close all the data files.
class DataFile
{
public:
    // == 'ADF1', Archive Data File 1
    static const uint32_t cookie = 0x41444631;

    /// Reference a data file.
    ///
    /// Either opens a new one, or returns another reference
    /// to a file that is in the DataFile cache.
    ///
    /// @param dirname: Path/directory up to the filename
    /// @param basename: filename inside dirname
    /// @param for_write: open for writing or read-only?
    ///
    /// The reference call will normalize the dirname/basename/filename.
    /// As an example, it's acceptable to pass a basename that still
    /// contains pieces of a pathname, which will then be moved
    /// into the dirname.
    ///
    /// @return The referenced DataFile. Do not delete; use release.
    /// @sa release
    /// @exception GenericException on error.
    static DataFile *reference(const stdString &dirname,
                               const stdString &basename, bool for_write);

    /// Add reference to current DataFile.
    DataFile *reference();

    /// Get reference count.
    size_t refCount() const
    {    return ref_count; }

    /// Indicates if this file was newly created.
    bool is_new_file;

    /// Does the file follow the 2-1-1 format w/ cookie and tags?
    bool isTaggedFile() { return is_tagged_file; }
    
    /// De-reference a data file (Call instead of delete).
    //
    /// @sa clear_cache
    void release();

    /// Returns true if DataFile is writable.
    bool is_writable() const
    { return for_write; }

    /// Get current file size in bytes.
    ///
    /// @exception GenericException on file seek error.
    FileOffset getSize() const;
    
    /// Closes and re-opens a DataFile.
    ///
    /// For synchr. with a file that's actively written
    /// by another prog. is might help to reopen.
    ///
    /// @exception GenericException on error.
    void reopen();

    /// Close as many data files as possible.
    ///
    /// Closes all data files that are fully released.
    /// For example, the SpreadsheetReader will invoke this
    /// in its destructor.
    /// Since there might be other readers still open,
    /// only the application can decide when to close
    /// <i>all</i> files, which it should do via close_all.
    ///
    /// @sa close_all
    ///
    /// @return Returns the number of files which are
    ///         left open because there is still
    ///         a reference to them.
    static size_t clear_cache();

    /// Close all data files.
    ///
    /// The application should invoke this at times
    /// where it assumes that all readers/writers
    /// are done to check consistency and prevent
    /// files left dangling open.
    ///
    /// @sa clear_cache
    /// @exception GenericException if data files are
    ///            referenced (because there's still a reader?).
    static void close_all();

    /// Get full name of data file.
    const stdString &getFilename() {   return filename; }

    /// Get directory name of data file.
    const stdString &getDirname () {   return dirname;  }

    /// Get base name of data file.
    const stdString &getBasename() {   return basename; }

    /// Read header at given offset.
    ///
    /// @return Alloc'ed DataHeader, to be relased by caller.
    /// @exception GenericException in case of error.
    class DataHeader *getHeader(FileOffset offset);

    /// Get size of a header with given parameters.
    size_t getHeaderSize(const stdString &name,
                         DbrType dbr_type, DbrCount dbr_count,
                         size_t num_samples);
    
    /// Add a new DataHeader to the file.
    ///
    /// @return Alloc'ed header.
    /// The header's data type and buffer size info
    /// will be initialized.
    /// Links (dir, prev, next) need to be configured and saved.
    /// @exception GenericException in case of error.    
    class DataHeader *addHeader(const stdString &name,
                                DbrType dbr_type, DbrCount dbr_count,
                                double period, size_t num_samples);

    /// Add CtrlInfo to the data file
    ///
    /// @param offset is set to offset of the info
    ///
    /// @exception GenericException in case of error.
    void addCtrlInfo(const CtrlInfo &info, FileOffset &offset);
private:
    friend class DataHeader;
    friend class CtrlInfo;
    friend class RawValue;
    // Attach DataFile to disk file of given name.
    // Existing file is opened, otherwise new one is created.
    // Unlike the reference() call, the constructor
    // expexts a normalized dirname/basename/filename.
    DataFile(const stdString &dirname,
             const stdString &basename,
             const stdString &filename, bool for_write);

    // Close file.
    ~DataFile();

    // prohibit assignment or implicit copy:
    // (these are not implemented, use reference() !)
    DataFile(const DataFile &other);
    DataFile &operator = (const DataFile &other);

    // The current data file
    AutoFilePtr file;
    size_t ref_count;
    bool   for_write;
    bool   is_tagged_file;
    stdString filename;
    stdString dirname;
    stdString basename;
};

/// Used by DataFile.

/// Each data block in the binary data files starts with
/// a DataHeader.
///
class DataHeader
{
public:
    /// Create header for given data file.

    /// DataHeader references and releases the DataFile.
    /// Note that operations like readNext might switch
    /// to another DataFile!
    DataHeader(DataFile *datafile);

    /// Destructor releases the current DataFile.
    ~DataHeader();
    
    enum // Scott Meyers' "enum hack":
    {   FilenameLength = 40     };

    // NOTE: For now, the layout of the following must not
    // change because it defines the on-disk layout (except for byte order)!

    /// The header data
    struct DataHeaderData
    {
        FileOffset      dir_offset;     ///< offset of the old directory entry
        FileOffset      next_offset;    ///< abs. offs. of data header in next buffer
        FileOffset      prev_offset;    ///< abs. offs. of data header in prev buffer
        FileOffset      curr_offset;    ///< rel. offs. from data header to free entry
        uint32_t        num_samples;    ///< number of samples in this buffer
        FileOffset      ctrl_info_offset;  ///< abs. offset to CtrlInfo
        uint32_t        buf_size;       ///< disk space alloc. for this buffer including sizeof(DataHeader)
        uint32_t        buf_free;       ///< remaining bytes in this buffer
        DbrType         dbr_type;       ///< ca type of data
        DbrCount        dbr_count;      ///< array dimension of this data type
        uint8_t         pad[4];         ///< to align double period...
        double          period;         ///< period at which the channel is archived (secs)
        epicsTimeStamp  begin_time;     ///< first time stamp of data in this file
        epicsTimeStamp  next_file_time; ///< first time stamp of data in the next file
        epicsTimeStamp  end_time;       ///< last time stamp in this buffer
        char            prev_file[FilenameLength]; ///< basename for prev. buffer
        char            next_file[FilenameLength]; ///< basename for next buffer
    } data;

    /// The currently used DataFile (DataHeader handles ref and release).
    DataFile *datafile;

    /// Offset in current file for this header.

    /// Note that this is the pointer to the DataHeaderData on the disk.
    /// Data files with tags have 'INFO' + channel name before that!
    FileOffset offset;
    
    /// Fill the data with zeroes, invalidate the data.
    void clear();

    /// Is offset valid?
    bool isValid();
    
    /// Returns number of unused samples in buffer
    size_t available();

    /// Returns max. number of values in buffer.
    size_t capacity();
    
    /// Read (and convert) from offset in current DataFile, updating offset.
    /// @exception GenericException on error.
    void read(FileOffset offset);

    /// Convert and write to current offset in current DataFile.
    /// @exception GenericException on error.
    void write() const;

    /// Read the next data header.
    bool read_next();

    /// Read the previous data header.
    bool read_prev();

    /// Helper to set data.prev_file/offset
    void set_prev(const stdString &basename, FileOffset offset);
     
    /// Helper to set data.next_file/offset
    void set_next(const stdString &basename, FileOffset offset);

    void show(FILE *f, bool full_detail);

private:
    bool get_prev_next(const char *name, FileOffset new_offset);
};

/// @}

#endif // !defined(_DATAFILE_H_)
