// -*- c++ -*-

#ifndef __AUTO_INDEX_H__
#define __AUTO_INDEX_H__

// Tools
#include <AutoPtr.h>
// Storage
#include <Index.h>
#include <NoCopy.h>

/** \ingroup Storage
 *  General Index for reading.
 * 
 *  Index which automatically picks ListIndex or FileIndex
 *  when reading, based on looking at the first few bytes
 *  in the index file.
 */
class AutoIndex : public Index
{
public:
    AutoIndex() {}

    ~AutoIndex();

    virtual bool open(const stdString &filename, char *buff, bool readonly=true);

    virtual void close();
    
    virtual class RTree *addChannel(const stdString &channel,
                                    stdString &directory);

    virtual class RTree *getTree(const stdString &channel,
                                 stdString &directory);

    virtual bool getFirstChannel(NameIterator &iter);

    virtual bool getNextChannel(NameIterator &iter);

private:
    PROHIBIT_DEFAULT_COPY(AutoIndex);
    stdString filename;
    AutoPtr<Index> index;
};

#endif
