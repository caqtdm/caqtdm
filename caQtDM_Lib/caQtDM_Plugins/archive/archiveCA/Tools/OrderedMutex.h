
#ifndef __ORDEREDMUTEX_H__
#define __ORDEREDMUTEX_H__

// Base
#include <epicsMutex.h>

// Tools
#include <GenericException.h>
#include <ToolsConfig.h>
#include <NoCopy.h>

/** \ingroup Tools
 *  A mutex with informational name and lock order.
 *  <p>
 *  Meant to help with deadlock-detection based on
 *  a predetermined lock order.
 *  For example, if an application defines locks with order
 *  <ul>
 *  <li><code>OrderedMutex a("a", 1);</code>
 *  <li><code>OrderedMutex b("b", 2);</code>
 *  <li><code>OrderedMutex c("c", 3);</code>
 *  </ul>,
 *  then one has to take them individually or in the order
 *  <code>a, b, c</code>.
 *  <p>
 *  Attempts to take these mutexes out of order,
 *  which could result in deadlocks,
 *  are detected.
 *  For example, locking <code>b, a</code> or <code>c, a</code>
 *  will result in a printout of all threads and the locks which
 *  they currently hold, followed by an exception.
 */
class OrderedMutex
{
public:
    /** Lock order used by Tools::Throttle. */
    static const size_t Throttle = 101;
    
    /** Lock order used by Tools::ConcurrentList. */
    static const size_t ConcurrentList = 100;

    /** Create mutex with name and lock order. */
    OrderedMutex(const char *name, size_t order);

    /** Destructor. */
    ~OrderedMutex();

    /** @return Returns the mutex name. */
    const stdString &getName() const
    {   return name; }

    /** @return Returns the mutex order. */
    size_t getOrder() const
    {   return order; }

    /** Lock the mutex.
     *  <p>
     *  Attempts to lock this mutex while higher-order
     *  mutexes are already taken will result in
     *  an exception.
     *  <p>
     *  Parameters file and line will be used in error messages.
     */
    void lock(const char *file, size_t line);
    
    /** Unlock the mutex. */
    void unlock();
    
private:
    PROHIBIT_DEFAULT_COPY(OrderedMutex);
    stdString name;
    size_t order;
    epicsMutexId mutex;
};

// Dumps the current lock list.
extern void LockMonitorDump();

#endif

