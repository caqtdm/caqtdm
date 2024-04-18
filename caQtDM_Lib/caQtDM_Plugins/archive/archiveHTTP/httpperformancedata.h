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

#ifndef HTTPPERFORMANCEDATA_H
#define HTTPPERFORMANCEDATA_H


#include "qdatetime.h"
#include "qelapsedtimer.h"
#include "qmutex.h"
#include "urlhandlerhttp.h"
class HttpPerformanceData
{
public:
    HttpPerformanceData();
    ~HttpPerformanceData();

    /*
     * Generates a HTML 4 report containing performance data for the last request.
     * */
    QString generateReport();

    /*
     * Adds a new request and automatically starts the timer to track response time.
     * */
    void addNewRequest(UrlHandlerHttp *urlHandler);

    /*
     * Adds a response to the latest request and automatically stops the timer to track response time.
     * */
    void addNewResponse(quint64 responseBytes, int httpStatusCode, bool receivedContinueAt, QDateTime continueAtTime);

private:
    QMutex m_globalMutex;
    QElapsedTimer m_requestTimer;
    bool m_finishedSuccessfully;
    bool m_inProgress;
    int m_httpStatusCode;
    QDateTime m_beginTime;
    QDateTime m_endTime;
    QDateTime m_lastRetrievedTime;
    bool m_receivedContinueAt;
    int m_numberOfRequests;
    int m_responseBytes;
    long m_responseTimeinMs;
    bool m_isBinned;
    int m_binCount;
};

#endif // HTTPPERFORMANCEDATA_H
