#ifndef BSREAD_WFHANDLING_H
#define BSREAD_WFHANDLING_H

#include <QThread>
#include <QDebug>
#include <QRunnable>
#include "knobData.h"
#include "mutexKnobData.h"
#include "bsread_channeldata.h"


class bsread_wfhandling :public QObject, public QRunnable
{
    Q_OBJECT
    void run()
     {
         //qDebug() << "Thread:" << QThread::currentThread();
         wfconvert();
     }
private:
    knobData* kDataP;
    bsread_channeldata * bsreadPVP;
    void wfconvert();
public:
    bsread_wfhandling(knobData* kData,bsread_channeldata * bsreadPV);
};

#endif // BSREAD_WFHANDLING_H
