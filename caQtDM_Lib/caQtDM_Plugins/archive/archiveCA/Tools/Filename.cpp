// Filename: Routines for handling file names,
// should support Unix and Win32
// kasemirk@ornl.gov

// System
#include <unistd.h>
// Tools
#include "Filename.h"
#include "GenericException.h"

static const char *current_dir = ".";

void Filename::build(const stdString &dirname, const stdString &basename,
                     stdString &filename)
{
    size_t dl = dirname.length();
    size_t bl = basename.length();
    if (bl == 0)
        throw GenericException(__FILE__, __LINE__,
                               "Filename::build called with empty basename");
    if (dl == 0  ||  dirname == current_dir)
    {
        filename = basename;
        return;
    }
    size_t new_len = dl + bl + 1;
    if (!filename.reserve(new_len))
        throw GenericException(__FILE__, __LINE__,
                               "Filename::build cannot alloc %zu",
                               new_len);
    filename = dirname;
    // Check if the dirname ends with a directory separator.
    if (dl > 0  &&
        dirname[dl-1] != '/'
#       ifdef WIN32
        &&
        dirname[dl-1] != '\\'
#       endif
       )
    {
#       ifdef WIN32
        if (dirname.find('\\') != stdString::npos)
            filename += '\\';
        else
#       endif    
            filename += '/';
    }
    filename += basename;
}

bool Filename::containsPath(const stdString &filename)
{
    if (filename.find('/') != stdString::npos)
        return true;
#ifdef WIN32
    if (filename.find('\\') != stdString::npos)
        return true;
#endif    
    return false;
}

bool Filename::containsFullPath(const stdString &filename)
{
    if (filename.length() < 1)
        return false;
    if (filename[0] == '/')
        return true;
#ifdef WIN32
    if (filename[0] == '\\')
        return true;
    if (filename.length() >= 3 &&
        filename[1] == ':' && filename[2] == '\\')
        return true;
#endif
    return false;
}

// Find the directory portion of given filename.
void Filename::getDirname(const stdString &filename, stdString &dirname)
{
    if (filename.empty())
    {
        dirname.assign((const char *)0, 0);
        return;
    } 
    stdString::size_type dir = filename.find_last_of('/');
#ifdef WIN32
    // For WIN32, both '/' and '\\' are possible:
    if (dir == filename.npos)
        dir = filename.find_last_of('\\');
#endif
    if (dir == filename.npos)
        dirname.assign((const char *)0, 0);
     else
        dirname = filename.substr(0, dir);
}                     

void Filename::getBasename(const stdString &filename, stdString &basename)
{
    if (filename.empty())
    {
        basename.assign((const char *)0, 0);
        return;
    }
    stdString::size_type base = filename.find_last_of('/');
#ifdef WIN32
    // For WIN32, both '/' and '\\' are possible:
    if (base == filename.npos)
        base = filename.find_last_of('\\');
#endif
    if (base != filename.npos)
    {
        basename = filename.substr(base+1);
        return;
    }
    basename = filename;
}
                            
bool Filename::getLinkedFilename(const stdString &filename, stdString &link)
{
    char buf[500];
/*    int len = readlink(filename.c_str(), buf, sizeof(buf));
    if (len <= 0)
        return false;
    link.assign(buf, len);
    return true; */
    return false;
}

