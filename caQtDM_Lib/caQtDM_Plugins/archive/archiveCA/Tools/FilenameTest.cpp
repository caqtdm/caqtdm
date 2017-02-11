
#include "UnitTest.h"
#include "GenericException.h"
#include "Filename.h"

TEST_CASE Filename_Test()
{
    try
    {
        stdString invalid;
        stdString dir("/usr/fred");
        stdString dir2("/usr/fred/");
        stdString base("file");
    
        TEST(!Filename::isValid(invalid));
        TEST(Filename::isValid(dir));
        TEST(Filename::isValid(base));
    
        stdString full;
        Filename::build("", base, full);
        TEST(full == "file");
    
        try
        {
            Filename::build(dir, "", full);
            FAIL("Didn't catch empty basename");
        }
        catch (GenericException &e)
        {
            TEST("Caught empty basename");
        }
    
        Filename::build(dir, base, full);
        TEST(full == "/usr/fred/file");
    
        Filename::build(dir2, base, full);
        TEST(full == "/usr/fred/file");
    
        TEST(Filename::containsPath(dir));
        TEST(Filename::containsPath(dir2));
        TEST(!Filename::containsPath(base));
    
        TEST(Filename::containsFullPath(dir));
        TEST(Filename::containsFullPath(dir2));
        TEST(!Filename::containsFullPath(base));
    
        Filename::getDirname(full, dir);
        TEST(dir == "/usr/fred");
    
        Filename::getBasename(full, base);
        TEST(base == "file");
    }
    catch (GenericException &e)
    {
        printf("Exception:\n%s\n", e.what());
        FAIL("Exception");
    }
    
    TEST_OK;
}


