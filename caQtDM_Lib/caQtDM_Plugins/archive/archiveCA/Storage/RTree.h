// -*- c++ -*-

#ifndef __RTREE_H__
#define __RTREE_H__

#include <FileOffsets.h>

// Tools
#include <epicsTimeHelper.h>
#include <AVLTree.h>
// Storage
#include <FileAllocator.h>

// When using the ArchiveDataTool to convert about 500 channels,
// 230MB of data, 102K directory file into an index, these were the
// results:
// Via NFS:
// M=3
// real    0m30.871s
// user    0m0.790s
// sys     0m2.030s
//  
// M=10
// real    0m23.944s
// user    0m0.830s
// sys     0m1.690s
//
// Local disk:
// No profiling, M=10
// real    0m17.148s
// user    0m0.700s
// sys     0m0.990s
//  
// No profiling, M=50
// real    0m3.402s  (!)
// user    0m1.290s
// sys     0m0.770s
//
// --> NFS is bad, small RTreeM values are bad.

/** \ingroup Storage
 *  Implements a file-based RTree.
 *
 * See the Antonin Guttman paper
 * "R-Trees: A Dynamic Index Structure for Spatial Searching"
 * (Proc. 1984 ACM-SIGMOD Conference on Management of Data, pp. 47-57).
 *
 * The records are time intervals start...end.
 * In addition to what's described in the Guttman paper,
 * - all records are non-overlapping
 *   (they might touch but they don't overlap);
 * - all records are sorted by time;
 * - node removal only for empty nodes, no reordering of records.
 */
class RTree
{
public:
    class Datablock
    {
    public:
        Datablock() : next_ID(0), data_offset(0), offset(0) {}
        IndexFileOffset    next_ID;       
        IndexFileOffset    data_offset;   /**< This block's offset in DataFile */
        stdString data_filename;     /**< DataFile for this block */
        
        IndexFileOffset offset;           /**< Location of DataBlock in index file */
        IndexFileOffset getSize() const;
        /** @exception GenericException on write error */
        void write(FILE *f, int file_offset_size) const;
        /** @exception GenericException on read error */
        void read(FILE *f, int file_offset_size);
    private:
        PROHIBIT_DEFAULT_COPY(Datablock);
    };

    class Record
    {
    public:
        Record();
        void clear();
        epicsTime  start, end;  // Range
        IndexFileOffset child_or_ID; // data block ID for leaf node; 0 if unused
        /** @exception GenericException on write error */
        void write(FILE *f, int file_offset_size) const;
        /** @exception GenericException on read error */
        void read(FILE *f, int file_offset_size);
    };

    class Node
    {
    public:
        Node(int M, bool leaf);
        Node(const Node &);
        ~Node();

        Node &operator = (const Node &);
        
        bool    isLeaf;  /**< Node or Leaf?        */ 
        IndexFileOffset  parent;  /**< 0 for root */
        Record  *record; /**< index records of this node */
        IndexFileOffset  offset;  /**< Location in file */
        
        /** Write to file at offset (needs to be set beforehand)
          * @exception GenericException on write error
          */
        void write(FILE *f, int file_offset_size) const;

        /** Read from file at offset (needs to be set beforehand)
         *  @exception GenericException on read error
         */
        void read(FILE *f, int file_offset_size);

        /** Obtain interval covered by this node
          * @return True if there is a valid interval, false if empty.
          */
        bool getInterval(epicsTime &start, epicsTime &end) const;
    private:
        int M;
        bool operator == (const Node &); // not impl.
    };

    /** \see constructor Rtree() */
    static const size_t anchor_size = 8;

    /** Attach RTree to FileAllocator.
     *
     * \param anchor: The RTree will deposit its root pointer there.
     *                Caller needs to assert that there are
     *                RTree::anchor_size
     *                bytes available at that location in the file.
     */
    RTree(FileAllocator &fa, IndexFileOffset anchor);
    
