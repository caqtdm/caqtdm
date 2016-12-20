// epicsTimeHelper

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "epicsTimeHelper.h"
#include "tsDefs.h"
#include "MsgLogger.h"

// Because of unknown differences in the epoch definition,
// the next two might be different!
const epicsTime nullTime; // uninitialized (=0) class epicsTime
static epicsTime nullStamp;      // epicsTime with epicsTimeStamp 0, 0

// Needs to be called for initialization
class initEpicsTimeHelper
{
public:
    initEpicsTimeHelper()
    {
        epicsTimeStamp  stamp;
        stamp.secPastEpoch = 0;
        stamp.nsec = 0;
        nullStamp = stamp;
    }
};

// Check if time is non-zero, whatever that could be
bool isValidTime(const epicsTime &t)
{
    return t != nullTime  &&  t != nullStamp;
}

// Convert string "mm/dd/yyyy" or "mm/dd/yyyy 00:00:00" or
// "mm/dd/yyyy 00:00:00.000000000" into epicsTime
// Result: true for OK
bool string2epicsTime(const stdString &txt, epicsTime &time)
{
    size_t i, tlen = txt.length();
    // number check ("ab/cd/efgh" is caught here)
    for (i=0; i<tlen; ++i)
        if (!strchr("/0123456789 :.", txt[i]))
            return false;
    //  0123456789
    // "mm/dd/yyyy" ?
    if (tlen < 10  ||  txt[2] != '/' || txt[5] != '/')
    {   // Special case for testing
        epicsTimeStamp stamp;
        stamp.secPastEpoch = atoi(txt.c_str());
        stamp.nsec = 0;
        time = stamp;
        return stamp.secPastEpoch <= 1000;
    }
    struct local_tm_nano_sec tm;
    memset(&tm, 0, sizeof(struct local_tm_nano_sec));
    tm.ansi_tm.tm_isdst = -1; /* don't know if daylight saving or not */
    tm.ansi_tm.tm_mon  = (txt[0]-'0')*10 + (txt[1]-'0') - 1;
    tm.ansi_tm.tm_mday = (txt[3]-'0')*10 + (txt[4]-'0');
    tm.ansi_tm.tm_year = (txt[6]-'0')*1000 + (txt[7]-'0')*100 +
                         (txt[8]-'0')*10 + (txt[9]-'0') - 1900;

    //  0123456789012345
    // "mm/dd/yyyy ..." - check the space after the date.
    if (tlen > 10 && txt[10] != ' ')
        return false;
    //  0123456789012345
    // "mm/dd/yyyy 00:00" - check the ':' in the time.
    if (tlen >= 16)
    {
        if (txt[13] != ':')
            return false;
        tm.ansi_tm.tm_hour = (txt[11]-'0')*10 + (txt[12]-'0');
        tm.ansi_tm.tm_min  = (txt[14]-'0')*10 + (txt[15]-'0');
    }
    //  0123456789012345678
    // "mm/dd/yyyy 00:00:00" - are there seconds?
    if (tlen >= 19)
    {
        if (txt[16] != ':')
            return false;
        tm.ansi_tm.tm_sec  = (txt[17]-'0')*10 + (txt[18]-'0');
    }
    //  01234567890123456789012345678
    // "mm/dd/yyyy 00:00:00.000" - are there milli but not nano seconds?
    if (tlen == 23 && txt[19] == '.')
        tm.nSec =
            (txt[20]-'0')*100000000 +
            (txt[21]-'0')*10000000 +
            (txt[22]-'0')*1000000;
    //  01234567890123456789012345678
    // "mm/dd/yyyy 00:00:00.000000000" - are there nano seconds?
    if (tlen == 29 && txt[19] == '.')
        tm.nSec =
            (txt[20]-'0')*100000000 +
            (txt[21]-'0')*10000000 +
            (txt[22]-'0')*1000000 +
            (txt[23]-'0')*100000 +
            (txt[24]-'0')*10000 +
            (txt[25]-'0')*1000 +
            (txt[26]-'0')*100 +
            (txt[27]-'0')*10 +
            (txt[28]-'0');   
    
    time = tm;
    return true;
}

