#include<stdio.h>
#include"ToolsConfig.h"
#include"math.h"

// Enable/disable certain tests in case one doesn't work out
// on your architecture
#define TEST_AUTOPTR
#define TEST_STRING
#define TEST_TIME
#define TEST_AVL
#define TEST_LOG
#define TEST_THREADS
#define TEST_TIMER
#define TEST_CA
#define TEST_BITSET
#define TEST_FUX

// Nothing should need to be touched from here down

#ifndef TEST_LOG
#define LOG_MSG printf
#endif

#define TEST(t) \
       if (t)                        \
           printf("OK  : %s\n", #t); \
       else                          \
       {                             \
           printf("FAIL: %s\n", #t); \
           exit(1);                  \
       }

#ifdef TEST_AUTOPTR
#include "AutoPtr.h"

static int deletes = 0;
class X
{
public:
    X() {}

    ~X()
    {
        ++deletes;    
    }

    int val;
};

void test_autoptr()
{
    printf("\nAutoPtr Tests\n");
    printf("------------------------------------------\n");
    {
        AutoPtr<int> i(new int);
        *i = 42;
        TEST(*i == 42);
    }
    {
        AutoPtr<X> x(new X);
        x->val = 42;
        (*x).val = 43;
        
        AutoPtr<X> x2(x);
        TEST(x2->val == 43);
    }
    {
        X x;
        AutoPtr<X> p(&x);
        p.release();
        p.assign(&x);
        p.release();
    }
    {
        int i;
        for (i=0; i<5; ++i)
        {
            AutoPtr<X> x(new X);
            x->val = i;
        }
    }
    TEST(deletes == 7);
}
#endif

// -----------------------------------------------------------------
#ifdef TEST_STRING
stdString addWorld(const stdString &in)
{
	stdString	result;
	result = in;
	result += "World !";
	return result;
}

int findChar(stdString s, char c)
{
	stdString::size_type pos = s.find_last_of(c);
	if (pos == s.npos)
        return -1;
	else
        return pos;
}

void test_string()
{
#ifdef USE_STD_STRING
    printf("\nString Tests (using std::string)\n");
#else
    printf("\nString Tests (using Tools/stdstring)\n");
#endif
    printf("------------------------------------------\n");

    stdString	a;

    a = stdString("Hello ") + "W";
    a += "o";
    a.append("rldxxx", 3);
    a += stdString("!");
    TEST(strcmp(a.c_str(), "Hello World!")==0);
    TEST(a.length() == 12);
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
}

#endif

#ifdef TEST_AVL
#include "AVLTree.h"

int sort_compare(const int &a, const int &b)
{   return b-a; }

int avl_last_number;

void avl_order_test(const int &i, void *)
{
    if (i < avl_last_number)
        printf("FAIL: Error in AVL tree order\n");
    avl_last_number = i;
}

const char *toString(const int &i)
{
    static char txt[10];
    sprintf(txt, "%d", i);
    return txt;
}   

void avl_test()
{
    printf("\nAVLTree Test\n");
    printf("------------------------------------------\n");
    AVLTree<int> tree;

    tree.add(RAND_MAX/2);
    int i, num;

    for (i=0; i<100; ++i)
    {
        num = rand();
        if (tree.find(i))
            continue;
        tree.add(num);
        TEST(tree.find(num));
        TEST(tree.selftest());
    }
    tree.make_dotfile("avl");
    avl_last_number = -10000;
    tree.traverse(avl_order_test, 0);
    tree.make_dotfile("avl");
    printf("Check avl.dot if you care\n");
}

#endif


// -----------------------------------------------------------------
#ifdef TEST_LOG
#include<MsgLogger.h>

void my_log(void *arg, const char *text)
{
    fputs(text, (FILE *)arg);
}
    
void test_log()
{
    printf("\nLog Test\n");
    printf("------------------------------------------\n");
    FILE *log_fd = fopen("msg_log_test.txt", "w");
    TheMsgLogger.SetPrintRoutine(my_log, log_fd);

    LOG_MSG("Hi\n");
    LOG_MSG("Test went %c%c%s", 'O', 'K', "\n");
    fclose(log_fd);
    TheMsgLogger.SetDefaultPrintRoutine();
    TEST("Check 'msg_log_test.txt' for result of MsgLogger test");
}
#endif

// -----------------------------------------------------------------
#ifdef TEST_TIME
#include"epicsTimeHelper.h"

void test_time()
{
    printf("\nTime Tests\n");
    printf("------------------------------------------\n");

    initEpicsTimeHelper();

    epicsTime t;
    TEST(isValidTime(t) == false);
    
    t = epicsTime::getCurrent();
    epicsTime start = t;
    t.show(10);
    TEST(isValidTime(t) == true);
    
    epicsTimeStamp stamp;
    memset(&stamp, 0, sizeof(epicsTimeStamp));
    t = stamp;
    TEST(isValidTime(t) == false);
    
    epicsTime now = epicsTime::getCurrent();
    stdString s_start, s_now;
    epicsTime2string(start, s_start);
    epicsTime2string(now, s_now);
    //printf("Start: %s\n", s_start.c_str());
    //printf("Now:   %s\n", s_now.c_str());
    TEST(start <= now);
    TEST(now > start);
    t = now;
    TEST(t == now);

    int year, month, day, hour, min, sec;
    unsigned long nano;
    epicsTime2vals(start, year, month, day, hour, min, sec, nano);
    char buf[50];
    sprintf(buf, "%02d/%02d/%04d %02d:%02d:%02d.%09lu",
            month, day, year, hour, min, sec, nano);
    puts(buf);
    TEST(strcmp(s_start.c_str(), buf) == 0);

    vals2epicsTime(year, month, day, hour, min, sec, nano, t);
    TEST(t == start);

    t.show(10);
    start.show(10);
    
    t += 60;
    TEST(t != start);
    TEST(t > start);
    TEST(t - start == 60.0);

    stdString txt;
    vals2epicsTime(1990, 3, 18, 12, 13, 44, 800000019L, now);
    epicsTime2string(now, txt); puts(txt.c_str());           TEST(txt == "03/18/1990 12:13:44.800000019");
    epicsTime2string(roundTimeDown(now, 0), txt);            TEST(txt == "03/18/1990 12:13:44.800000019");
    epicsTime2string(roundTimeDown(now, 0.5), txt);          TEST(txt == "03/18/1990 12:13:44.500000000");
    epicsTime2string(roundTimeDown(now, 1.0), txt);          TEST(txt == "03/18/1990 12:13:44.000000000");
    epicsTime2string(roundTimeDown(now, 10.0), txt);         TEST(txt == "03/18/1990 12:13:40.000000000");
    epicsTime2string(roundTimeDown(now, 30.0), txt);         TEST(txt == "03/18/1990 12:13:30.000000000");

    epicsTime2string(roundTimeDown(now, 50.0), txt);
    printf("Rounded by 50 secs: %s\n", txt.c_str());

    epicsTime2string(roundTimeDown(now, 60.0), txt);         TEST(txt == "03/18/1990 12:13:00.000000000");
    epicsTime2string(roundTimeDown(now, secsPerHour), txt);  TEST(txt == "03/18/1990 12:00:00.000000000");
    epicsTime2string(roundTimeDown(now, secsPerDay), txt);   TEST(txt == "03/18/1990 00:00:00.000000000");
    epicsTime2string(roundTimeDown(now, secsPerMonth), txt); TEST(txt == "03/01/1990 00:00:00.000000000");
    epicsTime2string(roundTimeDown(now, secsPerYear), txt);  TEST(txt == "01/01/1990 00:00:00.000000000");

    vals2epicsTime(1990, 3, 18, 12, 13, 44, 800000019L, now);
    epicsTime2string(roundTimeUp(now, 0), txt);            TEST(txt == "03/18/1990 12:13:44.800000019");
    epicsTime2string(roundTimeUp(now, 0.1), txt);          TEST(txt == "03/18/1990 12:13:44.900000000");    
    epicsTime2string(roundTimeUp(now, 0.5), txt);          TEST(txt == "03/18/1990 12:13:45.000000000");    
    epicsTime2string(roundTimeUp(now, 1.0), txt);          TEST(txt == "03/18/1990 12:13:45.000000000");
    epicsTime2string(roundTimeUp(now, 10.0), txt);         TEST(txt == "03/18/1990 12:13:50.000000000");
    epicsTime2string(roundTimeUp(now, 30.0), txt);         TEST(txt == "03/18/1990 12:14:00.000000000");
    
    epicsTime2string(roundTimeUp(now, 50.0), txt);
    printf("Rounded by 50 secs: %s\n", txt.c_str());
    
    epicsTime2string(roundTimeUp(now, 60.0), txt);         TEST(txt == "03/18/1990 12:14:00.000000000");
    epicsTime2string(roundTimeUp(now, secsPerHour), txt);  TEST(txt == "03/18/1990 13:00:00.000000000");
    
    epicsTime2string(roundTimeUp(now, secsPerDay), txt);   TEST(txt == "03/19/1990 00:00:00.000000000");
    epicsTime2string(roundTimeUp(now, secsPerMonth), txt); TEST(txt == "04/01/1990 00:00:00.000000000");
    epicsTime2string(roundTimeUp(now, secsPerYear), txt);  TEST(txt == "01/01/1991 00:00:00.000000000");
    
    vals2epicsTime(2000, 12, 31, 23, 59, 59, 999999999L, now);
    epicsTime2string(roundTimeDown(now, 0), txt);            TEST(txt == "12/31/2000 23:59:59.999999999");
    epicsTime2string(roundTimeDown(now, secsPerDay), txt);   TEST(txt == "12/31/2000 00:00:00.000000000");
    epicsTime2string(roundTimeDown(now, secsPerYear), txt);  TEST(txt == "01/01/2000 00:00:00.000000000");

    vals2epicsTime(1990, 3, 18, 23, 13, 44, 800000019L, now);
    epicsTime2string(roundTimeUp(now, 60*60*1), txt);        TEST(txt == "03/19/1990 00:00:00.000000000");
    epicsTime2string(roundTimeUp(now, 60*60*2), txt);        TEST(txt == "03/19/1990 01:00:00.000000000");

    vals2epicsTime(1990, 3, 18, 01, 13, 44, 800000019L, now);
    epicsTime2string(roundTimeUp(now, 60*60*1), txt);        TEST(txt == "03/18/1990 02:00:00.000000000");

    printf("Rounding up by 15 minutes:\n");
    vals2epicsTime(1990, 3, 18, 01, 13, 44, 800000019L, now);
    now = roundTimeUp(now, 900);    printf("%s\n", epicsTimeTxt(now, txt));
    now = roundTimeUp(now, 900);    printf("%s\n", epicsTimeTxt(now, txt));
    now = roundTimeUp(now, 900);    printf("%s\n", epicsTimeTxt(now, txt));
    now = roundTimeUp(now, 900);    printf("%s\n", epicsTimeTxt(now, txt));
    now = roundTimeUp(now, 900);    printf("%s\n", epicsTimeTxt(now, txt));
    now = roundTimeUp(now, 900);    printf("%s\n", epicsTimeTxt(now, txt));
}

#endif

// -----------------------------------------------------------------
#ifdef TEST_THREADS
// Test for the Thread code,
// basically this is the "Dining Philosophers" Example.
//
// Run it, stop with CTRL-C (takes some time to finish).
// The result should look like this:
//
// ./ThreadTest
//  A eating...
//  C eating...
//  E eating...
//  B eating...
//  D eating...
//  A eating...
//  ... and so on, then at the end:
//  A: got 3 chances
//  B: got 3 chances
//  C: got 3 chances
//  D: got 3 chances
//  E: got 3 chances
//  F: got 2 chances
//
// Point: all should have similar numbers.

#include<signal.h>
#include<epicsThread.h>
#include<epicsMutex.h>

bool go = true;

void quit(int)
{
    go = false;
}

const char *names[] =
{
    "A",
    "B",
    "C",
    "D",
    "E",
};
#define NUM ((int)(sizeof(names)/sizeof(const char *)))
#define TURN_LIMIT 40
epicsMutex forks[NUM];
class Philosopher *phils[NUM];

class Philosopher : public epicsThreadRunable
{
public:
    Philosopher(const char *name, int left, int right)
            : _thread(*this, name,
                      epicsThreadGetStackSize(epicsThreadStackSmall),
                      epicsThreadPriorityMedium)
    {
        _name = name;
        _left = left;
        _right = right;
        _count = 0;
    }

    void run()
    {
        while (go && _count < TURN_LIMIT)
        {
            forks[_left].lock();
            forks[_right].lock();
            LOG_MSG("%s eating, utilizing %s and %s\n",
                    _name, names[_left], names[_right]);
            ++_count;
            _thread.sleep(0.1);
            forks[_right].unlock();
            forks[_left].unlock();
            _thread.sleep(0.1);
        }
    }

    void start()
    {
        _thread.start();
    }

    void join()
    {
        _thread.exitWait();
    }

    void info()
    {
        LOG_MSG("%s got %d chances\n", _name, _count);
    }

private:
    epicsThread _thread;
    const char *_name;
    int _count;
    int _left, _right;
};

class Worker : public epicsThreadRunable
{
public:
    Worker()
            : _thread(*this, "Worker",
                      epicsThreadGetStackSize(epicsThreadStackSmall),
                      epicsThreadPriorityMedium)
    {
        _event = new epicsEvent(epicsEventEmpty);
        _worker_did_work = 0;
        _go = true;
        _thread.start();
    }
    ~Worker()
    {
        delete _event;
    }

    void run()
    {
        while (_go)
        {
            _event->wait();
            if (_go)
            {
                ++_worker_did_work;
                LOG_MSG("Worker handling request #%d\n", _worker_did_work);
            }
        }
    }

    void stop()
    {
        _go = false;
        _event->signal();
        _thread.exitWait();
    }
    
    void do_something()
    {
        _event->signal();
    }

    void test()
    {
        TEST(_worker_did_work == 5);
    }

private:
    epicsThread _thread;
    epicsEvent  *_event;
    bool        _go;
    int         _worker_did_work;
};

void test_threads()
{
    int i;

    printf("\nThread Tests\n");
    printf("------------------------------------------\n");
    printf("\n");
    printf("epicsEvent test\n");

    Worker *worker = new Worker();
    epicsThreadSleep(0.5);
    worker->do_something();
    epicsThreadSleep(0.5);
    worker->do_something();
    epicsThreadSleep(0.5);
    worker->do_something();
    epicsThreadSleep(0.5);
    worker->do_something();
    epicsThreadSleep(0.5);
    worker->do_something();
    epicsThreadSleep(0.5);
    worker->stop();
    worker->test();
    delete worker;

    printf("Philosopher (Mutex) test\n");
    printf("You can either wait until the configured\n");
    printf("turns for each philosopher are handled\n");
    printf("or press CTRL-C to stop early.\n");
    signal(SIGINT, quit);

    for (i=0; i<NUM; ++i)
        phils[i] = new Philosopher(names[i], i, (i+1) % NUM);

    for (i=0; i<NUM; ++i)
        phils[i]->start();

    for (i=0; i<NUM; ++i)
        phils[i]->join();

    for (i=0; i<NUM; ++i)
    {
        phils[i]->info();
        delete phils[i];
    }
    printf("You should see a statistic where all philosophers\n");
    printf("got approximately the same chances.\n");
}
#endif

#ifdef TEST_TIMER
#include<epicsTimer.h>

epicsTime  timer_test_start;

class OneShotTimer : public epicsTimerNotify
{
public:
    OneShotTimer(epicsTimerQueue &queue, double delay)
            : timer(queue.createTimer()), delay(delay)
    {
    }
    
    ~OneShotTimer()
    {
        timer.destroy();
    }
        
    void start()
    {
        timer.start(*this, delay);
        LOG_MSG("OneShotTimer started by thread 0x%08X\n",
                epicsThreadGetIdSelf());
    }

    virtual expireStatus expire (const epicsTime & currentTime)
    {
        double time = epicsTime::getCurrent() - timer_test_start;
        LOG_MSG("OneShotTimer expires in thread 0x%08X after %g secs\n",
                epicsThreadGetIdSelf(), time);
        TEST( fabs(time - delay) < 0.2 );
        return noRestart;
    }
private:
    epicsTimer &timer;
    double     delay;
};

class PeriodTimer : public epicsTimerNotify
{
public:
    PeriodTimer(epicsTimerQueue &queue, double period)
            : timer(queue.createTimer()), period(period)
    {
    }
    
    ~PeriodTimer()
    {
        timer.destroy();
    }
        
    void start()
    {
        timer.start(*this, period);
        period_start = epicsTime::getCurrent();
        LOG_MSG("PeriodTimer started by thread 0x%08X\n",
                epicsThreadGetIdSelf());
    }

    virtual expireStatus expire (const epicsTime & currentTime)
    {
        double time = epicsTime::getCurrent() - period_start;
        LOG_MSG("PeriodTimer expires in thread 0x%08X after %g secs\n",
                epicsThreadGetIdSelf(), time);
        TEST( fabs(time - period) < 0.2 );
        period_start = epicsTime::getCurrent();
        return expireStatus(restart, 1.5);
    }
private:
    epicsTimer &timer;
    epicsTime  period_start;
    double     period;
};

// Passive Queue requires a handler, in here we ignore the reschedule...
// epicsTimerQueueNotify is defined with pure virtual protected destructor?!
class PassiveHandler : public epicsTimerQueueNotify
{
public:
    virtual void reschedule()
    {
    }
    virtual double quantum ()
    {
        return 0;
    }
    void destroy()
    {
        delete this;
    }
protected:
    virtual ~PassiveHandler()
    {
    }
};

void test_timer()
{
    printf("\nThread Tests (active queue)\n");
    printf("------------------------------------------\n");
    epicsTimerQueueActive &act_queue = epicsTimerQueueActive::allocate(true);
    {
        timer_test_start = epicsTime::getCurrent();
        OneShotTimer once(act_queue, 4.0);
        PeriodTimer  period(act_queue, 1.5);
        once.start();
        period.start();
        epicsThreadSleep(10.0);
        double test_duration = epicsTime::getCurrent() - timer_test_start;
        TEST( fabs(test_duration - 10.0) < 0.1 );
    }
    act_queue.release();

    printf("\nThread Tests (single threaded)\n");
    printf("------------------------------------------\n");
    PassiveHandler *passive_handler = new PassiveHandler;
    epicsTimerQueuePassive &passive_queue =
        epicsTimerQueuePassive::create(*passive_handler);
    {
        timer_test_start = epicsTime::getCurrent();
        OneShotTimer once(passive_queue, 4.0);
        PeriodTimer  period(passive_queue, 1.5);
        once.start();
        period.start();
        double delay_to_next;
        epicsTime now = epicsTime::getCurrent();
        while (now - timer_test_start  < 10.0)
        {
            delay_to_next = passive_queue.process(now);
            if (delay_to_next > 1.0)
                delay_to_next = 1.0;
            epicsThreadSleep(delay_to_next);
            now = epicsTime::getCurrent();
        }
    }
}
#endif

#ifdef TEST_CA
#include<cadef.h>

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
        LOG_MSG("PV %s = %g in thread 0x%08X\n",
                (const char *)args.usr, *((double *)args.dbr),
                epicsThreadGetIdSelf());

    }
    else
    {
        LOG_MSG("PV %s type %ld in thread 0x%08X\n",
                (const char *)args.usr, args.type,
                epicsThreadGetIdSelf());
    }
}

