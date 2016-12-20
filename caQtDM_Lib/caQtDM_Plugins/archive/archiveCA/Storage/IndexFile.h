/// -*- c++ -*-

#ifndef __INDEX_FILE_H__
#define __INDEX_FILE_H__

// Tools
#include <AutoFilePtr.h>
// Storage
#include <Index.h>
#include <NameHash.h>
#include <RTree.h>

/// \addtogroup Storage
/// @{

/// Defines routines for the RTree-based index.
///
/// The IndexFile combines the NameHash for channel name
/// lookup with one RTree per channel into an index.
///
/// The file itself starts with the IndexFile cookie,
/// followed by the NameHash anchor.
/// Those two items constitute the 'reserved space'
/// all the remaining space is handled by the FileAllocator.
/// The ID of each NameHash entry points to an RTree anchor.
class IndexFile : public Index
{
public:

    // == 'CAI3', Chan. Arch. Index 3
    static const uint32_t cookie_64 = 0x43414933;
    // == 'CAI2', Chan. Arch. Index 2
    static const uint32_t cookie_32 = 0x43414932;

    IndexFile(int RTreeM = 50);

    ~IndexFile();

    /// The hash table size used for new channel name tables.
    static uint32_t ht_size;
    
    bool open(const stdString &filename, char *buff, bool readonly=true);

    void close();
    
    class RTree *addChannel(const stdString &channel, stdString &directory);

    class RTree *getTree(const stdString &channel, stdString &directory);

    bool getFirstChannel(NameIterator &iter);

    bool getNextChannel(NameIterator &iter);

    void showStats(FILE *f);   

    bool check(int level);
    
private:
    int RTreeM;
    AutoFilePtr f;
    FileAllocator fa;
    NameHash names;
    stdString dirname;
};

/// @}

#endif
