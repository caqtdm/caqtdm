// LockFile.cpp: implementation of the LockFile class.

// System
#include <sys/types.h>
#include <unistd.h>

// Tools
#include "Lockfile.h"
#include "AutoFilePtr.h"
#include "epicsTimeHelper.h"
#include "GenericException.h"

Lockfile::Lockfile(const char *filename, const char *program)
    : filename(filename)
{
    // Check for existing file
    {
        AutoFilePtr f(this->filename.c_str(), "rt");
        if (f)
        {
            char line[80];
            line[0] = '\0';
            fgets(line, sizeof (line), f);
            throw GenericException(__FILE__, __LINE__,
                                   "Found an existing lock file '%s' starting with\n\"%s ...\n",
                                   this->filename.c_str(), line);
        }
    }
    AutoFilePtr f(this->filename.c_str(), "wt");
    if (!f)
    {
        throw GenericException(__FILE__, __LINE__,
                               "cannot open lock file '%s'\n", this->filename.c_str());
    }
    stdString t;
    epicsTime2string(epicsTime::getCurrent(), t);
    fprintf(f, "%s started on %s\n\n", program, t.c_str());
    fprintf(f, "If you can read this, the program is still running\n");
    fprintf(f, "or was forced to exit without being able to clean all resources.\n\n");
    fprintf(f, "The original process ID was %lu\n", (unsigned long) getpid());
}

Lockfile::~Lockfile()
{
    remove(filename.c_str());
}

