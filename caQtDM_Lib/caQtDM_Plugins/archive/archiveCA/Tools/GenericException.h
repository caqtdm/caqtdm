// -*- c++ -*-

#if !defined(_GENERICEXCEPTION_H_)
#define _GENERICEXCEPTION_H_

#include <ToolsConfig.h>
#ifdef USE_STD_EXCEPTION
#include <exception>
#endif

#include <stdarg.h>

/// \ingroup Tools

/// Generic Exception: Base class for exceptions
///
/// An exception that provided info text with sourcefile & line information.
/// 
/// All exceptions should be thrown as an object/reference, not pointer,
/// and also caught by reference,
/// to avoid unnecessary copies and assert de-allocation:
/// <PRE>
///  try
///  {
///      ... throw GenericException(__FILE__, __LINE__, "Trouble at mill!");
///  }
///  catch (GenericException &e)
///  {
///      ... somehow print  e.what() ...
///  }
/// </PRE>
/// (According to Scott Meyers "More Effective C++",
///  a copy will be thrown, but Visual C++ seems to
///  efficiently throw without copying).
///
/// It's a good idea to print the exception information as given above
/// with the e.what() - string on a new line.
/// That line will then usually read "filename (line-#)"
/// and this format allows easy lookup from within IDEs.
/// The error string might include newlines and usually also ends in one.

class GenericException
#ifdef USE_STD_EXCEPTION
   : public std::exception
#endif
{
public:
    /// Construct with file and line info.
    GenericException(const char *sourcefile, size_t line)
        : sourcefile(sourcefile), line(line)
    {}

    /// Construct with file, line info and printf-type arguments.
    GenericException(const char *sourcefile, size_t line, const char *format, ...)
         __attribute__ ((format (printf, 4, 5)));

    // Strange but true, __attribute__ format seems to include
    // an initial 'this' argument in the position count.

    /// Virtual destructor to allow inheritance.
    virtual ~GenericException() throw () {}

    /// Retrieve an explanatory string.
    /// Default implementation will print source file and line.
    virtual const char *what() const throw ();

    /// Source file where exception was thrown.
    const char *getSourceFile() const
    {   return sourcefile; }

    /// Line in source file where exception was thrown.
    size_t getSourceLine() const
    {   return line;   }

    /// Return detail text (if set)
    const stdString &getDetail () const
    {   return detail; }

protected:
    /// Buffer for generating the file, line, detail string.
    /// Might be generated on demand in what() -> mutable.
    mutable stdString error_info;

    /// Possibly more detail on the error.
    mutable stdString detail;

    /// sprintf into stdString.
    /// @return The current error_info.c_str()
    const char *sprintf(stdString &s, const char *format, ...) const throw ();

    /// sprintf into stdString.
    /// @return The current error_info.c_str()    
    const char *sprintf(stdString &s, const char *format, va_list ap) const throw ()
              __attribute__ ((format (printf, 3, 0)));

private:
    const char  *sourcefile;
    size_t      line;
};

#endif // !defined(_GENERICEXCEPTION_H_)

