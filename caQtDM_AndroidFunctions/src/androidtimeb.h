#ifndef _SYS_TIMEB_H
#define _SYS_TIMEB_H        1

#include <time.h>

struct timeb {
        time_t time;                    /* Seconds since epoch, as from `time'.  */
        unsigned short int millitm;     /* Additional milliseconds.  */
        short int timezone;             /* Minutes west of GMT.  */
        short int dstflag;              /* Nonzero if Daylight Savings Time used.  */
      };

#ifdef __cplusplus
extern "C" {
#endif

    int ftime(struct timeb *tb);

#ifdef __cplusplus
}
#endif

#endif




