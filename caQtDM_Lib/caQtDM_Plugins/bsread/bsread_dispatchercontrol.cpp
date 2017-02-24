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
#include <QNetworkAccessManager>
#include <QApplication>
#include <QSslConfiguration>
#include <QDebug>
#include <QBuffer>
#include "bsread_dispatchercontrol.h"
#include "JSON.h"
#include "JSONValue.h"


bsread_dispatchercontrol::bsread_dispatchercontrol()
{
    loop = new QEventLoop(this);
    connect(qApp, SIGNAL(aboutToQuit()),this, SLOT(closeEvent()));
}
bsread_dispatchercontrol::~bsread_dispatchercontrol()
{
    this->setTerminate();
    loop->processEvents();
    loop->quit();
    //qDebug()<<"Dispatcher stop";
}

void bsread_dispatchercontrol::process()
{

    QNetworkAccessManager manager;
    //QEventLoop loop;
    int requestedchannels=0;
    terminate=false;
    QString msg="bsread Dispatcher started: ";
    msg.append(Dispatcher);


    messagewindowP->postMsgEvent(QtDebugMsg,(char*) msg.toLatin1().constData());
    //qDebug()<<"bsread Dispatcher: Start ThreadID: "<<QThread::currentThreadId();
    //Update and reconection handling
    while (!terminate){
        //QThread::msleep(200);

        ProcessLocker.lock();
        startReconnection.wait(&ProcessLocker,400);


        QString StreamDispatcher=Dispatcher;
        if (!StreamDispatcher.endsWith("/")){
            StreamDispatcher.append("/");
        }
        if (StreamDispatcher.endsWith("/")){
            StreamDispatcher.append("stream");
        }

        QUrl url(StreamDispatcher);
        requestChannel = QNetworkRequest(url);
        requestDelete  = QNetworkRequest(url);
#ifndef QT_NO_SSL
    if(url.toString().toUpper().contains("HTTPS")) {
        QSslConfiguration configChannel = requestChannel.sslConfiguration();
        configChannel.setPeerVerifyMode(QSslSocket::VerifyNone);
        requestChannel.setSslConfiguration(configChannel);

        QSslConfiguration configDelete = requestDelete.sslConfiguration();
        configDelete.setPeerVerifyMode(QSslSocket::VerifyNone);
        requestDelete.setSslConfiguration(configDelete);
    }

#endif

        // qDebug()<<"Check Pipeline";
        while(!ChannelsAddPipeline.isEmpty()){
            channelstruct candidate=get_AddChannel();
            //qDebug()<<"ADDChannel Pipeline :"<< candidate.channel<<candidate.index;
            //QMutexLocker lock(&ChannelLocker);
            Channels.insert(candidate.channel,candidate.index);

        }
        while(!ChannelsRemPipeline.isEmpty()){
            channelstruct candidate=get_RemChannel();
            //qDebug()<<"REMChannel Pipeline :"<< candidate.channel<<candidate.index;
            if ((Channels.contains(candidate.channel))){
                //QMutexLocker lock(&ChannelLocker);
                Channels.remove(candidate.channel,candidate.index);
            }
        }
       //  qDebug()<<"Check Connection Pipeline";
        while(!ConnectionDeletePipeline.isEmpty()){
            //qDebug()<<"Delete Connection Pipeline";
            QByteArray data_delete="";
            QString data="";
            data_delete.append("\"");
            data_delete.append(get_DeleteConnection().toLatin1());
            data_delete.append("\"");
            buff_delete_data.setData(data_delete);
            QByteArray postDataSize = QByteArray::number(data_delete.length());

            requestDelete.setRawHeader("Content-Type", "application/json");
            requestDelete.setRawHeader("Content-Length", postDataSize);
            replydelete =manager.sendCustomRequest(requestDelete,"DELETE",&buff_delete_data);
            //connect(replydelete, SIGNAL(finished()),this, SLOT(finishReplyDelete()));
            connect(replydelete, SIGNAL(finished()),this, SLOT(finishReplyDelete()));
            //qDebug()<<"Remove Connection :"<< data << postDataSize;
        }





        if(Channels.count()!=requestedchannels){
            //qDebug()<<"Checking Channels";
            QString data="{\"channels\":[ ";
            QMutexLocker lock(&ChannelLocker);
            QSet<QString> keys=QSet<QString>::fromList(Channels.keys());
            foreach( QString key,keys){
                if (!key.startsWith("bsread:")){ //removes all header channels
                        data.append("{\"name\":\"");
                        data.append(key);
                        data.append("\",\"modulo\":1,\"offset\":0},");

                }
            }
            data.remove(data.length()-1,1);
            data.append("],\"sendIncompleteMessages\":true,\"compression\":\"none\"}");

            if (!data.contains("channels\":[]")){
                QByteArray transferdata;
                transferdata.append(data);


                //qDebug() <<"Send Test Data"<< StreamDispatcher << data;

                requestChannel.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
                replyConnect = manager.post(requestChannel,transferdata);
                //qDebug() <<transferdata;
                connect(replyConnect, SIGNAL(readyRead()),this, SLOT(finishReplyConnect()));

                requestedchannels=Channels.count();
            }
            //qDebug() <<"Rec  finished: "<<requestedchannels;
        }
        ProcessLocker.unlock();
        loop->processEvents();
    }
    //qDebug()<<"bsread Dispatcher: finished ThreadID (" << QThread::currentThreadId()<< ")";
    msg="bsread Dispatcher finished";

    messagewindowP->postMsgEvent(QtDebugMsg,(char*) msg.toLatin1().constData());
    emit finished();

}
/*
void bsread_dispatchercontrol::deleteStream(QString *value)
{
    QEventLoop eventLoop;
    QNetworkAccessManager manager;
    QByteArray data="";
    QString StreamDispatcher=Dispatcher;
    if (!StreamDispatcher.endsWith("/")){
        StreamDispatcher.append("/");
    }
    if (StreamDispatcher.endsWith("/")){
        StreamDispatcher.append("stream");
    }
    QUrl url(StreamDispatcher);
    request=QNetworkRequest(url);

    data.append("\"");
    data.append(value->toLatin1());
    data.append("\"");
    qDebug()<<"From deleteStream thread: "<<QThread::currentThreadId();
    qDebug() <<"Delete Connection"<< data;
    //QObject::connect(&manager, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    buff_delete_data.setData(data);
    replydelete =manager.sendCustomRequest(request,"DELETE",&buff_delete_data);
    //connect(replydelete, SIGNAL(finished(QNetworkReply*)),this, SLOT(finishReplyDelete(QNetworkReply*)));
    //QObject::connect(&manager, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
    connect(replydelete, SIGNAL(readyRead()),this, SLOT(finishReplyDelete()));
    //eventLoop.exec();
    eventLoop.processEvents();
    if (replydelete->error() == QNetworkReply::NoError) {
        //success
         qDebug() << "Delete Connection Success" <<replydelete->readAll();
         replydelete->deleteLater();
     }
     else {
         //failure
         qDebug() << "Delete Connection Failure" <<replydelete->errorString();
         replydelete->deleteLater();
     }


 }
*/

