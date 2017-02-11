#ifndef BIN_IO_H
#define BIN_IO_H

#include <stdio.h>
#include <stdint.h>

/// \addtogroup Tools
/// @{

/// OS-independent binary I/O.

/// All values are written with the most significant
/// byte first, e.g. 0x12345678 is written as
/// 0x12, 0x34, ... 0x78.

/// Write 'value' to file with fixed byte order
bool writeLong(FILE *f, uint32_t value);

/// Read 'value' from file with fixed byte order
bool readLong(FILE *f, uint32_t *value);

/// Write 'value' to file with fixed byte order
bool writeShort(FILE *f, uint16_t value);

/// Read 'value' from file with fixed byte order
bool readShort(FILE *f, uint16_t *value);

inline bool writeByte(FILE *f, uint8_t byte)
{   return fwrite(&byte, 1, 1, f) == 1; }

inline bool readByte(FILE *f, uint8_t *byte)
{   return fread(byte, 1, 1, f) == 1; }

/// @}

#endif
// BIN_IO_H


