#include "httpperformancedata.h"

HttpPerformanceData::HttpPerformanceData()
{
    qDebug() << "creating a HttpPerformanceData";
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
        report = "Request is currently in Progress.";
    } else {
        report = "Request has finished.";
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
