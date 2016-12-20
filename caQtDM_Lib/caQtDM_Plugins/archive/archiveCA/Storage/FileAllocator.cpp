// System
#include <string.h>
// Tools
#include <MsgLogger.h>
#include <BinIO.h>
// Index
#include "FileAllocator.h"

#undef DEBUG_FA

// Layout of the data file:
//
// - From the beginning of the file, reserved_space
//   bytes are skipped.
// - Two list_node entries follow:
//   a) list of allocated units,
//   b) list of free units
// - Allocated or free data block follow:
//   each one starts with a list_node,
//   the user of the FileAllocator is handed
//   the offset to the next byte

// We read/write the contents ourself,
// not including possible pad bytes of the structure
static const IndexFileOffset list_node_size = 3 * sizeof(IndexFileOffset);

IndexFileOffset FileAllocator::minimum_size = 64;
IndexFileOffset FileAllocator::file_size_increment = 0;

FileAllocator::FileAllocator()
{
#ifdef DEBUG_FA
    printf("New FileAllocator\n");
#endif
    f = 0;
    file_offset_size = 64;
}

FileAllocator::~FileAllocator()
{
    if (f)
    {
        fprintf(stderr, "*****************************\n");
        fprintf(stderr, "FileAllocator wasn't detached\n");
        fprintf(stderr, "*****************************\n");
    }
#ifdef DEBUG_FA
    printf("Deleted FileAllocator\n");
#endif
}    

bool FileAllocator::attach(FILE *f, IndexFileOffset reserved_space, bool init)
{
#ifdef DEBUG_FA
    printf("FileAllocator::attach()\n");
#endif
    LOG_ASSERT(f != 0);
    this->f = f;
    this->reserved_space = reserved_space;
    if (fseeko(this->f, 0, SEEK_END))
        throw GenericException(__FILE__, __LINE__, "fseeko error");
    file_size = ftello(this->f);
    if (file_size < 0)
        throw GenericException(__FILE__, __LINE__, "ftello error");
    // File size should be
    // 0 for new file or at least reserved_space + list headers
    if (file_size == 0)
    {
        if (init == false)
            throw GenericException(__FILE__, __LINE__,
                                   "FileAllocator in read-only mode found empty file");
        // create empty list headers
        memset(&allocated_head, 0, list_node_size);
        memset(&free_head, 0, list_node_size);
        write_node(this->reserved_space, &allocated_head);
        write_node(this->reserved_space+list_node_size, &free_head);
        file_size = ftello(this->f);
        IndexFileOffset expected = reserved_space + 2*list_node_size;
        if (file_size != expected)
             throw GenericException(__FILE__, __LINE__,
                                    "Initialization error: "
                                    "Expected file size %ld, found %ld",
                                    (long) expected, (long) file_size);
        return true; 
    }
    IndexFileOffset expected = this->reserved_space + 2*list_node_size;
    if (file_size < expected)
        throw GenericException(__FILE__, __LINE__,
                               "FileAllocator: Broken file header,"
                               "expected at least %ld bytes",
                               (long) expected);
    // read existing list headers
    read_node(this->reserved_space, &allocated_head);
    read_node(this->reserved_space+list_node_size, &free_head);
    return false; // Didn't initialize the file
}

FILE *FileAllocator::getFile() const
{
    if (!f)
        fprintf(stderr, "FileAllocator::getFile called while closed\n");
    return f;
}

void FileAllocator::detach()
{
    f = 0;
#ifdef DEBUG_FA
    printf("FileAllocator::detach()\n");
#endif
}

