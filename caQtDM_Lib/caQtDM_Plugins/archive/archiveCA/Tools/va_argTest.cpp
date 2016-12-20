
// System
#include <stdio.h>
#include <stdarg.h>
// Tools
#include "UnitTest.h"

static void dump_ints(int i, ...)
{
    va_list    ap;
    va_start(ap, i);
    while (i != 0)
    {
        printf("int %d\n", i);
        i = va_arg(ap, int);
    }
    va_end(ap);
}

static void dump_size_t(size_t i, ...)
{
    va_list    ap;
    va_start(ap, i);
    while (i != 0)
    {
        printf("size_t %zu\n", i);
        i = va_arg(ap, size_t);
    }
    va_end(ap);
}

TEST_CASE va_arg_test()
{
    // This is mostly about 32 vs. 64 bit.
    TEST(sizeof(int) == 4);
    if (sizeof(size_t) == 4)
    {
        PASS("size_t uses 4 bytes, Looks like 32 bit system");
        // Here's the thing: When passing just a number to
        // a var-arg routine, that's 4 bytes long.
        // Not sure if int or size_t:
        TEST(sizeof(42) == 4);
    }
    else  if (sizeof(size_t) == 8)
    {
        PASS("size_t uses 8 bytes, Looks like 64 bit system");
        // On 64bit OS, that simple number is still passed in 4 bytes,
        // so we must not use "va_arg(size_t)" to fetch it!
        TEST(sizeof(42) == 4);
    }
    else
    {   FAIL("Unknown system"); }

    PASS("This really needs to run under valgrind.");
    dump_ints(1, 2, 3, 32767, -1, -32768, 0);
    PASS("If the following are indeed passed as size_t,");
    PASS("you should see funny numbers for the negative values.")
    PASS("If they are passed as int, you should see errors");
    PASS("in valgrind about uninitialized memory.")
    dump_size_t((size_t)1, (size_t)2, (size_t)3, (size_t)32767, (size_t)-1, (size_t)-32768, (size_t)0);

    TEST_OK;
}

