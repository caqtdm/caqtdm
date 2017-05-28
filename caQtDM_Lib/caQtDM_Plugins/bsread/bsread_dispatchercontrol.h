/*
 *  This file is part of the caQtDM Framework, developed at the Paul Scherrer Institut,
 *  Villigen, Switzerland
 *
 *  The caQtDM Framework is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  The caQtDM Framework is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the caQtDM Framework.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright (c) 2010 - 2015
 *
 *  Author:
 *    Helge Brands
 *  Contact details:
 *    helge.brands@psi.ch
 */
#ifndef BSREAD_DISPATCHERCONTROL_H
#define BSREAD_DISPATCHERCONTROL_H

#include <QObject>
#include <QThread>
#include <QBuffer>
#include <QMutex>
#include <QWaitCondition>
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

    void setMessagewindow(MessageWindow *value);
    void ChannelVerification(QNetworkAccessManager* manager);

signals:
    //void requestFinished();
    void finished();
private slots:
    void closeEvent();
public slots:
   void finishReplyConnect();
   void finishReplyDelete();
   void finishVerification();
   void process();


protected:

  QString Dispatcher;
  MessageWindow *messagewindowP;
  bool terminate;
  QList<QString> streams;
  QMultiMap<QString,int> Channels;
  QList<QString> bsreadChannels;
  QList<channelstruct> ChannelsAddPipeline;
  QMultiMap<QString,int> ChannelsToBeApprovePipeline;
  QMultiMap<QString,int> ChannelsApprovePipeline;
  QList<channelstruct> ChannelsRemPipeline;
  QList<QString> ConnectionDeletePipeline;
  QStringList tobeRemoved;
  //QEventLoop eventloop;
  QEventLoop *loop;
  QNetworkRequest requestChannel;
  QNetworkRequest requestDelete;
  QNetworkRequest requestVerification;
  QNetworkReply* replyConnect;
  QNetworkReply* replyVerification;
  QNetworkReply* replydelete;
  QBuffer buff_delete_data;

  QMutex ProcessLocker;
  QMutex ChannelAddPipelineLocker;
  QMutex ChannelRemPipelineLocker;
  QMutex ConnectionDeleteLocker;
  QMutex ChannelLocker;
  QWaitCondition startReconnection;

  channelstruct get_AddChannel();
  channelstruct get_RemChannel();
  void deleteStream(QString *value);
  QString get_DeleteConnection();
  void * zmqcontex;
  MutexKnobData *mutexknobdataP;
  QList<bsread_Decode*> bsreadconnections;
  QList<QThread*> bsreadThreads;


};


#endif // BSREAD_DISPATCHERCONTROL_H