    /** Initialize empty tree. Compare to reattach().
      * @exception GenericException on write error.
      */
    void init(int M);

    /** Re-attach to an existing tree. Compare to init().
      * @exception GenericException on write error.
      */
    void reattach();

    /** The 'M' value, i.e. Node size, of this RTree. */
    int getM() const
    { return M; }
    
    /** Return range covered by this RTree
      * @return True if there is a valid interval, false if empty.
      * @exception GenericException on read error
      */
    bool getInterval(epicsTime &start, epicsTime &end);

    /** Create and insert a new Datablock.
     *
     * Note: Once a data block (offset and filename) is inserted
     *       for a given start and end time, the RTree code assumes
     *       that it stays that way. I.e. if we try to insert the same
     *       start/end/offset/file again, this will result in a NOP
     *       and return false.
     *       It is an error to insert the same offset/file again with
     *       a different start and/or end time!
     *
     * @see updateLastDatablock for the special case up updating the _end_ time.
     * @return true if a new entry was created, false if offset and filename
     *         were already found.
     * @exception GenericException on write error.
     */
    bool insertDatablock(const epicsTime &start, const epicsTime &end,
                         IndexFileOffset data_offset,
                         const stdString &data_filename);
    
    /** Locate entry after start time.
     *
     * Updates Node & i and returns true if found.
     *
     * Specifically, the last record with data at or just before
     * the start time is returned, so that the user can then decide
     * if and how that value might extrapolate onto the start time.
     * There's one exception: When requesting a start time
     * that preceeds the first available data point, so that there is
     * no previous data point, the very first record is returned.
     *
     * @exception GenericException on read error.
     */
    bool searchDatablock(const epicsTime &start, Node &node, int &i,
                         Datablock &block) const;

    /** Locate first datablock in tree.
     *
     * @exception GenericException on read error.
     */
    bool getFirstDatablock(Node &node, int &i, Datablock &block) const;
    
    /** \see getFirstDatablock */
    bool getLastDatablock(Node &node, int &i, Datablock &block) const;    

    /** Get a sub-block that's under the current block.
     * 
     * A record might not only point to the 'main' data block,
     * the one originally inserted and commonly used
     * for data retrieval. It can also contain a chain of
     * data blocks that were inserted later (at a lower priority).
     * In case you care about those, invoke getNextChainedBlock()
     * until it returns false.
     *
     * @exception GenericException on read error.
     */
    bool getNextChainedBlock(Datablock &block) const;
    
    /** Absolutely no clue what this one could do.
     * @see getNextDatablock
     *
     * @exception GenericException on read error.
     */
    bool getPrevDatablock(Node &node, int &i, Datablock &block) const;

    /** @see getPrevDatablock
     *
     * @exception GenericException on read error.
     */
    bool getNextDatablock(Node &node, int &i, Datablock &block) const;
    
    /** Tries to update existing datablock.
     *
     * Tries to update the end time of the last datablock,
     * in case start, data_offset and data_filename all match.
     * Will otherwise fall back to insertDatablock.
     *
     * @return true if a new entry was created, false if offset and filename
     *         were already found.
     * @exception GenericException on write error.
     */
    bool updateLastDatablock(const epicsTime &start, const epicsTime &end,
                             IndexFileOffset data_offset, stdString data_filename);
    
    /** Create a graphviz 'dot' file. */
    void makeDot(const char *filename);

    /** Returns true if tree passes self test, otherwise prints errors.
     * 
     * On success, nodes will contain the number of nodes in the tree
     * and record contains the number of used records.
     * One can compare that to the total number of available records,
     * nodes*getM().
     */
    bool selfTest(unsigned long &nodes, unsigned long &records);

    mutable size_t cache_misses, cache_hits; 

private:
    PROHIBIT_DEFAULT_COPY(RTree);
    FileAllocator &fa;
    // This is the (fixed) offset into the file
    // where the RTree information starts.
    // It points to
    // IndexFileOffset current root offset
    // IndexFileOffset RTreeM
    IndexFileOffset anchor;
    
