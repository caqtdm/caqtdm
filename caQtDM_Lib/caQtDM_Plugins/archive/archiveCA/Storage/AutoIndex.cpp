// -*- c++ -*-

// Tools
#include <MsgLogger.h>
#include <Filename.h>
#include <IndexConfig.h>
#include <AutoFilePtr.h>
// Storage
#include "AutoIndex.h"
#include "IndexFile.h"
#include "ListIndex.h"

#undef DEBUG_AUTOINDEX

AutoIndex::~AutoIndex()
{
    close();
}

bool AutoIndex::open(const stdString &filename, char *buff, bool readonly)
{
    this->filename = filename;
    if (!readonly)
        throw GenericException(__FILE__, __LINE__,
                               "AutoIndex '%s' Writing is not supported!\n",
                               filename.c_str());
    // Try to open as ListIndex
    try
    {
        index = new ListIndex();
    }
    catch (...)
    {
        throw GenericException(__FILE__, __LINE__, "AutoIndex: No mem for");
    }
    try
    {
        bool ret = index->open(filename, buff, true);
#ifdef DEBUG_AUTOINDEX
        LOG_MSG("AutoIndex(%s) -> ListIndex\n", filename.c_str());
#endif
        return true;
    }
    catch (GenericException &e)
    {   // can't open as binary index; ignore.
        index = 0;
    }

    // Try to open as IndexFile
    try
    {
        index = new IndexFile();
    }
    catch (...)
    {       
        throw GenericException(__FILE__, __LINE__, "AutoIndex: No mem");
    }
    bool ret = index->open(filename, buff, true);
#ifdef DEBUG_AUTOINDEX
    LOG_MSG("AutoIndex(%s) -> IndexFile\n", filename.c_str());
#endif
    return ret;
}

// Close what's open. OK to call if nothing's open.
void AutoIndex::close()
{
    index = 0;
}
    
class RTree *AutoIndex::addChannel(const stdString &channel,
                                   stdString &directory)
{
    throw GenericException(__FILE__, __LINE__,
                           "AutoIndex: Tried to add '%s'",
                           channel.c_str());
}

class RTree *AutoIndex::getTree(const stdString &channel,
                                stdString &directory)
{
    return index->getTree(channel, directory);
}

bool AutoIndex::getFirstChannel(NameIterator &iter)
{
    return index->getFirstChannel(iter);
}

bool AutoIndex::getNextChannel(NameIterator &iter)
{
    return index->getNextChannel(iter);
}

