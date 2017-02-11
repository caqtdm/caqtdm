// Tools
#include "OrderedMutex.h"
#include "UnitTest.h"

TEST_CASE deadlock_test()
{
    OrderedMutex a("a", 1);
    OrderedMutex b("b", 2);
    try
    {   // Take in allowed order
        a.lock(__FILE__, __LINE__);
        b.lock(__FILE__, __LINE__);
        // Release, order does not matter.
        b.unlock();
        a.unlock();
        PASS("Took a, b");
        // Take in allowed order, more than once
        a.lock(__FILE__, __LINE__);
        a.lock(__FILE__, __LINE__);
        b.lock(__FILE__, __LINE__);
        // Release, order does not matter.
        a.unlock();
        b.unlock();
        a.unlock();
        PASS("Took a, a, b");
    }
    catch (GenericException &e)
    {
        FAIL("Caught exception");
    }
    try
    {   // Attempt to take in wrong oder.
        b.lock(__FILE__, __LINE__);
        a.lock(__FILE__, __LINE__);
        // Should not get here.
        FAIL("I reversed the lock order without problems?!");
        b.unlock();
        a.unlock();
    }
    catch (GenericException &e)
    {
        PASS("Caught exception:");
        printf("        %s\n", e.what());
        b.unlock(); // Release all locks
    }
    
    TEST_OK;
}
