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
    {
        pworker = worker;
        //qDebug() << "myThread::myThread()";
    }
    ~WorkerHttpThread()
    {
        //qDebug() << "myThread::~myThread()";
    }
    WorkerHTTP *workerhttp() { return pworker; }

    HttpRetrieval *getArchive()
    {
        if (pworker != (WorkerHTTP *) Q_NULLPTR) {
            return pworker->getArchive();
        } else {
            return (HttpRetrieval *) Q_NULLPTR;
        }
    }

private:
    WorkerHTTP *pworker;
};

#endif // WORKERHTTPTHREAD_H
