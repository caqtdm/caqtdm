#include "IndexConfig.h"
#include "GenericException.h"
#include "UnitTest.h"

TEST_CASE index_config_test()
{
    IndexConfig index_config;

    try
    {
        index_config.parse("nonexisting_file");
        FAIL("Should not get here");
    }
    catch (GenericException &e)
    {
        TEST_MSG(e.what(), "Caught exception for non-existing file");
    }

    try
    {
        TEST_MSG(index_config.parse("IndexConfig.h") == false, "Recognize non-XML");
    }
    catch (GenericException &e)
    {
        printf("%s", e.what());
        FAIL("Should not trow a message");
    }

    try
    {
        TEST(index_config.parse("../IndexTool/indexconfig.xml"));
        TEST(index_config.subarchives.size() == 2);
    }
    catch (GenericException &e)
    {
        printf("%s", e.what());
        FAIL("Should not trow a message");
    }

    TEST_OK;
}
