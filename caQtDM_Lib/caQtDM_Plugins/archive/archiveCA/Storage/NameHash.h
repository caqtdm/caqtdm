// -*- c++ -*-

#ifndef __NAME_HASH_H__
#define __NAME_HASH_H__

// Tools
#include <stdString.h>
#include <NoCopy.h>
// Storage
#include <FileAllocator.h>
#include <FileOffsets.h>
/// \addtogroup Storage
/// \@{

/// A file-based Hash table for strings.
///
/// Beginning at the 'anchor' position in the file,
/// NameHash deposits the start offset of the hash table
/// and the number of table entries.
///
/// Each hash table entry is a file offset that points
/// to the beginnig of the NameHash::Entry list for that
/// hash value.
class NameHash
{
public:
    /// One NameHash entry on the disk is stored like this:
    /// @code
    ///     long next
    ///     long ID
    ///     short length of name
    ///     short length of ID_txt
    ///     char  name[]   // Stored without the delimiting'\0' !!
    ///     char  ID_txt[] // Stored without the '\0' !!
    /// @endcode
    /// ID_txt might be "", but the name can never be empty.
    class Entry
    {
    public:
        stdString  name;  ///< Channel Name.
        stdString  ID_txt;///< String and numeric ID
        IndexFileOffset ID;    ///< (filename and offset to RTree for the channel).
        IndexFileOffset next;  ///< Offset to next entry w/ same hash value

        IndexFileOffset offset;///< Offset of this Entry

        /// Size of entry
        IndexFileOffset getSize() const;
        /// Write at offset.
        /// @exception GenericException on error.
        void write(FILE *f, int file_offset_size) const;

        /// Read from offset.
        /// @exception GenericException on error.
        void read(FILE *f, int file_offset_size);
    };

    static const uint32_t anchor_size = sizeof(IndexFileOffset) + sizeof(uint32_t);
    
    /// Constructor.
    ///
    /// @param anchor: The NameHash will deposit its root pointer there.
    ///                Caller needs to assert that there are anchor_size
    ///                bytes available at that location in the file.
    NameHash(FileAllocator &fa, IndexFileOffset anchor);

    /// Create a new hash table of given size.
    ///
    /// @param ht_size determines the hash table size and should be prime.
    /// @exception GenericException on error.
    void init(uint32_t ht_size=1009);

    /// Attach to existing hash table
    /// @exception GenericException on error.
    void reattach();

    /// Insert name w/ ID
    /// @return True if a new entry was generated, false if an existing
    ///         entry for that name was updated with possibly new information.
    /// @exception GenericException on error.
    bool insert(const stdString &name, const stdString &ID_txt, IndexFileOffset ID);

    /// Locate name and obtain its ID.
    /// @return Returns true on success, false when name is 
    /// @exception GenericException on error.
    bool find(const stdString &name, stdString &ID_txt, IndexFileOffset &ID);
    
    /// Start iterating over all entries (in table's order).
    ///
    /// @return Returns true if hashvalue & entry were set to something valid,
    ///         false if there's nothing to see.
    /// @exception GenericException on error.
    bool startIteration(uint32_t &hashvalue, Entry &entry);
    
    /// Get next entry during iteration.
    ///
    /// @pre start_iteration() was successfully invoked.
    /// @return Returns true if there was another entry found.
    /// @exception GenericException on error.
    bool nextIteration(uint32_t &hashvalue, Entry &entry);
    
    /// Get hash value (public to allow for test code)
    uint32_t hash(const stdString &name) const;  

    /// Generate info on table fill ratio and list length
    void showStats(FILE *f);
private:
    PROHIBIT_DEFAULT_COPY(NameHash);
    FileAllocator &fa;
    IndexFileOffset anchor;       // Where offset gets deposited in file
    uint32_t ht_size;   // Hash Table size (entries, not bytes)
    IndexFileOffset table_offset; // Start of HT in file
    /// Seek to hash_value, read offset.
    /// @exception GenericException on read error.
    void read_HT_entry(uint32_t hash_value, IndexFileOffset &offset);
    /// Seek to hash_value, write offset.
    /// @exception GenericException on write error.
    void write_HT_entry(uint32_t hash_value, IndexFileOffset offset) const;
};

/// \@}

#endif
