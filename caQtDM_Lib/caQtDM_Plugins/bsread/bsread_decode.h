#ifndef BSREAD_DECODE_H
#define BSREAD_DECODE_H

#include <QThread>
#include <QList>
#include "knobData.h"
#include "mutexKnobData.h"
#include "bsread_channeldata.h"

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

    QString getMainHeader() const;
    bool setMainHeader(char *value);
    void setHeader(char *value);
private:
    void * zmqsocket;
    bool running_decode;
    QString ConnectionPoint;
    MutexKnobData * KnobData;
    size_t message_size;
    QString MainHeader;
    long global_timestamp_epoch;
    long global_timestamp_ns;
    double pulse_id;
    QString main_htype;
    QString data_htype;
    QString hash;
    QString ChannelHeader;
    int channelcounter;
    QList<bsread_channeldata*> Channels;
    void bsread_SetChannelData(void *message);
    void bsread_SetChannelTimeStamp(void * timestamp);
    void bsread_EndofData();


};

#endif // BSREAD_DECODE_H
