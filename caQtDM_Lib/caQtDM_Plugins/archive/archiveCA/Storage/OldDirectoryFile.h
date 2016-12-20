#if !defined(_DIRECTORYFILE_H_)
#define _DIRECTORYFILE_H_

// System
#include <stdio.h>
// Tools
#include "ToolsConfig.h"
#include "epicsTimeHelper.h"
#include "string2cp.h"
// Storage
#include "HashTable.h"
#include "StorageTypes.h"

/// \addtogroup Storage
/// @{

class OldDirectoryFileIterator;

class OldDirectoryFileEntry
{
public:
    OldDirectoryFileEntry();

    enum
    {
        ChannelNameLength = 80,
        FilenameLength = 40,
        DataSize = 196
    };
    
    class Data
    {
    public:
        char           name[ChannelNameLength];// channel name
        FileOffset     next_entry_offset;      // offset of the next channel in the directory
        FileOffset     last_offset;            // offset of the last buffer saved for this channel
        FileOffset     first_offset;           // offset of the first buffer saved for this channel
        epicsTimeStamp create_time;
        epicsTimeStamp first_save_time;
        epicsTimeStamp last_save_time;
        char           last_file[FilenameLength];  // filename where the last buffer was saved
        char           first_file[FilenameLength]; // filename where the first buffer was saved
    }  data;

    void setFirst(const stdString &file, FileOffset offset);
    void setLast(const stdString &file, FileOffset offset);

    FileOffset offset; // .. in DirectoryFile where _data was read
    
    void clear();

    void init(const char *name=0);
    bool read(FILE *fd, FileOffset offset);
    bool write(FILE *fd, FileOffset offset);
};

inline void OldDirectoryFileEntry::clear()
{
    offset = INVALID_OFFSET;
    data.name[0] = '\0';
}

/// Old directory-file index of the binary storage format

/// The Directory File is a disk-based hash table:
///
/// First, there is a hash table with HashTable::HashTableSize
/// entries. Each entry maps a channel name to a file offset.
/// At the given file offset, there follows a list of DirectoryFileEntry
/// entries.
///
/// Find a channel:
/// - hash name, get offset from HashTable,
/// - read the DirectoryFileEntry at the given offset.
///   Does it match the name? If not, follow the "next"
///   pointer in the DirectoryFileEntry to the next Channel entry.
///
/// DirectoryFile::_next_free_entry points to the end
/// of the DirectoryFile.
class OldDirectoryFile
{
public:
    OldDirectoryFile();
    
    /// Attach DirectoryFile to disk file.
    
    /// Existing file is opened, otherwise new one is created
    /// for for_write==true.
    ///
    bool open(const stdString &filename, bool for_write=false);

    /// Close file.
    ~OldDirectoryFile();

    /// Get first entry.
    OldDirectoryFileIterator findFirst();

    /// Try to locate entry with given name.
    OldDirectoryFileIterator find(const stdString &name);

    /// Add new DirectoryEntry with given name.

    /// Entry will be empty, i.e. point to no data file.
    ///
    ///
    OldDirectoryFileIterator add(const stdString &name);

    /// Remove name from directory file.

    /// Will not remove data but only "pointers" to the data!
    ///
    ///
    bool remove(const stdString &name);

    /// Get name of directory file (the full path).
    const stdString &getDirname()    {   return _dirname;  }

    /// Readonly or also writable?
    bool isForWrite()                {   return _file_for_write; }

private:
    friend class OldDirectoryFileIterator;
    enum
    {
        FirstEntryOffset = HashTable::HashTableSize * sizeof(FileOffset)
    };

    // Prohibit assignment: two DirectoryFiles cannot access the same file
    // (However, more than one iterator are OK)
    OldDirectoryFile(const OldDirectoryFile &);
    OldDirectoryFile &operator =(const OldDirectoryFile &);

    // Read (first) FileOffset for given HashValue
    // Returns INVALID_OFFSET for error
    FileOffset readHTEntry(HashTable::HashValue entry) const;

    // Write (first) FileOffset for given HashValue
    bool writeHTEntry(HashTable::HashValue entry, FileOffset offset);

    // Search HT for the first non-empty entry:
    FileOffset lookForValidEntry(HashTable::HashValue start) const;

    stdString    _filename;
    stdString    _dirname;
    FILE *       _file;
    bool         _file_for_write;
    
    // Offset of next unused entry for add:
    FileOffset   _next_free_entry;
};

/// Iterator for entries in the old DirectoryFile

///
/// DirectoryFileIterator allows read/write access
/// to individual Channels in a DirectoryFile.
class OldDirectoryFileIterator
{
public:
    /// DirectoryFileIterator has to be bound to DirectoryFile:
    OldDirectoryFileIterator();
    OldDirectoryFileIterator(OldDirectoryFile *dir);
    OldDirectoryFileIterator(const OldDirectoryFileIterator &dir);

    /// getChannel must only be called when the iterator is valid.
    bool isValid() const   
    {   return entry.offset != INVALID_OFFSET; }

    /// The current entry.
    OldDirectoryFileEntry entry;
    
    /// Move to next DirectoryEntry.
    bool next();

    /// Write the entry back to disk.
    void save();

private:
    friend class OldDirectoryFile;
    void clear();

    bool findValidEntry(HashTable::HashValue start);

    bool operator == (const OldDirectoryFileIterator& rhs) const; // not impl.
    bool operator != (const OldDirectoryFileIterator& rhs) const; // not impl.

    OldDirectoryFile           *_dir;
    HashTable::HashValue    _hash;  // ... for _entry
};

/// @}

#endif
