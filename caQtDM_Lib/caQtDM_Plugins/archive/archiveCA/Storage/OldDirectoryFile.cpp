// OldDirectoryFile.cpp
//////////////////////////////////////////////////////////////////////

#include "MsgLogger.h"
#include "Filename.h"
#include "Conversions.h"
#include "OldDirectoryFile.h"

//#define LOG_DIRFILE

OldDirectoryFileEntry::OldDirectoryFileEntry()
{
    LOG_ASSERT(sizeof(data) == DataSize);
	init();
}

void OldDirectoryFileEntry::setFirst(const stdString &file, FileOffset offset)
{
    string2cp(data.first_file, file, FilenameLength);
    data.first_offset = offset;
}

void OldDirectoryFileEntry::setLast(const stdString &file, FileOffset offset)
{
    string2cp(data.last_file, file, FilenameLength);
    data.last_offset = offset;
}

void OldDirectoryFileEntry::init(const char *name)
{
	memset(&data, 0, sizeof(data));
	if (name)
	{
		strncpy(data.name, name, ChannelNameLength);
		data.name[ChannelNameLength-1] = '\0';
	}
	else
		data.name[0] = '\0';
}

bool OldDirectoryFileEntry::read(FILE *file, FileOffset offset)
{
	if (fseek(file, offset, SEEK_SET) != 0 ||
        (FileOffset) ftell(file) != offset  ||
		fread(&data, DataSize, 1, file) != 1)
        return false;
	FileOffsetFromDisk(data.next_entry_offset);
	FileOffsetFromDisk(data.last_offset);
	FileOffsetFromDisk(data.first_offset);
	epicsTimeStampFromDisk(data.create_time);
	epicsTimeStampFromDisk(data.first_save_time);
	epicsTimeStampFromDisk(data.last_save_time);
	this->offset = offset;
    return true;
}

bool OldDirectoryFileEntry::write(FILE *file, FileOffset offset)
{
	Data copy = data;

	FileOffsetToDisk(copy.next_entry_offset);
	FileOffsetToDisk(copy.last_offset);
	FileOffsetToDisk(copy.first_offset);
	epicsTimeStampToDisk(copy.create_time);
	epicsTimeStampToDisk(copy.first_save_time);
	epicsTimeStampToDisk(copy.last_save_time);
	if (fseek(file, offset, SEEK_SET) != 0  ||
        (FileOffset) ftell(file) != offset  ||
		fwrite(&copy, DataSize, 1, file) != 1)
    {
        return false;
    }
	this->offset = offset;
    fflush(file);
    return true;
}

//////////////////////////////////////////////////////////////////////
// OldDirectoryFile
//////////////////////////////////////////////////////////////////////

// Attach DiskBasedHashTable to disk file of given name.
// a) new file: setup Hash Table
// b) existing file for read-only: check HT
// c) existing file for read-write: check HT
OldDirectoryFile::OldDirectoryFile()
{
}

bool OldDirectoryFile::open(const stdString &filename, bool for_write)
{
    _filename = filename;
    Filename::getDirname(_filename, _dirname);
    _file_for_write = for_write;
    _file = fopen(filename.c_str(), "r+b");
    if (_file==0 && for_write)
        _file = fopen(filename.c_str(), "w+b");
    if (_file == 0)
        return false;

    // Does file contain HT?
    fseek(_file, 0, SEEK_END);
    _next_free_entry = ftell(_file);
    if (_next_free_entry < FirstEntryOffset)
    {
        if (!for_write) // ... but it should
        {
            LOG_MSG("OldDirectoryFile::open(%s): Missing HT\n",
                    filename.c_str());
            return false;
        }
        // Initialize HT:
        for (HashTable::HashValue entry = 0;
             entry < HashTable::HashTableSize; ++entry)
            writeHTEntry(entry, INVALID_OFFSET);
    
        // Next free entry = first entry after HT
        _next_free_entry = FirstEntryOffset;
    }
    
    // Check if file size = HT + N full entries
    FileOffset rest = (_next_free_entry - FirstEntryOffset)
        % OldDirectoryFileEntry::DataSize;
    if (rest)
        LOG_MSG("Suspicious directory file %s has a 'tail' of %d Bytes\n",
                filename.c_str(), rest);
    
#ifdef LOG_DIRFILE
    if (_file.isReadonly())
        LOG_MSG("(readonly) ");
    LOG_MSG("OldDirectoryFile %s\n", _filename);
#endif
    return true;
}

OldDirectoryFile::~OldDirectoryFile()
{
#ifdef LOG_DIRFILE
    if (_file.isReadonly())
        LOG_MSG("(readonly) ");
    LOG_MSG("~OldDirectoryFile %s\n", _filename);
#endif
    if (_file)
        fclose(_file);
}

OldDirectoryFileIterator OldDirectoryFile::findFirst()
{
    OldDirectoryFileIterator i(this);
    i.findValidEntry(0);

    return i;
}

// Try to locate entry with given name.
OldDirectoryFileIterator OldDirectoryFile::find(const stdString &name)
{
    OldDirectoryFileIterator i(this);

    i._hash   = HashTable::Hash(name.c_str());
    FileOffset offset = readHTEntry(i._hash);
    while (offset != INVALID_OFFSET)
    {
        i.entry.read(_file, offset);
        if (name == i.entry.data.name)
            return i;
        offset = i.entry.data.next_entry_offset;
    }
    i.entry.clear();
    
    return i;
}

