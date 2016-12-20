
#define INDEX_TEST

// System
#include <stdlib.h>
// Tools
#include <AutoPtr.h>
#include <BinIO.h>
#include <UnitTest.h>
// Storage
#include "RTree.h"
#include "FileAllocator.h"
#ifdef INDEX_TEST
#include "IndexFile.h"
#endif

// Test data for fill_test
typedef struct
{
    const char *start, *end;
    const char *file;
    long offset;
}  TestData;

// Used in manual
static TestData man_data1[] =
{
    { "1", "2", "FileA",  0x10 },
    { "2", "3", "FileA",  0x20 },
    { "3", "4", "FileA",  0x30 },
    { "4", "5", "FileA",  0x40 },
    { "5", "6", "FileA",  0x50 },
    { "6", "7", "FileA",  0x60 },
};
// ...
static TestData man_data2[] =
{
    { "4", "6", "FileB",  0x10 },
    { "6", "8", "FileB",  0x20 },
    { "8", "9", "FileB",  0x30 },
    { "9","10", "FileB",  0x40 },
};
// This is meant to handle all cases at least once.
static TestData fill_data[] =
{
    { "20", "21", "20-21",  1 },    // starting point
    { "10", "11", "10-11",  2 },    // insert left
    { "30", "31", "30-31",  3 },    // insert right
    { "40", "41", "40-41",  4 },    // overflow right
    { "10", "15", "10-15",  5 },    // left overlaps with existing
    { "39", "41", "38-41",  6 },    // right overlaps with existing
    { "26", "27", "26-27",  7 },    // overflow left
    { "26", "32", "26-32",  8 },    // left overlaps & interleaves 2 exist. recs
    { "25", "30", "25-30",  9 },    // right overlaps 2 exist. recs
    { "20", "21", "20-21(B)", 10 }, // fully hidden under existing record
    { "50", "51", "50-51",  11 },   // insert right
    { "60", "61", "60-61",  12 },   // overflow right
    { "70", "71", "70-71",  13 },   // insert right
    { "80", "81", "80-81",  14 },   // overflow right
    { "90", "91", "90-91",  15 },   // insert right
    { "95", "96", "95-96",  16 },   // overflow right
    { "96", "97", "96-97",  17 },   // insert right
    { "98", "99", "98-99",  18 },   // overflow right
    { "38", "39", "38-39",  19 },   // insert left in central node
    { "37", "38", "37-38",  20 },   // overflow left in central node
};

// Array of the above tests
typedef struct
{
    const char *index_name;
    const TestData *data;
    size_t num;
    const char *dotfile;
    const char *diff;
}   TestConfig;
static TestConfig test_config[] =
{
    {
        "test/tree.tst",
        fill_data, sizeof(fill_data)/sizeof(TestData),
        "test/test_data1.dot", "test/test_data1.dot.OK"
    },
    {
        "test/man_index_a",
        man_data1, sizeof(man_data1)/sizeof(TestData),
        "test/man_index_a.dot", 0
    },
    {
        "test/man_index_b",
        man_data2, sizeof(man_data2)/sizeof(TestData),
        "test/man_index_b.dot", 0
    }
};

TEST_CASE fill_tests()
{
    size_t test = 0, i=0;
    FileAllocator::minimum_size = 0;
    FileAllocator::file_size_increment = 0;
    try
    {   // Loop over test configs
        for (test=0; test<sizeof(test_config)/sizeof(TestConfig); ++test)
        {
            printf("Fill Test %zu: %s, %s\n",
                   test,
                   test_config[test].index_name,
                   test_config[test].dotfile);
            stdString directory;
            // Open File
            TEST_DELETE_FILE(test_config[test].index_name);
            AutoFilePtr f(test_config[test].index_name, "w+b");
            // Attach FileAllocator
            FileAllocator fa;
            fa.attach(f, RTree::anchor_size, true);
            // Add RTree
            AutoPtr<RTree> tree(new RTree(fa, 0));
            tree->init(3);

            unsigned long nodes, records;
            TEST(tree->selfTest(nodes, records));
            epicsTime start, end;
            size_t num = test_config[test].num;
            // Loop over data points
            for (i=0; i<num; ++i)
            {
                // Leave 'before' and 'after' of the last insertion
                if (i==(num-1))
                    tree->makeDot("/tmp/index0.dot");
                // Insert new data point
                string2epicsTime(test_config[test].data[i].start, start);
                string2epicsTime(test_config[test].data[i].end, end);
                stdString filename = test_config[test].data[i].file;
                if (!tree->insertDatablock(start, end,
                                           test_config[test].data[i].offset, filename))
                {
                    printf("Insert %s..%s: %zu failed\n",
                           test_config[test].data[i].start,
                           test_config[test].data[i].end, i);
                    FAIL("insertDatablock (1)");
                }
                // When inserted again, that should be a NOP
                if (tree->insertDatablock(start, end,
                                          test_config[test].data[i].offset, filename))
                {
                    printf("Re-Insert %s..%s: %zu failed\n",
                           test_config[test].data[i].start,
                           test_config[test].data[i].end, i);
                    FAIL("insertDatablock (2)");
                }
                // ... 'after' of the last insertion
                if (i==(num-1))
                    tree->makeDot("/tmp/index1.dot");
                // Check
                if (!tree->selfTest(nodes, records))
                {
                    FAIL("RTree::selfTest");
                }
            }
            printf("%ld nodes, %ld used records, %ld records total (%.1lf %%)\n",
                   nodes, records, nodes*tree->getM(),
                   records*100.0/(nodes*tree->getM()));
            tree->makeDot(test_config[test].dotfile);
            TEST(fa.dump(0));
            fa.detach();
            if (test_config[test].diff)
            {
                TEST_FILEDIFF(test_config[test].dotfile, test_config[test].diff);
            }
        }
    }
    catch (GenericException &e)
    {
        printf("Exception during sub-test %zu, item %zi:\n%s\n",
               test, i, e.what());
        FAIL("Exception");
    }
    TEST_OK;
}

