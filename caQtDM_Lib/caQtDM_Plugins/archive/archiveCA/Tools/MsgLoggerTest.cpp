

#include "MsgLogger.h"
#include "UnitTest.h"

TEST_CASE test_log()
{
    printf("\nLog Test\n");
    printf("------------------------------------------\n");
    {
        MsgLogger log("msg_log_test.txt");
        LOG_MSG("Hi\n");
        LOG_MSG("Test went %c%c%s", 'O', 'K', "\n");
        LOG_MSG("You should see this message in the log file,\n"
                "not on the screen\n");
    }
    LOG_MSG("You should see this message on the screen,\n"
            "not in the log file\n");
    TEST("Check 'msg_log_test.txt' for result of MsgLogger test");
    TEST_OK;
}

