// System
#include <stdio.h>
// Tools
#include <UnitTest.h>
#include <MsgLogger.h>
// Storage
#include "IndexFile.h"
#include "DataFile.h"
#include "PlotReader.h"

static size_t read_test(const stdString &index_name, const stdString &channel_name,
                        double delta,
                        const epicsTime *start = 0, const epicsTime *end = 0)
{
    stdString text;
    size_t num = 0;
    try
    {
        IndexFile index;
        index.open(index_name);
        PlotReader reader(index, delta);
        const RawValue::Data *value = reader.find(channel_name, start);
        while (value &&
               (end==0  ||  RawValue::getTime(value) < *end))
        {
            ++num;
            LOG_ASSERT(value == reader.get());
            reader.toString(text);
            printf("    %s\n", text.c_str());
            value = reader.next();
        }
    }
    catch (GenericException &e)
    {
        printf("Exception:\n%s\n", e.what());
        return 0;
    }
    return num;
}

TEST_CASE PlotReaderTest()
{
    TEST(read_test("../DemoData/index", "fred", 10.0) > 0);

    epicsTime start;
    TEST(string2epicsTime("03/23/2004 10:50:00", start));
    TEST(read_test("../DemoData/index", "fred", 10.0, &start) > 0);

    epicsTime end(start);
    end += 15;
    TEST(read_test("../DemoData/index", "fred", 5.0, &start, &end) > 0);

    TEST(DataFile::clear_cache() == 0);

    TEST_OK;
}

