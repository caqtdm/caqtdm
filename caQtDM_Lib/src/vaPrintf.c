/*
 *  This file is part of the caQtDM Framework, developed at the Paul Scherrer Institut,
 *  Villigen, Switzerland
 *
 *  The caQtDM Framework is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  The caQtDM Framework is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the caQtDM Framework.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright (c) 2010 - 2014
 *
 *  Author:
 *    Anton Mezger
 *  Contact details:
 *    anton.mezger@psi.ch
 */

#include "vaPrintf.h"

#if defined(_MSC_VER)
#define VAPRINTF_THREAD_LOCAL __declspec(thread)
#else
#define VAPRINTF_THREAD_LOCAL __thread
#endif

// bounded, thread-local buffer: callers run in CA callback threads
char* vaPrintf(const char *fmt, ...)
{
    static VAPRINTF_THREAD_LOCAL char errmsg[256] = {0};
    va_list     alist;
    int         status;

    va_start(alist, fmt);
    status = vsnprintf(errmsg, sizeof(errmsg), fmt, alist);
    if (status < 0) {
        va_end(alist);
        return (char*) 0;
    }
    va_end(alist);
    return errmsg;
}
