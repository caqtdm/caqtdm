// -*- c++ -*-

#ifndef __AUTO_FILE_PRT_H__
#define __AUTO_FILE_PRT_H__

// System
#include <stdio.h>
// Tool
#include <NoCopy.h>

/// \ingroup Tools

/// Auto-close FILE pointer wrapper.
class AutoFilePtr
{
public:
    /// Construct AutoFilePtr for given filename and mode.
    ///
    /// To test the result, use the bool operator.
    AutoFilePtr(const char *filename, const char *mode)
        : f(0)
    {
        open(filename, mode);
    }

    /// Construct AutoFilePtr for existing FILE,
    /// which is now controlled by the AutoFilePtr.
    AutoFilePtr(FILE *f = 0) : f(f) {}

    /// Destructor closes the FILE under control of this AutoFilePtr.
    ~AutoFilePtr()
    {
        close();
    }

    /// Open a (new) file.
    bool open(const char *filename, const char *mode)
    {
        set(fopen(filename, mode));
        return f != 0;
    }

    /// Close the current file.
    void close()
    {
        set(0);
    }

    /// Release control of the current file, closing it,
    /// and switch to a new file.
    void set(FILE *new_f)
    {
        if (f)
            fclose(f);
        f = new_f;
    }

    /// Is there an open file?
    operator bool () const
    {
        return f != 0;
    }

    /// Obtain the current FILE.
    operator FILE * () const
    {
        return f;
    }
private:
    PROHIBIT_DEFAULT_COPY(AutoFilePtr);
    FILE *f;
};

#endif
