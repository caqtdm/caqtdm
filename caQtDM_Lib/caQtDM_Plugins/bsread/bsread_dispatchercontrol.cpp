#include <QDebug>
#include "bsread_dispatchercontrol.h"
#include "JSON.h"
#include "JSONValue.h"

bsread_dispatchercontrol::bsread_dispatchercontrol(QObject *parent) : QThread(parent)
{

}

void bsread_dispatchercontrol::run()
{
    QNetworkAccessManager manager;
    int requestedchannels=0;


    //Update and reconection handling
    while (1){
        //QThread::sleep(200);


        ProcessLocker.lock();
        startReconnection.wait(&ProcessLocker,200);


        QString StreamDispatcher=Dispatcher;
        StreamDispatcher.append("stream");
        QUrl url(StreamDispatcher);
        request=QNetworkRequest(url);


        // Check Pipeline
        bool channelsadded=false;
        while(!ChannelsPipeline.isEmpty()){
            QString candidate=get_Channel();
            if (!(Channels.contains(candidate))){
                Channels.append(candidate);
                channelsadded=true;
            }
        }

        if(Channels.count()!=requestedchannels){
            QString data="{\"channel:\"[";
            QListIterator<QString> i(Channels);
            while (i.hasNext()){
                data.append("{\"name\":\"");
                data.append(i.next());
                data.append("\",\"modulo\":1,\"offset\":0}");
            }

            data.append("]},\"sendIncompleteMessages\":true,}");

            QByteArray transferdata;
            transferdata.append(data);

            qDebug() <<"Send TEst Data"<< StreamDispatcher << data;

            request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
            reply = manager.post(request,transferdata);

            connect(reply, SIGNAL(readyRead()),this, SLOT(finishReply()));
            requestedchannels=Channels.count();
            qDebug() <<"Rec  finished: "<<requestedchannels;
        }

        ProcessLocker.unlock();



    }




}



int bsread_dispatchercontrol::set_Dispatcher(QString *dispatcher)
{

    Dispatcher=*dispatcher;
    startReconnection.wakeAll();
    return 0;
}
int bsread_dispatchercontrol::add_Channel(QString *channel)
{
    QMutexLocker lock(&ChannelLocker);
    ChannelsPipeline.append(*channel);
    startReconnection.wakeAll();
    qDebug()<<"ADDChannel";
    return 0;
}

QString bsread_dispatchercontrol::get_Channel(){
    QMutexLocker lock(&ChannelLocker);
    if (!ChannelsPipeline.isEmpty()){
        QString result =ChannelsPipeline.front();
        ChannelsPipeline.removeFirst();
        return result;
    }else{
        return NULL;
    }
}


int bsread_dispatchercontrol::rem_Channel(QString *channel)
{
    if ((Channels.contains(*channel))){
        Channels.removeOne(*channel);
        qDebug()<<"REMOVEChannel";
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

                qDebug() << "stream :" << stream.toLatin1().constData();
            }

        }
    }



}
void bsread_dispatchercontrol::httpReadyRead()
{
    qDebug() <<"Testdata received http";
}



