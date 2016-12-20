
// Tools
#include "Bitset.h"
#include "GenericException.h"

BitSet::BitSet()
{
    bits = 0;
    size = 0;
    num = 0;
}

BitSet::~BitSet()
{
    delete [] bits;
}

void BitSet::grow(size_t minimum)
{
    if (size >= minimum)
        return;
    if (! minimum)
        return;
    num = (minimum-1) / 32 + 1;
    
    uint32_t *new_bits = 0;
    try
    {
        new_bits = new uint32_t[num];
    }
    catch (...)
    {
        throw GenericException(__FILE__, __LINE__,
                               "BitSet::grow(%zu) from %zu bits failed",
                               minimum, size);
    }
    memset(new_bits, 0, num*4);
    if (bits)
    {
        memcpy(new_bits, bits, size / 8);
        delete [] bits;
    }
    bits = new_bits;
    size = num * 32;
}

void BitSet::set(size_t bit)
{
    if (bit >= size)
        throw GenericException(__FILE__, __LINE__,
                               "BitSet::set(%zu) for only %zu bits failed",
                               bit, size);
    uint32_t *b = bits + (bit / 32);
    bit %= 32;
    *b |= (1 << bit);
}

void BitSet::clear(size_t bit)
{
    if(bit >= size)
        throw GenericException(__FILE__, __LINE__,
                               "BitSet::clear(%zu) for only %zu bits failed",
                               bit, size);
    uint32_t *b = bits + (bit / 32);
    bit %= 32;
    *b &= ~(1 << bit);
}

void BitSet::set(size_t bit, bool value)
{
    if (value)
        set(bit);
    else
        clear(bit);
}

bool BitSet::test(size_t bit) const
{
    if (bit >= size)
        throw GenericException(__FILE__, __LINE__,
                               "BitSet::test(%zu) for only %zu bits failed",
                               bit, size);
    uint32_t *b = bits + (bit / 32);
    bit %= 32;
    return !!(*b & (1 << bit));
}

bool BitSet::empty() const
{   return ! any(); }

bool BitSet::any() const
{
    uint32_t *b;
    size_t i;
    for (b=bits, i=num; i>0; --i)
    {
        if (*b)
            return true;
        ++b;
    }
    return false;
}

size_t BitSet::count() const
{
    size_t c = 0, i;
    uint32_t t, *b=bits;
    for (i=num; i>0; --i)
    {
        for (t=*b; t; t >>= 4) // hack taken from STL
            c += "\0\1\1\2\1\2\2\3\1\2\2\3\2\3\3\4" [t & 0xF];
        ++b;
    }
    return c;
}

// Excludes leading '0's...
stdString BitSet::to_string() const
{
    bool leading = false;
    size_t i=size;
    stdString s;
    s.reserve (i);
    while (i > 0)
    {
        --i;
        if (test(i))
        {
            s += '1';
            leading = true;
        }
        else
            if (leading || i==0)
                s += '0';
    }

    return s;
}

#if 0
BitSet & BitSet::operator = (const BitSet &rhs)
{
    num = rhs.num;
    size = rhs.size;
    if (num > 0)
    {
        bits = new uint32_t[num];
        memcpy (bits, rhs.bits, num*4);
    }
    else
        bits = 0;
    return *this;
}
#endif

