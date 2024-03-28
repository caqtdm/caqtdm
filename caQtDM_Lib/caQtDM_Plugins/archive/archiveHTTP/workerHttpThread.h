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
    HttpRetrieval *getHttpRetrieval();
    const bool isActive();
    void setIsActive(const bool &newIsActive);

private:
    WorkerHTTP *m_worker;
    bool m_isActive;
    QMutex m_mutex;
};

#endif // WORKERHTTPTHREAD_H
