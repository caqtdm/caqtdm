// System
#include <stdio.h>
#include <math.h>
// Base
#include <epicsThread.h>
#include <epicsThread.h>
// Tools
#include "ToolsConfig.h"
#include "AutoPtr.h"
#include "Guard.h"
#include "UnitTest.h"

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
                printf("Worker handling request #%d\n", _worker_did_work);
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

    int getRunCount()
    {
        return _worker_did_work;
    }

private:
    epicsThread _thread;
    epicsEvent  *_event;
    bool        _go;
    int         _worker_did_work;
};

TEST_CASE worker_thread()
{
    AutoPtr<Worker> worker(new Worker());
    epicsThreadSleep(0.5);
    worker->do_something();
    epicsThreadSleep(0.5);
    worker->do_something();
    epicsThreadSleep(0.5);
    worker->do_something();
    epicsThreadSleep(0.5);
    worker->stop();
    TEST(worker->getRunCount() == 3);
    TEST_OK;
}

// "Dining Philosophers" Example.
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

#define NUM 5
static const char *names[NUM] =
{
    "A",
    "B",
    "C",
    "D",
    "E",
};
#define TURN_LIMIT 5
static epicsMutex forks[NUM];
static class Philosopher *phils[NUM];

class Philosopher : public epicsThreadRunable
{
public:
    Philosopher(const char *name, int left, int right)
            : thread(*this, name,
                     epicsThreadGetStackSize(epicsThreadStackSmall),
                     epicsThreadPriorityMedium),
              name(name), left(left), right(right), count(0)
    {
    }

    void run()
    {
        while (count < TURN_LIMIT)
        {
            {
                epicsMutexGuard left(forks[this->left]);
                epicsMutexGuard right(forks[this->right]);
                printf("%s eating, utilizing %s and %s\n",
                        name, names[this->left], names[this->right]);
                ++count;
                thread.sleep(0.2);
            }
            thread.sleep(0.2);
        }
    }

    void start()
    {
        thread.start();
    }

    void join()
    {
        thread.exitWait();
    }

    void info()
    {
        printf("%s got %d chances\n", name, count);
    }

private:
    epicsThread thread;
    const char *name;
    int left, right;
    int count;
};

TEST_CASE test_threads()
{
    int i;


    for (i=0; i<NUM; ++i)
        phils[i] = new Philosopher(names[i], i, (i+1) % NUM);

    for (i=0; i<NUM; ++i)
        phils[i]->start();

    // Very interesting:
    // On Mac OS X 10.4.3 I noticed that the first
    // Philosopher would never run without this delay.
    // Since we join/waitForExit right after creation,
    // that thread would never enter the run state!
    // Have not seen this behavior anywhere else,
    // i.e. other OS have the threads enter the
    // runstate.
    epicsThreadSleep(1.0);

    for (i=0; i<NUM; ++i)
        phils[i]->join();

    for (i=0; i<NUM; ++i)
    {
        phils[i]->info();
        delete phils[i];
    }
    printf("You should see a statistic where all philosophers\n");
    printf("got approximately the same chances.\n");
    TEST_OK;
}