// Dump all the blocks for the major test case
TEST_CASE dump_blocks()
{
    try
    {
        AutoFilePtr f("test/tree.tst", "rb");
        FileAllocator fa;
        fa.attach(f, RTree::anchor_size, true);
        RTree tree(fa, 0);
        tree.reattach();
        unsigned long nodes, records;
        TEST(tree.selfTest(nodes, records));
        stdString s, e;
        RTree::Datablock block;
        RTree::Node node(tree.getM(), true);
        int idx;
        bool ok;
        for (ok = tree.getFirstDatablock(node, idx, block);
             ok;
             ok = tree.getNextDatablock(node, idx, block))
        {
            printf("%s - %s: '%s' @ 0x%lX\n",
                   epicsTimeTxt(node.record[idx].start, s),
                   epicsTimeTxt(node.record[idx].end, e),
                   block.data_filename.c_str(),
                   (unsigned long)block.data_offset);
            while (tree.getNextChainedBlock(block))
                printf("---  '%s' @ 0x%lX\n",
                       block.data_filename.c_str(),
                       (unsigned long)block.data_offset);
        }
        fa.detach();
    }
    catch (GenericException &e)
    {
        printf("Exception:\n%s\n", e.what());
        FAIL("Exception");
    }
    TEST_OK;
}

#ifdef INDEX_TEST
static TestData update_data[] =
{
    { "200", "210", "-last-", 1 }, // Last engine's last block
    { "209", "209", "-new-",  1 }, // New engine's block, hidden
    { "209", "220", "-new-",  1 }, // .. growing
    { "209", "230", "-new-",  1 }, // .. growing
    { "230", "270", "-new-",  1 }, // .. unreal, but handled as growing.
    { "280", "280", "-new2-", 1 }, // Second block of new engine, one sample 
    { "280", "281", "-new2-", 1 }, // .. growing
    { "280", "282", "-new2-", 1 }, // .. growing
    { "280", "283", "-new2-", 1 }, // .. growing
    { "280", "290", "-new2-", 1 }, // .. growing
};

TEST_CASE update_test()
{
    size_t i = 0, num = sizeof(update_data)/sizeof(TestData);

    TEST_DELETE_FILE("test/update.tst");
    try
    {
        IndexFile index(10);
        stdString directory;
        FileAllocator::minimum_size = 0;
        FileAllocator::file_size_increment = 0;
        FileAllocator fa;
        index.open("test/update.tst", false);
    
        AutoPtr<RTree> tree(index.addChannel("test", directory));
        TEST_MSG(tree, "Added Channel");
    
        unsigned long nodes, records;
        TEST_MSG(tree->selfTest(nodes, records), "Initial Self Test");
    
        epicsTime start, end;
        for (i=0; i<num; ++i)
        {
            if (i==(num-1))
                tree->makeDot("update0.dot");
            string2epicsTime(update_data[i].start, start);
            string2epicsTime(update_data[i].end, end);
            stdString filename = update_data[i].file;
            tree->updateLastDatablock(start, end, update_data[i].offset, filename);
            if (i==(num-1))
                tree->makeDot("update1.dot");
            if (!tree->selfTest(nodes, records))
            {
                FAIL("Self test");
            }
        }
        tree->makeDot("test/update_data.dot");
        TEST_MSG(tree->selfTest(nodes, records), "Final Self Test");
        TEST_FILEDIFF("test/update_data.dot", "test/update_data.dot.OK");
    }
    catch (GenericException &e)
    {
        printf("Exception while processing item %zu:\n%s\n", i, e.what());
        FAIL("Exception");
    }

    TEST_OK;
}
#endif

