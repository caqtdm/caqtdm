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

#ifndef stripplotthread_H
#define stripplotthread_H
#include <QThread>
#include <QTimer>
#include <stdio.h>

#include <sys/timeb.h>
#include <qtcontrols_global.h>

class QTCON_EXPORT stripplotthread : public QThread
{
    Q_OBJECT

public:
    stripplotthread(QObject *parent=0);
    void threadSetTimer(int timeInterval);

protected:
    virtual void run();

signals:
     void update();

private slots:
     void TimeOut();
     void runStop();
     void quitFromOtherThread() {
            quit();
         }

private:
    int interval;
};

#endif // stripplotthread_H
