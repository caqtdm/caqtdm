
// Base
#include <epicsThread.h>
// Tools
#include <UnitTest.h>
#include <Throttle.h>

TEST_CASE test_throttle()
{
     Throttle throttle("Test", 2.0);
     // OK to print one message.
     TEST(throttle.isPermitted());    
     // Then, messages are discouraged...
     TEST(!throttle.isPermitted());    

     TEST_MSG(1, "waiting a little bit...");
     epicsThreadSleep(2.5);
     // until one waited at least 2 seconds:
     TEST(throttle.isPermitted());    

     // immediately afterwards it's blocked...
     TEST(!throttle.isPermitted());    
     // unless reset
     throttle.reset();
     TEST(throttle.isPermitted());    
     TEST(!throttle.isPermitted());    

     TEST_MSG(1, "waiting a little bit...");
     epicsThreadSleep(2.5);
     // now it should be permitted, but we fire it:
     throttle.fire();
     TEST(!throttle.isPermitted());    

     TEST_OK;
}

