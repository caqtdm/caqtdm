// Tools
#include <MsgLogger.h>
#include <BinIO.h>
// Index
#include "RTree.h"

//  1240    lines before exceptions

#define RecordSize  ((sizeof (epicsTime)) * 2 + sizeof (IndexFileOffset))
#define NodeSize(M) (sizeof (bool) + sizeof (IndexFileOffset) + RecordSize*(M))

IndexFileOffset RTree::Datablock::getSize() const
{   //  next_ID, data offset, name size, name (w/o '\0')
    return sizeof(IndexFileOffset) + sizeof(IndexFileOffset) + 
        sizeof(short) + data_filename.length();
}

void RTree::Datablock::write(FILE *f, int file_offset_size) const
{
    if (fseeko(f, offset, SEEK_SET))
        throw GenericException(__FILE__, __LINE__, "fseeko(0x%08lX) failed",
                               (unsigned long) offset);
    if (!(WriteIndexFileOffset(f, next_ID, file_offset_size) && WriteIndexFileOffset(f, data_offset, file_offset_size) &&
          writeShort(f, data_filename.length()) &&
          fwrite(data_filename.c_str(), data_filename.length(), 1, f) == 1))
        throw GenericException(__FILE__, __LINE__, "write failed @ 0x%08lX",
                               (unsigned long) offset);
}

void RTree::Datablock::read(FILE *f, int file_offset_size)
{
    if (fseeko(f, offset, SEEK_SET))
        throw GenericException(__FILE__, __LINE__, "fseeko(0x%08lX) failed",
                               (unsigned long) offset);
    unsigned short len;
    char buf[300];
    if (!(ReadIndexFileOffset(f, &next_ID, file_offset_size) &&
          ReadIndexFileOffset(f, &data_offset, file_offset_size) &&
          readShort(f, &len)))
        throw GenericException(__FILE__, __LINE__, "read failed @ 0x%lX",
                               (unsigned long)offset);
    if (len >= sizeof(buf)-1)
        throw GenericException(__FILE__, __LINE__,
                               "Datablock filename exceeds buffer (%d)",len);
    if (fread(buf, len, 1, f) != 1)
        throw GenericException(__FILE__, __LINE__,
                               "Datablock filename read error @ 0x%lX",
                               (unsigned long) offset);
    buf[len] = '\0';
    data_filename.assign(buf, len);
    if (data_filename.length() != len)
        throw GenericException(__FILE__, __LINE__,
                               "Datablock filename length error @ 0x%lX\n",
                               (unsigned long)offset);
}
    
static void writeEpicsTime(FILE *f, const epicsTime &t)
{
    epicsTimeStamp stamp = t;
    if (!(writeLong(f, stamp.secPastEpoch) && writeLong(f, stamp.nsec)))
        throw GenericException(__FILE__, __LINE__, "write error");
}

