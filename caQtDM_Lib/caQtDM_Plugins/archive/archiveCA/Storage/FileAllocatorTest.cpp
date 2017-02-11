// System
#include <stdlib.h>
#include <string.h>
// Tools
#include <AutoFilePtr.h>
#include <UnitTest.h>
// Local
#include "FileAllocator.h"

TEST_CASE file_allocator_create_new_file()
{
    AutoFilePtr f("test/file_allocator.dat", "w+b");
    TEST_MSG(f, "Create new file");

    FileAllocator fa;
    long o1, o2, o3, o4, o5, o6, o7;
    TEST_MSG(fa.attach(f, 1000, true) == true, "attach and initialize");
    TEST_MSG(fa.dump(), "Consistency Check\n");

    TEST_MSG((o1=fa.allocate(1000)), "allocate 1000");
    printf("Got offset %ld\n", o1);
    TEST_MSG(fa.dump(0), "Consistency Check\n");

    TEST_MSG((o2=fa.allocate(2000)), "allocate 2000");
    printf("Got offset %ld\n", o2);
    TEST_MSG(fa.dump(0), "Consistency Check\n");

    TEST_MSG((o3=fa.allocate(3000)), "allocate 3000");
    printf("Got offset %ld\n", o3);
    TEST_MSG(fa.dump(0), "Consistency Check\n");

    TEST_MSG((o4=fa.allocate(4000)), "allocate 4000");
    printf("Got offset %ld\n", o4);
    TEST_MSG(fa.dump(0), "Consistency Check\n");

    TEST_MSG((o5=fa.allocate(5000)), "allocate 5000");
    printf("Got offset %ld\n", o5);
    TEST_MSG(fa.dump(0), "Consistency Check\n");

    TEST_MSG((o6=fa.allocate(6000)), "allocate 6000");
    printf("Got offset %ld\n", o6);
    TEST_MSG(fa.dump(0), "Consistency Check\n");

    TEST_MSG((o7=fa.allocate(7000)), "allocate 7000");
    printf("Got offset %ld\n", o7);
    TEST_MSG(fa.dump(), "Consistency Check\n");

    fa.free(o1);
    TEST_MSG(fa.dump(), "Consistency Check after free 1000 (first block)\n");

    fa.free(o7);
    TEST_MSG(fa.dump(), "Consistency Check after free 7000 (last block)\n");

    fa.free(o5);
    TEST_MSG(fa.dump(), "Consistency Check after free 5000 (embedded block)\n");

    fa.free(o4);
    TEST_MSG(fa.dump(), "Consistency Check after free 4000 (adjacent block, causing merge)\n");

    fa.free(o2);
    TEST_MSG(fa.dump(), "Consistency Check after free 2000 (next to free first block)\n");

    fa.free(o3);
    TEST_MSG(fa.dump(), "Consistency Check after free 3000 (next to free first block)\n");

    TEST_MSG((o1=fa.allocate(4000)), "allocate 4000 (from first free block)");
    printf("Got offset %ld\n", o1);
    TEST_MSG(fa.dump(), "Consistency Check\n");

    TEST_MSG((o2=fa.allocate(500)), "allocate 500 (from first free block)");
    printf("Got offset %ld\n", o2);
    TEST_MSG(fa.dump(), "Consistency Check\n");

    TEST_MSG((o3=fa.allocate(1500)), "allocate 1500 (from first free block)");
    printf("Got offset %ld\n", o3);
    TEST_MSG(fa.dump(), "Consistency Check\n");

    fa.detach();
    TEST_OK;
}

TEST_CASE file_allocator_open_existing()
{
    AutoFilePtr f("test/file_allocator.dat", "r+b");
    TEST_MSG(f, "Re-opened file");

    FileAllocator fa;
    TEST_MSG(fa.attach(f, 1000, false) == false, "re-attach");
    TEST_MSG(fa.dump(), "Consistency Check");

    puts("-- upping file size increment to 50000 --");
    FileAllocator::file_size_increment = 50000;
    long o1, o2, o3;
    TEST_MSG((o1=fa.allocate(10000)), "allocate 10000");
    printf("Got offset %ld\n", o1);
    TEST_MSG(fa.dump(), "Consistency Check\n");

    TEST_MSG((o1=fa.allocate(10000)), "allocate 10000");
    printf("Got offset %ld\n", o1);
    TEST_MSG(fa.dump(), "Consistency Check\n");

    TEST_MSG((o1=fa.allocate(10000)), "allocate 10000");
    printf("Got offset %ld\n", o1);
    TEST_MSG(fa.dump(), "Consistency Check\n");

    TEST_MSG((o2=fa.allocate(5000)), "allocate 5000");
    printf("Got offset %ld\n", o2);
    TEST_MSG(fa.dump(), "Consistency Check\n");

    TEST_MSG((o3=fa.allocate(5000)), "allocate 5000");
    printf("Got offset %ld\n", o3);
    TEST_MSG(fa.dump(), "Consistency Check\n");
    {
        AutoFilePtr file("test/file_allocator.out", "wt");
        TEST_MSG(fa.dump(1, file), "Consistency Check\n");
    }
    TEST_FILEDIFF("test/file_allocator.out", "test/file_allocator.OK");

    TEST("Performing some random tests");
    long o[10];
    memset(o, 0, sizeof(o));
    int i;
    for (i=0; i<500; ++i)
    {   // Randomly free an existing block
        if (o[i%10] && rand()>RAND_MAX/2)
        {
            fa.free(o[i%10]);
            o[i%10] = 0;
        }
        else
        {   // allocate another one
            o[i%10] = fa.allocate(rand() & 0xFFFF);
            if (o[i%10] == 0)
                printf("Alloc failed\n");
        }
        if (! fa.dump(0))
            FAIL("Consistency check failed");
    }
    TEST("If you didn't see any error message, that went fine.");
    fa.detach();

    TEST_OK;
}

