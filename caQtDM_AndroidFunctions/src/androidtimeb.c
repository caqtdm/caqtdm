#include <androidtimeb.h>


int ftime(struct timeb *tb) {
        struct timeval  tv;
        struct timezone tz;

        if (gettimeofday (&tv, &tz) < 0) return -1;

        tb->time    = tv.tv_sec;
        tb->millitm = (tv.tv_usec + 500) / 1000;

        if (tb->millitm == 1000) {
             ++tb->time;
             tb->millitm = 0;
        }
        tb->timezone = tz.tz_minuteswest;
        tb->dstflag  = tz.tz_dsttime;

        return 0;
    }





