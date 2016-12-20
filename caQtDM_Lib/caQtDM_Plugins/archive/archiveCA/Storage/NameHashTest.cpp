
// Tools
#include <BinIO.h>
#include <AutoFilePtr.h>
#include <GenericException.h>
#include <UnitTest.h>
// Index
#include "NameHash.h"

TEST_CASE name_hash_test()
{
    AutoFilePtr f("test/names.ht", "w+b");
    TEST_MSG(f, "Created file");

    try
    {
        FileAllocator fa;
        TEST_MSG(fa.attach(f, NameHash::anchor_size, true) == true, "FileAllocator initialized");
        NameHash names(fa, 0);
        stdString ID_txt;
        ID_txt = "ID";
        names.init(7);
        TEST(names.insert("fred",   ID_txt, 1) == true);
        TEST(names.insert("freddy", ID_txt, 2) == true);
        TEST(names.insert("james",  ID_txt, 2) == true);
        TEST(names.insert("james",  ID_txt, 3) == false);
        TEST(names.insert("James",  ID_txt, 4) == true);

        IndexFileOffset ID;
        TEST(names.find("freddy", ID_txt, ID));
        TEST(ID == 2);

        NameHash::Entry entry;
        uint32_t hashvalue;
        bool valid = names.startIteration(hashvalue, entry);
	size_t name_count = 0;
        while (valid)
        {
            ++name_count;
            printf("        Hash: %4ld - Name: %-30s ID: %ld\n",
                   (long)hashvalue, entry.name.c_str(), (long)entry.ID);
            valid = names.nextIteration(hashvalue, entry);
        }
        TEST(name_count == 4);
        names.showStats(stdout);
        fa.detach();
    }
    catch (GenericException &e)
    {
        printf("Error: %s", e.what());
        FAIL("I didn't expect the Spanish Inquisition nor this exception");
    }
    TEST_OK;
}

