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
 *  Authors:
 *    Marty Kraimer, Anton Mezger
 *  Contact details:
 *    anton.mezger@psi.ch, mrkraimer@comcast.net
 */

#include <iostream>
using namespace std;


#include "callbackThread.h"

namespace epics { namespace pvData {

CallbackThread::~CallbackThread()
{
cout << "CallbackThread::~CallbackThread()\n";
    stop();
cout << "CallbackThread::~CallbackThread() returning\n";
}

void CallbackThread::run()
{
    CallbackRequesterPtr callbackRequester;
    while(true) 
    {
        epicsThreadSleep(.2);
        if(runStop.tryWait()) {
            runReturn.signal();
            return;
        }    
        {
             epics::pvData::Lock xx(mutex);
             if(monitorQueue.empty()) continue;
             callbackRequester = monitorQueue.front();
             monitorQueue.pop();
        }
        while(callbackRequester) {
            callbackRequester->callback();
            epics::pvData::Lock xx(mutex);
            if(monitorQueue.empty()) break;
            callbackRequester = monitorQueue.front();
            monitorQueue.pop();
        }
    }
}

}}
