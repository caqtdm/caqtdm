
#include "Bitset.h"
#include "UnitTest.h"

TEST_CASE test_bitset()
{
    BitSet s;
    TEST(strcmp(s.to_string().c_str(), "") == 0);
    s.grow(10);
    TEST(strcmp(s.to_string().c_str(), "0") == 0);
    s.set(1);
    s.set(2, true);
    s.set(0);
    TEST(strcmp(s.to_string().c_str(), "111") == 0);
    s.grow(40);
    TEST(strcmp(s.to_string().c_str(), "111") == 0);
    TEST(s.count() == 3);
    TEST(s.any() == true);
    s.clear(0);
    s.set(2, false);
    s.clear(1);
    TEST(s.count() == 0);
    TEST(s.any() == false);
    s.set(30);
    TEST(strcmp(s.to_string().c_str(), "1000000000000000000000000000000") == 0);
    TEST(s.test(30) == true);
    TEST(s.test(3) == false);
    TEST(s.capacity() == 64);
    TEST_OK;
}

