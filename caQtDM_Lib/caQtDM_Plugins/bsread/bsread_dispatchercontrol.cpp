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
    //Special Channels
    bsreadChannels.append("bsread:hash");
    bsreadChannels.append("bsread:pulse_id");
    bsreadChannels.append("bsread:htype");
    bsreadChannels.append("bsread:global_timestamp_ns");
    bsreadChannels.append("bsread:global_timestamp_sec");

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


    if (!ChannelsAddPipeline.isEmpty()){
        ChannelVerification(&manager);
    }

        // qDebug()<<"Check Pipeline";
        while(!ChannelsApprovePipeline.isEmpty()){
            Channels+=ChannelsApprovePipeline;
            ChannelsApprovePipeline.clear();
        }


        if (tobeRemoved.count()>0){
          requestedchannels=0;
          //qDebug()<<"tobeRemoved Pipeline :" << tobeRemoved.count() << Channels.count() <<tobeRemoved.at(0).trimmed()<< Channels.first();
          for (int x=0;x<=tobeRemoved.count()-1;x++){
                QString chan=tobeRemoved.at(x).trimmed();
                //qDebug()<<"search :"<< chan;
                QMultiMap<QString, int>::iterator i = Channels.find(chan);
                 while (i != Channels.end() && i.key() == chan) {
                     qDebug()<<"found :"<< chan << i.value();
                     rem_Channel(chan,i.value());
                     ++i;
                 }
           }
          tobeRemoved.clear();
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





        if((Channels.count()!=requestedchannels)){
            //qDebug()<<"Checking Channels: "<< Channels.count();
            QString data="{\"channels\":[ ";
            QMutexLocker lock(&ChannelLocker);
            QSet<QString> keys=QSet<QString>::fromList(Channels.keys());
            foreach( QString key,keys){
                if (!key.startsWith("bsread:")){ //removes all header channels
                        data.append("{\"name\":\"");
                        data.append(key);
                        //data.append("\",\"modulo\":1,\"offset\":0},");
                        data.append("\"},");
                }
            }
            data.remove(data.length()-1,1);
            data.append("],\"sendIncompleteMessages\":true,\"compression\":\"none\",");
            data.append("\"mapping\":{\"incomplete\":\"fill-null\"},");
            data.append("\"channelValidation\":{\"inconsistency\":\"keep-as-is\"}}");


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
    QString msg="bsread: ";
    msg.append(httpdata);

    if (msg.contains("exception")){
        messagewindowP->postMsgEvent(QtCriticalMsg,(char*) msg.toLatin1().constData());
    }else{

        messagewindowP->postMsgEvent(QtDebugMsg,(char*) msg.toLatin1().constData());
    }
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

            if (jsonobj.find(L"exception") != jsonobj.end() && jsonobj[L"exception"]->IsString()) {
                qDebug()<< "Check exception:";
                tobeRemoved.clear();
                QString ExceptionError=QString::fromWCharArray(jsonobj[L"exception"]->AsString().c_str());
                if (ExceptionError.startsWith("java.lang.IllegalArgumentException")){
                    if (jsonobj.find(L"message") != jsonobj.end() && jsonobj[L"message"]->IsString()) {
                        QString ErrorString=QString::fromWCharArray(jsonobj[L"message"]->AsString().c_str());
                        QStringList ErrorChannels = ErrorString.split(",", QString::SkipEmptyParts);
                        if (ErrorChannels.count()>0){

                            qDebug()<< "ErrorChannels.count():" << ErrorChannels.count();

                            if (ErrorChannels.at(0).contains("recorded:")){
                                tobeRemoved.append(ErrorChannels.at(0).split(": ", QString::SkipEmptyParts).at(1).split(" - ",QString::SkipEmptyParts).at(0));
                                if (ErrorChannels.count()>1){
                                    for (int x=1;x<ErrorChannels.count()-1;x++){
                                        tobeRemoved.append(ErrorChannels.at(x).split(" - ",QString::SkipEmptyParts).at(0));
                                    }
                                }



                            }

                            qDebug()<< "tobeRemoved:" <<tobeRemoved;

                        }



                    }

                }
            }//if jsonobj
            //qDebug()<< tobeRemoved;

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

void bsread_dispatchercontrol::ChannelVerification(QNetworkAccessManager* manager){

    QMultiMap<QString,int> CheckChannels;
    QString ChannelQueryVerification=Dispatcher;
    if (!ChannelQueryVerification.endsWith("/")){
        ChannelQueryVerification.append("/");
    }
    if (ChannelQueryVerification.endsWith("/")){
        ChannelQueryVerification.append("channels/state");
    }

    QUrl url(ChannelQueryVerification);
    requestVerification = QNetworkRequest(url);
#ifndef QT_NO_SSL
    if(url.toString().toUpper().contains("HTTPS")) {
        QSslConfiguration configChannel = requestVerification.sslConfiguration();
        configChannel.setPeerVerifyMode(QSslSocket::VerifyNone);
        requestVerification.setSslConfiguration(configChannel);
    }
#endif

    QString data="{\"channels\":[ ";
    while(!ChannelsAddPipeline.isEmpty()){
        channelstruct candidate=get_AddChannel();
        ChannelsToBeApprovePipeline.insert(candidate.channel,candidate.index);
    }

    QSet<QString> keys=QSet<QString>::fromList(ChannelsToBeApprovePipeline.keys());
    foreach( QString key,keys){
        if (!key.startsWith("bsread:")){ //removes all header channels
            data.append("\"");
            data.append(key);
            data.append("\",");
        }
    }
    data.remove(data.length()-1,1);
    data.append("]}");
    if (!data.contains("channels\":[]")){
        QByteArray transferdata;
        transferdata.append(data);
        requestVerification.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
        replyVerification = manager->post(requestVerification,transferdata);
        //qDebug()<<"ChannelVerification() :"<<url.toString() <<" : "<<transferdata;
        connect(replyVerification, SIGNAL(readyRead()),this, SLOT(finishVerification()));
    }
    //qDebug()<<"ChannelVerification() :"<< data;
}
void bsread_dispatchercontrol::finishVerification()
{

    JSONArray jsonobj;
    QObject* obj = sender();
    QNetworkReply* reply_local = qobject_cast<QNetworkReply*>(obj);

    QByteArray httpdata;

    httpdata=reply_local->readAll();
    reply_local->deleteLater();
    JSONValue *MainMessageJ = JSON::Parse(httpdata);

    QString msg="bsread: ";
    msg.append(httpdata);
    if (MainMessageJ!=NULL){
        if(!MainMessageJ->IsArray()) {
            qDebug()<<"finishVerification() : MainMessageJ=NULL";
            delete(MainMessageJ);
        } else {
            jsonobj=MainMessageJ->AsArray();
            //qDebug()<<"finishVerification() : Step array :"<< jsonobj.size();

            for (unsigned int i = 0; i < jsonobj.size(); i++)
            {
                JSONObject jsonobj2=jsonobj[i]->AsObject();
                if (jsonobj2.find(L"recording") != jsonobj2.end() && jsonobj2[L"recording"]->IsBool())
                {
                    if (jsonobj2[L"recording"]->AsBool()){
                        //qDebug()<<"finishVerification() : Recording ok";
                        QString name="";
                        if (jsonobj2.find(L"channel") != jsonobj2.end() && jsonobj2[L"channel"]->IsObject())
                        {
                            JSONObject jsonobj3=jsonobj2[L"channel"]->AsObject();


                            if (jsonobj3.find(L"name") != jsonobj3.end() && jsonobj3[L"name"]->IsString()) {
                                name=QString::fromWCharArray(jsonobj3[L"name"]->AsString().c_str());
                            }

                        }

                        QList<int> values = ChannelsToBeApprovePipeline.values(name);
                        for (int i = 0; i < values.size(); ++i){
                            channelstruct candidate;
                            candidate.channel=name;
                            candidate.index=values.at(i);
                            ChannelsApprovePipeline.insert(candidate.channel,candidate.index);
                            ChannelsToBeApprovePipeline.remove(name);
                        }

                    }
                }
            }
        }
    }

    // for special bsread channels
    for (int j = 0; j < bsreadChannels.size(); ++j){
        QList<int> values = ChannelsToBeApprovePipeline.values(bsreadChannels.at(j));
        for (int i = 0; i < values.size(); ++i){
            channelstruct candidate;
            candidate.channel=bsreadChannels.at(j);
            candidate.index=values.at(i);
            ChannelsApprovePipeline.insert(candidate.channel,candidate.index);
            ChannelsToBeApprovePipeline.remove(bsreadChannels.at(j));
        }
    }

    qDebug()<<"finishVerification() :"<< ChannelsApprovePipeline.count();// httpdata;

    if (msg.contains("exception")){
        messagewindowP->postMsgEvent(QtCriticalMsg,(char*) msg.toLatin1().constData());
        // Request all channels we have, and see what will come
        ChannelsApprovePipeline+=ChannelsToBeApprovePipeline;
        ChannelsToBeApprovePipeline.clear();
    }
}
