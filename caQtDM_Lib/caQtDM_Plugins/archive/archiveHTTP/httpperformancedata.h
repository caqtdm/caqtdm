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
    QString generateReport();
    void addNewRequest(UrlHandlerHttp *urlHandler);
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
    QVector<int> m_responseBytes;
    QVector<long> m_responseTimesinMs;
};

#endif // HTTPPERFORMANCEDATA_H
