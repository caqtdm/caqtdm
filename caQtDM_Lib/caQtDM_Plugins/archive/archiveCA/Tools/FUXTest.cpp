
#include "FUX.h"
#include "GenericException.h"
#include "UnitTest.h"

TEST_CASE test_fux()
{
    FUX fux;
    FUX::Element *xml_doc = 0;

    try
    {
        xml_doc = fux.parse("does_not_exist.xml");
        xml_doc = (FUX::Element *)2;
    }
    catch (GenericException &e)
    {
        TEST("Caught Exception:");
        printf("%s", e.what());
    }
    TEST(xml_doc != (FUX::Element *)2);

    xml_doc = fux.parse("test.xml");
    TEST(xml_doc != 0);
    fux.DTD="../Engine/engineconfig.dtd";
    fux.dump(stdout);
    TEST(xml_doc->find("write_period") != 0);
    TEST(xml_doc->find("group") != 0);
    TEST(xml_doc->find("group")->getName() == "group");
    TEST(xml_doc->find("group")->getValue().empty());
    TEST(xml_doc->find("channel") == 0);

    FUX::Element *group = xml_doc->find("group");
    TEST(group->getParent() == xml_doc);
    TEST(group->find("name") != 0);
    TEST(group->find("name")->getValue() == "Vac");
    TEST(group->find("channel") != 0);
    TEST(group->find("channel")->find("name") != 0);
    TEST(group->find("channel")->find("name")->getValue() == "vac1");
    TEST(group->find("channel")->find("period")->getValue() == "1");
    TEST(group->find("quark") == 0);

    try
    {
        xml_doc = 0;
        xml_doc = fux.parse("damaged.xml");
        xml_doc = (FUX::Element *)2;
    }
    catch (GenericException &e)
    {
        TEST("Caught Exception:");
        printf("%s", e.what());
    }
    TEST(xml_doc != (FUX::Element *)2);
    TEST_OK;
}

