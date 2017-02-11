
// Tools
#include "GenericException.h"
#include "Lockfile.h"
#include "UnitTest.h"
#include "AutoFilePtr.h"

static const char *name = "test.lck";

TEST_CASE test_lockfile()
{
    try
    {
        // Create a lock file.
        Lockfile lock(name, "UnitTest");
        // Check that it exists.
        AutoFilePtr read_test;
        TEST_MSG(read_test.open(name, "rt"), "Dump of the Lock file:");
        char buffer[100];
        while (fgets(buffer, sizeof(buffer), read_test))
            fputs(buffer, stdout);
        read_test.close();
        PASS("------");
        try
        {
            Lockfile another(name, "UnitTest");
            FAIL("Managed to create another lock file");
        }
        catch (GenericException &e)
        {
            PASS("Prevented duplicate lock file.");
        }
    }
    catch (GenericException &e)
    {
        FAIL(e.what());
    }
    TEST_OK;
}
