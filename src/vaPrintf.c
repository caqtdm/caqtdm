#include "vaPrintf.h"

char* vaPrintf(const char *fmt, ...)
{
    static char errmsg[256] = {0};
    va_list     alist;
    int         status;

    va_start(alist, fmt);
    status = vsprintf(errmsg, fmt, alist);
    if (status == EOF) return (char*) 0;
    va_end(alist);
    return errmsg;
}