IndexFileOffset FileAllocator::allocate(IndexFileOffset num_bytes)
{
    if (num_bytes < minimum_size)
        num_bytes = minimum_size;
    list_node node;
    IndexFileOffset node_offset;
    // Check free list for a valid entry
    node_offset = free_head.next;
    while (node_offset)
    {
        read_node(node_offset, &node);
        if (node.bytes >= num_bytes)
        {   // Found an appropriate block on the free list.
            if (node.bytes < num_bytes + list_node_size + minimum_size)
            {   // not worth splitting, return as is:
                remove_node(reserved_space+list_node_size, &free_head,
                            node_offset, &node);
                insert_node(reserved_space, &allocated_head,
                            node_offset, &node);
                return node_offset + list_node_size;
            }
            else
            {
                // Split requested size off, correct free list
                node.bytes -= list_node_size + num_bytes;
                free_head.bytes -= list_node_size + num_bytes;
                write_node(node_offset, &node);
                write_node(reserved_space+list_node_size, &free_head);
                // Create, insert & return new node
                IndexFileOffset new_offset =
                    node_offset + list_node_size + node.bytes;
                list_node new_node;
                new_node.bytes = num_bytes;
                insert_node(reserved_space, &allocated_head,
                            new_offset, &new_node);
                return new_offset + list_node_size;
            }
        }
        node_offset = node.next;
    }
    // Need to append new block.
    // grow file
    if (fseeko(f, file_size+list_node_size+num_bytes-1, SEEK_SET) != 0  ||
        fwrite("", 1, 1, f) != 1)
        throw GenericException(__FILE__, __LINE__, "Write Error");
    // write new node
    node.bytes = num_bytes;
    node.prev = allocated_head.prev;
    node.next = 0;
    write_node(file_size, &node);
    // maybe update what used to be the last block
    if (allocated_head.prev)
    {
        read_node(allocated_head.prev, &node);
        node.next = file_size;
        write_node(allocated_head.prev, &node);
    }
    // update head of list
    allocated_head.bytes += num_bytes;
    if (allocated_head.next == 0)
        allocated_head.next = file_size;
    allocated_head.prev = file_size;
    write_node(reserved_space, &allocated_head);
    // Update overall file size, return offset of new data block
    IndexFileOffset data_offset = file_size + list_node_size;
    file_size = data_offset + num_bytes;
    if (file_size_increment > 0)
    {
        // Grow to desired multiple of file_size_increment
        // after the fact, but this way it can use the
        // allocate() & free() calls as is:
        if (file_size % file_size_increment)
        {
            size_t bytes_to_next_increment =
                ((file_size / file_size_increment)+1)*file_size_increment
                - file_size;
            // We'll allocate the block and free it
            // so that the file grows as intended and
            // the unused mem is on the free list
            this->free(this->allocate(bytes_to_next_increment - list_node_size));
        }
    }
    return data_offset;
}

void FileAllocator::free(IndexFileOffset block_offset)
{
    // list_node should precede the memory block,
    // so it cannot start before reserved_space + heads + 1st buffer node,
    // and it cannot start beyond the known file size.
    if (block_offset < reserved_space + 3*list_node_size  ||
        block_offset >= file_size)
        throw GenericException(__FILE__, __LINE__,
                               "FileAllocator::free, impossible offset %ld\n",
                               (unsigned long)block_offset);
    list_node node;
    IndexFileOffset node_offset = block_offset - list_node_size;
    read_node(node_offset, &node);
    if (node_offset + node.bytes > file_size)
        throw GenericException(__FILE__, __LINE__,
                               "FileAllocator::free called with broken node %ld\n",
                               (unsigned long)block_offset);
    // remove node at 'offset' from list of allocated blocks,
    // add node to list of free blocks
    remove_node(reserved_space, &allocated_head, node_offset, &node);
    insert_node(reserved_space+list_node_size, &free_head,
                node_offset, &node);
    // Check if we can merge with the preceeding block
    list_node pred;
    IndexFileOffset pred_offset = node.prev;
    if (pred_offset)
    {
        read_node(pred_offset, &pred);
        if (pred_offset + list_node_size + pred.bytes == node_offset)
        {   // Combine this free'ed node with prev. block
            pred.bytes += list_node_size + node.bytes;
            // skip the current node
            pred.next = node.next;
            write_node(pred_offset, &pred);
            if (pred.next)
            {   // correct back pointer
                read_node(pred.next, &node);
                node.prev = pred_offset;
                write_node(pred.next, &node);
            }
            else
            {   // we changed the tail of the free list
                free_head.prev = pred_offset;
            }
            free_head.bytes += list_node_size;
            write_node(reserved_space+list_node_size, &free_head);
            
            // 'pred' is now the 'current' node
            node_offset = pred_offset;
            node = pred;
        }
    }

    // Check if we can merge with the succeeding block
    list_node succ;
    IndexFileOffset succ_offset = node.next;
    if (succ_offset)
    {
        if (node_offset + list_node_size + node.bytes == succ_offset)
        {   // Combine this free'ed node with following block
            read_node(succ_offset, &succ);
            node.bytes += list_node_size + succ.bytes;
            // skip the next node
            node.next = succ.next;
            write_node(node_offset, & node);
            if (node.next)
            {   // correct back pointer
                list_node after;
                read_node(node.next, &after);
                after.prev = node_offset;
                write_node(node.next, &after);
            }
            else
            {   // we changed the tail of the free list
                free_head.prev = node_offset;
            }
            free_head.bytes += list_node_size;
            write_node(reserved_space+list_node_size, &free_head);
        }
    }
}

