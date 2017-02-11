// --------------------------------------------------------
// $Id: CGIDemangler.cpp,v 1.8 2012/07/04 09:08:46 jud Exp $
//
// Please refer to NOTICE.txt,
// included as part of this distribution,
// for legal information.
//
// Kay-Uwe Kasemir, kasemir@lanl.gov
// --------------------------------------------------------

// Tools
#include "CGIDemangler.h"
#include "MemoryBuffer.h"

// Convert a two-char hex string into the char it represents
static char x2c(const char *what)
{
   char digit;

   digit = (what[0] >= 'A' ? ((what[0] & 0xdf) - 'A')+10 : (what[0] - '0'));
   digit *= 16;
   digit += (what[1] >= 'A' ? ((what[1] & 0xdf) - 'A')+10 : (what[1] - '0'));
   return digit;
}

// Reduce any %xx escape sequences to the characters they represent
void CGIDemangler::unescape(char *url)
{
    int i,j;

    for(i=0,j=0; url[j]; ++i,++j)
    {
        if((url[i] = url[j]) == '%')
        {
            url[i] = x2c(&url[j+1]) ;
            j+= 2 ;
        }
    }
    url[i] = '\0' ;
}

void CGIDemangler::unescape(stdString &text)
{
    size_t total = text.length() + 1;
    MemoryBuffer<char> buf(total);
    memcpy(buf.mem(), text.c_str(), total);
    unescape(buf.mem());
    text = buf.mem();
}

void CGIDemangler::analyseVar(char *var)
{
    char *eq;

    // name / value
    if ((eq=strchr(var, '=')))
    {
        *eq = '\0';
        unescape(var);
        unescape(eq+1);
        stdString name, value;
        name = var;
        value = eq+1;
        add(name, value);
    }
}

void CGIDemangler::parse (const char *const_input)
{
    // Since we're changing the string in place,
    // we need to create a copy of the const input:
    size_t i =  strlen(const_input)+1;
    MemoryBuffer<char> buf(i);
    char *input = buf.mem();
    memcpy(input, const_input, i);

    // Change all plusses back to spaces
    for(i=0; input[i]; i++)
        if (input[i] == '+') input[i] = ' ' ;

    // Split on "&" to extract the name-value pairs
    char *var = strtok(input, "&");
    while (var)
    {
        analyseVar (var);
        var = strtok (NULL, "&");
    }
}

void CGIDemangler::add (const stdString &name, const stdString &value)
{
    _vars.insert (stdMap<stdString, stdString>::value_type(name, value));
}

stdString CGIDemangler::find (const stdString &name) const
{
    stdMap<stdString, stdString>::const_iterator pos = _vars.find (name);
    if (pos != _vars.end())
        return pos->second;
    else
        return "";
}

