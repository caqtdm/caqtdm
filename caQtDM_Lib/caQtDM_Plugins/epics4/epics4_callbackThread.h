/*
 *  This file is part of the caQtDM Framework, developed at the Paul Scherrer Institut,
 *  Villigen, Switzerland
 *
 *  The caQtDM Framework is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  The caQtDM Framework is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the caQtDM Framework.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright (c) 2010 - 2014
 *
 *  Author:
 *    Anton Mezger
 *  Contact details:
 *    anton.mezger@psi.ch
 */
#ifndef CALLBACKTHREAD_H
#define CALLBACKTHREAD_H


#include <queue>
#include <epicsThread.h>
#include <pv/event.h>
#include <pv/lock.h>

namespace epics { namespace pvData {


class epics4_CallbackThread;
typedef std::tr1::shared_ptr<epics4_CallbackThread> CallbackThreadPtr;

class epics4_CallbackRequester;
typedef std::tr1::shared_ptr<epics4_CallbackRequester> CallbackRequesterPtr;

class epicsShareClass epics4_CallbackRequester
{
public:
    virtual ~epics4_CallbackRequester(){}
    virtual void callback() = 0;
};

class epics4_CallbackThread;
typedef std::tr1::shared_ptr<epics4_CallbackThread> CallbackThreadPtr;

class epicsShareClass  epics4_CallbackThread :
    public epicsThreadRunable
{
    std::queue<CallbackRequesterPtr> monitorQueue;
    std::tr1::shared_ptr<epicsThread> thread;
    epics::pvData::Mutex mutex;
    epics::pvData::Event runStop;
    epics::pvData::Event runReturn;
public:
    POINTER_DEFINITIONS(epics4_CallbackThread);
    ~epics4_CallbackThread();
    virtual void run();
    void startThread()
    {
         thread =  std::tr1::shared_ptr<epicsThread>(new epicsThread(
            *this,
            "callbackThread",
            epicsThreadGetStackSize(epicsThreadStackSmall),
            epicsThreadPriorityLow));
         thread->start();
    }
    void queueRequest(CallbackRequesterPtr const & callbackRequester)
    {
        epics::pvData::Lock xx(mutex);
        monitorQueue.push(callbackRequester);
    }
    static CallbackThreadPtr create()
    {
         CallbackThreadPtr t(new epics4_CallbackThread());
         t->startThread();
         return t;
    } 
    void stop()
    {
        runStop.signal();
        runReturn.wait();
    }
private:
    epics4_CallbackThread()
    {}
};


}}


#endif // CALLBACKTHREAD_H
