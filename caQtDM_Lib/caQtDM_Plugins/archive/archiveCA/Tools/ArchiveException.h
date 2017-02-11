// -*- c++ -*-

#ifndef __ARCHIVEEXCEPTION_H__
#define __ARCHIVEEXCEPTION_H__

// System
#include <stdio.h>
// Tools
#include <GenericException.h>

// \ingroup Tools

/// Exception used by older LibIO code.
class ArchiveException : public GenericException
{
public:
    typedef enum
    {
        NoError,    // This should never occur...
        Fail,        // Failure
        Invalid,    // Invalid: not initialized, wrong type, ...
        OpenError,    // Cannot open existing new file
        CreateError,// Cannot create new file
        ReadError,    // Read Error
        WriteError,    // Write Error
        Unsupported // Not Supported (dbr type,...)
    } Code;

    ArchiveException (const char *sourcefile, size_t line, Code code);

    ArchiveException(const char *sourcefile, size_t line,
                     Code code, const stdString &detail);

    ~ArchiveException() throw () 
    {}
    
    Code getErrorCode () const
    {    return code;   }
    
private:
    Code code;
};

// inline? Then e.g. Invalid would have to be given as
// ChannelArchiveException::Invalid...
#define throwArchiveException(code)    \
   throw ArchiveException(__FILE__, __LINE__,ArchiveException::code)
#define throwDetailedArchiveException(code,detail) \
   throw ArchiveException(__FILE__, __LINE__,ArchiveException::code, detail)

#endif //__ARCHIVEEXCEPTION_H__
