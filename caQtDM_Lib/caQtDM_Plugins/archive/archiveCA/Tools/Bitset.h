// -*- c++ -*-
#ifndef __BITSET_H__
#define __BITSET_H__

// System
#include<stdint.h>
// Tools
#include<stdString.h>
#include<NoCopy.h>

/// \ingroup Tools BitSet, inspired by STL bitset.
///
/// Reasons for reinventing this type of class:
///
/// - The STL version was not part of the Sept 1999 egcs compiler.
/// - This BitSet grows during runtime, size is not locked at compiletime.
///
class BitSet
{
public:
    BitSet ();
    ~BitSet ();

    /// Grow so that size is at least minimum
    
    /// All the following set/clear/test operations
    /// only work with bits 0...minimum, so you need
    /// to grow the BitSet to the required size before
    /// accessing any bits in there!
    ///
    /// @exception GenericException
    void grow(size_t minimum);
    
    /// Set bit to 1
    ///
    /// @exception GenericException
    void set(size_t bit);

    /// Clear bit, i.e. set to 0.
    ///
    /// @exception GenericException
    void clear(size_t bit);

    /// Set bit to 0 or 1
    ///
    /// @exception GenericException
    void set(size_t bit, bool value);

    /// Check individual bit.
    ///
    /// @exception GenericException
    bool test(size_t bit) const;

    /// Whole bitset empty?
    bool empty() const;

    /// Any bit set?
    bool any() const;

    /// Number of bits set
    size_t count() const;

    /// Number of bits in Bitset
    size_t capacity() const
    { return size; }

    /// Return 001000.... type of string
    stdString to_string() const;

private:
    PROHIBIT_DEFAULT_COPY(BitSet);

    uint32_t *bits;
    size_t num;   // # of W32 that _bits points to
    size_t size;  // bits used
};

#endif //__BITSET_H__

