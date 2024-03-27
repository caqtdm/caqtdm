#ifndef WORKERHTTPTHREAD_H
#define WORKERHTTPTHREAD_H

#include <QList>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QThread>
#include <QTimer>
#include <qwt.h>

#include "httpretrieval.h"
#include "workerHttp.h"

class Q_DECL_EXPORT WorkerHttpThread : public QThread
{
    Q_OBJECT

public:
    WorkerHttpThread(WorkerHTTP *worker)
    { // 672947 124704
        //  672947 126748
        m_worker = worker;
        //qDebug() << "myThread::myThread()";
    }
    ~WorkerHttpThread()
    {
        //qDebug() << "myThread::~myThread()";
        PRINTFLUSH("WorkerHTTPThread::~WorkerHTTPThread()");
    }
    WorkerHTTP *workerhttp() { return m_worker; }

    HttpRetrieval *getHttpRetrieval()
    {
        if (m_worker != (WorkerHTTP *) Q_NULLPTR) {
            return m_worker->getArchive();
        } else {
            return (HttpRetrieval *) Q_NULLPTR;
        }
    }

    const bool isActive () {
        return m_isActive;
    }

    void setIsActive(const bool &newIsActive) {
        m_mutex.lock();
        m_isActive = newIsActive;
        m_mutex.unlock();
    }

private:
    WorkerHTTP *m_worker;
    bool m_isActive = true;
    QMutex m_mutex;
};

#endif // WORKERHTTPTHREAD_H
