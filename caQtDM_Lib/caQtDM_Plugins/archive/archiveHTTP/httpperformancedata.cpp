#include "httpperformancedata.h"

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
        report = "Request is currently in Progress.<br>";
    } else {
        if (m_finishedSuccessfully) {
            report = "Request has finished successfully. <br>";
        } else {
            report = "Request has finished with errors. <br>";
        }
    }
    // Qt can't really handle this table and the background color which it should inherit (which works for regular html, just not for Qt)
    // would be gone not only for this table, but also for all subsequent elements if we used normal html. To workaround this bug,
    // we close the existing div, which caQtDM_Lib created, specify the background color for this table, and then reopen the div we previously closed
    // such that the rest of the text is still rendered correctly.
    //report += InfoPostfix;
    //report += "<table style='background-color:lightyellow; width:100%;'>";
    /*report += "<tr><td>Last request queried data since:</td><td>" + m_beginTime.toString() + "</td></tr>"
              + "<tr><td>Data has been retrieved until:</td><td>" + m_lastRetrievedTime.toString() + "</td></tr>"
              + "<tr><td>API server response took</td><td>" + QString::number(m_responseTimesinMs[0]) + "ms</td></tr>"
              + "<tr><td>Response has size:</td><td>" + QString::number(m_responseBytes[0] / 1000) + "kb</td></tr>"
              + "<tr><td>Http response code was:</td><td>" + QString::number(m_httpStatusCode) + "</td></tr>"
              + "<tr><td>Request received continueAt:</td><td>" + QVariant(m_receivedContinueAt).toString() + "</td></tr>";*/
    report += "Last request queried data since:  " + m_beginTime.toString() + "<br>"
              + "Data has been retrieved until:      " + m_lastRetrievedTime.toString() + "<br>"
              + "API server response took:             " + QString::number(m_responseTimesinMs[0]) + "ms<br>"
              + "Response has size:                         " + QString::number(m_responseBytes[0] / 1000) + "kb<br>"
              + "Http response code was:              " + QString::number(m_httpStatusCode) + "<br>"
              + "Request received continueAt:      " + QVariant(m_receivedContinueAt).toString() + "<br>";
    //report += "</table>";
    //report += InfoPrefix;

    return report;
}

void HttpPerformanceData::addNewRequest(UrlHandlerHttp *urlHandler)
{
    QMutexLocker locker(&m_globalMutex);
    m_requestTimer.start();
    if (!m_inProgress) {
        m_beginTime = urlHandler->beginTime();
        m_endTime = urlHandler->endTime();
        m_inProgress = true;
    }
    m_numberOfRequests++;
}

void HttpPerformanceData::addNewResponse(quint64 responseBytes, int httpStatusCode, bool receivedContinueAt, QDateTime continueAtTime)
{
    long responseTimeInMs = m_requestTimer.elapsed();
    m_requestTimer.invalidate();
    QMutexLocker locker(&m_globalMutex);
    m_responseTimesinMs.append(responseTimeInMs);
    m_responseBytes.append(responseBytes);
    m_httpStatusCode = httpStatusCode;
    if (receivedContinueAt) {
        // Only set it to true, if it is false, leave it as is, maybe it was true before so it still received a continueAt, just not this time.
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
