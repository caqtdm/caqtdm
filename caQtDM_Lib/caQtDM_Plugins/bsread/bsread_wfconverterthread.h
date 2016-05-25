#ifndef WFCONVERTERTHREAD_H
#define WFCONVERTERTHREAD_H

#include <QObject>
#include <QMutex>
#include "bsread_wfhandling.h"


class bsread_wfConverterThread : public QObject
{
    Q_OBJECT
private:
    QList<bsread_wfhandling*>* WfDataHandlerQueueP;
    QMutex* WfDataHandlerLockerP;


public:
    explicit bsread_wfConverterThread(QObject *parent = 0);

    void setWfDataHandlerLocker(QMutex *value);
    void setWfDataHandlerQueue(QList<bsread_wfhandling *> *value);

signals:

public slots:
    void process();

};

#endif // WFCONVERTERTHREAD_H
