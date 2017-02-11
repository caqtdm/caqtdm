// -*- c++ -*-
#ifndef __MEMORYBUFFER_H__
#define __MEMORYBUFFER_H__

// Tools
#include <GenericException.h>
#include <NoCopy.h>
#include <stdlib.h>

/** \ingroup Tools
 *  A memory region that can be resized.
 * 
 *  A MemoryBuffer<T> which has reserved size,
 *  may grow in size (new, no realloc)
 *  and is automatically deallocated.
 */
template <class T>
class MemoryBuffer
{
public:
    /// Constructor: Buffer is initially empty.
    MemoryBuffer() : memory(0), size(0)    {}

    MemoryBuffer(size_t wanted) : memory(0), size(0)
    {
        reserve(wanted);
    }

    /// Destructor.
    ~MemoryBuffer()
    {
        if (memory)
            free(memory);
    }

    /// Reserve or grow buffer.
    ///
    /// @exception GenericException
    void reserve(size_t wanted)
    {
        if (size < wanted)
        {
            if (memory)
                free(memory);
            memory = (char *)calloc(wanted, 1);
            if (!memory)
                throw GenericException(__FILE__, __LINE__,
                                       "MemoryBuffer::reserve(%zu) failed",
                                       wanted);
            size = wanted;
        }
    }

    /// Access as (T *)
    const T *mem() const
    {    return (const T *)memory; }

    /// Access as (T *)
    T *mem()
    {    return (T *) memory; }

    /// Get current size.
    size_t capacity() const
    {    return size; }

private:
    PROHIBIT_DEFAULT_COPY(MemoryBuffer);
    char    *memory;
    size_t    size;
};

#endif //__MEMORYBUFFER_H__
