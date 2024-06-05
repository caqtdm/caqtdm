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

#include <workerHttpThread.h>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QThread>
#include <QTimer>
#include <qwt.h>

#include "httpretrieval.h"
#include "workerHttp.h"

WorkerHttpThread::WorkerHttpThread(WorkerHTTP *worker)
{
    m_isActive = true;
    m_worker = worker;
}
WorkerHttpThread::~WorkerHttpThread()
{
}

HttpRetrieval* WorkerHttpThread::getHttpRetrieval()
{
    if (m_worker != (WorkerHTTP *) Q_NULLPTR) {
        return m_worker->getArchive();
    } else {
        return (HttpRetrieval *) Q_NULLPTR;
    }
}

const bool WorkerHttpThread::isActive()
{
    return m_isActive;
}

void WorkerHttpThread::setIsActive(const bool &newIsActive)
{
    m_mutex.lock();
    m_isActive = newIsActive;
    if (m_worker) {
        m_worker->setIsActive(newIsActive);
    }
    m_mutex.unlock();
}
