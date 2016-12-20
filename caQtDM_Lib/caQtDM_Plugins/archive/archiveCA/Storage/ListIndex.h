// -*- c++ -*-

#ifndef __LIST_INDEX_H__
#define __LIST_INDEX_H__

// Tools
#include <AutoPtr.h>
// Storage
#include <Index.h>

/* \ingroup Storage
 * Index based on list of sub-archives.
 * <p>
 * Ideally, an index configuration conforming to indexfile.dtd
 * is used to create a master index via the ArchiveIndexTool.
 * Alternatively, that list of sub-archives can be used with
 * a ListIndex, which then acts like a master index by simply
 * querying the sub-archives one by one:
 * - Less efficient for retrieval
 * - Easier to set up and maintain, because
 *   you only create the config file without need
 *   to run the ArchiveIndexTool whenever any of
 *   the sub-archives change.
 * <p>
 * It is important to remember that the samples are not "merged"
 * in any way. When data for a channel is requested, the very
 * first sub-archive which includes that channel is used.
 * Assume the following sub-archive:
 * 1) Channels A, B, X
 * 2) Channels C, D, X (but different time range for X)
 * <p>
 * We can now request data for channels A, B, C and D as if
 * they were all in one combined archive.
 * For channel X, however, will will mostly get data from
 * sub-archive 1, EXCEPT(!!) when we were just looking at channels
 * C or D from archive 2, because for efficiency reasons,
 * any channel lookup will first try the currently open
 * sub-archive.
 * <p>
 * Conclusion: You will often not able to predict what
 * sub-archive you are using when channels are in more than
 * one sub-archive.
 * You should only use the ListIndex for non-overlapping
 * sub-archives. This makes some sense, because why would
 * you want to archive channels in more than one sub-archive
 * in the first place?
 */
class ListIndex : public Index
{
public:
    ~ListIndex();

    /** Open the index.
     *
     * @param filename: Name of a file that lists sub-archives.
     * @param readonly: Must be 'true', because ListIndex doesn't
     *                  support writing.
     * @exception GenericException On error, including readonly==false.
     */
    virtual bool open(const stdString &filename, char *buff, bool readonly=true);

    virtual void close();
    
    virtual class RTree *addChannel(const stdString &channel,
                                    stdString &directory);

    virtual class RTree *getTree(const stdString &channel,
                                 stdString &directory);

    virtual bool getFirstChannel(NameIterator &iter);

    virtual bool getNextChannel(NameIterator &iter);

private:
    stdString filename;
    // List of all the sub-archives.
    // Whenever one is opened because we look
    // for a channel in one or because we list
    // all channels, it stays open for performance.
    // NOTE: If this ever changes, those indices which
    // returned an RTree * must be kept open until this
    // ListIndex is removed.
    // Tried AutoPtr<IndexFile>, but the SubArchInfo is used
    // in a stdList, which requires several copy constructor
    // calls of SubArchInfo, and maintaining the index * myself
    // seemed easier.
    class SubArchInfo
    {
    public:
        SubArchInfo(stdString name) : name(name), index(0) {}
        stdString name;
        Index *index;
    };
    stdList<SubArchInfo> sub_archs;
    // List of all names w/ iterator
    stdList<stdString> names;
    stdList<stdString>::const_iterator current_name;
    // helper for AVLTree::traverse
    static void name_traverser(const stdString &name, void *self);
};

#endif
