#ifndef BSREAD_DISPATCHERCONTROL_H
#define BSREAD_DISPATCHERCONTROL_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>
#include <QUrl>
#include "bsread_decode.h"
#include "controlsinterface.h"

typedef struct{
 QString channel;
 int     index;
}channelstruct;



class QNetworkAccessManager;

class bsread_dispatchercontrol : public QObject
{
Q_OBJECT

public:
    bsread_dispatchercontrol();
    ~bsread_dispatchercontrol();
    int set_Dispatcher(QString *dispatcher);

    int add_Channel(QString channel,int index);
    int rem_Channel(QString channel,int index);



    void setZmqcontex(void *value);
    void setMutexknobdataP(MutexKnobData *value);

    void setTerminate();

signals:
    //void requestFinished();
    void finished();
public slots:
   void finishReply();
   void process();
   //void httpReadyRead();

protected:

  QString Dispatcher;
  bool terminate;
  QList<QString> streams;
  QMultiMap<QString,int> Channels;
  QList<channelstruct> ChannelsPipeline;
  QNetworkRequest request;
  QNetworkReply* reply;
  QMutex ProcessLocker;
  QMutex ChannelPipelineLocker;
  QMutex ChannelLocker;
  QWaitCondition startReconnection;

  channelstruct get_Channel();
  void * zmqcontex;
  MutexKnobData *mutexknobdataP;
  QList<bsread_Decode*> bsreadconnections;
  QList<QThread*> bsreadThreads;
};


#endif // BSREAD_DISPATCHERCONTROL_H
