
#include "ArchiveException.h"
#include "UnitTest.h"
#include "stdlib.h"

class Huge
{
public:
    Huge()
    {
        mem = new char[0x7FFFFFFFL];
    }
    ~Huge()
    {
        delete [] mem;
    }
    char *mem;
};

TEST_CASE how_new_fails()
{
    size_t num = 0x7FFFFFFFL;
    Huge *mem = 0;

    // Assume that IN_VALGRIND is set by <somebody>
    // when running under valgrind.
    if (getenv("IN_VALGRIND"))
    {
        TEST("Under valgrind, we can't run the memory exhaustion test");
    }
    else
    {
        try
        {
            mem = new Huge[num];
            // We should not reach this point
            TEST(mem != 0);
            delete [] mem;
        }
        // In theory, this should happen
        //    catch (std::bad_alloc)
        // But with the gcc 4.0 on Mac OS X 10.4,
        // we get a 'malloc' error message
        // and then end up here:
        catch (...)
        {
            TEST_MSG(1, "Caught an exception from new");
            mem = (Huge *)1;
        }
        TEST_MSG(mem == (Huge *)1, "Handled the 'new' failure");
    }
    TEST_OK;
}

TEST_CASE various_exception_tests()
{
    int exception_count = 0;

    try
    {
        throw GenericException(__FILE__, __LINE__);
        FAIL("should not get here");
    }
    catch (GenericException &e)
    {
        printf("     %s", e.what());
        ++exception_count;
    }

    try
    {
        throw GenericException(__FILE__, __LINE__, "Hello %s", "World");
        FAIL("should not get here");
    }
    catch (GenericException &e)
    {
        printf("     %s", e.what());
        ++exception_count;
    }
    try
    {
        throwArchiveException(Invalid);
        FAIL("should not get here");
    }
    catch (GenericException &e)
    {
        printf("     %s", e.what());
        ++exception_count;
    }

    try
    {
        throwDetailedArchiveException(Invalid, "in a test");
        FAIL("should not get here");
    }
    catch (GenericException &e)
    {
        printf("     %s", e.what());
        ++exception_count;
    }

    // Nested
    try
    {
        try
        {
            throwDetailedArchiveException(Invalid, "Level 1");
            FAIL("should not get here");
        }
        catch (GenericException &e)
        {
            ++exception_count;
            throw GenericException(__FILE__, __LINE__, "Level 2: Caught %s",
                                   e.what());
        }
    }
    catch (GenericException &e)
    {
        printf("     %s", e.what());
        ++exception_count;
    }

    // Rethrow
    try
    {
        try
        {
            throwDetailedArchiveException(Invalid, "Rethrown");
            FAIL("should not get here");
        }
        catch (GenericException &e)
        {
            ++exception_count;
            throw e;
        }
    }
    catch (GenericException &e)
    {
        printf("     %s", e.what());
        ++exception_count;
    }


    TEST(exception_count == 8);
    TEST_OK;
}

class ThrowInDescructor
{
public:
    ~ThrowInDescructor()
    {
        if (std::uncaught_exception())
            printf("ThrowInDescructor doesn't throw because of uncaught_exception\n");
        else
            throw  GenericException(__FILE__, __LINE__, "I throw in my destructor");
    }
    int dummy;
};

TEST_CASE double_throw()
{
    try
    {
        ThrowInDescructor tid;
        tid.dummy = 1;
        TEST_MSG(tid.dummy, "Created ThrowInDescructor test class");
        // This exception causes tid to be destructed,
        // so then an exception is throws while
        // there is already an active exception...
        throw  GenericException(__FILE__, __LINE__, "Test");
        FAIL("Should not get here");
    }
    catch (GenericException &e)
    {
        TEST(e.getDetail() == "Test");
    }
    TEST_OK;
}