static void readEpicsTime(FILE *f, epicsTime &t)
{
    epicsTimeStamp stamp;
    if (! (readLong(f, (uint32_t *)&stamp.secPastEpoch) &&
           readLong(f, (uint32_t *)&stamp.nsec)))
        throw GenericException(__FILE__, __LINE__, "read error");
    if (stamp.nsec < 1000000000L)
    {
        t = stamp;
        return;
    }
    // This should never happen but obviously does,
    // and R3.14.8 will abort, so we have to catch it:
    size_t nsec = (size_t) stamp.nsec;
    stamp.nsec = 0;
    t = stamp;
    stdString txt;
    epicsTime2string(t, txt);
#if 0
    // In principle, one should ignore those values,
    // throw an exception, so we get a good message
    // which includes the context:
    throw GenericException(__FILE__, __LINE__,
#else
    // However, it's often better to patch the nsecs
    // and show the data. Unfortunalely, we don't know
    // the channel name etc. at this point, so we can't
    // provide a good error message.
    LOG_MSG(
#endif
        "RTree: Invalid time stamp with %zu nsecs: %s\n",
        nsec, txt.c_str());
}

RTree::Record::Record()
{
    child_or_ID = 0;
}

void RTree::Record::clear()
{
    start = nullTime;
    end = nullTime;
    child_or_ID = 0;
}

void RTree::Record::write(FILE *f, int file_offset_size) const
{
    writeEpicsTime(f, start);
    writeEpicsTime(f, end);
    if (!WriteIndexFileOffset(f, child_or_ID, file_offset_size))
        throw GenericException(__FILE__, __LINE__, "write error");
}

void RTree::Record::read(FILE *f, int file_offset_size)
{
    readEpicsTime(f, start);
    readEpicsTime(f, end);
    if (!ReadIndexFileOffset(f, &child_or_ID, file_offset_size))
        throw GenericException(__FILE__, __LINE__, "read error");
}

RTree::Node::Node(int M, bool leaf) : M(M)
{
    LOG_ASSERT(M > 2);
    isLeaf = leaf;
    parent = 0;
    record = new Record[M];
    LOG_ASSERT(record != 0);
    offset = 0;
}

RTree::Node::Node(const Node &rhs)
{
    M = rhs.M;
    LOG_ASSERT(M > 2);
    isLeaf = rhs.isLeaf;
    parent = rhs.parent;
    record = new Record[M];
    LOG_ASSERT(record != 0);
    int i;
    for (i=0; i<M; ++i)
        record[i] = rhs.record[i];
    offset = rhs.offset;
}

RTree::Node::~Node()
{
    delete [] record;
}

RTree::Node &RTree::Node::operator = (const Node &rhs)
{
    if (M != rhs.M)
    {
        LOG_ASSERT(M == rhs.M);
    }
    isLeaf = rhs.isLeaf;
    parent = rhs.parent;
    int i;
    for (i=0; i<M; ++i)
        record[i] = rhs.record[i];
    offset = rhs.offset;
    return *this;
}

void RTree::Node::write(FILE *f, int file_offset_size) const
{
    if (fseeko(f, offset, SEEK_SET))
        throw GenericException(__FILE__, __LINE__, "fseeko(0x%08lX) failed",
                               (unsigned long) offset);
    if (! (writeByte(f, isLeaf) &&
           WriteIndexFileOffset(f, parent, file_offset_size)))
        throw GenericException(__FILE__, __LINE__, "write failed");
    int i;
    for (i=0; i<M; ++i)
        record[i].write(f, file_offset_size);
}

void RTree::Node::read(FILE *f, int file_offset_size)
{
    if (fseeko(f, offset, SEEK_SET))
        throw GenericException(__FILE__, __LINE__, "fseeko(0x%08lX) failed",
                               (unsigned long) offset);
    uint8_t c;
    if (! (readByte(f, &c) && ReadIndexFileOffset(f, &parent, file_offset_size)))
        throw GenericException(__FILE__, __LINE__, "read failed");
    isLeaf = c > 0;
    int i;
    for (i=0; i<M; ++i)
        record[i].read(f, file_offset_size);
}

bool RTree::Node::getInterval(epicsTime &start, epicsTime &end) const
{
    bool valid = false;
    int i;
    for (i=0; i<M; ++i)
    {
        if (!record[i].child_or_ID)
            continue;
        if (i==0  ||  start > record[i].start)
        {
            start = record[i].start;
            valid = true;
        }
        if (i==0  ||  end < record[i].end)
        {
            end = record[i].end;
            valid = true;
        }
    }
    return valid;
}

RTree::RTree(FileAllocator &fa, IndexFileOffset anchor)
        :  fa(fa), anchor(anchor), root_offset(0), M(-1)
{
    cache_misses = cache_hits = 0;
}

void RTree::init(int M)
{
    if (M <= 2)
        throw GenericException(__FILE__, __LINE__,
                               "RTree::init(%d): M should be > 2", M);
    this->M = M;
    // Create initial Root Node = Empty Leaf
    root_offset = fa.allocate(NodeSize(M));
    Node node(M, true);
    node.offset = root_offset;
    write_node(node);
    // Update Root pointer
    if (! (fseeko(fa.getFile(), anchor, SEEK_SET)==0 &&
           WriteIndexFileOffset(fa.getFile(), root_offset, fa.file_offset_size)==true &&
           writeLong(fa.getFile(), M) == true))
        throw GenericException(__FILE__, __LINE__,
                               "write error @ 0x%08lX",
                               (unsigned long) anchor);
}

void RTree::reattach()
{
    uint32_t RTreeM;
    if (!(fseeko(fa.getFile(), anchor, SEEK_SET)==0 &&
          ReadIndexFileOffset(fa.getFile(), &root_offset, fa.file_offset_size)==true &&
          readLong(fa.getFile(), &RTreeM) == true))
        throw GenericException(__FILE__, __LINE__,
                               "read error @ 0x%08lX",
                               (unsigned long) anchor);
    if (RTreeM < 1  ||  RTreeM > 100)
        throw GenericException(__FILE__, __LINE__,
                               "RTree::reattach: Suspicious RTree M %ld\n",
                               (long)RTreeM);
    M = RTreeM;
}

bool RTree::getInterval(epicsTime &start, epicsTime &end)
{
    Node node(M, true);
    node.offset = root_offset;
    read_node(node);
    return node.getInterval(start, end);
}

bool RTree::searchDatablock(const epicsTime &start, Node &node, int &i,
                            Datablock &block) const
{
    if (!search(start, node, i))
        return false;
    block.offset = node.record[i].child_or_ID;
    block.read(fa.getFile(), fa.file_offset_size);
    return true;
}

bool RTree::getFirstDatablock(Node &node, int &i, Datablock &block) const
{
    if (!getFirst(node, i))
        return false;
    block.offset = node.record[i].child_or_ID;
    block.read(fa.getFile(), fa.file_offset_size);
    return true;
}

bool RTree::getLastDatablock(Node &node, int &i, Datablock &block) const
{  
    if (!getLast(node, i))
        return false;
    block.offset = node.record[i].child_or_ID;
    block.read(fa.getFile(), fa.file_offset_size);
    return true;
}

bool RTree::getNextChainedBlock(Datablock &block) const
{
    if (block.next_ID == 0)
        return false;
    block.offset = block.next_ID;
    block.read(fa.getFile(), fa.file_offset_size);
    return true;
}

bool RTree::getPrevDatablock(Node &node, int &i, Datablock &block) const
{
    if (!prev(node, i))
        return false;
    block.offset = node.record[i].child_or_ID;
    block.read(fa.getFile(), fa.file_offset_size);
    return true;
}

bool RTree::getNextDatablock(Node &node, int &i, Datablock &block) const
{
    if (!next(node, i))
        return false;
    block.offset = node.record[i].child_or_ID;
    block.read(fa.getFile(), fa.file_offset_size);
    return true;
}

bool RTree::updateLastDatablock(const epicsTime &start,
                                const epicsTime &end,
                                IndexFileOffset data_offset,
                                stdString data_filename)
{
    Node node(M, true);
    int i;
    if (getLast(node, i))
    {
        // Likely scenarios:
        // Engine added values to an existing block, so only
        // the end time changed.
        // --> update end time, done.
        // After a restart, the last engine wrote 10-20
        // (20 being the "off" sample),
        // new engine now keeps adding from the most recent CA value
        // (e.g. 15) on: 15-15, 15-20, 15-21, 15-22, ...
        // The 15-20 part of the new engine's block is hidden
        // below what the last engine wrote.
        // --> keep updating the last block as  20-21, 20-22, ...;
        //     hidden part 15..20 is inserted again, which ends up as a NOP.
        Datablock block;
        block.offset = node.record[i].child_or_ID;
        block.read(fa.getFile(), fa.file_offset_size);
        // Is this the one and only block under the last node
        // and does it point to offset/filename? 
        if (block.next_ID == 0 &&
            block.data_offset   == data_offset &&
            block.data_filename == data_filename)
        {
            int additions = 0;
            //   Last block's range:      |---------------|
            //     New/update range:   xxxxxxxxxxxxxxxxxxxx------|
            if (start <= node.record[i].end &&
                end > node.record[i].end)
            {   // Update end time
                node.record[i].end = end;
                write_node(node);
                adjust_tree(node, 0);
                ++additions;
            }
            // Need to insert the 'xxxx' section?
            if (start < node.record[i].start)
            {
                if (insertDatablock(start, node.record[i].start,
                                    data_offset, data_filename))
                    ++additions;
            }
            return additions > 0;
        }
    }
    // Fallback: Last-block-update wahoo didn't work.
    return insertDatablock(start, end, data_offset, data_filename);
}

void RTree::makeDot(const char *filename)
{
    FILE *dot = fopen(filename, "wt");
    if (!dot)
        return;

    fprintf(dot, "# Example for dotting & viewing:\n");
    fprintf(dot, "# dot -Tpng -o x.png %s && eog x.png &\n", filename);
    fprintf(dot, "\n");
    fprintf(dot, "digraph RTree\n");
    fprintf(dot, "{\n");
    fprintf(dot, "\tnode [shape = record, height=.1];\n");
    make_node_dot(dot, fa.getFile(), root_offset);
    fprintf(dot, "}\n");
    fclose(dot);
}

bool RTree::selfTest(unsigned long &nodes, unsigned long &records)
{
    nodes = records = 0;
    try
    {
        self_test_node(nodes, records,
                       root_offset, 0, epicsTime(), epicsTime());
    }
    catch (GenericException &e)
    {
        LOG_MSG("RTree Error: %s", e.what());
        return false;
    }
    return true;
}

// Comparison routine for AVLTree (node_cache)
int sort_compare(const RTree::Node &a, const RTree::Node &b)
{    return b.offset - a.offset; }

void RTree::read_node(Node &node) const
{
    if (node_cache.find(node))
    {
        ++cache_hits;
        return;
    }
    ++cache_misses;
    node.read(fa.getFile(), fa.file_offset_size);
    node_cache.add(node);
}

void RTree::write_node(const Node &node)
{
    node_cache.add(node);
    node.write(fa.getFile(), fa.file_offset_size);
}    

void RTree::self_test_node(unsigned long &nodes, unsigned long &records,
                           IndexFileOffset n, IndexFileOffset p,
                           epicsTime start, epicsTime end)
{
    stdString txt1, txt2, txt3, txt4;
    epicsTime s, e;
    int i;
    Node node(M, true);
    node.offset = n;
    read_node(node);
    ++nodes;
    node.getInterval(s, e);
    if (node.parent != p)
        throw GenericException(__FILE__, __LINE__,
                               "Node @ 0x%lX, %s ... %s: "
                               "parent = 0x%lX != 0x%lX\n",
                               (unsigned long)node.offset,
                               epicsTimeTxt(s, txt1), epicsTimeTxt(e, txt2),
                               (unsigned long)node.parent,
                               (unsigned long)p);
    if (p && (s != start || e != end))
                throw GenericException(__FILE__, __LINE__,
                                       "Node @ 0x%lX: Node Interval %s ... %s\n"
                                       "              Parent        %s ... %s\n",
                                       (unsigned long)node.offset,
                                       epicsTimeTxt(s, txt1),
                                       epicsTimeTxt(e, txt2),
                                       epicsTimeTxt(start, txt3),
                                       epicsTimeTxt(end, txt4));
    if (node.record[0].child_or_ID)
        ++records;
    for (i=1; i<M; ++i)
    {
        if (node.record[i].child_or_ID)
        {
            ++records;
            if (node.record[i-1].end > node.record[i].start)
                throw GenericException(__FILE__, __LINE__,
                                       "Node @ 0x%lX: Records not in order\n",
                                       (unsigned long)node.offset);
            if (!node.record[i-1].child_or_ID) 
                throw GenericException(__FILE__, __LINE__,
                                       "Node @ 0x%lX: "
                                       "Empty record before filled one\n",
                                       (unsigned long)node.offset);
        }
    }
    if (node.isLeaf)
        return; // OK until end of this branch
    for (i=0; i<M; ++i)
    {
        if (node.record[i].child_or_ID)
            self_test_node(nodes, records,
                           node.record[i].child_or_ID,
                           node.offset,
                           node.record[i].start,
                           node.record[i].end);
    }       
}

void RTree::make_node_dot(FILE *dot, FILE *f, IndexFileOffset node_offset)
{
    Datablock datablock;
    stdString txt1, txt2;
    int i;
    Node node(M, true);
    node.offset = node_offset;
    node.read(f, fa.file_offset_size);
    fprintf(dot, "\tnode%ld [ label=\"", (unsigned long)node.offset);
    for (i=0; i<M; ++i)
    {
        if (i>0)
            fprintf(dot, "|");
        epicsTime2string(node.record[i].start, txt1);
        epicsTime2string(node.record[i].end, txt2);
        if (txt1.length() > 10)
            fprintf(dot, "<f%d> %s \\r-%s \\r", i, txt1.c_str(), txt2.c_str());
        else
            fprintf(dot, "<f%d>%s-%s", i, txt1.c_str(), txt2.c_str());
    }    
    fprintf(dot, "\"];\n");
    if (node.isLeaf)
    {
        for (i=0; i<M; ++i)
        {
            datablock.offset = node.record[i].child_or_ID;
            if (datablock.offset)
                fprintf(dot, "\tnode%ld:f%d->id%ld;\n",
                        (unsigned long)node.offset, i,
                        (unsigned long)datablock.offset);
            while (datablock.offset)
            {
                datablock.read(f, fa.file_offset_size);
                fprintf(dot, "\tid%lu "
                        "[ label=\"'%s' \\r@ 0x%lX \\r\",style=filled ];\n",
                        (unsigned long)datablock.offset,
                        datablock.data_filename.c_str(),
                        (unsigned long)datablock.data_offset);
                if (datablock.next_ID)
                {
                    fprintf(dot, "\tid%lu -> id%ld;\n",
                            (unsigned long)datablock.offset,
                            (long)datablock.next_ID);
                }
                datablock.offset = datablock.next_ID;
            }
        }
    }
    else
    {
        for (i=0; i<M; ++i)
        {            
            if (node.record[i].child_or_ID)
                fprintf(dot, "\tnode%lu:f%d->node%ld:f0;\n",
                        (unsigned long)node.offset, i,
                        (long)node.record[i].child_or_ID);
        }
        for (i=0; i<M; ++i)
        {
            if (node.record[i].child_or_ID)
                make_node_dot(dot, f, node.record[i].child_or_ID);
        }
    }
}

bool RTree::search(const epicsTime &start, Node &node, int &i) const
{
    node.offset = root_offset;
    bool go;
    do
    {
        read_node(node);
        if (start < node.record[0].start) // request before start of tree?
            return getFirst(node, i);
        for (go=false, i=M-1;  i>=0;  --i)
        {   // Find right-most record with data at-or-before 'start'
            if (node.record[i].child_or_ID == 0)
                continue; // nothing
            if (node.record[i].start <= start)
            {
                if (node.isLeaf)   // Found!
                    return true;
                else
                {   // Search subtree
                    node.offset = node.record[i].child_or_ID;
                    go = true;
                    break;
                }
            }
        }
    }
    while (go);
    return false;
}

bool RTree::getFirst(Node &node, int &i) const
{
    // Descent using leftmost children
    node.offset = root_offset;
    while (node.offset)
    {
        read_node(node);
        for (i=0; i<M; ++i) // Locate leftmost record
        {
            if (node.record[i].child_or_ID)
                break;
        }
        if (i>=M)
            return false; // nothing
        if (node.isLeaf)  // Done or continue to go down?
            return true; // Found it!
        // Point offset to child, one level down in tree.
        node.offset = node.record[i].child_or_ID;
    }    
    return false; // nothing
}

bool RTree::getLast(Node &node, int &i) const
{
    // Descent using rightmost children
    node.offset = root_offset;
    while (node.offset)
    {
        read_node(node);
        for (i=M-1; i>=0; --i) // Locate rightmost record
            if (node.record[i].child_or_ID)
                break;
        if (i<0)
            return false; // nothing
        if (node.isLeaf)  // Done or continue to go down?
            return true; // Found it!
        // Point offset to child, one level down in tree.
        node.offset = node.record[i].child_or_ID;
    }    
    return false; // nothing
}

bool RTree::prev_next(Node &node, int &i, int dir) const
{
    LOG_ASSERT(node.isLeaf);
    LOG_ASSERT(i>=0  &&  i<M);
    LOG_ASSERT(dir == -1  ||  dir == 1);
    i += dir;
    // Another rec. in curr.node?
    if (i>=0 && i<M && node.record[i].child_or_ID)
        return true;
    Node parent(M, true);
    // Go up to parent nodes...
    while (true)
    {
        if (!(parent.offset = node.parent))
            return false;
        read_node(parent);
        for (i=0; i<M; ++i)
            if (parent.record[i].child_or_ID == node.offset)
                break;
        if (i>=M)
            throw GenericException(__FILE__, __LINE__,
                                   "RTree::next: child_or_ID "
                                   "not listed in parent?\n");
        i += dir;
        if (i>=0 && i<M && parent.record[i].child_or_ID)
            break;
        // else: go up another level
        node = parent;
    }
    node.offset = parent.record[i].child_or_ID;
    // Decend using rightmost (prev) or leftmost (next)  children
    i = 0;
    while (node.offset)
    {
        read_node(node);
        if (dir < 0)
            for (i=M-1; i>0; --i)
                if (node.record[i].child_or_ID)
                    break;
        if (node.isLeaf)
            return node.record[i].child_or_ID != 0;
        node.offset = node.record[i].child_or_ID;
    }
    return false;
}

// Insertion follows Guttman except as indicated
bool RTree::insertDatablock(const epicsTime &start,
                            const epicsTime &end,
                            IndexFileOffset data_offset,
                            const stdString &data_filename)
{
    stdString txt1, txt2;
    int       i;
    size_t    additions = 0;
    Datablock block, new_block;
    Node      node(M, true);
    LOG_ASSERT(start <= end);
    node.offset = root_offset;
    choose_leaf(start, end, node);
    for (i=0; i<M; ++i) // find record[i] <= [start...end]
    {   // Stop on first empty record
        if (node.record[i].child_or_ID == 0)
            break;
        // Check for the 4 possible overlap situations.
        // Note: Overlap! Just "touching" is not an "overlap".
        // Block is added to all records that cover it so that
        // we'll find it when re-building a master index.
        if (node.record[i].start <= start  &&  end <= node.record[i].end)
        {   // (1) Existing record:  |------------|
            //     New block      :     |---|
            //     ==> Add block to existing record
            return add_block_to_record(node, i, data_offset, data_filename);
        }
        if (start < node.record[i].start  &&
            node.record[i].start < end && end <= node.record[i].end)
        {   // (2) Existing record:         |-------|
            //     New block      :     |--------|
            //     ==> Add non-overlap  |###|           
            if (add_block_to_record(node, i, data_offset, data_filename))
                ++additions;
            if (insertDatablock(start, node.record[i].start,
                                  data_offset, data_filename))
                ++additions;
            return additions>0;
        }
        if (node.record[i].start <= start && start < node.record[i].end &&
            node.record[i].end < end)
        {   // (3) Existing record:     |-------|
            //     New block      :        |--------|
            //     ==> Add non-overlap          |###|
            if (add_block_to_record(node, i, data_offset, data_filename))
                ++additions;
            if (insertDatablock(node.record[i].end, end,
                                data_offset, data_filename))
                ++additions;
            return additions>0;
        }
        if (start < node.record[i].start && node.record[i].end < end)
        {
            // (4) Existing record:        |---|
            //     New block      :     |----------|
            //     ==> Add non-overlaps |##| + |###|
            if (add_block_to_record(node, i, data_offset, data_filename))
                ++additions;
            if (insertDatablock(start, node.record[i].start,
                                data_offset, data_filename))
                ++additions;
            if (insertDatablock(node.record[i].end, end,
                                data_offset, data_filename))
                ++additions;
            return additions>0;
        }
        // Otherwise: records are sorted in time. Does new entry belong here?
        if (end <= node.record[i].start)
            break;
    }
    // Need to insert new block and record at record[i]
    write_new_datablock(data_offset, data_filename, new_block);
    Node overflow(M, true);
    bool caused_overflow, rec_in_overflow;
    insert_record_into_node(node, i,
                            start, end, new_block.offset,
                            overflow,
                            caused_overflow, rec_in_overflow);
    if (caused_overflow)
        adjust_tree(node, &overflow);
    else
        adjust_tree(node, 0);
    return true;
}

bool RTree::add_block_to_record(const Node &node, int i,
                                IndexFileOffset data_offset,
                                const stdString &data_filename)
{
    LOG_ASSERT(node.isLeaf);
    Datablock block;
    block.next_ID = node.record[i].child_or_ID;
    while (block.next_ID) // run over blocks under record
    {
        block.offset = block.next_ID;
        block.read(fa.getFile(), fa.file_offset_size);
        if (block.data_offset == data_offset &&
            block.data_filename == data_filename)
            return false; // found an existing datablock
    }
    // Block's now the last in the chain
    Datablock new_block;
    write_new_datablock(data_offset, data_filename, new_block);
    block.next_ID = new_block.offset;
    block.write(fa.getFile(), fa.file_offset_size);
    return true; // added a new datablock
}

void RTree::write_new_datablock(IndexFileOffset data_offset,
                                const stdString &data_filename,
                                Datablock &block)
{
    block.next_ID = 0;
    block.data_offset = data_offset;
    block.data_filename = data_filename;
    block.offset = fa.allocate(block.getSize());
    block.write(fa.getFile(), fa.file_offset_size);
}

// Check if intervals s1...e1 and s2...e2 overlap.
// Note: They might touch, e.g. s1 <= e1  <= s2 <= e2,
//       that's still not considered an overlap
inline bool do_intervals_overlap(const epicsTime &s1, const epicsTime &e1,
                                 const epicsTime &s2, const epicsTime &e2)
{
    if (e1 <= s2   ||  e2 <= s1)
        return false;
    return true;
}

void RTree::choose_leaf(const epicsTime &start, const epicsTime &end,
                        Node &node)
{
    read_node(node);
    if (node.isLeaf)
        return; // done.
    // If a subtree already contains data for the time range
    // or there's an overlap, use that one. Otherwise follow
    // the RTree paper:
    // Find entry which needs the least enlargement.
    epicsTime t0, t1;
    double enlarge, min_enlarge=0;
    int i, min_i=-1;
    for (i=0; i<M; ++i)
    {
        if (!node.record[i].child_or_ID)
            continue;
        if (do_intervals_overlap(node.record[i].start, node.record[i].end,
                                 start, end))
        {
            node.offset = node.record[i].child_or_ID;
            choose_leaf(start, end, node);
            return;
        }
        // t0 .. t1 = union(start...end, record[i].start...end)
        if (start < node.record[i].start)
            t0 = start;
        else
            t0 = node.record[i].start;
        if (end > node.record[i].end)
            t1 = end;
        else
            t1 = node.record[i].end;
        enlarge = (t1 - t0) - (node.record[i].end - node.record[i].start);
        // Pick rightmost record of those with min. enlargement
        if (i==0  ||  enlarge <= min_enlarge)
        {
            min_enlarge = enlarge;
            min_i = i;
        }
    }
    node.offset = node.record[min_i].child_or_ID;
    LOG_ASSERT(node.offset != 0);
    choose_leaf(start, end, node);
}

// Need to insert new start/end/ID into node's record[idx].
// If that causes an overflow, use the overflow node.
// Overflow needs to be initialized:
// All records 0, isLeaf as it needs to be,
// but mustn't be allocated, yet: This routine will allocate
// if overflow gets used.
// caused_overflow indicates if the overflow Node is used.
// rec_in_overflow indicates if the new record ended up in overflow.
// Node gets written, overflow doesn't get written.
void RTree::insert_record_into_node(Node &node, int idx,
                                    const epicsTime &start,
                                    const epicsTime &end, IndexFileOffset ID,
                                    Node &overflow,
                                    bool &caused_overflow,
                                    bool &rec_in_overflow)
{
    int j;
    if (idx<M)
    {
        rec_in_overflow = false;
        overflow.record[0] = node.record[M-1]; // With last rec. into overflow,
        for (j=M-1; j>idx; --j) // shift all recs right from idx on.   
            node.record[j] = node.record[j-1];
        node.record[idx].start = start;
        node.record[idx].end = end;
        node.record[idx].child_or_ID = ID;
    }
    else
    {
        rec_in_overflow = true;
        overflow.record[0].start = start;
        overflow.record[0].end = end;
        overflow.record[0].child_or_ID = ID;
    }
    caused_overflow = overflow.record[0].child_or_ID != 0;
    if (caused_overflow)
    {
        // Need to split node because of overflow
        overflow.offset = fa.allocate(NodeSize(M));
        int cut = M/2+1;
        // TODO: This results in a 50/50 split
        // Maybe it's better to split 70/30 because
        // the Engine will insert consecutive data?
        
        // There are M records in node and 1 in overflow.
        // Shift from node.record[cut] on into into overflow.
        overflow.record[M-cut] = overflow.record[0];
        for (j=cut; j<M; ++j)
        {   // copy from node and clear the copied entries in node
            LOG_ASSERT(j-cut >= 0);
            LOG_ASSERT(j-cut < M);
            overflow.record[j-cut] = node.record[j];
            node.record[j].clear();
        }
        rec_in_overflow = (idx >= cut);
    }
    write_node(node);
}

// This is the killer routine which keeps the tree balanced.
void RTree::adjust_tree(Node &node, Node *new_node)
{
    int i;
    epicsTime start, end;
    Node parent(M, true);
    parent.offset = node.parent;
    if (!parent.offset) // reached root?
    {   
        if (!new_node)
            return; // done
        // Have new_node parallel to root
        //    [ node == root ]  [ new_node ]
        //  -> grow taller, add new root
        //    [ new_root ]
        //    [ node ]  [ new_node ]
        Node new_root(M, false);
        new_root.offset = fa.allocate(NodeSize(M));
        // new_root.child[0] = node
        if (!node.getInterval(new_root.record[0].start, new_root.record[0].end))
            throw GenericException(__FILE__, __LINE__, "Empty node?");
        new_root.record[0].child_or_ID = node.offset;
        node.parent = new_root.offset;
        // new_root.child[1] = new_node
        if (!new_node->getInterval(new_root.record[1].start,
                                   new_root.record[1].end))
            throw GenericException(__FILE__, __LINE__, "Empty new_node?");
        new_node->parent = new_root.offset;
        new_root.record[1].child_or_ID = new_node->offset;
        // Write all
        write_node(node);
        write_node(*new_node);
        write_node(new_root);
        // Update Root pointer
        root_offset = new_root.offset;
        if (! (fseeko(fa.getFile(), anchor, SEEK_SET)==0 &&
               WriteIndexFileOffset(fa.getFile(), root_offset, fa.file_offset_size)==true))
            throw GenericException(__FILE__, __LINE__, "write error @ 0x%08lX",
                                   (unsigned long) anchor);
        return; // done.
    }
    read_node(parent);
    for (i=0; i<M; ++i)   // update parent's interval for node
    {
       if (parent.record[i].child_or_ID == node.offset)
        {
            node.getInterval(start, end);
            if (start!=parent.record[i].start || end!=parent.record[i].end)
            {
                parent.record[i].start = start;
                parent.record[i].end   = end;
                write_node(parent);
            }
            break;
        }
    }
    if (!new_node) // Done at this level, go on up.
    {
        adjust_tree(parent, 0);
        return;
    }
    // Have to add new_node to parent
    if (!new_node->getInterval(start, end))
        throw GenericException(__FILE__, __LINE__, "Empty node?");
    for (i=0; i<M; ++i)
        if (parent.record[i].child_or_ID == 0 || end <= parent.record[i].start)
            break;  // new entry belongs into parent.record[i]
    Node overflow(M, false);    
    bool caused_overflow, rec_in_overflow;
    insert_record_into_node(parent, i, start, end, new_node->offset,
                            overflow, caused_overflow, rec_in_overflow);
    if (rec_in_overflow)
        new_node->parent = overflow.offset;
    else
        new_node->parent = parent.offset;
    write_node(*new_node);
    if (!caused_overflow)
    {
        adjust_tree(parent, 0); // no overflow; go on up.
        return;
    }
    // Either new_node or overflow from parent ended up in overflow
    write_node(overflow);
    // Adjust 'parent' pointers of all children that were moved to overflow
    Node overflow_child(M, true);
    for (i=0; i<M; ++i)
    {
        overflow_child.offset = overflow.record[i].child_or_ID;
        if (overflow_child.offset == 0 ||
            overflow_child.offset == new_node->offset)
            continue;
        read_node(overflow_child);
        overflow_child.parent = overflow.offset;
        write_node(overflow_child);
    }
    adjust_tree(parent, &overflow);
}

// Follows Guttman except that we don't care about
// half-filled nodes. Only empty nodes get removed.
bool RTree::remove(const epicsTime &start, const epicsTime &end, IndexFileOffset ID)
{
    int i;
    Node node(M, true);
    node.offset = root_offset;
    bool go;
    // search from root on down
    do
    {
        read_node(node);
        for (go=false, i=0;  i<M;  ++i)
        {   // Find left-most record that includes our target interval
            if (node.record[i].child_or_ID == 0)
                return false;
            if (!node.isLeaf &&
                node.record[i].start <= start && node.record[i].end >= end)
            {   // Search subtree.
                node.offset = node.record[i].child_or_ID;
                go = true;
                break;
            }
            if (node.isLeaf &&
                node.record[i].start == start && node.record[i].end == end &&
                node.record[i].child_or_ID == ID)
            {   // Found the entry.
                remove_record(node, i);
                return true;
            }
        }
    }
    while (go);
    return false;
}

void RTree::remove_record(Node &node, int i)
{
    int j;
    // Delete original entry
    for (j=i; j<M-1; ++j)
        node.record[j] = node.record[j+1];
    node.record[j].start = nullTime;
    node.record[j].end = nullTime;
    node.record[j].child_or_ID = 0;
    write_node(node);
    condense_tree(node);
}

void RTree::condense_tree(Node &node)
{
    int i, j=-1;
    if (node.parent==0)
    {   // reached root
        if (!node.isLeaf)
        {   // Count children of root
            int children = 0;
            for (i=0; i<M; ++i)
                if (node.record[i].child_or_ID)
                {
                    ++children;
                    j=i;
                }
            if (children==1)
            {   // only child_or_ID j left => make that one root
                IndexFileOffset old_root = node.offset;
                root_offset = node.offset = node.record[j].child_or_ID;
                read_node(node);
                node.parent = 0;
                write_node(node);
                if (fseeko(fa.getFile(), anchor, SEEK_SET) != 0)
                    throw GenericException(__FILE__, __LINE__, "seek failed");
                WriteIndexFileOffset(fa.getFile(), root_offset, fa.file_offset_size);
                fa.free(old_root);
            }
        }
        return;
    }
    bool empty = true;
    for (i=0; i<M; ++i)
        if (node.record[i].child_or_ID)
        {
            empty = false;
            break;
        }
    Node parent(M, true);
    parent.offset = node.parent;
    read_node(parent);
    for (i=0; i<M; ++i)
        if (parent.record[i].child_or_ID == node.offset)
        {
            if (empty)
            {   // Delete the empty node, remove from parent
                fa.free(node.offset);
                for (j=i; j<M-1; ++j)
                    parent.record[j] = parent.record[j+1];
                parent.record[M-1].start = nullTime;
                parent.record[M-1].end   = nullTime;
                parent.record[M-1].child_or_ID = 0;
            }
            else // Update parent with current interval of node
                if (!node.getInterval(parent.record[i].start,
                                      parent.record[i].end))
                    throw GenericException(__FILE__, __LINE__, "empty node?");
            // Write updated parent and move up the tree
            write_node(parent);
            condense_tree(parent);
            return;
        }
    throw GenericException(__FILE__, __LINE__,
                           "Cannot find child_or_ID in parent\n");
}

bool RTree::updateLast(const epicsTime &start, const epicsTime &end,
                       IndexFileOffset ID)
{
    int i;
    Node node(M, true);
    if (!getLast(node, i))
        return false;
    if (node.record[i].child_or_ID != ID  ||
        node.record[i].start != start)
        return false; // Cannot update, different data block
    // Update end time, done.
    node.record[i].end = end;
    write_node(node);
    adjust_tree(node, 0);
    return true;
}


