
// System
#include <stdint.h>
// Tools
#include "Conversions.h"
#include "UnitTest.h"

TEST_CASE test_conversions()
{
    uint8_t raw[] = { 0x01, 0x23, 0x45, 0x67 };
    uint32_t i32 = *( (uint32_t *)raw );
    uint16_t i16 = *( (uint16_t *)raw );

    ULONGFromDisk(i32);
    USHORTFromDisk(i16);
    TEST_MSG(i32 == 0x01234567, "Convert 32 bit number");
    TEST_MSG(i16 == 0x0123, "Convert 16 bit number");
    TEST_OK;
}

