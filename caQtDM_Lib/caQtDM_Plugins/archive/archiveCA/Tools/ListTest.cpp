
// Tools
#include "MsgLogger.h"
#include "GenericException.h"
#include "ConcurrentList.h"
#include "UnitTest.h"

TEST_CASE test_concurrent_list()
{
    stdString fred("fred");
    stdString freddy("freddy");
    stdString jane("jane");
    stdString janet("janet");
    stdString bob("bob");
    
    ConcurrentList<stdString> subscribers;
    TEST(subscribers.isEmpty() == true);
    TEST(subscribers.size()    == 0);
    TEST(subscribers.removeIfFound(&bob) == false);
    subscribers.add(&fred);
    TEST(subscribers.isEmpty() == false);
    TEST(subscribers.size()    == 1);
    subscribers.add(&freddy);
    TEST(subscribers.isEmpty() == false);
    TEST(subscribers.size()    == 2);
    subscribers.add(&jane);
    TEST(subscribers.isEmpty() == false);
    TEST(subscribers.size()    == 3);
    subscribers.add(&janet);
    TEST(subscribers.isEmpty() == false);
    TEST(subscribers.size()    == 4);
    
    TEST(subscribers.removeIfFound(&bob) == false);
    
    // This test assumes a certain order in which
    // elements are added to the list,
    // which could change with different implementations
    // of the ConcurrentList.
    
    COMMENT("Simple Iteration");
    ConcurrentListIterator<stdString> s(subscribers.iterator());
    TEST(*s.next() == "fred");
    TEST(*s.next() == "freddy");
    TEST(*s.next() == "jane");
    TEST(*s.next() == "janet");
    TEST(s.next() == 0);
    TEST(s.next() == 0);
    TEST(s.next() == 0);
    TEST(s.next() == 0);
        
    COMMENT("Iteration where 'fred' is removed while iterator is on it");
    // Start over: Position on first entry, "fred"
    s = subscribers.iterator();
    // Remove element, ...
    subscribers.remove(&fred);
    // but iterator was already on the element, so you still get it:
    TEST(*s.next() == "fred");
    TEST(*s.next() == "freddy");
    TEST(*s.next() == "jane");
    TEST(*s.next() == "janet");
    TEST(s.next() == 0);
    TEST(s.next() == 0);

    COMMENT("Iteration where 'fred' is gone, but 'bob' was added.");
    COMMENT("Then add 'fred' again while iterating.");
    subscribers.add(&bob);
    s = subscribers.iterator();
    TEST(*s.next() == "bob");
    TEST(*s.next() == "freddy");
    subscribers.add(&fred);
    TEST(*s.next() == "jane");
    TEST(*s.next() == "janet");
    TEST(*s.next() == "fred");
    TEST(s.next() == 0);
    TEST(s.next() == 0);
    
    TEST_OK;
}

TEST_CASE test_list()
{
    printf("------------------------------------------\n");
    printf("List Test\n");
    printf("------------------------------------------\n");

    stdList<stdString> list;
    
    list.push_back(stdString("A"));
    list.push_back(stdString("B"));
    list.push_back(stdString("C"));

    printf("Dump\n");
    TEST(list.size() == 3);
    stdList<stdString>::iterator i;
    for (i = list.begin(); i!=list.end(); ++i)
    {
        printf("        %s\n", i->c_str());
    }
    
    printf("Append while traversing\n");
    i = list.begin();
    printf("        %s\n", i->c_str());
    ++i;
    list.push_back(stdString("D added while traversing"));
    for (/**/; i!=list.end(); ++i)
    {
        printf("        %s\n", i->c_str());
    }

#if 0
    printf("Delete 'A' while traversing\n");
    i = list.begin();
    // This results in crash, since we remove an element
    list.pop_front();
    // ... and then try to access the deleted element:
    printf("        %s\n", i->c_str());
    ++i;
    for (/**/; i!=list.end(); ++i)
    {
        printf("        %s\n", i->c_str());
    }
#endif

    TEST_OK;
}

