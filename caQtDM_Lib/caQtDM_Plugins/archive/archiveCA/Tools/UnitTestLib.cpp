
// System
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
// Tools
#include "GenericException.h"

bool test_delete_file(const char *filename)
{
    if (unlink(filename) == 0)
        return true; // File is now gone.
    int e = errno;
    if (e == ENOENT) // File didn't exist, same difference.
        return true;
    // Some other error.
    return false;
}

bool test_filediff(const char *filename1, const char *filename2)
{
    char diff_cmd[200];
    int len = snprintf(diff_cmd, sizeof(diff_cmd),
                       "diff '%s' '%s'", filename1, filename2);
    if (len < 0  ||  len >= (int)sizeof(diff_cmd))
        throw GenericException(__FILE__, __LINE__,
                               "snprintf returned len %d", len);
    int result = system(diff_cmd);
    return result == 0;
}


