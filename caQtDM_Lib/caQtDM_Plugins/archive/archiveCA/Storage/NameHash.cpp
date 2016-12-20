// -*- c++ -*-
// Tools
#include <BinIO.h>
#include <MsgLogger.h>
// Index
#include "NameHash.h"

IndexFileOffset NameHash::Entry::getSize() const
{   /// next, id, name.len, ID_txt.len, name, ID_txt
    return sizeof(IndexFileOffset) + sizeof(IndexFileOffset) + 
        sizeof(short) + sizeof(short) + 
        name.length() + ID_txt.length();
}

void NameHash::Entry::write(FILE *f, int file_offset_size) const
{
    if (fseeko(f, offset, SEEK_SET) != 0)
        throw GenericException(__FILE__, __LINE__,
                               "seek(0x%08lX) error",
                               (unsigned long) offset);
    if (!(WriteIndexFileOffset(f, next, file_offset_size) &&
          WriteIndexFileOffset(f, ID, file_offset_size) &&
          writeShort(f, name.length()) &&
          writeShort(f, ID_txt.length()) &&
          fwrite(name.c_str(), name.length(), 1, f) == 1))
        throw GenericException(__FILE__, __LINE__,
                               "write error at 0x%08lX",
                               (unsigned long) offset);
    size_t len = ID_txt.length();
    if (len > 0  &&  fwrite(ID_txt.c_str(), len, 1, f) != 1)
        throw GenericException(__FILE__, __LINE__,
                               "write error at 0x%08lX",
                               (unsigned long) offset);
}

void NameHash::Entry::read(FILE *f, int file_offset_size)
{
    char buffer[100];
    unsigned short name_len, ID_len;
    if (!(fseeko(f, offset, SEEK_SET)==0 &&
          ReadIndexFileOffset(f, &next, file_offset_size) &&
          ReadIndexFileOffset(f, &ID, file_offset_size) &&
          readShort(f, &name_len) &&
          readShort(f, &ID_len)))
        throw GenericException(__FILE__, __LINE__,
                               "read error at 0x%08lX",
                               (unsigned long) offset);
    if (name_len >= sizeof(buffer)-1)
        throw GenericException(__FILE__, __LINE__,
                               "Entry's name (%d) exceeds buffer size\n",
                               (int)name_len);
    if (ID_len >= sizeof(buffer)-1)
        throw GenericException(__FILE__, __LINE__,
                               "Entry's ID_txt (%d) exceeds buffer size\n",
                               (int)ID_len);
    if (fread(buffer, name_len, 1, f) != 1)
        throw GenericException(__FILE__, __LINE__,
                               "Read error for name of entry @ 0x%lX\n",
                               (unsigned long)offset);
    name.assign(buffer, name_len);
    if (name.length() != name_len)
        throw GenericException(__FILE__, __LINE__,
                               "NameHash: Error for name '%s' @ 0x%lX\n",
                               name.c_str(), (unsigned long)offset);
    if (ID_len > 0)
    {
        if (fread(buffer, ID_len, 1, f) != 1)
            throw GenericException(__FILE__, __LINE__,
                                   "Read error for ID_txt of entry @ 0x%lX\n",
                                   (unsigned long)offset);
        ID_txt.assign(buffer, ID_len);
        if (ID_txt.length() != ID_len)
            throw GenericException(__FILE__, __LINE__,
                                   "Error for ID_txt '%s' @ 0x%lX\n",
                                   ID_txt.c_str(), (unsigned long)offset);
    }
    else
        ID_txt.assign(0, 0);
}

NameHash::NameHash(FileAllocator &fa, IndexFileOffset anchor)
        : fa(fa), anchor(anchor), ht_size(0), table_offset(0)
{}

void NameHash::init(uint32_t ht_size)
{
    this->ht_size = ht_size;
    if (!(table_offset = fa.allocate(sizeof(IndexFileOffset)*ht_size)))
        throw GenericException(__FILE__, __LINE__,
                               "NameHash::init: Cannot allocate hash table\n");
    if (fseeko(fa.getFile(), table_offset, SEEK_SET))
        throw GenericException(__FILE__, __LINE__,
                               "NameHash::init: Cannot seek to hash table\n");
    uint32_t i;
    for (i=0; i<ht_size; ++i)
        if (!WriteIndexFileOffset(fa.getFile(), 0, fa.file_offset_size))
            throw GenericException(__FILE__, __LINE__,
                               "NameHash::init: Cannot write to hash table\n");
    if (fseeko(fa.getFile(), anchor, SEEK_SET) != 0 ||
        !WriteIndexFileOffset(fa.getFile(), table_offset, fa.file_offset_size) ||
        !writeLong(fa.getFile(), ht_size))
        throw GenericException(__FILE__, __LINE__,
                               "NameHash::init: Cannot write anchor info\n");
}

void NameHash::reattach()
{
    if (fseeko(fa.getFile(), anchor, SEEK_SET) != 0 ||
        !ReadIndexFileOffset(fa.getFile(), &table_offset, fa.file_offset_size) ||
        !readLong(fa.getFile(), &ht_size))
        throw GenericException(__FILE__, __LINE__,
                               "NameHash::readLong: Cannot read anchor info\n");
}
    