// Add a new entry to HT.
//
// After calling this routine the current entry
// is undefined. It must be initialized and
// then written with saveEntry ().
OldDirectoryFileIterator OldDirectoryFile::add(const stdString &name)
{
    OldDirectoryFileIterator i(this);
    const char *cname = name.c_str();

    i._hash = HashTable::Hash(cname);
    FileOffset offset = readHTEntry(i._hash);

    if (offset == INVALID_OFFSET) // Empty HT slot:
        writeHTEntry(i._hash, _next_free_entry);
    else
    {       // Follow the entry chain that hashed to this value:
        FileOffset next = offset;
        while (next != INVALID_OFFSET)
        {
            i.entry.read(_file, next);
            if (name == i.entry.data.name) // already there?
                return i;
            next = i.entry.data.next_entry_offset;
        }
        // i.entry: last entry in chain.
        // make that one point to new entry:
        i.entry.data.next_entry_offset = _next_free_entry;
        i.entry.write(_file, i.entry.offset);
    }

    // Last entry points now to _next_free_entry.
    // Create the new entry there:
    i.entry.init(cname);
    i.entry.data.next_entry_offset = INVALID_OFFSET;
    i.entry.write(_file, _next_free_entry);
    fflush(_file);
    _next_free_entry += OldDirectoryFileEntry::DataSize;
    
    return i;
}

// Remove name from directory file.
// Will not remove data but only "pointers" to the data!
bool OldDirectoryFile::remove(const stdString &name)
{
    OldDirectoryFileIterator i(this);
    HashTable::HashValue hash = HashTable::Hash(name.c_str());
    FileOffset prev=0, offset = readHTEntry(hash);

    // Follow the channel chain that hashes to this value:
    while (offset != INVALID_OFFSET)
    {
        i.entry.read(_file, offset);
        if (name == i.entry.data.name)
        {
            // unlink this entry from list of names that share 'hash'
            if (prev == 0) // first entry in list?
            {
                // Make hash table point to the next channel,
                // skipping this one
                writeHTEntry(hash, i.entry.data.next_entry_offset);
                return true;
            }
            else
            {
                // Make previous entry skip this one
                offset = i.entry.data.next_entry_offset;
                i.entry.read(_file, prev);
                i.entry.data.next_entry_offset = offset;
                i.entry.write(_file, prev);
                return true;
            }
        }
        prev = offset;
        offset = i.entry.data.next_entry_offset;
    }
    return false;
}

FileOffset OldDirectoryFile::readHTEntry(HashTable::HashValue entry) const
{
    FileOffset offset;
    FileOffset pos = entry * sizeof(FileOffset);
    
    if (fseek(_file, pos, SEEK_SET) != 0 ||
        (FileOffset) ftell(_file) != pos   ||
        fread(&offset, sizeof(FileOffset), 1, _file) != 1)
        return INVALID_OFFSET;
    FileOffsetFromDisk(offset);
    return offset;
}

bool OldDirectoryFile::writeHTEntry(HashTable::HashValue entry, FileOffset offset)
{       // offset is value parm -> safe to convert in place
    FileOffsetToDisk (offset);
    FileOffset pos = entry * sizeof(FileOffset);
    if (fseek(_file, pos, SEEK_SET) != 0 ||
        (FileOffset) ftell(_file) != pos   ||
        fwrite(&offset, sizeof(FileOffset), 1, _file) != 1)
        return false;
    return true;
}

//////////////////////////////////////////////////////////////////////
// OldDirectoryFileIterator
//////////////////////////////////////////////////////////////////////

void OldDirectoryFileIterator::clear()
{
    _dir = 0;
    _hash = HashTable::HashTableSize;
    entry.clear();
}

OldDirectoryFileIterator::OldDirectoryFileIterator()
{
    clear();
}

OldDirectoryFileIterator::OldDirectoryFileIterator(OldDirectoryFile *dir)
{
    clear();
    _dir = dir;
}

OldDirectoryFileIterator::OldDirectoryFileIterator(const OldDirectoryFileIterator &dir)
{
    clear();
    *this = dir;
}

bool OldDirectoryFileIterator::next()
{
    if (_hash >= HashTable::HashTableSize ||
        entry.offset == INVALID_OFFSET ||
        _dir == 0)
        return false;
    
    // Have a current entry.
    // Ask it for pointer to next entry:
    FileOffset next = entry.data.next_entry_offset;
    if (next != INVALID_OFFSET)
    {
        entry.read(_dir->_file, next);
        return isValid();
    }
    // End of entries that relate to current _hash value,
    // switch to next value:
    return findValidEntry(_hash + 1);
}

// Search HT for the first non-empty entry:
bool OldDirectoryFileIterator::findValidEntry(HashTable::HashValue start)
{
    entry.clear();
    if (!_dir)
        return false;
    
    // Loop HT from 'start'
    FileOffset tmp;
    for (_hash = start; _hash < HashTable::HashTableSize; ++_hash)
    {
        // Get first entry's position from HT
        tmp = _dir->readHTEntry(_hash);
        // If valid, read that entry
        if (tmp != INVALID_OFFSET)
        {
            entry.read(_dir->_file, tmp);
            return isValid();
        }
    }
    return false;
}

void OldDirectoryFileIterator::save()
{
    if (_dir)
        entry.write(_dir->_file, entry.offset);
}

