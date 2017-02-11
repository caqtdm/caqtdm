// -*- c++ -*-
#ifndef __STORAGE_TYPES_H
#define __STORAGE_TYPES_H

#include <stdint.h>

/// \addtogroup Storage
/// @{
///

/// FileOffset is used as a system independent
/// type for, well, offsets into files.
///
typedef uint32_t FileOffset;

// Used internally for offsets inside files:
const FileOffset INVALID_OFFSET = 0xffffffff;

/// @}
///
///

#endif
