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
    mutexknobdataP = Q_NULLPTR;
    //Special Channels
    bsreadChannels.append("bsread:hash");
    bsreadChannels.append("bsread:pulse_id");
    bsreadChannels.append("bsread:htype");
    bsreadChannels.append("bsread:global_timestamp_ns");
    bsreadChannels.append("bsread:global_timestamp_sec");

    DispatcherChannels.append("bsread:bsmodulo");
    DispatcherChannels.append("bsread:bsoffset");
    DispatcherChannels.append("bsread:bsinconsistency");
    DispatcherChannels.append("bsread:bsmapping");
    DispatcherChannels.append("bsread:bsstrategy");

    bsread_internalchannel *opt;

    opt=new bsread_internalchannel(this,"bsread:bsmodulo","bsmodulo");
    opt->setData(Q_NULLPTR,bsread_internalchannel::in_string);
    opt->setString("1");
    DispatcherChannels_Connected.insert(opt->getPv_name(),opt);

    opt=new bsread_internalchannel(this,"bsread:bsoffset","bsoffset");
    opt->setData(Q_NULLPTR,bsread_internalchannel::in_string);
    opt->setString("0");
    DispatcherChannels_Connected.insert(opt->getPv_name(),opt);


    opt=new bsread_internalchannel(this,"bsread:bsinconsistency","bsinconsistency");
    opt->setData(Q_NULLPTR,bsread_internalchannel::in_enum);
    opt->addEnumString("drop");
    opt->addEnumString("keep-as-is");
    opt->addEnumString("adjust-individual");
    opt->addEnumString("adjust-global");
    opt->setString("keep-as-is");
    DispatcherChannels_Connected.insert(opt->getPv_name(),opt);

    opt=new bsread_internalchannel(this,"bsread:bsmapping","bsmapping");
    opt->setData(Q_NULLPTR,bsread_internalchannel::in_enum);
    opt->addEnumString("provide-as-is");
    opt->addEnumString("drop");
    opt->addEnumString("fill-null");
    opt->setString("fill-null");
    DispatcherChannels_Connected.insert(opt->getPv_name(),opt);

    opt=new bsread_internalchannel(this,"bsread:bsstrategy","bsstrategy");
    opt->setData(Q_NULLPTR,bsread_internalchannel::in_enum);
    opt->addEnumString("complete-all");
    opt->addEnumString("complete-latest");
    opt->setString("complete-all");
    DispatcherChannels_Connected.insert(opt->getPv_name(),opt);


}
bsread_dispatchercontrol::~bsread_dispatchercontrol()
{
    this->setTerminate();
    loop->quit();
    delete(loop);
    //qDebug()<<"Dispatcher stop";
    //printf("bsread Dispatcher destructor\n");
   //fflush(stdout);

}

