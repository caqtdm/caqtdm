// System
#include <stdlib.h>
// Index/Tools
#include "BinIO.h"

bool writeLong(FILE *f, uint32_t value)
{
    uint8_t c[4];
    c[0] = (uint8_t) (value >> 24) & 0xFF;
    c[1] = (uint8_t) (value >> 16) & 0xFF;
    c[2] = (uint8_t) (value >>  8) & 0xFF;
    c[3] = (uint8_t) (value & 0xFF);
    return fwrite(c, 4, 1, f) == 1;
}

bool readLong(FILE *f, uint32_t *value)
{
    uint8_t c[4];
    if (fread(c, 4, 1, f) != 1)
        return false;
    *value =
        ((uint32_t)c[0]) << 24 |
        ((uint32_t)c[1]) << 16 |
        ((uint32_t)c[2]) <<  8 |
         (uint32_t)c[3];
    return true;
}

bool writeShort(FILE *f, uint16_t value)
{
    uint8_t c[2];
    c[0] = (uint8_t) (value >>  8) & 0xFF;
    c[1] = (uint8_t) (value & 0xFF);
    return fwrite(c, 2, 1, f) == 1;
}

bool readShort(FILE *f, uint16_t *value)
{
    uint8_t c[2];
    if (fread(c, 2, 1, f) != 1)
        return false;
    *value =
        ((uint16_t)c[0]) <<  8 |
         (uint16_t)c[1];
    return true;
};


