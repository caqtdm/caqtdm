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
