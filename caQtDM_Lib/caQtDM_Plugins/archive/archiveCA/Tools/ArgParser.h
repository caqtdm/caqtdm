// -------------- -*- c++ -*-
// $Id: ArgParser.h,v 1.8 2012/07/04 09:08:46 jud Exp $
//
// Please refer to NOTICE.txt,
// included as part of this distribution,
// for legal information.
//
// Kay-Uwe Kasemir, kasemir@lanl.gov
// --------------------------------------------------------

#ifndef __ARGPARSER_H__
#define __ARGPARSER_H__

#include <ToolsConfig.h>
#include <NoCopy.h>

/// \addtogroup Tools
/// @{

class CmdArg;

/// General purpose argument parser for
/// command-line programs.
///
/// Example usage:
/// \code
/// int main(int argc, const char *argv[])
/// { 
///     CmdArgParser parser(argc, argv);
///     parser.setHeader("ExampleProgram");
///     parser.setArgumentsInfo("<filename>");
///     CmdArgFlag debug(parser, "debug", "Enable debugging");
///     CmdArgInt  limit(parser, "limit", "<secs>", "Time limit in seconds");
///     CmdArgString text(parser, "text", "<text>", "A string argument");
///     limit.set(5); // default
///     if (parser.parse() == false)
///         return -1;
///     if (parser.getArguments().size() != 1)
///     {
///         parser.usage();
///         return -1;
///     }
///     stdString filename = parser.getArgument(0);
///     if (debug)
///         ...
///     int time_limit =  limit.get();
/// \endcode   
///
class CmdArgParser
{
public:
    /// Initialize CmdArgParser with main's argc/argv.
    CmdArgParser(int argc, char const * const *argv);

    void addOption(CmdArg *option);

    /// Print out usage based on all registered CmdArgxxx classes.
    void usage();

    /// Set additional header text (default: nothing).
    void setHeader(const char *header);

    /// Set additional footer text (default: nothing).
    void setFooter(const char *footer);

    /// Description for remaining arguments (other than options)
    void setArgumentsInfo(const char *args_info);

    /// Parse Arguments,
    /// \return false on error
    bool parse();

    /// Remaining arguments (all that didn't start with '-')
    const stdVector<const char *> &getArguments();

    /// Get a single one of the remaining arguments
    const char *getArgument(size_t i);

private:
    PROHIBIT_DEFAULT_COPY(CmdArgParser);
    const char *_header;
    const char *_footer;
    const char *_args_info;
    const char *_progname;        // Name of Program
    int _argc;                    // Original argc/argv minus _progname
    char const * const *_argv;
    stdList<CmdArg *> _options;      // Available options
    stdVector<const char *> _args;   // remaining args
};

/// Base class for all arguments,
/// \see CmdArgFlag for boolean arguments,
/// \see CmdArgDouble for double arguments etc.
class CmdArg
{
public:
    /// All arguments follow this schema:
    /// \param args specifies the CmdArgParser,
    /// \param option is the (long) option string, excluding the '-',
    /// \param description is the one-line description.
    CmdArg(CmdArgParser &args,
           const char *option,
           const char *arguments,
           const char *description);
    virtual ~CmdArg();

    // Rest is used internally
    
    // Called with option (minus '-'),
    // determine how many characters match
    size_t findMatches(const char *option) const;

    // Show info on option for command line
    void usage_option() const;

    // Get string length of option w/o description
    size_t option_size() const;
    
    // Show option description
    void usage(size_t tab) const;

    // Parse arguments from current option.
    // args: following arg
    // Result: 0: error
    //         1: option ok, no argument needed
    //         2: option ok, argument swallowed
    virtual size_t parse(const char *arg) = 0;

protected:
    PROHIBIT_DEFAULT_COPY(CmdArg);
    const char *_option;
    const char *_arguments;
    const char *_description;
};

/// Implements a boolean argument.
class CmdArgFlag : public CmdArg
{
public:
    /// Create optional boolean argument: option and description.
    CmdArgFlag(CmdArgParser &args,
               const char *option, const char *description);
    
    /// Get the value.
    operator bool() const;

    /// Set the default value.
    void set(bool value=true)
    {   _value = value; }

    virtual size_t parse(const char *arg);
private:
    bool _value;
};

/// Imlpement an integer-typed argument.
class CmdArgInt : public CmdArg
{
public:
    /// Create the argument:
    /// \param option: option test without the leading '-'
    /// \param argument_name: a description like "<seconds>"
    /// \param description: guess what
    CmdArgInt(CmdArgParser &args, const char *option,
               const char *argument_name, const char *description);
    
    /// Set the default.
    void set(int value);

    /// Get the value.
    operator int() const;

    /// Get the value.
    int get() const;

    virtual size_t parse(const char *arg);

private:
    int _value;
};

/// Implements a double-typed argument.
class CmdArgDouble : public CmdArg
{
public:
    /// \sa CmdArgInt
    CmdArgDouble(CmdArgParser &args, const char *option,
                 const char *argument_name, const char *description);
    
    /// Set the default.
    void set(double value);

    /// Get the value.
    operator double() const;

    virtual size_t parse(const char *arg);
private:
    double _value;
};

/// Implements a string-typed argument.
class CmdArgString : public CmdArg
{
public:
    /// \sa CmdArgInt
    CmdArgString(CmdArgParser &args, const char *option,
                 const char *argument_name, const char *description);

    /// Set the default.
    void set(const stdString &value);

    /// Get the value.
    operator const stdString &() const;

    /// Get the value.
    const stdString &get() const;

    /// Was option provided?
    bool isSet() const;
    
    virtual size_t parse(const char *arg);
    
private:
    stdString _value;
};

/// @}

// inlines ----------------------------------------------

inline void CmdArgParser::setHeader(const char *header)
{   _header = header; }

inline void CmdArgParser::setFooter(const char *footer)
{   _footer = footer; }

inline void CmdArgParser::setArgumentsInfo(const char *args_info)
{   _args_info = args_info; }

inline const stdVector<const char *> &CmdArgParser::getArguments()
{   return _args; }

inline const char *CmdArgParser::getArgument(size_t i)
{   return _args[i]; }


inline CmdArgFlag::operator bool() const
{   return _value; }

inline void CmdArgInt::set(int value)
{   _value = value; }

inline CmdArgInt::operator int() const
{   return _value; }
 
inline int CmdArgInt::get() const
{   return _value; }

inline void CmdArgDouble::set(double value)
{   _value = value; }

inline CmdArgDouble::operator double() const
{   return _value; }

inline void CmdArgString::set(const stdString &value)
{   _value = value; }

inline CmdArgString::operator const stdString &() const
{   return _value; }
 
inline const stdString &CmdArgString::get() const
{   return _value; }

inline bool CmdArgString::isSet() const
{   return _value.length() > 0; }


#endif //  __ARGPARSER_H__

