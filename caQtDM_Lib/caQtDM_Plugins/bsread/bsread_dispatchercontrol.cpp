#include <QDebug>
#include "bsread_dispatchercontrol.h"
#include "JSON.h"
#include "JSONValue.h"

bsread_dispatchercontrol::bsread_dispatchercontrol()
{

}

void bsread_dispatchercontrol::process()
{
    QNetworkAccessManager manager;
    QEventLoop loop;
    int requestedchannels=0;



    //Update and reconection handling
    while (1){
        //QThread::sleep(200);


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
        request=QNetworkRequest(url);


        // Check Pipeline
        bool channelsadded=false;
        while(!ChannelsPipeline.isEmpty()){
            channelstruct candidate=get_Channel();
            //qDebug()<<"ADDChannel Pipeline :"<< candidate.channel<<candidate.index;
             QMutexLocker lock(&ChannelLocker);
             Channels.insert(candidate.channel,candidate.index);

             channelsadded=true;
        }

        if(Channels.count()!=requestedchannels){
            QString data="{\"channel:\"[";
            QMutexLocker lock(&ChannelLocker);
            QSet<QString> keys=QSet<QString>::fromList(Channels.keys());
            foreach( QString key,keys){
                data.append("{\"name\":\"");
                data.append(key);
                data.append("\",\"modulo\":1,\"offset\":0}");
            }

            data.append("]},\"sendIncompleteMessages\":true,}");

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




}



int bsread_dispatchercontrol::set_Dispatcher(QString *dispatcher)
{

    Dispatcher=*dispatcher;

    startReconnection.wakeAll();
    return 0;
}
int bsread_dispatchercontrol::add_Channel(QString channel,int index)
{
    QMutexLocker lock(&ChannelPipelineLocker);
    channelstruct channeldata;
    channeldata.channel=channel;
    channeldata.index=index;
    ChannelsPipeline.append(channeldata);
    //startReconnection.wakeAll();
    //qDebug()<<"ADDChannel"<< channel << index;
    return 0;
}

channelstruct bsread_dispatchercontrol::get_Channel(){
    QMutexLocker lock(&ChannelPipelineLocker);
    channelstruct result;
    if (!ChannelsPipeline.isEmpty()){
        result =ChannelsPipeline.front();
        ChannelsPipeline.removeFirst();

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
   QMutexLocker lock(&ChannelLocker);
    if ((Channels.contains(channel))){
        Channels.remove(channel,index);
        //qDebug()<<"REMOVEChannel";
        startReconnection.wakeAll();
        return 0;
    }else{
        return -1;
    }
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

