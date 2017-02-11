
#include"ArchiveException.h"

static const char *error_text[] =
{
/* NoError */        "NoError: This should never occur...",
/* Fail  */            "Failure",
/* Invalid */        "Invalid",
/* OpenError */        "Cannot open file",
/* CreateError*/    "Cannot create new file",
/* ReadError */        "Read Error",
/* WriteError */    "Write Error",
/* Unsupported */    "Not Supported",
};

ArchiveException::ArchiveException(const char *sourcefile, size_t line, Code code)
        : GenericException(sourcefile, line,
                           "Archive Exception: %s", error_text[code]),
          code(code)
{}

ArchiveException::ArchiveException(const char *sourcefile, size_t line,
                                   Code code, const stdString &detail)
        : GenericException(sourcefile, line,
                           "Archive Exception: %s,\n%s\n",
                           error_text[code], detail.c_str()),
          code(code)
{}