bool NameHash::insert(const stdString &name,
                      const stdString &ID_txt, IndexFileOffset ID)
{
    LOG_ASSERT(name.length() > 0);
    uint32_t h = hash(name);
    Entry entry;
    read_HT_entry(h, entry.offset);
    FILE *f = fa.getFile();
    if (entry.offset == 0)
    {   // First entry for this hash value
        entry.name   = name;
        entry.ID_txt = ID_txt;
        entry.ID     = ID;
        entry.next   = 0;
        entry.offset = fa.allocate(entry.getSize());
        entry.write(f, fa.file_offset_size);
        write_HT_entry(h, entry.offset);
        return true; // new entry
    }    
    while (true)
    {
        entry.read(f, fa.file_offset_size);
        if (entry.name == name)
        {   // Update existing entry
            entry.ID_txt = ID_txt;
            entry.ID     = ID;
            entry.write(f, fa.file_offset_size);
            return false; // old entry
        }
        if (!entry.next)
            break;
        // read next entry in list
        entry.offset = entry.next;
    }
    // Add new entry to end of list for current hash value
    Entry new_entry;
    new_entry.name   = name;
    new_entry.ID_txt = ID_txt;
    new_entry.ID     = ID;
    new_entry.next   = 0;
    new_entry.offset = fa.allocate(new_entry.getSize());
    entry.next = new_entry.offset;
    new_entry.write(f, fa.file_offset_size);
    entry.write(f, fa.file_offset_size);
    return true; // new entry
}

bool NameHash::find(const stdString &name, stdString &ID_txt, IndexFileOffset &ID)
{
    LOG_ASSERT(name.length() > 0);
    uint32_t h = hash(name);
    Entry entry;
    FILE *f = fa.getFile();
    read_HT_entry(h, entry.offset);
    while (entry.offset)
    {
        entry.read(f, fa.file_offset_size);
        if (entry.name == name)
        {   // Found!
            ID_txt = entry.ID_txt;
            ID     = entry.ID;
            return true;
        }
        entry.offset = entry.next;
    }
    // No read error, but nothing found, either.
    return false;
}

bool NameHash::startIteration(uint32_t &hashvalue, Entry &entry)
{
    FILE *f = fa.getFile();
    if (fseeko(f, table_offset, SEEK_SET))
        throw GenericException(__FILE__, __LINE__,
                               "Seek 0x%08lX failed\n",
                               (unsigned long)table_offset);
    for (hashvalue=0; hashvalue<ht_size; ++hashvalue)
    {   // Find first uses entry in hash table
        if (!ReadIndexFileOffset(f, &entry.offset, fa.file_offset_size))
            throw GenericException(__FILE__, __LINE__,
                                   "Cannot read hash table @ 0x%08lX\n",
                                   (unsigned long)table_offset);
        if (entry.offset)
            break;
    }
    if (! entry.offset)
        return false; // nothing found
    entry.read(f, fa.file_offset_size);
    return true; // found the initial entry
}

bool NameHash::nextIteration(uint32_t &hashvalue, Entry &entry)
{
    if (entry.next) // Is another entry in list for same hashvalue?
        entry.offset = entry.next;
    else
    {   // Find next used entry in hash table
        for (++hashvalue; hashvalue<ht_size; ++hashvalue)
        {
            read_HT_entry(hashvalue, entry.offset);
            if (entry.offset)
                break;
        }
    }
    if (hashvalue >= ht_size  ||  entry.offset == 0)
        return false; // no more entries for you!
    entry.read(fa.getFile(), fa.file_offset_size);
    return true; // found another entry
}

// From Sergei Chevtsov's rtree code:
uint32_t NameHash::hash(const stdString &name) const
{
    const int8_t *c = (const int8_t *)name.c_str();
    uint32_t h = 0;
    while (*c)
        h = (128*h + *(c++)) % ht_size;
    return (uint32_t)h;
}

void NameHash::showStats(FILE *f)
{
    unsigned long l, used_entries = 0, total_list_length = 0, max_length = 0;
    unsigned long hashvalue;
    Entry entry;
    for (hashvalue=0; hashvalue<ht_size; ++hashvalue)
    {
        read_HT_entry(hashvalue, entry.offset);
        if (entry.offset)
        {
            ++used_entries;
            l = 0;
            do
            {
                entry.read(fa.getFile(), fa.file_offset_size);
                ++l;
                ++total_list_length;
                entry.offset = entry.next;
            }
            while (entry.offset);
            if (l > max_length)
                max_length = l;
        }
    }
    fprintf(f, "Hash table fill ratio: %ld out of %ld entries (%ld %%)\n",
            used_entries, (unsigned long)ht_size, used_entries*100/ht_size);
    if (used_entries > 0)
        fprintf(f, "Average list length  : %ld entries\n",
                total_list_length / used_entries);
    fprintf(f, "Maximum list length  : %ld entries\n", max_length);
}

void NameHash::read_HT_entry(uint32_t hash_value, IndexFileOffset &offset)
{
    LOG_ASSERT(hash_value >= 0 && hash_value < ht_size);
    IndexFileOffset o = table_offset + hash_value * (fa.file_offset_size / 8);
    if (!(fseeko(fa.getFile(), o, SEEK_SET)==0 &&
          ReadIndexFileOffset(fa.getFile(), &offset, fa.file_offset_size)))
        throw GenericException(__FILE__, __LINE__,
                               "Cannot read HT entry @ 0x%lX\n",
                               (long)hash_value);
} 

void NameHash::write_HT_entry(uint32_t hash_value,
                              IndexFileOffset offset) const
{
    LOG_ASSERT(hash_value >= 0 && hash_value < ht_size);
    IndexFileOffset o = table_offset + hash_value * (fa.file_offset_size / 8);
    if (!(fseeko(fa.getFile(), o, SEEK_SET)==0 &&
          WriteIndexFileOffset(fa.getFile(), offset, fa.file_offset_size)))
        throw GenericException(__FILE__, __LINE__,
                               "Cannot write HT entry @ 0x%lX\n",
                               (long)hash_value);
}    

