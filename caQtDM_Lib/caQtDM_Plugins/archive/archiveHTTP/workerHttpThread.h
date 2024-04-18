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
 *  Copyright (c) 2010 - 2024
 *
 *  Author:
 *    Erik Schwarz
 *  Contact details:
 *    erik.schwarz@psi.ch
 */

#ifndef WORKERHTTPTHREAD_H
#define WORKERHTTPTHREAD_H

#include "httpretrieval.h"
#include "workerHttp.h"

class Q_DECL_EXPORT WorkerHttpThread : public QThread
{
    Q_OBJECT

public:
    WorkerHttpThread(WorkerHTTP *worker);
    ~WorkerHttpThread();

    /*
     * Returns a pointer to the httpRetrieval currently associated with the workerHttp in this thread.
     * If no httpRetrieval is currently associated, it returns a Q_NULLPTR.
     * Due to multithreading, this function is dangerous and should only be used with extreme caution
     * */
    HttpRetrieval *getHttpRetrieval();

    const bool isActive();
    void setIsActive(const bool &newIsActive);

private:
    WorkerHTTP *m_worker;
    bool m_isActive;
    QMutex m_mutex;
};

#endif // WORKERHTTPTHREAD_H
