// --------------------------------------------------------
// $Id: ArgParser.cpp,v 1.8 2012/07/04 09:08:46 jud Exp $
//
// Please refer to NOTICE.txt,
// included as part of this distribution,
// for legal information.
//
// Kay-Uwe Kasemir, kasemir@lanl.gov
// --------------------------------------------------------

#include"Filename.h"
#include"ArgParser.h"
#include<stdio.h>
#include<stdlib.h>

CmdArgParser::CmdArgParser(int argc, char const * const *argv)
{
    _progname = argv[0];
    _argc = argc-1;
    _argv = &argv[1];
    _header = 0;
    _footer = 0;
    _args_info = 0;
}

void CmdArgParser::addOption(CmdArg *option)
{
    if (option)
        _options.push_back(option);
}

void CmdArgParser::usage()
{
    stdList<CmdArg *>::iterator option;
    stdString fullname, prog;
    fullname = _progname;
    Filename::getBasename(fullname, prog);

    if (_header)
        fprintf(stderr, "%s", _header);
    fprintf(stderr, "USAGE: %s ", prog.c_str());
    if (_options.size() < 3)
    {
        for (option = _options.begin(); option != _options.end(); ++option)
            (*option)->usage_option();
    }
    else
        fprintf(stderr, "[Options]");
    if (_args_info)
        fprintf(stderr, " %s", _args_info);
    fprintf(stderr, "\n\n");

    fprintf(stderr, "Options:\n");
    size_t tab = 0, o_tab;
    for (option = _options.begin(); option != _options.end(); ++option)
    {
        o_tab = (*option)->option_size();
        if (o_tab > tab)
            tab = o_tab;
    }
    for (option = _options.begin(); option != _options.end(); ++option)
        (*option)->usage(tab+3);
    if (_footer)
        fprintf(stderr, "%s", _footer);
}

bool CmdArgParser::parse()
{
    stdList<CmdArg *>::iterator option;
    size_t matches;

    int i = 0;
    while (i < _argc)
    {   // simple arg
        if (_argv[i][0] != '-')
        {
            _args.push_back(_argv[i]);
            ++i;
            continue;
        }

        for (option=_options.begin(); option != _options.end(); ++option)
        {
            matches = (*option)->findMatches(_argv[i]+1);
            if (matches > 0)
            {
                bool trailing_arg = matches < strlen (_argv[i]+1);
                const char *arg = trailing_arg ?
                                  _argv[i]+matches+1 : _argv[i+1];

                switch ((*option)->parse(arg))
                {
                case 1:
                    ++i;
                    break;
                case 2:
                    if (trailing_arg)
                        ++i;
                    else
                        i += 2;
                    break;
                default:
                    fprintf(stderr, "Error in option -%c\n", _argv[i][1]);
                    usage();
                    return false;
                }
                break;
            }
        }
        if (option == _options.end())
        {
            fprintf(stderr, "Error in option -%s\n", _argv[i]);
            usage();
            return false;
        }
    }

    return true;
}

CmdArg::CmdArg(CmdArgParser &args, const char *option,
               const char *arguments, const char *description)
{
    args.addOption(this);
    _option = option;
    _arguments   = arguments;
    _description = description;
}

CmdArg::~CmdArg()
{
}

size_t CmdArg::findMatches(const char *option) const
{
    size_t matches = 0;

    while (_option[matches] && option[matches])
    {
        if (option[matches] == _option[matches])
            ++matches;
        else
            return 0;
    }

    return matches;
}

void CmdArg::usage_option() const
{
    fprintf(stderr, " -%s", _option);
    if (_arguments)
        fprintf(stderr, " %s", _arguments);
}

size_t CmdArg::option_size() const
{
    size_t len = strlen(_option);
    if (_arguments)
        len += strlen(_arguments);
    return len;
}

void CmdArg::usage(size_t tab) const
{
    fprintf(stderr, "  -%s", _option);
    size_t skip = tab - strlen(_option);

    if (_arguments)
    {
        fprintf(stderr, " %s", _arguments);
        size_t space = 1 + strlen (_arguments);
        if (skip > space)
            skip -= space;
        else
            skip = 1;
    }
    while (skip-- > 0)
        fprintf(stderr, " ");
    fprintf(stderr, "%s\n", _description);
}

CmdArgFlag::CmdArgFlag(CmdArgParser &args, const char *option,
                       const char *description)
        : CmdArg(args, option, 0, description)
{
    _value = false;
}

size_t CmdArgFlag::parse (const char *arg)
{
    _value = true;
    return 1;
}

CmdArgInt::CmdArgInt(CmdArgParser &args, const char *option,
                     const char *arguments, const char *description)
        : CmdArg(args, option, arguments, description)
{
    _value = 0;
}

size_t CmdArgInt::parse(const char *arg)
{
    if (arg && *arg)
    {
        _value = atoi(arg);
        return 2;
    }
    return 0;
}

CmdArgDouble::CmdArgDouble(CmdArgParser &args, const char *option,
                           const char *arguments, const char *description)
        : CmdArg (args, option, arguments, description)
{
    _value = 0;
}

size_t CmdArgDouble::parse(const char *arg)
{
    if (arg && *arg)
    {
        _value = atof(arg);
        return 2;
    }
    return 0;
}

CmdArgString::CmdArgString(CmdArgParser &args, const char *option,
                            const char *arguments, const char *description)
        : CmdArg(args, option, arguments, description)
{
    _value = (const char *)0;
}

size_t CmdArgString::parse(const char *arg)
{
    if (arg && *arg)
    {
        _value = arg;
        return 2;
    }
    return 0;
}

