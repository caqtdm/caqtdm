#ifndef BSREAD_DECODE_H
#define BSREAD_DECODE_H

#include <QThread>
#include "knobData.h"
#include "mutexKnobData.h"


class bsread_Decode : public QThread
{
public:
    bsread_Decode(void * Context,QString ConnectionPoint);

    void run();


    void *getZmqsocket() const;

    QString getConnectionPoint() const;

    bool getRunning_decode() const;

    MutexKnobData *getKnobData() const;
    void setKnobData(MutexKnobData *value);
    size_t getMessage_size() const;

private:
    void * zmqsocket;
    bool running_decode;
    QString ConnectionPoint;
    MutexKnobData * KnobData;
    size_t message_size;


};

#endif // BSREAD_DECODE_H
