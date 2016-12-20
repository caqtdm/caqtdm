
#include<cadef.h>

#include "ToolsConfig.h"
#include "MsgLogger.h"
#include "UnitTest.h"

typedef enum
{
    not_connected,
    subscribed,
    got_data
} CA_State;

static CA_State state = not_connected;
static evid event_id;
static size_t num_monitors = 0;

static void event_cb(struct event_handler_args args)
{
    state = got_data;
    ++num_monitors;
    if (args.type == DBF_DOUBLE)
    {
        LOG_MSG("PV %s = %g in thread 0x%08lX\n",
                (const char *)args.usr, *((double *)args.dbr),
                (unsigned long)epicsThreadGetIdSelf());

    }
    else
    {
        LOG_MSG("PV %s type %ld in thread 0x%08lX\n",
                (const char *)args.usr, args.type,
                (unsigned long)epicsThreadGetIdSelf());
    }
}

static void connection_cb(struct connection_handler_args args)
{
    LOG_MSG("PV %s connection_cb %s in thread 0x%08lX\n",
            (const char *)ca_puser(args.chid),
            (args.op == CA_OP_CONN_UP ? "UP" : "DOWN"),
            (unsigned long)epicsThreadGetIdSelf());

    if (args.op == CA_OP_CONN_UP &&
        state < subscribed)
    {
        ca_add_masked_array_event(ca_field_type(args.chid),
                                  ca_element_count(args.chid),
                                  args.chid,
                                  event_cb, ca_puser(args.chid),
                                  0.0, 0.0, 0.0, &event_id,
                                  DBE_VALUE | DBE_ALARM | DBE_LOG);
        state = subscribed;
    }
}

TEST_CASE test_ca()
{
    printf("\nChannelAccess Tests (requires 'janet' from excas)\n");
    SEVCHK(ca_context_create(ca_enable_preemptive_callback),
           "ca_context_create");

    LOG_MSG("Connecting from thread 0x%08lX\n",
            (unsigned long)epicsThreadGetIdSelf());

    chid chid;
    SEVCHK(ca_create_channel("janet",
                             connection_cb, (void *) "janet",
                             CA_PRIORITY_ARCHIVE,
                             &chid),
           "ca_create_channel");
    SEVCHK(ca_flush_io(), "ca_pend_io");
    epicsThreadSleep(5.0);

    LOG_MSG("Cleanup from thread 0x%08lX\n",
            (unsigned long)epicsThreadGetIdSelf());
    ca_clear_channel(chid);
    ca_context_destroy();

    TEST(state == got_data);
    printf("Received %zd monitored values\n",
           num_monitors);
    // Result depends on machine load.
    // Hope to get 50% of the values:
    TEST(abs(num_monitors - 50) < 25);
    TEST_OK;
}


