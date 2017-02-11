// Base
#include <epicsThread.h>

// Tools
#include "OrderedMutex.h"
#include "GenericException.h"
#include "Guard.h"
#include "MsgLogger.h"

#define DETECT_DEADLOCK

/** Monitor the locks that one thread currently holds. */
class ThreadList
{
public:
    /** Initialize with first thread and lock. */
    ThreadList(epicsThreadId thread, OrderedMutex *lock) : thread(thread)
    {
        add(lock);
    }
    
    /** @return Returns the thread handled by this list. */
    epicsThreadId getThread() const
    {   return thread; }

    /** Add a lock that this thread wants to take. */
    bool add(const OrderedMutex *lock)
    {
        locks.push_back(lock);
        return check();
    }

    /** Remove a lock from list for this thread. */
    void remove(const OrderedMutex *lock);

    bool isEmpty() const
    {
        return locks.empty();
    }

    /** Dump info about locks. */
    void dump() const;
    
private:
    epicsThreadId thread;
    stdList<const OrderedMutex *> locks;
    /** Check lock order. */
    bool check() const;
};

void ThreadList::remove(const OrderedMutex *lock)
{
    // Remove lock from the list of locks for this thread.
    stdList<const OrderedMutex *>::iterator i;
    for (i = locks.begin();  i != locks.end();  ++i)
    {
        if (*i == lock)
        {
            locks.erase(i);
            return;
        }
    }
    if (getenv("ABORT_ON_ERRORS"))
    {
        LOG_MSG("ThreadList: Unknown lock '%s'\n", lock->getName().c_str());
        abort();
    }
    throw GenericException(__FILE__, __LINE__, "Unknown lock '%s'",
                           lock->getName().c_str());
}

void ThreadList::dump() const
{
    char name[100];
    epicsThreadGetName(thread, name, sizeof(name));
    fprintf(stderr, "Thread '%s': ", name);
    bool first = true;
    stdList<const OrderedMutex *>::const_iterator i;
    for (i = locks.begin();  i != locks.end();  ++i)
    {
        const OrderedMutex *l = *i;
        if (first)
            first = false;
        else
            fprintf(stderr, ", ");
        fprintf(stderr, "'%s' (%zu)", l->getName().c_str(), l->getOrder());
    }
    printf("\n");
}

bool ThreadList::check() const
{
    stdList<const OrderedMutex *>::const_iterator i = locks.begin(); 
    if (i == locks.end())
        return true; // empty
    const OrderedMutex *l = *i;
    size_t previous = l->getOrder();
    ++i;
    while (i != locks.end())
    {
        l = *i;
        size_t order = l->getOrder();
        if (order < previous)
            return false;
        previous = order;
        ++i;
    }
    return true;
}

/** Monitor which thread takes which locks. */
class LockMonitor
{
public:
    /** Get/create the singleton LockMonitor. */
    static LockMonitor *getInstance();

    /** Record that given thread tries to take some mutex. */
    void add(const char *file, size_t line,
             epicsThreadId thread, OrderedMutex &lock);

    /** Record that given thread release some mutex. */
    void remove(epicsThreadId thread, OrderedMutex &lock);
    
    void dump();
private:
    /** Constructor. There should only be one lock monitor. */
    LockMonitor(){}

    /** Destructor. */
    ~LockMonitor() {}
    
    static LockMonitor *lock_monitor;
    epicsMutex mutex;
    stdList<ThreadList> threads;
    void dump(epicsMutexGuard &guard);
    
    static void cleanup();
};

// Singleton
LockMonitor *LockMonitor::lock_monitor = 0;

void LockMonitor::cleanup()
{
    if (lock_monitor)
    {
        delete lock_monitor;
        lock_monitor = 0;
    }
}

LockMonitor *LockMonitor::getInstance()
{
    if (! lock_monitor)
    {
        lock_monitor = new LockMonitor();
        atexit(cleanup);
    }
    return lock_monitor;
}

