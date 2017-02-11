
// Base
#include <epicsThread.h>
// Tools
#include <UnitTest.h>
#include <ThrottledMsgLogger.h>

TEST_CASE test_msg_throttle()
{
     ThrottledMsgLogger throttle("Test", 2.0);
     // OK to print one message.
     throttle.LOG_MSG("Hello!\n");
     // Then, messages are discouraged...
     TEST(!throttle.isPermitted());    
     throttle.LOG_MSG("Hello, too!\n");
     throttle.LOG_MSG("Hello, too!\n");
     throttle.LOG_MSG("Hello, too!\n");
     throttle.LOG_MSG("Hello, too!\n");
     throttle.LOG_MSG("Hello, too!\n");

     TEST_MSG(1, "waiting a little bit...");
     epicsThreadSleep(2.5);
     throttle.LOG_MSG("Hello again\n");

     TEST_OK;
}

