#include "ToolsConfig.h"
#include "GenericException.h"

const stdString::size_type stdString::npos = static_cast<size_type>(-1); 

char stdString::operator [] (size_t index) const
{
    if (index >= _len)
        throw GenericException(__FILE__, __LINE__,
                               "stdString [] index %zd, len %zd", index, _len);
    return _str[index];
}

stdString & stdString::assign(const char *s, size_type len)
{
    if (!s || !len)
    {    // assignment of NULL string
        if (_res > 0)
        {
            *_str = '\0';
            _len = 0;
        }
        return *this;
    }
    if (reserve(len))
    {
        memcpy(_str, s, len);
        _str[len] = '\0';
        _len = len;
    }
    return *this;
}

stdString & stdString::append(const char *s, size_type len)
{
    if (len > 0)
    {
        if (reserve (_len + len))
        {
            memcpy (_str+_len, s, len);
            _len += len;
            _str[_len] = '\0';
        }
    }
    return *this;
}

int stdString::compare(const stdString &rhs) const
{
    if (_str)
    {
        if (rhs._str)
            return strcmp(_str, rhs._str);
        // _str > NULL
        return +1;
    }
    if (rhs._str) // NULL < rhs
        return -1;
    return 0;
}

bool stdString::reserve(size_type len)
{
    if (len <= _res)
        return true;

    char *prev = _str;
    try
    {
        _str = new char [len+1];
    }
    catch (...)
    {
        _str = 0;
    }
    if (!_str)
    {
        _res = 0;
        _len = 0;
        return false;
    }
    _res = len;
    if (prev)
    {
        memcpy(_str, prev, _len+1);
        delete [] prev;
    }
    return true;
}

// get [from, up to n elements
stdString stdString::substr(size_type from, size_type n) const
{
    stdString s;
    
    if (from >= _len)
        return s;
    if (n == npos  ||  from+n > _len)
        n = _len - from;

    s.assign(_str + from, n);

    return s;
}         

