
#include "ToolsConfig.h"
#include "epicsTimeHelper.h"
#include "UnitTest.h"

TEST_CASE test_time()
{
    struct local_tm_nano_sec tm;
    tm.ansi_tm.tm_year = 2003 - 1900;
    tm.ansi_tm.tm_mon  = 4 - 1;
    tm.ansi_tm.tm_mday = 10;
    tm.ansi_tm.tm_hour = 11;
    tm.ansi_tm.tm_min  = 13;
    tm.ansi_tm.tm_sec  = 14;
    tm.ansi_tm.tm_isdst   = -1;
    tm.nSec = 987000000;
    epicsTime time = tm;
    time.show(10);

    stdString text("04/10/2003 11:13:14.987000000");
    epicsTime parsed;
    TEST(string2epicsTime(text, parsed));
    TEST(parsed == time);

    text = "04/10/2003 11:13:14.987";
    TEST(string2epicsTime(text, parsed));
    TEST(parsed == time);

    text = "04/10/2003 11:13:14";
    TEST(string2epicsTime(text, parsed));
    TEST(parsed != time);

    tm.nSec = 0;
    time = tm;
    TEST(parsed == time);

    text = "04/10/2003 11:13";
    TEST(string2epicsTime(text, parsed));
    TEST(parsed != time);

    tm.ansi_tm.tm_sec = 0;
    time = tm;
    TEST(parsed == time);

    text = "04/10/2003";
    TEST(string2epicsTime(text, parsed));
    TEST(parsed != time);

    tm.ansi_tm.tm_hour = 0;
    tm.ansi_tm.tm_min  = 0;
    time = tm;
    TEST(parsed == time);

    epicsTimeStamp stamp;
    stamp.secPastEpoch = 10000;
    stamp.nsec = 1000000000;
    try
    {
        time = stamp;
        FAIL("Could assign nsecs > 1e9");
    }
    catch (std::exception &e)
    {
        printf("  OK  : caught '%s'\n", e.what());
    }


    // At Wed Apr  9 10:43:37 MDT 2003 (daylight saving on),
    //  Win32 adds 1hour...
    // Convert 03/18/1990 12:13:44.800000019L back and forth:
    tm.ansi_tm.tm_year = 1990 - 1900;
    tm.ansi_tm.tm_mon  = 3 - 1;
    tm.ansi_tm.tm_mday = 18;
    tm.ansi_tm.tm_hour = 12;
    tm.ansi_tm.tm_min  = 13;
    tm.ansi_tm.tm_sec  = 44;
    tm.ansi_tm.tm_isdst   = -1;
    tm.nSec = 800000019L;

    // to epicsTime
    time = tm;

    // back to tm
    tm = time;
    const char *dst;
    switch (tm.ansi_tm.tm_isdst)
    {
        case 0:
            dst = "standard";
            break;
        case 1:
            dst = "daylight saving";
            break;
        default:
            dst = "unknown";
    }
    printf("%02d/%02d/%04d %02d:%02d:%02d.%09ld (%s)\n",
           tm.ansi_tm.tm_mon + 1,
           tm.ansi_tm.tm_mday,
           tm.ansi_tm.tm_year + 1900,
           tm.ansi_tm.tm_hour,
           tm.ansi_tm.tm_min,
           tm.ansi_tm.tm_sec,
           tm.nSec,
           dst);


    epicsTime t;
    TEST(isValidTime(t) == false);
    
    t = epicsTime::getCurrent();
    epicsTime start = t;
    TEST(isValidTime(t) == true);
    
    memset(&stamp, 0, sizeof(epicsTimeStamp));
    t = stamp;
    TEST(isValidTime(t) == false);
    
    epicsTime now = epicsTime::getCurrent();
    stdString s_start, s_now;
    epicsTime2string(start, s_start);
    epicsTime2string(now, s_now);
    //printf("Start: %s\n", s_start.c_str());
    //printf("Now:   %s\n", s_now.c_str());
    TEST(start <= now);
    TEST(now > start);
    t = now;
    TEST(t == now);

    int year, month, day, hour, min, sec;
    unsigned long nano;
    epicsTime2vals(start, year, month, day, hour, min, sec, nano);
    char buf[50];
    sprintf(buf, "%02d/%02d/%04d %02d:%02d:%02d.%09lu",
            month, day, year, hour, min, sec, nano);
    puts(buf);
    TEST(strcmp(s_start.c_str(), buf) == 0);

    vals2epicsTime(year, month, day, hour, min, sec, nano, t);
    TEST(t == start);

    t += 60;
    TEST(t != start);
    TEST(t > start);
    TEST(t - start == 60.0);

    stdString txt;
    vals2epicsTime(1990, 3, 18, 12, 13, 44, 800000019L, now);
    epicsTime2string(now, txt); puts(txt.c_str());           TEST(txt == "03/18/1990 12:13:44.800000019");
    epicsTime2string(roundTimeDown(now, 0), txt);            TEST(txt == "03/18/1990 12:13:44.800000019");
    epicsTime2string(roundTimeDown(now, 0.5), txt);          TEST(txt == "03/18/1990 12:13:44.500000000");
    epicsTime2string(roundTimeDown(now, 1.0), txt);          TEST(txt == "03/18/1990 12:13:44.000000000");
    epicsTime2string(roundTimeDown(now, 10.0), txt);         TEST(txt == "03/18/1990 12:13:40.000000000");
    epicsTime2string(roundTimeDown(now, 30.0), txt);         TEST(txt == "03/18/1990 12:13:30.000000000");

    epicsTime2string(roundTimeDown(now, 50.0), txt);
    printf("Rounded by 50 secs: %s\n", txt.c_str());

    epicsTime2string(roundTimeDown(now, 60.0), txt);         TEST(txt == "03/18/1990 12:13:00.000000000");
    epicsTime2string(roundTimeDown(now, secsPerHour), txt);  TEST(txt == "03/18/1990 12:00:00.000000000");
    epicsTime2string(roundTimeDown(now, secsPerDay), txt);   TEST(txt == "03/18/1990 00:00:00.000000000");
    epicsTime2string(roundTimeDown(now, secsPerMonth), txt); TEST(txt == "03/01/1990 00:00:00.000000000");
    epicsTime2string(roundTimeDown(now, secsPerYear), txt);  TEST(txt == "01/01/1990 00:00:00.000000000");

    vals2epicsTime(1990, 3, 18, 12, 13, 44, 800000019L, now);
    epicsTime2string(roundTimeUp(now, 0), txt);            TEST(txt == "03/18/1990 12:13:44.800000019");
    epicsTime2string(roundTimeUp(now, 0.1), txt);          TEST(txt == "03/18/1990 12:13:44.900000000");    
    epicsTime2string(roundTimeUp(now, 0.5), txt);          TEST(txt == "03/18/1990 12:13:45.000000000");    
    epicsTime2string(roundTimeUp(now, 1.0), txt);          TEST(txt == "03/18/1990 12:13:45.000000000");
    epicsTime2string(roundTimeUp(now, 10.0), txt);         TEST(txt == "03/18/1990 12:13:50.000000000");
    epicsTime2string(roundTimeUp(now, 30.0), txt);         TEST(txt == "03/18/1990 12:14:00.000000000");
    
    epicsTime2string(roundTimeUp(now, 50.0), txt);
    printf("Rounded by 50 secs: %s\n", txt.c_str());
    
    epicsTime2string(roundTimeUp(now, 60.0), txt);         TEST(txt == "03/18/1990 12:14:00.000000000");
    epicsTime2string(roundTimeUp(now, secsPerHour), txt);  TEST(txt == "03/18/1990 13:00:00.000000000");
    
    epicsTime2string(roundTimeUp(now, secsPerDay), txt);   TEST(txt == "03/19/1990 00:00:00.000000000");
    epicsTime2string(roundTimeUp(now, secsPerMonth), txt); TEST(txt == "04/01/1990 00:00:00.000000000");
    epicsTime2string(roundTimeUp(now, secsPerYear), txt);  TEST(txt == "01/01/1991 00:00:00.000000000");
    
    vals2epicsTime(2000, 12, 31, 23, 59, 59, 999999999L, now);
    epicsTime2string(roundTimeDown(now, 0), txt);            TEST(txt == "12/31/2000 23:59:59.999999999");
    epicsTime2string(roundTimeDown(now, secsPerDay), txt);   TEST(txt == "12/31/2000 00:00:00.000000000");
    epicsTime2string(roundTimeDown(now, secsPerYear), txt);  TEST(txt == "01/01/2000 00:00:00.000000000");

    vals2epicsTime(1990, 3, 18, 23, 13, 44, 800000019L, now);
    epicsTime2string(roundTimeUp(now, 60*60*1), txt);        TEST(txt == "03/19/1990 00:00:00.000000000");
    epicsTime2string(roundTimeUp(now, 60*60*2), txt);        TEST(txt == "03/19/1990 01:00:00.000000000");

    vals2epicsTime(1990, 3, 18, 01, 13, 44, 800000019L, now);
    epicsTime2string(roundTimeUp(now, 60*60*1), txt);        TEST(txt == "03/18/1990 02:00:00.000000000");

    printf("Rounding up by 15 minutes:\n");
    vals2epicsTime(1990, 3, 18, 01, 13, 44, 800000019L, now);
    now = roundTimeUp(now, 900);    printf("%s\n", epicsTimeTxt(now, txt));
    now = roundTimeUp(now, 900);    printf("%s\n", epicsTimeTxt(now, txt));
    now = roundTimeUp(now, 900);    printf("%s\n", epicsTimeTxt(now, txt));
    now = roundTimeUp(now, 900);    printf("%s\n", epicsTimeTxt(now, txt));
    now = roundTimeUp(now, 900);    printf("%s\n", epicsTimeTxt(now, txt));
    now = roundTimeUp(now, 900);    printf("%s\n", epicsTimeTxt(now, txt));
    TEST_OK;
}

