

// Tools
#include <UnitTest.h>
// Storage
#include "DataFile.h"

TEST_CASE test_data_file()
{
    try
    {
        // Get read-only reference.
        DataFile *df1 = DataFile::reference("../DemoData", "20040305", false);
        TEST_MSG(df1, "Opened file");
        TEST(df1->refCount() == 1);

        // Get another one, basically the same file.
        DataFile *df1a = DataFile::reference("", "../DemoData/20040305", false);
        TEST_MSG(df1a, "Opened again");
        TEST(df1 == df1a);
        TEST(df1->refCount() == 2);

        // One file should remain cached.
        TEST(DataFile::clear_cache() == 1);

        DataFile *df1b = DataFile::reference("../DemoData/", "20040305", false);
        TEST_MSG(df1b, "Opened again");
        TEST(df1 == df1b);
        TEST(df1->refCount() == 3);

        // Get a new ref. this time for writing.
        DataFile *df2 = DataFile::reference("", "../DemoData/20040305", true);
        TEST_MSG(df2, "Opened for writing");
        TEST(df1 != df2);

        df2->release();
        df1b->release();
        TEST(df1->refCount() == 2);
        df1a->release();
        TEST(df1->refCount() == 1);
        df1->release();
        TEST(df1->refCount() == 0);

        TEST(DataFile::clear_cache() == 0);

        DataFile::close_all();
    }
    catch (GenericException &e)
    {
        printf("Exception:\n%s\n", e.what());
        FAIL("Caught exception");
    }

    TEST_OK;
}
