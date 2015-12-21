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

class QNetworkAccessManager;

class bsread_dispatchercontrol : public QThread
{
Q_OBJECT

public:
    explicit bsread_dispatchercontrol(QObject *parent=0);
    int set_Dispatcher(QString *dispatcher);
    void run();
    int add_Channel(QString *channel);
    int rem_Channel(QString *channel);



signals:
   void requestFinished();

private slots:
   void finishReply();
   void httpReadyRead();

protected:

  QString Dispatcher;
  QList<QString> streams;
  QList<QString> Channels;
  QList<QString> ChannelsPipeline;
  QNetworkRequest request;
  QNetworkReply* reply;
  QMutex ProcessLocker;
  QMutex ChannelLocker;
  QWaitCondition startReconnection;

  QString get_Channel();




};


#endif // BSREAD_DISPATCHERCONTROL_H