void bsread_dispatchercontrol::process()
{

    QNetworkAccessManager manager;
    bool init_reconnection=false;

    int requestedchannels=0;
    terminate=false;
    QString msg="bsread Dispatcher started: ";
    msg.append(Dispatcher);


    messagewindowP->postMsgEvent(QtDebugMsg,(char*) msg.toLatin1().constData());
    //qDebug()<<"bsread Dispatcher: Start ThreadID: "<<QThread::currentThreadId();
    //Update and reconection handling

    if (!optionsP.empty()){

        processOption(optionsP,"bsmodulo");
        processOption(optionsP,"bsoffset");
        processOption(optionsP,"bsinconsistency");
        processOption(optionsP,"bsmapping");
        processOption(optionsP,"bsstrategy");

    }



    while (!terminate){
        //QThread::msleep(200);

        ProcessLocker.lock();
        startReconnection.wait(&ProcessLocker,400);

        init_reconnection=false;
        QString l_bsmodulo=get_internalChannel("bsread:bsmodulo")->getString();
        init_reconnection=init_reconnection||get_internalChannel("bsread:bsmodulo")->getProc();
        QString l_bsoffset=get_internalChannel("bsread:bsoffset")->getString();
        init_reconnection=init_reconnection||get_internalChannel("bsread:bsoffset")->getProc();
        QString l_bsinconsistency=get_internalChannel("bsread:bsinconsistency")->getString();
        init_reconnection=init_reconnection||get_internalChannel("bsread:bsinconsistency")->getProc();
        QString l_bsmapping=get_internalChannel("bsread:bsmapping")->getString();
        init_reconnection=init_reconnection||get_internalChannel("bsread:bsmapping")->getProc();
        QString l_bsstrategy=get_internalChannel("bsread:bsstrategy")->getString();
        init_reconnection=init_reconnection||get_internalChannel("bsread:bsstrategy")->getProc();


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
#ifndef CAQTDM_SSL_IGNORE
#ifndef QT_NO_SSL
        if(url.toString().toUpper().contains("HTTPS")) {
            QSslConfiguration configChannel = requestChannel.sslConfiguration();
            configChannel.setPeerVerifyMode(QSslSocket::VerifyNone);
            #if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
                configChannel.setProtocol(QSsl::TlsV1);
            #endif
            requestChannel.setSslConfiguration(configChannel);

            QSslConfiguration configDelete = requestDelete.sslConfiguration();
            configDelete.setPeerVerifyMode(QSslSocket::VerifyNone);
            #if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
                configDelete.setProtocol(QSsl::TlsV1);
            #endif
            requestDelete.setSslConfiguration(configDelete);
        }

#endif
#endif

        if (!ChannelsAddPipeline.isEmpty()){
            ChannelVerification(&manager);
        }

        //qDebug()<<"Check Pipeline";
        while(!ChannelsApprovePipeline.isEmpty()){
            Channels+=ChannelsApprovePipeline;
            ChannelsApprovePipeline.clear();
            init_reconnection=true;
        }


        if (tobeRemoved.count()>0){
            requestedchannels=0;
            //qDebug()<<"tobeRemoved Pipeline :" << tobeRemoved.count() << Channels.count() <<tobeRemoved.at(0).trimmed()<< Channels.first();
            for (int x=0;x<=tobeRemoved.count()-1;x++){
                QString chan=tobeRemoved.at(x).trimmed();
                //qDebug()<<"search :"<< chan;
                QMultiMap<QString, int>::iterator i = Channels.find(chan);
                while (i != Channels.end() && i.key() == chan) {
                    //qDebug()<<"found :"<< chan << i.value();
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
                init_reconnection=true;
            }
        }

        //qDebug()<<"Check Connection Pipeline";
        while((!ConnectionDeletePipeline.isEmpty())){
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
            #if QT_VERSION < QT_VERSION_CHECK(4, 8, 0)
			  //delete without data !!!
              replydelete =manager.deleteResource(requestDelete);
              connect(replydelete, SIGNAL(finished()),this, SLOT(finishReplyDelete()));
			#else
			  replydelete =manager.sendCustomRequest(requestDelete,"DELETE",&buff_delete_data);
              connect(replydelete, SIGNAL(finished()),this, SLOT(finishReplyDelete()));
            #endif

            //connect(replydelete, SIGNAL(finished()),this, SLOT(finishReplyDelete()));

            //qDebug()<<"Remove Connection :"<< data << postDataSize;

        }




        if(init_reconnection){
            //qDebug()<<"Checking Channels: "<< Channels.count() << "init_reconnection" << init_reconnection;

            init_reconnection=false;
            QString data="{\"channels\":[ ";
            QMutexLocker lock(&ChannelLocker);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            QSet<QString> keys=QSet<QString>::fromList(Channels.keys());
#else
            QSet<QString> keys(Channels.keys().begin(),Channels.keys().end());
#endif
            foreach( QString key,keys){
                if (!key.startsWith("bsread:")){ //removes all header channels
                    if (!key.contains(".BSREADSHAPE")&&!key.contains(".ENC_GROUP")&&!key.contains(".ENC_TYPE")&&!key.contains(".ENC_SUBTYPE")){//removes shape waveform channels
                        data.append("{\"name\":\"");
                        data.append(key+"\"");
                        data.append(",\"modulo\": "+l_bsmodulo);
                        data.append(",\"offset\": "+l_bsoffset);
                        data.append("},");
                    }
                }
            }
            data.remove(data.length()-1,1);
            data.append("],\"sendIncompleteMessages\":true,\"compression\":\"none\",");
            data.append("\"mapping\":{\"incomplete\":\""+l_bsmapping+"\"},");
            data.append("\"channelValidation\":{\"inconsistency\":\""+l_bsinconsistency+"\"}}");
            data.append("\"sendBehavior\":{\"strategy\":\""+l_bsstrategy+"\"}}");

            if (!data.contains("channels\":[]")){
                QByteArray transferdata;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                transferdata.append(data);
#else
                transferdata.append(data.toLatin1());
#endif
                msg="Dispatcher Request (";
                msg.append(QString::number(Channels.count()));
                msg.append(")");
                messagewindowP->postMsgEvent(QtDebugMsg,(char*) msg.toLatin1().constData());


                qDebug() <<"Send Test Data"<< StreamDispatcher << transferdata;

                requestChannel.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
                replyConnect = manager.post(requestChannel,transferdata);
                //qDebug() <<transferdata;
                connect(replyConnect, SIGNAL(readyRead()),this, SLOT(finishReplyConnect()));

                requestedchannels=Channels.count();
            }else{
               // delete all streams because no receive data is requested
               cleanStreamConnections(0);
            }
            //qDebug() <<"Rec  finished: "<<requestedchannels;
        }
        ProcessLocker.unlock();
        loop->processEvents();

    }
    //qDebug()<<"bsread Dispatcher: finished ThreadID (" << QThread::currentThreadId()<< ")";
    //msg="bsread Dispatcher finished";

    //messagewindowP->postMsgEvent(QtDebugMsg,(char*) msg.toLatin1().constData());
    emit finished();
    //printf("bsread Dispatcher finished\n");
    //fflush(stdout);

}


bsread_internalchannel* bsread_dispatchercontrol::get_internalChannel(QString value){
    bsread_internalchannel *bsreadPV=NULL;
    QMap<QString, QPointer<bsread_internalchannel> >::iterator i = DispatcherChannels_Connected.find(value);
    while (i !=DispatcherChannels_Connected.end() && i.key() == value) {
        bsreadPV = i.value();
        break;
    }
    return bsreadPV;
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
void bsread_dispatchercontrol::setOptions(QMap<QString, QString> options){
    optionsP=options;
}

void bsread_dispatchercontrol::processOption(QMap<QString, QString> options, QString option)
{
    bsread_internalchannel* ich;
    QMap<QString, QString>::const_iterator m = options.find(option);
    while (m != options.end() && m.key() == option) {
        ich=get_internalChannel("bsread:"+option);
        if (ich){
         ich->setString(m.value());
        }
        ++m;
    }

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


int bsread_dispatchercontrol::filldispatcherchannels2(bsread_internalchannel *channel,int index){

    if (mutexknobdataP){
         knobData* kData = mutexknobdataP->GetMutexKnobDataPtr(index);
         if (kData){
             if (channel->getType()==bsread_internalchannel::in_string){
                 //qDebug() << " FILL Channel:" << channel->getPv_name() << index;
                 kData->edata.fieldtype=caSTRING;
                 kData->edata.nelm=10;
                 kData->edata.dataSize=10;
                 QString data=channel->getString();
                 kData->edata.valueCount=data.length()+1;
                 kData->edata.dataB=malloc(kData->edata.nelm+1);

                 qstrncpy((char*) kData->edata.dataB, data.toLatin1().data(), (size_t) kData->edata.dataSize);
                 ((char*) kData->edata.dataB)[data.length()+1]='\0';

             }
             if (channel->getType()==bsread_internalchannel::in_enum){
                 QString data=channel->getEnumStrings();
                 //qDebug() << " FILL Channel:" << channel->getPv_name() << index <<data.length()<<data.toLatin1().data() ;

                 kData->edata.fieldtype=caENUM;
                 kData->edata.nelm=(int)strlen(data.toLatin1().data());
                 kData->edata.dataSize=(int)strlen(data.toLatin1().data());
                 kData->edata.enumCount=channel->getEnumCount();
                 kData->edata.valueCount=1;
                 kData->edata.ivalue=channel->getEnumIndex();

                 //qDebug() << " ENUM:" << kData->edata.valueCount << kData->edata.ivalue;

                 kData->edata.dataB=malloc(kData->edata.nelm+2);

                 qstrncpy((char*) kData->edata.dataB,data.toLatin1().data(),(size_t) kData->edata.dataSize);
                 ((char*) kData->edata.dataB)[data.length()+1]='\0';

             }


             qstrncpy(kData->edata.fec,"localhost",caqtdm_string_t_length);
             kData->edata.severity=0;
             kData->edata.connected = true;
             kData->edata.accessR = true;
             kData->edata.accessW = true;
             kData->edata.monitorCount++;
             mutexknobdataP->SetMutexKnobData(kData->index, *kData);
             mutexknobdataP->SetMutexKnobDataReceived(kData);

         }

    }


    return 0;
}
int bsread_dispatchercontrol::add_Channel(QString channel,int index)
{
    QMutexLocker lock(&ChannelAddPipelineLocker);

    if (DispatcherChannels.contains(channel)){

        bsread_internalchannel *bsreadPV=NULL;
        QMap<QString, QPointer<bsread_internalchannel> >::iterator i = DispatcherChannels_Connected.find(channel);
        while (i !=DispatcherChannels_Connected.end() && i.key() == channel) {
            bsreadPV = i.value();
            break;
        }
       if (bsreadPV){
         //qDebug() << " Ping: bsreadPV" <<index ;
         bsreadPV->addIndex(index);
         filldispatcherchannels2(bsreadPV,index);
       }



       //DispatcherChannelsConnected.insert(channel,index);


    }else{
        channelstruct channeldata;
        channeldata.channel=channel;
        channeldata.index=index;
        ChannelsAddPipeline.append(channeldata);
        //startReconnection.wakeAll();
        //qDebug()<<"ADDChannel"<< channel << index;
    }
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
    //qDebug()<<"setMutexknobdataP"<<mutexknobdataP;
}

void bsread_dispatchercontrol::setZmqcontex(void *value)
{
    zmqcontex = value;
}


int bsread_dispatchercontrol::rem_Channel(QString channel,int index)
{
    QMutexLocker lock(&ChannelRemPipelineLocker);
    bsread_internalchannel *bsreadPV=NULL;
    QMap<QString, QPointer<bsread_internalchannel> >::iterator i = DispatcherChannels_Connected.find(channel);
    while (i !=DispatcherChannels_Connected.end() && i.key() == channel) {
        bsreadPV = i.value();
        break;
    }

    if (bsreadPV){
        bsreadPV->deleteIndex(index);

    }else{
        channelstruct channeldata;
        channeldata.channel=channel;
        channeldata.index=index;
        ChannelsRemPipeline.append(channeldata);
    }
        //qDebug()<<"REMChannel"<< channel << index;
    return 0;

}

int bsread_dispatchercontrol::set_Channel(char *pv, double rdata, int32_t idata, char *sdata, char *object, char *errmess, int forceType)
{
    //bsreadThreads.last()
    Q_UNUSED(forceType);
    Q_UNUSED(errmess);
    Q_UNUSED(object);
    Q_UNUSED(idata);
    Q_UNUSED(rdata);

    QString PV_String=QString(pv);

    //qDebug() << "bsread_dispatchercontrol::set_Channel" << PV_String;

    bsread_internalchannel *bsreadPV=get_internalChannel(PV_String);

    if (bsreadPV){
      // update variable
      switch (bsreadPV->getType()){
      case bsread_internalchannel::in_string:{
        QString data=QString(sdata);
        bsreadPV->setString(data);
        break;
      }
      case bsread_internalchannel::in_enum:{
        QString data=QString(sdata);
        bsreadPV->setString(data);
        break;
      }

      }

     //update all index connections
     //qDebug() << "Count:" << bsreadPV->getIndexCount();

     for (int d=0;d<bsreadPV->getIndexCount();d++){
       //qDebug() << "Index:" << d<< bsreadPV->getIndex(d);
       knobData* kData = mutexknobdataP->GetMutexKnobDataPtr(bsreadPV->getIndex(d));
       if (kData){
           //qDebug() << "Ping:" << d;
           switch (bsreadPV->getType()){
               case bsread_internalchannel::in_string:{
                 qstrncpy((char *)kData->edata.dataB, sdata,(size_t)kData->edata.dataSize);
                 break;
               }
               case bsread_internalchannel::in_enum:{
                 kData->edata.ivalue=bsreadPV->getEnumIndex();
                 break;
               }

           }
           kData->edata.connected = true;
           kData->edata.monitorCount++;

           mutexknobdataP->SetMutexKnobData(kData->index, *kData);
           mutexknobdataP->SetMutexKnobDataReceived(kData);

     }

    }
     return true;
   }

    return false;
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
    if (MainMessageJ!=Q_NULLPTR){
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


#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            QSet<QString> keys=QSet<QString>::fromList(Channels.keys());
#else
            QSet<QString> keys(Channels.keys().begin(),Channels.keys().end());
#endif
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
                cleanStreamConnections(1);
                // Remove internal data processing flags
                QMap<QString, QPointer<bsread_internalchannel> >::iterator i;
                for (i = DispatcherChannels_Connected.begin(); i != DispatcherChannels_Connected.end(); ++i) i.value()->resetProc();

                //qDebug() << "bsreadPlugin:" << stream.toLatin1().constData();
            }

            if (jsonobj.find(L"exception") != jsonobj.end() && jsonobj[L"exception"]->IsString()) {
                qDebug()<< "Check exception:";
                tobeRemoved.clear();
                QString ExceptionError=QString::fromWCharArray(jsonobj[L"exception"]->AsString().c_str());
                if (ExceptionError.startsWith("java.lang.IllegalArgumentException")){
                    if (jsonobj.find(L"message") != jsonobj.end() && jsonobj[L"message"]->IsString()) {
                        QString ErrorString=QString::fromWCharArray(jsonobj[L"message"]->AsString().c_str());
                        QStringList ErrorChannels = ErrorString.split(",", SKIP_EMPTY_PARTS);
                        if (ErrorChannels.count()>0){

                            qDebug()<< "ErrorChannels.count():" << ErrorChannels.count();

                            if (ErrorChannels.at(0).contains("recorded:")){
                                tobeRemoved.append(ErrorChannels.at(0).split(": ", SKIP_EMPTY_PARTS).at(1).split(" - ",SKIP_EMPTY_PARTS).at(0));
                                if (ErrorChannels.count()>1){
                                    for (int x=1;x<ErrorChannels.count()-1;x++){
                                        tobeRemoved.append(ErrorChannels.at(x).split(" - ",SKIP_EMPTY_PARTS).at(0));
                                    }
                                }



                            }

                            //qDebug()<< "tobeRemoved:" <<tobeRemoved;

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
void bsread_dispatchercontrol::cleanStreamConnections(int check){

    while (bsreadconnections.count()>check){
        //qDebug() << "Delete bsread_Decode:" <<bsreadconnections.first();
        bsreadconnections.first()->setTerminate();
        qDebug() << "Delete bsread_Decode";
        QString connection=QString(bsreadconnections.first()->getConnectionPoint());

        deleteStream(&connection);

        bsreadThreads.first()->quit();
        bsreadThreads.first()->wait();
        delete(bsreadThreads.first());
        delete(bsreadconnections.first());

        bsreadconnections.removeFirst();
        bsreadThreads.removeFirst();
    }

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
#ifndef CAQTDM_SSL_IGNORE
#ifndef QT_NO_SSL
    if(url.toString().toUpper().contains("HTTPS")) {
        QSslConfiguration configChannel = requestVerification.sslConfiguration();
        configChannel.setPeerVerifyMode(QSslSocket::VerifyNone);
        #if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
            configChannel.setProtocol(QSsl::TlsV1);
        #endif
        requestVerification.setSslConfiguration(configChannel);
    }
#endif
#endif

    QString data="{\"channels\":[ ";
    while(!ChannelsAddPipeline.isEmpty()){
        channelstruct candidate=get_AddChannel();

        // for special waveform shape channels
/*
        if (candidate.channel.contains(".BSREADSHAPE")){
            ChannelsApprovePipeline.insert(candidate.channel,candidate.index);
        }else{
            ChannelsToBeApprovePipeline.insert(candidate.channel,candidate.index);
        }
*/
        // Channel Check Removed!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        ChannelsApprovePipeline.insert(candidate.channel,candidate.index);
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            QSet<QString> keys=QSet<QString>::fromList(ChannelsToBeApprovePipeline.keys());
#else
            QSet<QString> keys(ChannelsToBeApprovePipeline.keys().begin(),ChannelsToBeApprovePipeline.keys().end());
#endif
    foreach( QString key,keys){
        if (!key.startsWith("bsread:")){ //removes all header channels
            if (!key.contains(".BSREADSHAPE")){//removes shape waveform channels
                data.append("\"");
                data.append(key);
                data.append("\",");
            }
        }
    }
    data.remove(data.length()-1,1);
    data.append("]}");
    if (!data.contains("channels\":[]")){
        QByteArray transferdata;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        transferdata.append(data);
#else
        transferdata.append(data.toLatin1());
#endif
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
    if (MainMessageJ!=Q_NULLPTR){
        if(!MainMessageJ->IsArray()) {
            qDebug()<<"finishVerification() : MainMessageJ=Q_NULLPTR";
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

    if (ChannelsToBeApprovePipeline.count()>0){
     QString msg_not="bsread: Not approved channels ";
     msg_not.append(QString("%1").arg(ChannelsToBeApprovePipeline.count()));
     messagewindowP->postMsgEvent(QtCriticalMsg,(char*) msg_not.toLatin1().constData());

    }


    if (msg.contains("exception")){
        messagewindowP->postMsgEvent(QtCriticalMsg,(char*) msg.toLatin1().constData());
        // Request all channels we have, and see what will come
        ChannelsApprovePipeline+=ChannelsToBeApprovePipeline;
        ChannelsToBeApprovePipeline.clear();
    }
}
