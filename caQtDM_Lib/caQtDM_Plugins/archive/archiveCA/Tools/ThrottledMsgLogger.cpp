
// System
#include <stdarg.h>
// Tools
#include "ThrottledMsgLogger.h"

void ThrottledMsgLogger::LOG_MSG(const char *format, ...)
{
    va_list ap;
    if (isPermitted())
    {
        va_start(ap, format);
        ::LOG_MSG(format, ap);
        va_end(ap);
        too_many = false;
    }
    else if (!too_many)
    {
        too_many = true;
        ::LOG_MSG("%s: More messages suppressed for %g seconds....\n",
                  name.c_str(), getThrottleThreshold());
    }
}

