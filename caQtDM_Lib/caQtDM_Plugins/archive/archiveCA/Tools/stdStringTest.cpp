
#include "ToolsConfig.h"
#include "GenericException.h"
#include "UnitTest.h"

static stdString addWorld(const stdString &in)
{
    stdString    result;
    result = in;
    result += "World !";
    return result;
}

static int findChar(stdString s, char c)
{
    stdString::size_type pos = s.find_last_of(c);
    if (pos == s.npos)
        return -1;
    else
        return pos;
}


TEST_CASE test_string()
{
    stdString    a;
    a = stdString("Hello ") + "W";
    a += "o";
    a.append("rldxxx", 3);
    a += stdString("!");
    TEST(strcmp(a.c_str(), "Hello World!")==0);
    TEST(a.length() == 12);

    TEST(a[0] == 'H');
    TEST(a[11] == '!');
    try
    {
         char c = a[20];
         TEST(c == '?');
    }
    catch (GenericException &got_exception)
    {
        TEST(&got_exception != 0);
    }
    TEST(a.length() == strlen(a.c_str()));

    a = " World!";
    a = "Hello" + a;
    TEST(strcmp(a.c_str(), "Hello World!")==0);
    TEST(a.length() == 12);
    TEST(a.length() == strlen(a.c_str()));

    a = "Ho ";
    TEST(strcmp(a.c_str(), "Ho ")==0);
    TEST(a.length() == 3);
    TEST(a.length() == strlen(a.c_str()));


    stdString b = addWorld(a);
    TEST(strcmp(b.c_str(), "Ho World !")==0);
    TEST(b.length() == 10);
    TEST(b.length() == strlen(b.c_str()));

    TEST(findChar(b, 'H') == 0);
    TEST(findChar(b, 'W') == 3);
    TEST(findChar(b, 'x') == -1);
    TEST(findChar(b, '!') == 9);
    TEST(findChar(b, 'o') == 4);
    TEST(b.substr(findChar(b, 'W'), 5) == stdString("World"));
    TEST_OK;
}

