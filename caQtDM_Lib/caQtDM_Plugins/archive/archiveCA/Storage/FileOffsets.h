#ifndef __FILEOFFSETS_H__
#define __FILEOFFSETS_H__

#include "BinIO.h"
#include <stddef.h>

inline bool writeUint64(FILE *f, uint64_t value)
{
    uint8_t c[8];
    c[0] = (uint8_t) (value >> 56) & 0xFF;
    c[1] = (uint8_t) (value >> 48) & 0xFF;
    c[2] = (uint8_t) (value >> 40) & 0xFF;
    c[3] = (uint8_t) (value >> 32) & 0xFF;
    c[4] = (uint8_t) (value >> 24) & 0xFF;
    c[5] = (uint8_t) (value >> 16) & 0xFF;
    c[6] = (uint8_t) (value >>  8) & 0xFF;
    c[7] = (uint8_t) (value & 0xFF);
    return fwrite(c, 8, 1, f) == 1;
}

inline bool readUint64(FILE *f, uint64_t *value)
{
    uint8_t c[8];
    if (fread(c, 8, 1, f) != 1)
    {
        return false;
    }
    *value =
        ((uint64_t)c[0]) << 56 |
        ((uint64_t)c[1]) << 48 |
        ((uint64_t)c[2]) << 40 |
        ((uint64_t)c[3]) << 32 |
        ((uint64_t)c[4]) << 24 |
        ((uint64_t)c[5]) << 16 |
        ((uint64_t)c[6]) <<  8 |
         (uint64_t)c[7];
    return true;
}

#define IndexFileOffset uint64_t

inline bool ReadIndexFileOffset(FILE *f, uint64_t *value, int size)
{
    if(size == 32)
    {            
        uint32_t v;
        bool result = readLong(f, &v);
        *value = v;
        return result;
    }
    else
    {
        return readUint64(f, value);
    }
}

inline bool WriteIndexFileOffset(FILE *f, uint64_t value, int size)
{
    if(size == 32)
    {
        return writeLong(f, value);
    }
    else
    {
        return writeUint64(f, value);
    }
}

#endif
