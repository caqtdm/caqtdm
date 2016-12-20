// -*- c++ -*-
#ifndef __FILENAMETOOL_H__
#define __FILENAMETOOL_H__

#include <ToolsConfig.h>

/// \ingroup Tools
///
/// Basename, dirname and other file name related helpers.
/// 
/// On UNIX systems, the filenames are build using
/// slashes (/),
/// on WIN32 systems both slashes and backslashes (\)
/// are allowed for input.
/// The generated names are always build using slashes
/// since both the UNIX and WIN32 system routines
/// can handle those.
class Filename
{
public:
    /// Check if non-empty.
    static bool isValid(const stdString &name)
    {    return ! name.empty();    }    

    /// Check if non-empty.
    static bool isValid(const char *name)
    {    return name != 0  &&  name[0] != '\0';    }

    /// Build filename from dir. and basename.
    static void build(const stdString &dirname,
                      const stdString &basename,
                      stdString &filename);

    /// Returns true if filename contains a path/directory.
    ///
    /// Really only checks if the filename contains a path separator.
    static bool containsPath(const stdString &filename);

    /// Returns true if filename contains a full path/directory.
    ///
    /// Really only checks if the filename starts with a path separator.
    static bool containsFullPath(const stdString &filename);

    /// Get directory (path) from full path/filename.
    static void getDirname(const stdString &filename, stdString &dirname);

    /// Get basename from full filename.
    static void getBasename(const stdString &filename, stdString &basename);

    /// If filename is a link, get target of link. Else return false.
    ///
    /// Implementation detail:
    /// It is OK for filename and link to refer to the same stdString.
    static bool getLinkedFilename(const stdString &filename, stdString &link);
};

#endif //__FILENAMETOOL_H__