    // IndexFileOffset to the root = content of what's at anchor
    IndexFileOffset root_offset;

    int M;
    
    mutable AVLTree<Node> node_cache;

    /** @exception GenericException on read error */
    void read_node(Node &node) const;

    /** @exception GenericException on write error */
    void write_node(const Node &node);
    
    /** @exception GenericException on error */
    void self_test_node(unsigned long &nodes, unsigned long &records,
                        IndexFileOffset n, IndexFileOffset p,
                        epicsTime start, epicsTime end);
    
    void make_node_dot(FILE *dot, FILE *f, IndexFileOffset node_offset);

    bool search(const epicsTime &start, Node &node, int &i) const;

    /** Set node & record index to first entry in tree.
     * @exception GenericException on read error
     */
    bool getFirst(Node &node, int &i) const;

    /** Set node & record index to last entry in tree.
     * @exception GenericException on read error
     */
    bool getLast(Node &node, int &i) const;

    /** If node & i were set to a valid entry by search(), update to prev. */
    bool prev(Node &node, int &i) const
    {    return prev_next(node, i, -1); }
    
    /** If node & i were set to a valid entry by search(), update to next. */
    bool next(Node &node, int &i) const
    {    return prev_next(node, i, +1); }

    /** Like prev() or next(). Dir must be +-1. */
    bool prev_next(Node &node, int &i, int dir) const;
    
    /** @return true if new block offset/filename was added under node/i,
     *         false if block with offset/filename was already there.
     * @exception GenericException if something's messed up.
     */
    bool add_block_to_record(const Node &node, int i,
                            IndexFileOffset data_offset,
                            const stdString &data_filename);
    
    /** Configure block for data_offset/name,
     * allocate space in file and write.
     * @exception GenericException on write error.
     */
    void write_new_datablock(IndexFileOffset data_offset,
                             const stdString &data_filename,
                             Datablock &block);
    
    /** Sets node to selected leaf for new entry start/end/ID.
     * Invoke by setting node.offset == root_offset.
     * @exception GenericException on read error.
     */
    void choose_leaf(const epicsTime &start, const epicsTime &end, Node &node);

    /** @exception GenericException on read/write error. */
    void insert_record_into_node(Node &node,
                                 int idx,
                                 const epicsTime &start,
                                 const epicsTime &end,
                                 IndexFileOffset ID,
                                 Node &overflow,
                                 bool &caused_overflow, bool &rec_in_overflow);
    
    /** Adjusts tree from node on upwards (update parents).
     * If new_node!=0, it's added to node's parent,
     * handling all resulting splits.
     * adjust_tree will write new_node, but not necessarily node!
     * @exception GenericException on read/write error
     */
    void adjust_tree(Node &node, Node *new_node);

    /** Remove entry from tree. */
    bool remove(const epicsTime &start, const epicsTime &end, IndexFileOffset ID);

    /** Remove record i from node, condense tree.
     * @exception GenericException on read/write error.
     */
    void remove_record(Node &node, int i);

    /** Starting at a node that was just made "smaller",
     * go back up to root and update all parents.
     * @exception GenericException on read/write error
     */
    void condense_tree(Node &node);

    /** Special 'update' call for usage by the ArchiveEngine.
     *
     * The engine usually appends to the last buffer.
     * So most of the time, the ID and start time in this
     * call have not changed, only the end time has been extended,
     * and the intention is to update the tree.
     * Sometimes, however, the engine created a new block,
     * in which case it will call append_latest a final time
     * to update the end time and then it'll follow with an insert().
     * \return True if start & ID refer to the existing last block
     *         and the end time was succesfully updated.
     */
    bool updateLast(const epicsTime &start,
                    const epicsTime &end, IndexFileOffset ID);
};

#endif

