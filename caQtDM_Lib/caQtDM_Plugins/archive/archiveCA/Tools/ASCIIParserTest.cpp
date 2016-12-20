#include "ASCIIParser.h"
#include "UnitTest.h"

TEST_CASE test_ascii_parser()
{   
    ASCIIParser parser;
    stdString parm, val;

    TEST(parser.open("ascii_test.txt"));
    TEST(parser.nextLine());
    TEST(parser.getLine() == "fred");
    TEST(parser.nextLine());
    TEST(parser.getLine() == "Jane Doe");
    TEST(parser.nextLine());
    TEST(parser.getParameter(parm, val));
    TEST(parm == "parm");
    TEST(val == "42");
    TEST(parser.nextLine());
    TEST(parser.getParameter(parm, val));
    TEST(parm == "another");
    TEST(val == "x");
    TEST(parser.nextLine());
    TEST(parser.getParameter(parm, val));
    TEST(parm == "a");
    TEST(val == "b");
    TEST(parser.nextLine());
    TEST(parser.getParameter(parm, val) == false);
    TEST(parser.nextLine());
    TEST(parser.getParameter(parm, val) == false);
    TEST_OK;
}

