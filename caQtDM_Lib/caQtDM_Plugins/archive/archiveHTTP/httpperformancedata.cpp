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

#include "httpperformancedata.h"
#include <QVariant>

HttpPerformanceData::HttpPerformanceData()
{
    m_receivedContinueAt = false;
    m_inProgress = false;
    m_finishedSuccessfully = false;
}


HttpPerformanceData::~HttpPerformanceData()
{
}

QString HttpPerformanceData::generateReport() {
    QMutexLocker locker(&m_globalMutex);
    QString report;
    if (m_inProgress) {
        report = "Request is currently in Progress. <br>";
    } else {
        if (m_finishedSuccessfully) {
            report = "Request has finished successfully. <br>";
        } else {
            report = "Request has finished with errors. <br>";
        }
    }
    // The spacing seems weird but when displayed, then everything should align.
    report += "Last update queried data since:  " + m_beginTime.toLocalTime().toString("HH:mm:ss yyyy-MM-dd") + "<br>"
              + "Data has been retrieved until:     " + m_lastRetrievedTime.toLocalTime().toString("HH:mm:ss yyyy-MM-dd") + "<br>"
              + "API server response took:            " + QString::number(m_responseTimeinMs) + "ms<br>"
              + "Response had size:                       " + QString::number(m_responseBytes / 1000) + "kb<br>"
              + "Http response code was:             " + QString::number(m_httpStatusCode) + "<br>"
              + "Request received continueAt:     " + QVariant(m_receivedContinueAt).toString() + "<br>"
              + "Data is binned:                              " + QVariant(m_isBinned).toString() + "<br>";
    if (m_isBinned) {
        report += "Number of bins:                            " + QString::number(m_isBinned) + "<br>";
    }

    return report;
}

void HttpPerformanceData::addNewRequest(UrlHandlerHttp *urlHandler)
{
    QMutexLocker locker(&m_globalMutex);
    m_requestTimer.start();
    if (!m_inProgress) {
        m_beginTime = urlHandler->beginTime();
        m_endTime = urlHandler->endTime();
        m_isBinned = urlHandler->binned();
        m_binCount = urlHandler->binCount();
        m_inProgress = true;
    }
    m_numberOfRequests++;
}

void HttpPerformanceData::addNewResponse(quint64 responseBytes, int httpStatusCode, bool receivedContinueAt, QDateTime continueAtTime)
{
    long responseTimeInMs = m_requestTimer.elapsed();
    m_requestTimer.invalidate();
    QMutexLocker locker(&m_globalMutex);
    m_responseTimeinMs = responseTimeInMs;
    m_responseBytes = responseBytes;
    m_httpStatusCode = httpStatusCode;
    if (receivedContinueAt) {
        // Only set it to true, if it is false then leave it as is, maybe it was true before so it still received a continueAt, just not this time.
        m_receivedContinueAt = true;
        m_lastRetrievedTime = continueAtTime;
    } else if (httpStatusCode == 200) {
        m_finishedSuccessfully = true;
        m_lastRetrievedTime = m_endTime;
        m_inProgress = false;
    } else {
        m_finishedSuccessfully = false;
        m_inProgress = false;
    }
}
