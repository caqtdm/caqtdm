// Tools
#include <UnitTest.h>
// Storage
#include "RawValue.h"

// Not a full test because the formatted output
// isn't compared to the expected result;
// Only checking string length for consistency.
static bool fmt(double d)
{
    char buffer[50];
    size_t l;

    printf("\n");
    l = RawValue::formatDouble(d, RawValue::DEFAULT, 6, buffer, sizeof(buffer));
    printf("DEFAULT   : '%s' (%u)\n", buffer, (unsigned)l);
    if (strlen(buffer) != l) { FAIL("String length"); }

    l = RawValue::formatDouble(d, RawValue::DECIMAL, 6, buffer, sizeof(buffer));
    printf("DECIMAL   : '%s' (%u)\n", buffer, (unsigned)l);
    if (strlen(buffer) != l) { FAIL("String length"); }

    l = RawValue::formatDouble(d, RawValue::ENGINEERING, 6, buffer, sizeof(buffer));
    printf("ENGINEERING: '%s' (%u)\n", buffer, (unsigned)l);
    if (strlen(buffer) != l) { FAIL("String length"); }

    l = RawValue::formatDouble(d, RawValue::EXPONENTIAL, 6, buffer, sizeof(buffer));
    printf("EXPONENTIAL: '%s' (%u)\n", buffer, (unsigned)l);
    if (strlen(buffer) != l) { FAIL("String length"); }

    TEST_OK;
}

TEST_CASE RawValue_format()
{
    TEST(fmt(0.0));
    TEST(fmt(-0.321));
    TEST(fmt(1.0e-12));
    TEST(fmt(-1.0e-12));
    TEST(fmt(3.14e-7));
    TEST(fmt(3.14));
    TEST(fmt(3.14e+7));
    TEST(fmt(-3.14e+7));
    TEST(fmt(-0.123456789));
    TEST_OK;
}

TEST_CASE RawValue_compare()
{
    RawValueAutoPtr a(RawValue::allocate(DBR_TIME_DOUBLE, 1, 1));
    RawValueAutoPtr b(RawValue::allocate(DBR_TIME_DOUBLE, 1, 1));

    // Equal values are equal
    RawValue::setDouble(DBR_TIME_DOUBLE, 1, a, 3.14);
    RawValue::setDouble(DBR_TIME_DOUBLE, 1, b, 3.14);
    epicsTime now = epicsTime::getCurrent();
    RawValue::setTime(a, now);
    RawValue::setTime(b, now);
    TEST(RawValue::hasSameValue(DBR_TIME_DOUBLE, 1, a, b) == true);

    // Time stamp changes are still considered equal
    RawValue::setTime(b, now + 1);
    TEST(RawValue::hasSameValue(DBR_TIME_DOUBLE, 1, a, b) == true);

    // Status changes are not
    a->status = 1;
    TEST(RawValue::hasSameValue(DBR_TIME_DOUBLE, 1, a, b) == false);
    TEST_OK;
}

TEST_CASE RawValue_auto_ptr()
{
    {
        RawValueAutoPtr val;

        val = RawValue::allocate(DBR_TIME_DOUBLE, 1, 1);
        TEST_MSG(val, "Allocated");

        TEST(RawValue::setDouble(DBR_TIME_DOUBLE, 1, val, 3.14));
        
        double d;
        TEST(RawValue::getDouble(DBR_TIME_DOUBLE, 1, val, d, 0));
        TEST(d == 3.14);

        // This one was caught by valgrind as a duplicate free(),
        // though hard to build into a unit-test:
        // RawValue::free(val);

        RawValueAutoPtr copy;
        copy = val;
        TEST_MSG(!val, "AutoPtr copy released the original");

        TEST(RawValue::getDouble(DBR_TIME_DOUBLE, 1, copy, d, 0));
        TEST(d == 3.14);

        long l;
        TEST(RawValue::getLong(DBR_TIME_DOUBLE, 1, copy, l, 0));
        TEST(l == 3);

        stdString txt;
        RawValue::getValueString(txt, DBR_TIME_DOUBLE, 1, copy);
        TEST_MSG(txt.length() > 0, "Convert to string");
        printf("       -> '%s'\n", txt.c_str());
    }
    TEST_OK;
}

