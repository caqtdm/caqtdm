#ifndef stripplotthread_H
#define stripplotthread_H
#include <QThread>
#include <QTimer>
#include <stdio.h>

#include <sys/timeb.h>
#include <qtcontrols_global.h>

class QTCON_EXPORT stripplotthread : public QThread
{
    Q_OBJECT

public:
    stripplotthread(QObject *parent=0);
    void threadSetTimer(int timeInterval);

protected:
    virtual void run();

signals:
     void update();


private slots:
     void TimeOut();
     void runStop();
     void quitFromOtherThread() {
            quit();
         }

private:
    int interval;
};

#endif // stripplotthread_H
