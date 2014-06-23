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

#include "stripplotthread.h"

stripplotthread::stripplotthread(QObject *parent) : QThread(parent)
{
}

void stripplotthread::threadSetTimer(int timeInterval)
{
    interval = timeInterval;
    //printf("new interval requested = %d\n", interval);
}

void stripplotthread:: run(){
    QTimer timer;

    connect(&timer, SIGNAL(timeout()), this, SLOT(TimeOut()), Qt::DirectConnection);
    timer.setInterval(interval);
    timer.start();   // puts one event in the threads event queue
    emit update();   // for first shot
    exec();

    timer.stop();
}

void stripplotthread::runStop()
{
    // quit request coming from another thread
    if(currentThread() != this){
        QMetaObject::invokeMethod(this, "quitFromOtherThread", Qt::DirectConnection);

    // quit request coming from this thread
    } else {
        quit();
    }
}

void stripplotthread::TimeOut()
{
    emit update();
}
