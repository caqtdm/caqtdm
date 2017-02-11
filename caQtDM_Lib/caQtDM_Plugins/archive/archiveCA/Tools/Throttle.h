// -*- c++ -*-
#ifndef __THROTTLE_H__
#define __THROTTLE_H__

// Base
#include <epicsTime.h>
// Tools
#include <ToolsConfig.h>
#include <OrderedMutex.h>

/** \ingroup Tools
 *  Timer for throttling messages, thread safe.
 * 
 *  In order to avoid hundreds of similar messages,
 *  use the Throttle to check if it's OK to print
 *  the message.
 */
class Throttle
{
public:
    /** Create throttle with the given name and threshold. */
    Throttle(const char *name, double seconds_between_messages);
    
    /** Reset the throttle so that isPermitted will return 'true'. */
    void reset();

    /** Fire the throttle so that isPermitted will return 'false'
     *  until the threshold is exceeded.
     */
    void fire();

    /** @return Returns true when it's OK to print message at given time. */
    bool isPermitted(const epicsTime &when);

    /** @return Returns true when it's OK to print another message now. */
    bool isPermitted();
    
    /** @return Returns the message threshold in seconds. */
    double getThrottleThreshold()
    {
        return seconds;
    }
    
private:
    PROHIBIT_DEFAULT_COPY(Throttle);
    OrderedMutex mutex;
    double seconds;
    epicsTime last;
};

#endif

