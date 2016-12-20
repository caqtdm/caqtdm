// -*- c++ -*-
#include "ToolsConfig.h"

#ifdef CONVERSION_REQUIRED
// System
#include<stdint.h>

// EPICS
#include<osiSock.h>
#include<db_access.h>

inline void ULONGFromDisk(uint32_t &item)
{    item = ntohl (item);    }

inline void ULONGToDisk(uint32_t &item)
{    item = htonl (item);    }

inline void USHORTFromDisk(uint16_t &item)
{    item = ntohs (item);    }

inline void USHORTToDisk(uint16_t &item)
{
    uint16_t big_endian;
    uint8_t *p = (uint8_t *)&big_endian;
    p[0] = item >> 8;
    p[1] = item & 0xFF;
    item = big_endian;
}

inline void DoubleFromDisk(double &d)
{
    uint32_t  cvrt_tmp = ntohl(((uint32_t *)&d)[0]);
    ((uint32_t *)&d)[0] = ntohl(((uint32_t *)&d)[1]);
    ((uint32_t *)&d)[1] = cvrt_tmp;
}

inline void DoubleToDisk(double &d)
{
    uint32_t  cvrt_tmp = htonl(((uint32_t *)&d)[0]);
    ((uint32_t *)&d)[0] = htonl(((uint32_t *)&d)[1]);
    ((uint32_t *)&d)[1] = cvrt_tmp;
}

inline void FloatFromDisk(float &d)
{   *((uint32_t *)&d) = ntohl(*((uint32_t *)&d)); }

inline void FloatToDisk(float &d)
{   *((uint32_t *)&d) = htonl(*((uint32_t *)&d)); }

inline void epicsTimeStampFromDisk(epicsTimeStamp &ts)
{
    ts.secPastEpoch = ntohl(ts.secPastEpoch);
    ts.nsec = ntohl(ts.nsec);
}

inline void epicsTimeStampToDisk(epicsTimeStamp &ts)
{
    ts.secPastEpoch = ntohl(ts.secPastEpoch);
    ts.nsec = ntohl(ts.nsec);
}

#define SHORTFromDisk(s)    USHORTFromDisk((uint16_t &)s)
#define SHORTToDisk(s)        USHORTToDisk((uint16_t &)s)
#define LONGFromDisk(l)        ULONGFromDisk((uint32_t &)l)
#define LONGToDisk(l)        ULONGToDisk((uint32_t &)l)

#else

#define ULONGFromDisk(s) {}
#define ULONGToDisk(i)   {}
#define USHORTFromDisk(i) {}
#define USHORTToDisk(i)   {}
#define DoubleFromDisk(i) {}
#define DoubleToDisk(i)   {}
#define FloatFromDisk(i)   {}
#define FloatToDisk(i)   {}
#define epicsTimeStampFromDisk(i) {}
#define epicsTimeStampToDisk(i) {}
#define SHORTFromDisk(i) {}
#define SHORTToDisk(i) {}
#define LONGFromDisk(i) {}
#define LONGToDisk(i) {}

#endif // CONVERSION_REQUIRED

#define FileOffsetFromDisk ULONGFromDisk
#define FileOffsetToDisk ULONGToDisk

