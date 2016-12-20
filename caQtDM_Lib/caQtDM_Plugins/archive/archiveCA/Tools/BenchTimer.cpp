// -*- c++ -*-

// System
#include <stdio.h>
// Tools
#include "BenchTimer.h"

stdString BenchTimer::toString()
{
    double t = runtime();
    char buf[100];

    if (t < 1.0)
        sprintf(buf, "%.3f ms", t*1000.0);
    else if (t < 60.0)
        sprintf(buf, "%.3f s", t);
    else if (t < 60.0*60.0)
    {
        int m = (int)(t/60.0);
        t -= m*60.0;
        sprintf(buf, "%d min, %.3f s", m, t);
    }
    else
    {
        int h = (int)(t/(60.0*60.0));
        t -= h*60.0*60.0;
        int m = (int)(t/60.0);
        t -= m*60.0;
        sprintf(buf, "%02d:%02d:%02.3f", h, m, t);
    }
    return stdString(buf);
}