bool FileAllocator::dump(int level, FILE *f)
{
    bool ok = true;
    try
    {
        list_node allocated_node, free_node;
        IndexFileOffset allocated_offset, free_offset;
        IndexFileOffset allocated_prev = 0, free_prev = 0;
        IndexFileOffset allocated_mem = 0, allocated_blocks = 0;
        IndexFileOffset free_mem = 0, free_blocks = 0;
        IndexFileOffset next_offset = 0;
        if (level > 0)
            fprintf(f, "bytes in file: %ld. Reserved/Allocated/Free: %ld/%ld/%ld\n",
                   (long)file_size,
                   (long)reserved_space,
                   (long)allocated_head.bytes,
                   (long)free_head.bytes);
        allocated_offset = allocated_head.next;
        free_offset = free_head.next;
        while(allocated_offset || free_offset)
        {
            // Show the node that's next in the file
            if (allocated_offset &&
                (free_offset == 0 || free_offset > allocated_offset))
            {
                read_node(allocated_offset, &allocated_node);
                ++allocated_blocks;
                allocated_mem += allocated_node.bytes;
                if (level > 0)
                    fprintf(f, "Allocated Block @ %10ld: %10ld bytes\n",
                           (long)allocated_offset, (long)allocated_node.bytes);
                if (next_offset && next_offset != allocated_offset)
                {
                    fprintf(f, "! There is a gap, %ld unmaintained bytes "
                           "before this block!\n",
                           (long)allocated_offset - (long)next_offset);
                    ok = false;
                }
                if (allocated_prev != allocated_node.prev)
                {
                    fprintf(f, "! Block's ''prev'' pointer is broken!\n");
                    ok = false;
                }
                next_offset = allocated_offset + list_node_size + allocated_node.bytes;
                allocated_prev = allocated_offset;
                allocated_offset = allocated_node.next;
            }
            else if (free_offset)
            {
                read_node(free_offset, &free_node);
                ++free_blocks;
                free_mem += free_node.bytes;
                if (level > 0)
                    fprintf(f, "Free      Block @ %10ld: %10ld bytes\n",
                           (long)free_offset, (long)free_node.bytes);
                if (next_offset && next_offset != free_offset)
                {
                    fprintf(f, "! There is a gap, %ld unmaintained bytes "
                           "before this block!\n", (long)free_offset - (long)next_offset);
                    ok = false;
                }
                if (free_prev != free_node.prev)
                {
                    fprintf(f, "! Block's ''prev'' pointer is broken!\n");
                    ok = false;
                }
                next_offset = free_offset + list_node_size + free_node.bytes;
                free_prev = free_offset;
                free_offset = free_node.next;
            }
        }
        if (file_size !=
            (reserved_space +
             2*list_node_size + // allocated/free list headers
             allocated_blocks*list_node_size +
             allocated_head.bytes +
             free_blocks*list_node_size +
             free_head.bytes))
        {
            fprintf(f, "! The total file size does not compute!\n");
            ok = false;
        }
        if (allocated_mem != allocated_head.bytes)
        {
            fprintf(f, "! The amount of allocated space does not compute!\n");
            ok = false;
        }
        if (free_mem != free_head.bytes)
        {
            fprintf(f, "! The amount of allocated space does not compute!\n");
            ok = false;
        }
    }
    catch (GenericException &e)
    {
        fprintf(f, "Error: %s", e.what());
        return false;
    }
    return ok;
}

