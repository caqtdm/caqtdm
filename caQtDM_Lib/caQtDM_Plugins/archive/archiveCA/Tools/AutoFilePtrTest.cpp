// System
#include <string.h>
// Tools
#include <AutoFilePtr.h>
#include <UnitTest.h>

TEST_CASE bogus_auto_file_ptr()
{
    AutoFilePtr bogus_file(fopen("notthere", "r"));
    TEST(! bogus_file);
    TEST_OK;
}

TEST_CASE auto_file_ptr()
{
    char line[100];
    FILE *stale_copy;
    {
        AutoFilePtr file_ok("AutoFilePtr.h", "r");
        TEST(file_ok);
        TEST(fread(line, 1, 20, file_ok) == 20);
        line[15] = '\0';
        TEST(strcmp(line, "// -*- c++ -*-\n") == 0);
        stale_copy = file_ok;
    }
    // Now the file should be closed,
    // and stale_copy should point to an invalid
    // entry. Is there a way to check?
    // The following will result in 'valgrind'
    // errors and sometimes cause a deferred crash,
    // so that's NOT the way to do it:
    //   TEST(fread(line, 1, 20, stale_copy) == 0);
    TEST_OK;
}


