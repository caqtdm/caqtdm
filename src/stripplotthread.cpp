#include "stripplotthread.h"

stripplotthread::stripplotthread(QObject *parent) : QThread(parent)
{
}

void stripplotthread::threadSetTimer(int timeInterval)
{
    interval = timeInterval;
    //printf("new interval requested = %d\n", interval);
}

void stripplotthread:: run(){
    QTimer timer;

    connect(&timer, SIGNAL(timeout()), this, SLOT(TimeOut()), Qt::DirectConnection);
    timer.setInterval(interval);
    timer.start();   // puts one event in the threads event queue
    exec();

    timer.stop();
}


void stripplotthread::runStop()
{
    // quit request coming from another thread
    if(currentThread() != this){
        QMetaObject::invokeMethod(this, "quitFromOtherThread", Qt::DirectConnection);

    // quit request coming from this thread
    } else {
        quit();
    }
}

void stripplotthread::TimeOut()
{
    emit update();
}
