// -*- c++ -*-
#if !defined(_REGULAREXPRESSION_H_)
#define _REGULAREXPRESSION_H_

#include <ToolsConfig.h>

/// \ingroup Tools Wrapper for Unix/GNU regex library.
class RegularExpression  
{
public:
    /// Create a regular expression for a "glob" pattern:
    /// question mark - any character
    /// star          - many characters
    /// case insensitive
    static stdString fromGlobPattern(const stdString &glob);

    /// Create RegularExpression with pattern for further matches.
    ///
    /// @exception GenericException on compilation error
    RegularExpression(const char *pattern, bool case_sensitive=true)
    {
        set(pattern, case_sensitive);
    }

    RegularExpression(const RegularExpression &rhs)
    {
        set(rhs._pattern.c_str(), rhs._case_sensitive);
    }

    ~RegularExpression();

    /// Test if 'input' matches current pattern.
    ///
    /// Currently uses
    /// - EXTENDED regular expression
    /// - case sensitive
    /// - input must be anchored for full-string match,
    ///   otherwise substrings will match:
    ///     abc        matches        b
    ///     abc     does not match    $b^
    ///
    /// When no pattern was supplied, anything matches!
    bool doesMatch(const char *input);

    bool doesMatch(const stdString &input)
    { return doesMatch(input.c_str()); }
    
private:
    friend class ToRemoveGNUCompilerWarning;

    void set(const char *pattern, bool case_sensitive=true);

    // Use create/reference/unreference instead of new/delete:
    RegularExpression & operator = (const RegularExpression &); // intentionally not implemented
    
    RegularExpression();

    stdString _pattern;
    bool    _case_sensitive;
    void    *_compiled_pattern;
    int     _refs;
};

#endif // !defined(_REGULAREXPRESSION_H_)
 
