// Tools
#include "Guard.h"
#include "Throttle.h"

Throttle::Throttle(const char *name, double seconds_between_messages)
  : mutex(name, OrderedMutex::Throttle), seconds(seconds_between_messages)
{
}

void Throttle::reset()
{
    last = epicsTime();
}

void Throttle::fire()
{
    last = epicsTime::getCurrent();
}

bool Throttle::isPermitted(const epicsTime &when)
{
    Guard guard(__FILE__, __LINE__, mutex);
    double time_passed = when - last;
    if (time_passed < seconds)
        return false;
    last = when;
    return true;
}

bool Throttle::isPermitted()
{
    epicsTime now(epicsTime::getCurrent());
    return isPermitted(now);
}
    
