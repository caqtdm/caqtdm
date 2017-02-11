// LockFile.h: interface for the LockFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_LOCKFILE_H_)
#define _LOCKFILE_H_

#include <ToolsConfig.h>

/// \ingroup Tools
/// Lock file.
///
/// Generate a lock file containing the generation time.
/// Meant as a portable way to prevent multiple instances
/// of a program to run, also gives evidence of a non-graceful
/// exit.
class Lockfile  
{
public:
    /// Try to generate lock file.
    /// @exception GenericException
    Lockfile(const char *filename, const char *program);

    /// Remove lockfile.
    ~Lockfile();

private:
    stdString filename;
};

#endif
