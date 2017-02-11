// --------------------------------------------------------
// $Id: ASCIIParser.cpp,v 1.8 2012/07/04 09:08:46 jud Exp $
//
// Please refer to NOTICE.txt,
// included as part of this distribution,
// for legal information.
//
// Kay-Uwe Kasemir, kasemir@lanl.gov
// --------------------------------------------------------

#include "ASCIIParser.h"
#include <ctype.h>

ASCIIParser::ASCIIParser()
    : line_no(0)
{}

bool ASCIIParser::open(const stdString &file_name)
{
    return file.open(file_name.c_str(), "rt");
}

bool ASCIIParser::nextLine()
{
    char buf[1024];
    char *ch;
    size_t i;

    while (true)
    {
        if (fgets(buf, sizeof(buf), file) == 0)
        {   // got nothing -> quit
            line.assign((const char *)0, 0);
            return false;
        }
        ++line_no;

        ch = buf;
        i = 0;
        // skip leading white space
        while (*ch && isspace(*ch))
        {
            ++ch;
            ++i;
            if (i >= sizeof(buf))
            {
                line.assign((const char *)0, 0);
                return false;
            }
        }
        if (! *ch)
            return nextLine(); // empty line

        // skip comment lines
        if (*ch == '#')
            continue; // try next line

        // remove trailing white space
        i = strlen(ch);
        while (i > 0  && isspace(ch[i-1]))
            --i;
        ch[i] = '\0';

        line = ch;
        return true;
    }
}

// Get trimmed substring from line.
// s : start index to use
// e : index of last char to consider
// Both s & e must be valid, not point to '\0', and s < e
// On success, result is placed in result.
static bool get_trimmed_substring(const stdString line, size_t s, size_t e, stdString &result)
{
    if (s > e)
        return false;
    // Skip initial whitespace
    while (isspace(line[s]))
    {
        ++s;
        if (s > e)
            return false;
    }
    // Skip trailing whitespace
    while (isspace(line[e]))
    {
        --e;
        if (e < s)
            return false;
    }
    result = line.substr(s, e-s+1);
    return result.length() > 0;
}

bool ASCIIParser::getParameter(stdString &parameter, stdString &value)
{
    size_t sep = line.find('=');
    size_t l = line.length();
    if (sep == line.npos   ||   sep == 0  ||  sep == l-1)
        return false;
    return get_trimmed_substring(line, 0, sep-1, parameter) &&
           get_trimmed_substring(line, sep+1, l-1, value);
}

// EOF ASCIIParser.cpp
