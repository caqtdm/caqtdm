
#include "AutoFilePtr.h"
#include "BinIO.h"
#include "UnitTest.h"

TEST_CASE bin_io_write()
{
    AutoFilePtr f("/tmp/bin_io_test.bin", "w");
    TEST(writeLong(f, 0x01234567));
    TEST(writeShort(f, 0x0123));
    TEST(writeByte(f, 0x01));
    TEST_OK;
}

TEST_CASE bin_io_read()
{
    uint8_t i8;
    AutoFilePtr f("/tmp/bin_io_test.bin", "r");
    // long
    TEST(readByte(f, &i8) && i8 == 0x01);
    TEST(readByte(f, &i8) && i8 == 0x23);
    TEST(readByte(f, &i8) && i8 == 0x45);
    TEST(readByte(f, &i8) && i8 == 0x67);
    // short
    TEST(readByte(f, &i8) && i8 == 0x01);
    TEST(readByte(f, &i8) && i8 == 0x23);
    // byte
    TEST(readByte(f, &i8) && i8 == 0x01);
    // end
    TEST(readByte(f, &i8) == false);
    TEST_OK;
}