static void connection_cb(struct connection_handler_args args)
{
    TEST(args.op == CA_OP_CONN_UP);

    LOG_MSG("PV %s connection_cb %s in thread 0x%08X\n",
            (const char *)ca_puser(args.chid),
            (args.op == CA_OP_CONN_UP ? "UP" : "DOWN"),
            epicsThreadGetIdSelf());
    
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

void test_ca()
{
    printf("\nChannelAccess Tests (requires 'janet' from excas)\n");
    printf("------------------------------------------\n");
    SEVCHK(ca_context_create(ca_enable_preemptive_callback),
           "ca_context_create");

    LOG_MSG("Connecting from thread 0x%08X\n",
            epicsThreadGetIdSelf());

    chid chid;
    SEVCHK(ca_create_channel("janet",
                             connection_cb, (void *) "janet",
                             CA_PRIORITY_ARCHIVE,
                             &chid),
           "ca_create_channel");
    SEVCHK(ca_flush_io(), "ca_pend_io");
    epicsThreadSleep(5.0);
    
    LOG_MSG("Cleanup from thread 0x%08X\n",
            epicsThreadGetIdSelf());
    ca_clear_channel(chid);
    ca_context_destroy();

    TEST(state == got_data);
    printf("Received %zd monitored values\n",
           num_monitors);
    TEST(abs(num_monitors - 50) < 10);
}

#endif


#ifdef TEST_BITSET

#include "Bitset.h"

void test_bitset()
{
    printf("\nBitset  Tests\n");
    printf("------------------------------------------\n");
    BitSet s;
    TEST(strcmp(s.to_string().c_str(), "") == 0);
    s.grow(10);
    TEST(strcmp(s.to_string().c_str(), "0") == 0);
    s.set(1);
    s.set(2, true);
    s.set(0);
    TEST(strcmp(s.to_string().c_str(), "111") == 0);
    s.grow(40);
    TEST(strcmp(s.to_string().c_str(), "111") == 0);
    TEST(s.count() == 3);
    TEST(s.any() == true);
    s.clear(0);
    s.set(2, false);
    s.clear(1);
    TEST(s.count() == 0);
    TEST(s.any() == false);
    s.set(30);
    TEST(strcmp(s.to_string().c_str(), "1000000000000000000000000000000") == 0);
    TEST(s.test(30) == true);
    TEST(s.test(3) == false);
    TEST(s.capacity() == 64);
}

#endif

#ifdef TEST_FUX
#include "FUX.h"
void test_fux()
{
    printf("\nFUX-based XML Tests\n");
    printf("------------------------------------------\n");
    FUX fux;
    FUX::Element *xml_doc;
    xml_doc = fux.parse("test.xml");
    TEST(xml_doc != 0);
    if (xml_doc == 0)
        return;
    fux.DTD="../Engine/engineconfig.dtd";
    fux.dump(stdout);
    TEST(xml_doc->find("write_period") != 0);
    TEST(xml_doc->find("group") != 0);
    TEST(xml_doc->find("channel") == 0);
    FUX::Element *group = xml_doc->find("group");
    TEST(group->parent == xml_doc);
    TEST(group->find("name") != 0);
    TEST(group->find("channel") != 0);
    TEST(group->find("quark") == 0);
}
#endif

// -----------------------------------------------------------------
// -----------------------------------------------------------------
// -----------------------------------------------------------------

#include <epicsTime.h>

int main ()
{
#ifdef TEST_AUTOPTR
    test_autoptr();
#endif

#ifdef TEST_STRING
    test_string();
#endif

#ifdef TEST_BITSET
    test_bitset();
#endif

#ifdef TEST_AVL
    avl_test();
#endif

#ifdef TEST_FUX
    test_fux();
#endif
    
#ifdef TEST_TIME
    struct local_tm_nano_sec tm;
    tm.ansi_tm.tm_year = 2003 - 1900;
    tm.ansi_tm.tm_mon  = 4 - 1;
    tm.ansi_tm.tm_mday = 10;
    tm.ansi_tm.tm_hour = 11;
    tm.ansi_tm.tm_min  = 0;
    tm.ansi_tm.tm_sec  = 0;
    tm.ansi_tm.tm_isdst   = -1;
    tm.nSec = 0;
    epicsTime time = tm;
    time.show(10);
   
    // At Wed Apr  9 10:43:37 MDT 2003 (daylight saving on),
    //  Win32 adds 1hour...
    // Convert 03/18/1990 12:13:44.800000019L back and forth:
    tm.ansi_tm.tm_year = 1990 - 1900;
    tm.ansi_tm.tm_mon  = 3 - 1;
    tm.ansi_tm.tm_mday = 18;
    tm.ansi_tm.tm_hour = 12;
    tm.ansi_tm.tm_min  = 13;
    tm.ansi_tm.tm_sec  = 44;
    tm.ansi_tm.tm_isdst   = -1;
    tm.nSec = 800000019L;
    
    // to epicsTime
    time = tm;

    // back to tm
    tm = time;
    const char *dst;
    switch (tm.ansi_tm.tm_isdst)
    {
        case 0:
            dst = "standard";
            break;
        case 1:
            dst = "daylight saving";
            break;
        default:
            dst = "unknown";
    }
    printf("%02d/%02d/%04d %02d:%02d:%02d.%09ld (%s)\n",
           tm.ansi_tm.tm_mon + 1,
           tm.ansi_tm.tm_mday,
           tm.ansi_tm.tm_year + 1900,
           tm.ansi_tm.tm_hour,
           tm.ansi_tm.tm_min,
           tm.ansi_tm.tm_sec,
           tm.nSec,
           dst);    
    test_time();
#endif
#ifdef TEST_LOG
    test_log();
#endif
#ifdef TEST_THREADS
    test_threads();
#endif
#ifdef TEST_TIMER
    test_timer();
#endif
#ifdef TEST_CA
    test_ca();
#endif

	return 0;
}
