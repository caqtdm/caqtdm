#ifndef __STRING_TO_CP__
#define __STRING_TO_CP__

#include <ToolsConfig.h>
#include <MsgLogger.h>

// Safely (i.e. w/o overruns and '\0'-limited)
// copy a std-string into a char [].
//
// string::copy isn't available on all platforms, so strncpy is used.
inline void string2cp(char *dest, const stdString &src, size_t maxlen)
{
    if (src.length() >= maxlen)
    {
        LOG_MSG("string2cp: Truncating '%s' to %zu chars.\n",
                src.c_str(), maxlen);
        strncpy(dest, src.c_str(), maxlen);
        dest[maxlen-1] = '\0';
    }
    else
        strncpy(dest, src.c_str(), maxlen);
}

#endif