void LockMonitor::add(const char *file, size_t line,
                      epicsThreadId thread, OrderedMutex &lock)
{
    epicsMutexGuard guard(mutex);
    // Add lock to the list of locks for that thread.
    stdList<ThreadList>::iterator i;
    for (i = threads.begin();  i != threads.end();  ++i)
    {
        if (i->getThread() == thread)
        {
            if (!i->add(&lock))
            {
                fprintf(stderr, "=========================================\n");
                fprintf(stderr, "Violation of lock order in\n");
                fprintf(stderr, "file %s, line %zu:\n\n", file, line);
                dump(guard);
                fprintf(stderr, "=========================================\n");
                // Remove the offending one, since we won't lock it
                i->remove(&lock);
                if (getenv("ABORT_ON_ERRORS"))
                {
                    LOG_MSG("%s (%zu): Violation of lock order\n",
                            file, line);
                    abort();
                }
                // else
                throw GenericException(file, line, "Violation of lock order");
            }
            return;
        }
    }
    threads.push_back(ThreadList(thread, &lock));
}

void LockMonitor::remove(epicsThreadId thread, OrderedMutex &lock)
{
    epicsMutexGuard guard(mutex);
    // Remove lock from the list of locks for that thread.
    stdList<ThreadList>::iterator i;
    for (i = threads.begin();  i != threads.end();  ++i)
    {
        if (i->getThread() == thread)
        {
            i->remove(&lock);
            if (i->isEmpty())
                threads.erase(i);
            return;
        }
    }
    if (getenv("ABORT_ON_ERRORS"))
    {
        LOG_MSG("LockMonitor: Unknown thread\n");
        dump(guard);
        abort();
    }
    throw GenericException(__FILE__, __LINE__, "Unknown thread");
}

void LockMonitor::dump()
{
    epicsMutexGuard guard(mutex);
    dump(guard);
}

void LockMonitor::dump(epicsMutexGuard &guard)
{
    if (threads.size() == 0)
        fprintf(stderr, "No locks\n");
    // Remove lock from the list of locks for that thread.
    stdList<ThreadList>::iterator i;
    for (i = threads.begin();  i != threads.end();  ++i)
        i->dump();
}

void LockMonitorDump()
{
    LockMonitor *lm = LockMonitor::getInstance();
    LOG_ASSERT(lm);
    lm->dump();
}

OrderedMutex::OrderedMutex(const char *name, size_t order)
        : name(name), order(order), mutex(epicsMutexCreate())
{
    LOG_ASSERT(mutex);
}

OrderedMutex::~OrderedMutex()
{
    epicsMutexDestroy(mutex);
}

void OrderedMutex::lock(const char *file, size_t line)
{
#ifdef DETECT_DEADLOCK
    LockMonitor::getInstance()->add(file, line, epicsThreadGetIdSelf(), *this);
#endif
    if (epicsMutexLock(mutex) != epicsMutexLockOK)
    {
        if (getenv("ABORT_ON_ERRORS"))
        {
            LOG_MSG("%s (%zu): mutex lock '%s' failed\n",
                file, line, name.c_str());
            abort();
        }
        // else
        throw GenericException(file, line, "mutex lock '%s' failed",
                               name.c_str());
    }
#ifdef DETECT_DEADLOCK
    if (getenv("TRACE_MUTEX"))
    {
        fprintf(stderr, "%25s:%4zu Lock   : ", file, line);
        LockMonitor::getInstance()->dump();
    }
#endif
}
    
void OrderedMutex::unlock()
{
    epicsMutexUnlock(mutex);
#ifdef DETECT_DEADLOCK
    LockMonitor::getInstance()->remove(epicsThreadGetIdSelf(), *this);
    if (getenv("TRACE_MUTEX"))
    {
        fprintf(stderr, "                               Unlock : ");
        LockMonitor::getInstance()->dump();
    }
#endif
}

