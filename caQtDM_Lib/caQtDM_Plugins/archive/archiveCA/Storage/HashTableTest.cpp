// Tools
#include <UnitTest.h>
// Local
#include "HashTable.h"
  
TEST_CASE hash_table_test()
{
    // Don't know what to test other than assert
    // that we get the same result as we got
    // when the test was created.
    TEST(0x0045 == HashTable::Hash("Fred Flintstone"));
    TEST(0x0000 == HashTable::Hash("aaaaaaaaaa"));
    TEST(0x004D == HashTable::Hash("zzzzzzzzz"));
    TEST(0x0045 == HashTable::Hash("Fred Flintstone"));
    TEST(0x0000 == HashTable::Hash("aaaaaaaaaa"));
    TEST(0x004D == HashTable::Hash("zzzzzzzzz"));

    TEST_OK;
}