void bsread_dispatchercontrol::setMessagewindow(MessageWindow *value)
{
    messagewindowP = value;
}


void bsread_dispatchercontrol::setTerminate()
{
    terminate = true;
    startReconnection.wakeAll();
}



int bsread_dispatchercontrol::set_Dispatcher(QString *dispatcher)
{

    Dispatcher=*dispatcher;

    startReconnection.wakeAll();
    return 0;
}
int bsread_dispatchercontrol::add_Channel(QString channel,int index)
{
    QMutexLocker lock(&ChannelAddPipelineLocker);
    channelstruct channeldata;
    channeldata.channel=channel;
    channeldata.index=index;
    ChannelsAddPipeline.append(channeldata);
    //startReconnection.wakeAll();
    //qDebug()<<"ADDChannel"<< channel << index;
    return 0;
}

channelstruct bsread_dispatchercontrol::get_AddChannel(){
    QMutexLocker lock(&ChannelAddPipelineLocker);
    channelstruct result;
    if (!ChannelsAddPipeline.isEmpty()){
        result =ChannelsAddPipeline.front();
        ChannelsAddPipeline.removeFirst();

    }else{
        result.channel="";
        result.index=-1;

    }
    return result;
}
void bsread_dispatchercontrol::deleteStream(QString *value)
{
    QMutexLocker lock(&ConnectionDeleteLocker);
    ConnectionDeletePipeline.append(*value);

}
QString bsread_dispatchercontrol::get_DeleteConnection(){
    QMutexLocker lock(&ConnectionDeleteLocker);
    QString result;
    if (!ConnectionDeletePipeline.isEmpty()){
        result =ConnectionDeletePipeline.front();
        ConnectionDeletePipeline.removeFirst();
    }else{
        result="";
    }
    return result;
}
channelstruct bsread_dispatchercontrol::get_RemChannel(){
    QMutexLocker lock(&ChannelRemPipelineLocker);
    channelstruct result;
    if (!ChannelsRemPipeline.isEmpty()){
        result =ChannelsRemPipeline.front();
        ChannelsRemPipeline.removeFirst();

    }else{
        result.channel="";
        result.index=-1;

    }
    return result;
}






void bsread_dispatchercontrol::setMutexknobdataP(MutexKnobData *value)
{
    mutexknobdataP = value;
}

void bsread_dispatchercontrol::setZmqcontex(void *value)
{
    zmqcontex = value;
}


