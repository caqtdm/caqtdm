// System
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
// Base
#include <epicsVersion.h>
// Tools
#include <ArgParser.h>
// Index
#include "FileAllocator.h"

bool verbose;

int main(int argc, const char *argv[])
{
    CmdArgParser parser(argc, argv);
    parser.setHeader("File Allocator Tool version "
                     EPICS_VERSION_STRING
                     ", built " __DATE__ ", " __TIME__ "\n\n");
    parser.setArgumentsInfo("<file> <reserved bytes>");
    CmdArgFlag   be_verbose (parser, "verbose", "Verbose mode");
    
    if (! parser.parse())
        return -1;
    if (parser.getArguments().size() != 2)
    {
        parser.usage();
        return -1;
    }
    verbose = be_verbose;

    stdString file_name = parser.getArgument(0);
    long reserved = atol(parser.getArgument(1));
    
    FILE *f = fopen(file_name.c_str(), "rb");
    if (!f)
    {
        fprintf(stderr, "Cannot open '%s'\n",
                file_name.c_str());
        return -1;
    }
    
    FileAllocator fa;
    if (!fa.attach(f, reserved, true))
    {
        fprintf(stderr, "Cannot attach FileAllocator\n");
        return -1;
    }   
    fa.dump(verbose ? 10 : 0);
    fa.detach();
    fclose(f);
    
    return 0;
}

