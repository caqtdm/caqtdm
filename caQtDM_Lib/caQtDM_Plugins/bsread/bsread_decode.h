#ifndef BSREAD_DECODE_H
#define BSREAD_DECODE_H

#include <QThread>
#include <QList>
#include <QAtomicInt>
#include "knobData.h"
#include "mutexKnobData.h"
#include "bsread_channeldata.h"

class bsread_Decode : public QObject
{
    Q_OBJECT
public:
    bsread_Decode(void * Context,QString ConnectionPoint);
    ~bsread_Decode();

    void *getZmqsocket() const;

    QString getConnectionPoint() const;

    bool getRunning_decode() const;

    MutexKnobData *getKnobData() const;
    void setKnobData(MutexKnobData *value);
    size_t getMessage_size() const;

    QString getMainHeader() const;
    bool setMainHeader(char *value, size_t size);
    void setHeader(char *value, size_t size);
    bool bsread_DataMonitorConnection(QString channel,int index);
    bool bsread_DataMonitorConnection(knobData *kData);
    bool bsread_DataMonitorUnConnect(knobData *kData);
    void setTerminate();
public slots:
    void process();
signals:
    void finished();
private:
    QMutex mutex;
    void * zmqsocket;
    bool running_decode;
    QString ConnectionPoint;
    MutexKnobData * bsread_KnobDataP;
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
    QMap<QString,bsread_channeldata*> ChannelSearch;

    QList<int> listOfIndexes;
    QList<QString> listOfRequestedChannels;
    void bsread_SetChannelData(void *message);
    void bsread_SetChannelTimeStamp(void * timestamp);
    void bsread_InitHeaderChannels();
    void bsread_TransferHeaderData();
    void bsread_EndofData();
    bool terminate;


    void bsread_DataTimeOut();
    void bsread_Delay();
};

#endif // BSREAD_DECODE_H
