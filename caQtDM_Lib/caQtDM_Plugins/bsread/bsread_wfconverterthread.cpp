#include "bsread_wfconverterthread.h"

void bsread_wfConverterThread::setWfDataHandlerLocker(QMutex *value)
{
    WfDataHandlerLockerP = value;
}

void bsread_wfConverterThread::setWfDataHandlerQueue(QList<bsread_wfhandling *> *value)
{
    WfDataHandlerQueueP = value;
}

void bsread_wfConverterThread::process()
{
   bsread_wfhandling* job;

    WfDataHandlerLockerP->lock();
    job=WfDataHandlerQueueP->front();
    WfDataHandlerQueueP->removeFirst();
    WfDataHandlerLockerP->unlock();
    job->wfconvert();
}

bsread_wfConverterThread::bsread_wfConverterThread(QObject *parent) : QObject(parent)
{




}