int bsread_dispatchercontrol::rem_Channel(QString channel,int index)
{
    QMutexLocker lock(&ChannelRemPipelineLocker);
    channelstruct channeldata;
    channeldata.channel=channel;
    channeldata.index=index;
    ChannelsRemPipeline.append(channeldata);
    //qDebug()<<"REMChannel"<< channel << index;
    return 0;

}




void bsread_dispatchercontrol::finishReplyConnect()
{

    JSONObject jsonobj;
    QString stream;
    QString streamType;
    QObject* obj = sender();
    QNetworkReply* reply_local = qobject_cast<QNetworkReply*>(obj);
    QMutexLocker lock(&ChannelLocker);

    QByteArray httpdata;
    streams.clear();
    //qDebug()<<"From finishReplyConnect thread: ("<<QThread::currentThreadId()<<")";

    httpdata=reply_local->readAll();
    reply_local->deleteLater();
    JSONValue *MainMessageJ = JSON::Parse(httpdata);
    //qDebug() << "DATA:" <<httpdata;

    if (MainMessageJ!=NULL){
        if(!MainMessageJ->IsObject()) {
            delete(MainMessageJ);
        } else {
            jsonobj=MainMessageJ->AsObject();
            if (jsonobj.find(L"configuration") != jsonobj.end() && jsonobj[L"configuration"]->IsObject())
            {
                JSONObject jsonobj2=jsonobj[L"configuration"]->AsObject();
                if (jsonobj2.find(L"streamType") != jsonobj2.end() && jsonobj2[L"streamType"]->IsString()) {
                    streamType=QString::fromWCharArray(jsonobj2[L"streamType"]->AsString().c_str());
                }
            }
            qDebug()<< "streamType:"<<streamType;
            if (jsonobj.find(L"stream") != jsonobj.end() && jsonobj[L"stream"]->IsString()) {
                stream=QString::fromWCharArray(jsonobj[L"stream"]->AsString().c_str());
                streams.append(stream);
                bsreadconnections.append(new bsread_Decode(zmqcontex,stream,streamType));
                bsreadThreads.append(new QThread(this));

                bsreadconnections.last()->setKnobData(mutexknobdataP);


                QSet<QString> keys=QSet<QString>::fromList(Channels.keys());

                foreach( QString key,keys)
                    foreach(int value,Channels.values(key)){
                        //qDebug()<< "Register:("<< key << value << ")";
                        bsreadconnections.last()->bsread_DataMonitorConnection(key,value);
                    }


                bsreadconnections.last()->moveToThread(bsreadThreads.last());
                //qDebug() << "Create bsread_Decode:" <<bsreadconnections.last();
                connect(bsreadThreads.last(), SIGNAL(started()), bsreadconnections.last(), SLOT(process()));
                connect(bsreadconnections.last(), SIGNAL(finished()), bsreadThreads.last(), SLOT(quit()));
                //connect(bsreadThreads.last(), SIGNAL(finished()), bsreadThreads.last(), SLOT(deleteLater()));
                //connect(bsreadconnections.last(), SIGNAL(finished()), bsreadconnections.last(), SLOT(deleteLater()));

                bsreadThreads.last()->start();



                while (bsreadconnections.count()>1){
                    //qDebug() << "Delete bsread_Decode:" <<bsreadconnections.first();
                    QString connection=QString(bsreadconnections.first()->getConnectionPoint());
                    deleteStream(&connection);
                    bsreadconnections.first()->setTerminate();
                    bsreadThreads.first()->quit();
                    bsreadThreads.first()->wait();
                    delete(bsreadThreads.first());
                    delete(bsreadconnections.first());

                    bsreadconnections.removeFirst();




                    bsreadThreads.removeFirst();
                }

                //qDebug() << "bsreadPlugin:" << stream.toLatin1().constData();
            }

        }
    }

  //qDebug()<<"finishReplyConnect: finished ThreadID (" << QThread::currentThreadId()<< ")";
  //qDebug()<<"END: finishReplyConnect";
}

void bsread_dispatchercontrol::finishReplyDelete()
{
    QObject* obj = sender();
    QNetworkReply* reply_local = qobject_cast<QNetworkReply*>(obj);
    QMutexLocker lock(&ChannelLocker);
    QByteArray httpdata=reply_local->readAll();
    //qDebug() << "DeleteReply:" <<httpdata;
    //qDebug()<<"From finishReplyDelete thread: ("<<QThread::currentThreadId()<<")";
}

void bsread_dispatchercontrol::closeEvent(){
   //qDebug() << "bsread_dispatchercontrol:closeEvent ";
   this->setTerminate();
    while (bsreadconnections.count()!=0){
       //qDebug() << "closeEvent Delete bsread_Decode:" <<bsreadconnections.first();
        bsreadconnections.first()->setTerminate();
       bsreadThreads.first()->quit();
       bsreadThreads.first()->wait();
       delete(bsreadThreads.first());
       delete(bsreadconnections.first());
       bsreadconnections.removeFirst();
       bsreadThreads.removeFirst();
   }
}

