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
#include <QDebug>
#include "bsread_dispatchercontrol.h"
#include "JSON.h"
#include "JSONValue.h"


bsread_dispatchercontrol::bsread_dispatchercontrol()
{

}
bsread_dispatchercontrol::~bsread_dispatchercontrol()
{
    this->setTerminate();
    startReconnection.wakeAll();
}

void bsread_dispatchercontrol::process()
{
    QNetworkAccessManager manager;
    QEventLoop loop;
    int requestedchannels=0;
    terminate=false;
    QString msg="bsread Dispatcher started: ";
    msg.append(Dispatcher);

    messagewindowP->postMsgEvent(QtDebugMsg,(char*) msg.toLatin1().constData());

    //Update and reconection handling
    while (!terminate){
        //QThread::sleep(200);


        ProcessLocker.lock();
        startReconnection.wait(&ProcessLocker,600);


        QString StreamDispatcher=Dispatcher;
        if (!StreamDispatcher.endsWith("/")){
            StreamDispatcher.append("/");
        }
        if (StreamDispatcher.endsWith("/")){
            StreamDispatcher.append("stream");
        }

        QUrl url(StreamDispatcher);
        request=QNetworkRequest(url);


        // Check Pipeline
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






        if(Channels.count()!=requestedchannels){
            QString data="{\"channels\":[";
            QMutexLocker lock(&ChannelLocker);
            QSet<QString> keys=QSet<QString>::fromList(Channels.keys());
            foreach( QString key,keys){
                data.append("{\"name\":\"");
                data.append(key);
                data.append("\",\"modulo\":1,\"offset\":0},");
            }
            data.remove(data.length()-1,1);
            data.append("],\"sendIncompleteMessages\":true,\"compression\":\"none\"}");

            QByteArray transferdata;
            transferdata.append(data);

            //qDebug() <<"Send TEst Data"<< StreamDispatcher << data;

            request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
            reply = manager.post(request,transferdata);

            connect(reply, SIGNAL(readyRead()),this, SLOT(finishReply()));
            requestedchannels=Channels.count();
            //qDebug() <<"Rec  finished: "<<requestedchannels;
        }

        ProcessLocker.unlock();
        loop.processEvents();


    }
    msg="bsread Dispatcher finished";

    messagewindowP->postMsgEvent(QtDebugMsg,(char*) msg.toLatin1().constData());
    emit finished();



}
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




void bsread_dispatchercontrol::finishReply()
{

    JSONObject jsonobj;
    QString stream;
    QObject* obj = sender();
    QNetworkReply* reply_local = qobject_cast<QNetworkReply*>(obj);
    QMutexLocker lock(&ChannelLocker);

    QByteArray httpdata;
    streams.clear();


    httpdata=reply_local->readAll();
    JSONValue *MainMessageJ = JSON::Parse(httpdata);
    //qDebug() << "DATA:" <<httpdata;

    if (MainMessageJ!=NULL){
        if(!MainMessageJ->IsObject()) {
            delete(MainMessageJ);
        } else {
            jsonobj=MainMessageJ->AsObject();
            if (jsonobj.find(L"stream") != jsonobj.end() && jsonobj[L"stream"]->IsString()) {
                stream=QString::fromWCharArray(jsonobj[L"stream"]->AsString().c_str());
                streams.append(stream);
                bsreadconnections.append(new bsread_Decode(zmqcontex,stream));
                bsreadThreads.append(new QThread(this));

                bsreadconnections.last()->setKnobData(mutexknobdataP);


                QSet<QString> keys=QSet<QString>::fromList(Channels.keys());

                foreach( QString key,keys)
                    foreach(int value,Channels.values(key)){
                        //qDebug()<< "Register:("<< key << value << ")";
                        bsreadconnections.last()->bsread_DataMonitorConnection(key,value);
                    }


                bsreadconnections.last()->moveToThread(bsreadThreads.last());
                connect(bsreadThreads.last(), SIGNAL(started()), bsreadconnections.last(), SLOT(process()));
                connect(bsreadconnections.last(), SIGNAL(finished()), bsreadThreads.last(), SLOT(quit()));
                connect(bsreadThreads.last(), SIGNAL(finished()), bsreadThreads.last(), SLOT(deleteLater()));
                connect(bsreadconnections.last(), SIGNAL(finished()), bsreadconnections.last(), SLOT(deleteLater()));


                bsreadThreads.last()->start();



                while (bsreadconnections.count()>1){
                    bsreadconnections.first()->setTerminate();

                    bsreadconnections.removeFirst();

                    bsreadThreads.removeFirst();
                }

                qDebug() << "stream :" << stream.toLatin1().constData();
            }

        }
    }



}

