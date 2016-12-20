// System
#include <stdio.h>
// Tools
#include <UnitTest.h>
// Storage
#include "IndexFile.h"
#include "RawDataReader.h"
#include "SpreadsheetReader.h"
#include "DataFile.h"

TEST_CASE spreadsheet_dump()
{
    stdString txt, stat;
    size_t i;
    IndexFile index;
    index.open("../DemoData/index");
    {
        SpreadsheetReader sheet(index);

        stdVector<stdString> names;
        names.push_back("fred");
        names.push_back("jane");

        bool found_any = sheet.find(names);
        TEST_MSG(found_any, "Found channels");
        TEST(sheet.getNum() == names.size());

        while (found_any)
        {   // Line: time ... 
            epicsTime2string(sheet.getTime(), txt);
            printf("%s", txt.c_str());
            // .. values ...
            for (i=0; i<sheet.getNum(); ++i)
            {
                const RawValue::Data *value = sheet.get(i);
                if (value)
                {
                    RawValue::getValueString(txt,
                                             sheet.getType(i), sheet.getCount(i),
                                             value, & sheet.getInfo(i));
                    RawValue::getStatus(value, stat);
                    printf(" | %20s %20s", txt.c_str(), stat.c_str());
                }
                else
                    printf(" | #N/A                                    ");
    
            }
            printf("\n");
            found_any = sheet.next();
        }
    }
    TEST(DataFile::clear_cache() == 0);
    TEST_OK;
}


TEST_CASE spreadsheet_values()
{
    stdString txt, stat;
    size_t i;
    IndexFile index;
    index.open("../DemoData/index");

    // We happen to know that pv2 starts earlier...
    stdString pv1("DoublePV");
    stdString pv2("DTL_HPRF:Tnk1:T");
    epicsTime start;
    {
        RawDataReader raw(index);
        const RawValue::Data *value = raw.find(pv2, 0);
        start = RawValue::getTime(value);
    }
    {
        SpreadsheetReader sheet(index);

        stdVector<stdString> names;
        names.push_back(pv1);
        names.push_back(pv2);
        
        bool found_any = sheet.find(names, &start);  

        // All should be found:
        TEST_MSG(found_any, "Found channels");
        TEST(sheet.getNum() == names.size());
        TEST(sheet.found(0));
        TEST(sheet.found(1));
        // p1 has no data for now
        TEST(!sheet.get(0));
        TEST( sheet.get(1));

        while (found_any)
        {   // Line: time ... 
            epicsTime2string(sheet.getTime(), txt);
            printf("%s", txt.c_str());
            // .. values ...
            for (i=0; i<sheet.getNum(); ++i)
            {
                const RawValue::Data *value = sheet.get(i);
                if (value)
                {
                    RawValue::getValueString(txt,
                                             sheet.getType(i), sheet.getCount(i),
                                             value, & sheet.getInfo(i));
                    RawValue::getStatus(value, stat);
                    printf(" | %20s %20s", txt.c_str(), stat.c_str());
                }
                else
                    printf(" | #N/A                                    ");
    
            }
            printf("\n");
            found_any = sheet.next();
        }
    }
    TEST(DataFile::clear_cache() == 0);
    TEST_OK;
}