// Convert epicsTime into "mm/dd/yyyy 00:00:00.000000000"
bool epicsTime2string (const epicsTime &time, stdString &txt)
{
    char buffer[50];
    epicsTimeStamp stamp = time;
    if (stamp.nsec == 0 && stamp.secPastEpoch <= 1000)
    {
        sprintf(buffer, "%d", stamp.secPastEpoch);
        txt = buffer;
        return true;
    }
    if (! isValidTime(time))
    {
        txt = "00:00:00";
        return false;
    }
    struct local_tm_nano_sec tm = (local_tm_nano_sec) time;
    sprintf(buffer, "%02d/%02d/%04d %02d:%02d:%02d.%09ld",
            tm.ansi_tm.tm_mon + 1,
            tm.ansi_tm.tm_mday,
            tm.ansi_tm.tm_year + 1900,
            tm.ansi_tm.tm_hour,
            tm.ansi_tm.tm_min,
            tm.ansi_tm.tm_sec,
            tm.nSec);
    txt = buffer;
    return true;
}

const char *epicsTimeTxt(const epicsTime &time, stdString &txt)
{
    return epicsTime2string(time, txt) ? txt.c_str() : "invalid";
}

void epicsTime2vals(const epicsTime &time,
                    int &year, int &month, int &day,
                    int &hour, int &min, int &sec, unsigned long &nano)
{
    if (isValidTime(time))
    {
        struct local_tm_nano_sec tm = (local_tm_nano_sec) time;
        year  = tm.ansi_tm.tm_year + 1900;
        month = tm.ansi_tm.tm_mon + 1;
        day   = tm.ansi_tm.tm_mday;
        hour  = tm.ansi_tm.tm_hour;
        min   = tm.ansi_tm.tm_min;
        sec   = tm.ansi_tm.tm_sec;
        nano  = tm.nSec;
    }
    else
    {
       year = month = day = hour = min = sec = 0;
       nano = 0;
    }
}

void vals2epicsTime(int year, int month, int day,
                    int hour, int min, int sec, unsigned long nano,
                    epicsTime &time)
{
    struct local_tm_nano_sec tm;
    tm.ansi_tm.tm_year = year - 1900;
    tm.ansi_tm.tm_mon  = month - 1;
    tm.ansi_tm.tm_mday = day;
    tm.ansi_tm.tm_hour = hour;
    tm.ansi_tm.tm_min  = min;
    tm.ansi_tm.tm_sec  = sec;
    tm.ansi_tm.tm_isdst   = -1;
    tm.nSec = nano;
    time = tm;
}

// Round down
#define NSEC 1000000000L
epicsTime roundTimeDown(const epicsTime &time, double secs)
{
    if (secs <= 0.0)
        return time;
    struct local_tm_nano_sec tm;
    unsigned long round, full_secs;
    if (secs < 1.0)
    {
        epicsTimeStamp stamp = (epicsTimeStamp)time;
        round =(unsigned long)(secs * NSEC);
        stamp.nsec = (stamp.nsec / round)*round;
        return epicsTime(stamp);
    }
    else if (secs < secsPerDay)
    {
    full_secs = (unsigned long)secs;
        epicsTimeStamp stamp = (epicsTimeStamp)time;
        // secs >= 1.0, so nanosecs are 0
        round = stamp.secPastEpoch / full_secs;
        stamp.secPastEpoch = (epicsUInt32) (round*secs);
        stamp.nsec = 0;
        return epicsTime(stamp);
    }
    else if (secs < secsPerMonth)
    {
    full_secs = (unsigned long)secs;
        tm = (local_tm_nano_sec) time;
        round = full_secs/secsPerDay;
        tm.nSec = 0;
        tm.ansi_tm.tm_sec = 0;
        tm.ansi_tm.tm_min = 0;
        tm.ansi_tm.tm_hour = 0;
        tm.ansi_tm.tm_mday = (tm.ansi_tm.tm_mday / round) * round;
    }
    else if (secs < secsPerYear)
    {
    full_secs = (unsigned long)secs;
        tm = (local_tm_nano_sec) time;
        round = full_secs/secsPerMonth;
        tm.nSec = 0;
        tm.ansi_tm.tm_sec = 0;
        tm.ansi_tm.tm_min = 0;
        tm.ansi_tm.tm_hour = 0;
        tm.ansi_tm.tm_mday = 1;
        tm.ansi_tm.tm_mon = (tm.ansi_tm.tm_mon / round) * round;
    }
    else
    {
        tm = (local_tm_nano_sec) time;
        tm.nSec = 0;
        tm.ansi_tm.tm_sec = 0;
        tm.ansi_tm.tm_min = 0;
        tm.ansi_tm.tm_hour = 0;
        tm.ansi_tm.tm_mday = 1;
        tm.ansi_tm.tm_mon = 0;
    }
    // TODO: round weeks, fortnights?
    return epicsTime(tm);
}