void FileAllocator::read_node(IndexFileOffset offset, list_node *node)
{
    if (! (fseeko(f, offset, SEEK_SET) == 0  &&
           ReadIndexFileOffset(f, &node->bytes, file_offset_size) &&
           ReadIndexFileOffset(f, &node->prev,  file_offset_size) &&
           ReadIndexFileOffset(f, &node->next,  file_offset_size)))
        throw GenericException(__FILE__, __LINE__,
                               "FileAllocator node read at 0x%08lX failed",
                               (unsigned long)offset);
}

void FileAllocator::write_node(IndexFileOffset offset, const list_node *node)
{
    if (! (fseeko(f, offset, SEEK_SET) == 0  &&
           WriteIndexFileOffset(f, node->bytes, file_offset_size) &&
           WriteIndexFileOffset(f, node->prev,  file_offset_size) &&
           WriteIndexFileOffset(f, node->next,  file_offset_size)))
        throw GenericException(__FILE__, __LINE__,
                               "FileAllocator node write at 0x%08lX failed",
                               (unsigned long)offset);
}

void FileAllocator::remove_node(IndexFileOffset head_offset, list_node *head,
                                IndexFileOffset node_offset, const list_node *node)
{
    list_node tmp;
    IndexFileOffset tmp_offset;

    if (head->next == node_offset)
    {   // first node; make head skip it
        head->bytes -= node->bytes;
        head->next = node->next;
        if (head->next == 0) // list now empty?
            head->prev = 0;
        else
        {   // correct 'prev' ptr of what's now the first node
            read_node(head->next, &tmp);
            tmp.prev = 0;
            write_node(head->next, &tmp);
        }
        write_node(head_offset, head);
    }
    else
    {   // locate a node that's not the first
        tmp_offset = head->next;
        read_node(tmp_offset, &tmp);
        while (tmp.next != node_offset)
        {
            tmp_offset = tmp.next;
            if (!tmp_offset)
                throw GenericException(__FILE__, __LINE__,
                                       "node not found");
            read_node(tmp_offset, &tmp);
        }
        // tmp/tmp_offset == node before the one to be removed
        tmp.next = node->next;
        write_node(tmp_offset, &tmp);
        if (node->next)
        {   // adjust prev pointer of node->next
            read_node(node->next, &tmp);
            tmp.prev = tmp_offset;
            write_node(node->next, &tmp);
        }
        head->bytes -= node->bytes;
        if (head->prev == node_offset)
            head->prev = node->prev;
        write_node(head_offset, head);
    }
}

void FileAllocator::insert_node(IndexFileOffset head_offset, list_node *head,
                                 IndexFileOffset node_offset, list_node *node)
{
    // add node to list of free blocks
    if (head->next == 0)
    {   // first in free list
        head->next = node_offset;
        head->prev = node_offset;
        node->next = 0;
        node->prev = 0;
        head->bytes += node->bytes;
        write_node(node_offset, node);
        write_node(head_offset, head);
        return;
    }
    // insert into free list, sorted by position (for nicer dump() printout)
    if (node_offset < head->next)
    {   // new first item
        node->prev = 0;
        node->next = head->next;
        write_node(node_offset, node);
        list_node tmp;
        read_node(node->next, &tmp);
        tmp.prev = node_offset;
        write_node(node->next, &tmp);
        head->next = node_offset;
        head->bytes += node->bytes;
        write_node(head_offset, head);
        return;
    }
    // find proper location in free list
    list_node pred;
    IndexFileOffset pred_offset = head->next;
    read_node(pred_offset, &pred);
    while (pred.next && pred.next < node_offset)
    {
        pred_offset = pred.next;
        read_node(pred_offset, &pred);
    }
    // pred.next == 0  or >= node_offset: insert here!
    node->next = pred.next;
    node->prev = pred_offset;
    write_node(node_offset, node);
    if (pred.next)
    {
        list_node after;
        read_node(pred.next, &after);
        after.prev = node_offset;
        write_node(pred.next, &after);
    }
    pred.next = node_offset;
    write_node(pred_offset, &pred);
    if (head->prev == pred_offset)
        head->prev = node_offset;
    head->bytes += node->bytes;
    write_node(head_offset, head);
}

