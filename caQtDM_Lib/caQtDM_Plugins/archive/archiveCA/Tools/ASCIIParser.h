// -------------------------------------------- -*- c++ -*-
// $Id: ASCIIParser.h,v 1.8 2012/07/04 09:08:46 jud Exp $
//
// Please refer to NOTICE.txt,
// included as part of this distribution,
// for legal information.
//
// Kay-Uwe Kasemir, kasemir@lanl.gov
// --------------------------------------------------------

#ifndef __ASCII_PARSER__
#define __ASCII_PARSER__

// System
#include <stdio.h>
// Tools
#include <ToolsConfig.h>
#include <AutoFilePtr.h>
#include <NoCopy.h>

/// \ingroup Tools

/// Helper class for programming an ASCII-file parser:

/// The ASCIIParser assists in writing an ASCII file parser:
/// <ul>
/// <li> Opens/closes file,
/// <li> skips comments (number sign),
/// <li> removes leading and trainling white space from lines,
/// <li> extracts parameter and value from "parameter=value" lines.
/// </ul>
class ASCIIParser
{
public:
    ASCIIParser();
    
    /// Open file for parsing.
    ///
    /// Only opens the file, does not read the first line.
    /// Result: true for success
    bool open(const stdString &file_name);

    /// Read next line from file, skipping comment lines.
    ///
    /// Result: false for error, hit end of file, ...
    bool nextLine();

    /// Get current line as string, excluding '\n'.
    const stdString & getLine() const;

    /// Get number of current line
    size_t getLineNo() const;

    /// Try to extract parameter=value pair from current line.
    ///
    /// Result: found parameter?
    bool getParameter(stdString &parameter, stdString &value);

private:
    PROHIBIT_DEFAULT_COPY(ASCIIParser);
    AutoFilePtr   file;
    size_t        line_no;
    stdString     line;
};

inline const stdString &ASCIIParser::getLine() const
{   return line;   }

inline size_t ASCIIParser::getLineNo() const
{   return line_no; }

#endif