// Round up
epicsTime roundTimeUp(const epicsTime &time, double secs)
{
    if (secs <= 0.0)
        return time;
    struct local_tm_nano_sec tm;
    unsigned long round, full_secs;
    if (secs < 1.0)
    {
        epicsTimeStamp stamp = (epicsTimeStamp)time;
        round =(unsigned long)(secs * NSEC);
        stamp.nsec =
            ((stamp.nsec / round) + 1)*round;
        while (stamp.nsec >= NSEC)
        {
            stamp.secPastEpoch += 1;
            stamp.nsec -= NSEC;
        }
        return epicsTime(stamp);
    }
    else if (secs < secsPerDay)
    {
    full_secs = (unsigned long)secs;
        epicsTimeStamp stamp = (epicsTimeStamp)time;
        // secs >= 1.0, so nanosecs are 0
        round = stamp.secPastEpoch / full_secs;
        stamp.secPastEpoch = (epicsUInt32) ((round+1)*full_secs);
        stamp.nsec = 0;
        return epicsTime(stamp);
    }
    else if (secs < secsPerMonth)
    {
    full_secs = (unsigned long)secs;
        tm = (local_tm_nano_sec) time;
        round = full_secs/secsPerDay;
        tm.nSec = 0;
        tm.ansi_tm.tm_sec = 0;
        tm.ansi_tm.tm_min = 0;
        tm.ansi_tm.tm_hour = 0;
        tm.ansi_tm.tm_mday = (tm.ansi_tm.tm_mday / round + 1) * round;
    }
    else if (secs < secsPerYear)
    {
    full_secs = (unsigned long)secs;
        tm = (local_tm_nano_sec) time;
        round = full_secs/secsPerMonth;
        tm.nSec = 0;
        tm.ansi_tm.tm_sec = 0;
        tm.ansi_tm.tm_min = 0;
        tm.ansi_tm.tm_hour = 0;
        tm.ansi_tm.tm_mday = 1;
        tm.ansi_tm.tm_mon = (tm.ansi_tm.tm_mon / round + 1) * round;
    }
    else
    {
        tm = (local_tm_nano_sec) time;
        tm.nSec = 0;
        tm.ansi_tm.tm_sec = 0;
        tm.ansi_tm.tm_min = 0;
        tm.ansi_tm.tm_hour = 0;
        tm.ansi_tm.tm_mday = 1;
        tm.ansi_tm.tm_mon = 0;
        tm.ansi_tm.tm_year += 1;
    }
    // TODO: round weeks, fortnights?
    return epicsTime(tm);
}

// Find timestamp near 'time' which is a multiple of 'secs'.
epicsTime roundTime(const epicsTime &time, double secs)
{
    epicsTime up = roundTimeUp(time, secs);
    epicsTime down = roundTimeDown(time, secs);
    if (time - down  <  up - time)
        return down;
    return up;
}

