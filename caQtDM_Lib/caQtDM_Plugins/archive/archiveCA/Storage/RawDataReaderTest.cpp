// System
#include <stdio.h>
// Tools
#include <UnitTest.h>
#include <MsgLogger.h>
// Storage
#include "IndexFile.h"
#include "AutoIndex.h"
#include "DataFile.h"
#include "RawDataReader.h"

static size_t read_test(const stdString &index_name, const stdString &channel_name,
                        const epicsTime *start = 0, const epicsTime *end = 0)
{
    stdString text;
    size_t num = 0;
    try
    {
        IndexFile index;
        index.open(index_name);
        RawDataReader reader(index);
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

static size_t auto_read_test(const stdString &index_name, const stdString &channel_name,
                        const epicsTime *start = 0, const epicsTime *end = 0)
{
    stdString text;
    size_t num = 0;
    try
    {
        AutoIndex index;
        index.open(index_name);
        RawDataReader reader(index);
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


TEST_CASE RawDataReaderTest()
{
    TEST(read_test("../DemoData/index", "fred") == 87);

    epicsTime start;
    TEST(string2epicsTime("03/23/2004 10:50:42.400561000", start));
    TEST(read_test("../DemoData/index", "fred", &start) == 10);
    TEST(auto_read_test("../DemoData/index", "fred", &start) == 10);
    TEST(auto_read_test("list_index.xml", "fred", &start) == 10);

    epicsTime end(start);
    end += 5;
    TEST(read_test("../DemoData/index", "fred", &start, &end) == 3);
    TEST(auto_read_test("../DemoData/index", "fred", &start, &end) == 3);
    TEST(auto_read_test("list_index.xml", "fred", &start, &end) == 3);

    TEST(DataFile::clear_cache() == 0);
    TEST_OK;
}

static size_t dual_read_test(const stdString &index_name, const stdString &channel_name,
                             const epicsTime *start = 0, const epicsTime *end = 0)
{
    stdString text;
    size_t num = 0;
    try
    {
        IndexFile index1, index2;
        index1.open(index_name);
        index2.open(index_name);
        RawDataReader reader1(index1);
        RawDataReader reader2(index2);
        const RawValue::Data *value1 = reader1.find(channel_name, start);
        const RawValue::Data *value2 = reader2.find(channel_name, start);
        while ((value1 &&
                (end==0  ||  RawValue::getTime(value1) < *end)) ||
               (value2 &&
                (end==0  ||  RawValue::getTime(value2) < *end)) )
        {
            if (value1 && (end==0  ||  RawValue::getTime(value1) < *end))
            {
                ++num;
                LOG_ASSERT(value1 == reader1.get());
                reader1.toString(text);
                printf("1) %s\n", text.c_str());
                value1 = reader1.next();
            }
            if (value2 && (end==0  ||  RawValue::getTime(value2) < *end))
            {
                ++num;
                LOG_ASSERT(value2 == reader2.get());
                reader2.toString(text);
                printf("2) %s\n", text.c_str());
                value2 = reader2.next();
            }
        }
    }
    catch (GenericException &e)
    {
        printf("Exception:\n%s\n", e.what());
        return 0;
    }
    return num;
}

TEST_CASE DualRawDataReaderTest()
{
    TEST(dual_read_test("../DemoData/index", "fred") == 2*87);

    epicsTime start;
    TEST(string2epicsTime("03/23/2004 10:50:42.400561000", start));
    TEST(dual_read_test("../DemoData/index", "fred", &start) == 2*10);

    epicsTime end(start);
    end += 5;
    TEST(dual_read_test("../DemoData/index", "fred", &start, &end) == 2*3);

    TEST(DataFile::clear_cache() == 0);
    TEST_OK;
}

