#include <QThread>
#include <QDebug>
#include <QAtomicInt>
#include "zmq.h"
#include <exception>
#include "bsread_decode.h"
#include "knobData.h"
#include "JSON.h"
#include "JSONValue.h"
#include "bsread_channeldata.h"


bsread_Decode::bsread_Decode(void * Context,QString ConnectionPoint)
{
    int rc;
    int value;
    //qDebug() << "bsreadPlugin: ConnectionPoint :"<< ConnectionPoint;
    zmqsocket=zmq_socket(Context, ZMQ_PULL);
    if (!zmqsocket) {
        printf ("error in zmq_socket: %s\n", zmq_strerror (errno));
    }
    value=0;
    rc=zmq_setsockopt(zmqsocket,ZMQ_LINGER,&value,sizeof(value));
    if (rc != 0) {
        printf ("error in zmq_setsockopt: %s(%s)\n", zmq_strerror (errno),ConnectionPoint.toLatin1().constData());

    }
    rc = zmq_connect (zmqsocket, ConnectionPoint.toLatin1().constData());
    if (rc != 0) {
        printf ("error in zmq_connect: %s(%s)\n", zmq_strerror (errno),ConnectionPoint.toLatin1().constData());
        //qDebug() << "bsreadPlugin: ConnectionPoint faild";
        running_decode=false;
    }else{
        running_decode=true;
        channelcounter=0;
    }


}
bsread_Decode::~bsread_Decode()
{
	QMutexLocker locker(&mutex);

    while (zmq_term(zmqsocket)==-1){
       if(zmq_errno()==EFAULT) {
           break;
       }else{
         qDebug() << "bsreadPlugin: Terminaion ZMQ failed";
       }
    }


    zmq_close(zmqsocket);
}



void bsread_Decode::process()
{
    int rc;
    zmq_msg_t msg;
    int64_t more;
    int64_t notReceivedCounter=0;
    QString last_hash="This will never be seen";
    size_t more_size = sizeof (more);
    rc = zmq_msg_init (&msg);
    terminate=false;



    while (!terminate){
        //printf("Message\n");
        rc = zmq_msg_recv (&msg,zmqsocket,ZMQ_DONTWAIT);
        if (rc > 0) {
            notReceivedCounter=0;
            setMainHeader((char*)zmq_msg_data(&msg),zmq_msg_size (&msg));

            zmq_getsockopt (zmqsocket, ZMQ_RCVMORE, &more, &more_size);
            if (more){

                rc = zmq_msg_recv (&msg,zmqsocket,0);
                if (rc < 0) {
                    printf ("error in zmq_recvmsg(Header): %s\n", zmq_strerror (errno));
                }
                if (QString::compare(last_hash, hash, Qt::CaseInsensitive)){
                    setHeader((char*)zmq_msg_data(&msg),zmq_msg_size (&msg));
                    last_hash=hash;
                }
                zmq_getsockopt (zmqsocket, ZMQ_RCVMORE, &more, &more_size);
                while(more){
                    rc = zmq_msg_recv (&msg,zmqsocket,0);
                    if (rc < 0) {
                        printf ("error in zmq_recvmsg(Data): %s\n", zmq_strerror (errno));
                    }

                     bsread_SetChannelData(zmq_msg_data(&msg));

                    zmq_getsockopt (zmqsocket, ZMQ_RCVMORE, &more, &more_size);
                    if (more){
                        rc = zmq_msg_recv (&msg,zmqsocket,0);
                        if (rc < 0) {
                            printf ("error in zmq_recvmsg(Timestamp): %s\n", zmq_strerror (errno));
                        }
                        bsread_SetChannelTimeStamp(zmq_msg_data(&msg));
                        zmq_getsockopt (zmqsocket, ZMQ_RCVMORE, &more, &more_size);
                    }

                }
                bsread_EndofData();
            }
        }else{
            if (zmq_errno()==EAGAIN){
                //bsread_Delay();
                notReceivedCounter++;
                if (notReceivedCounter>10){
                    bsread_DataTimeOut();
                }
            }
				bsread_Delay();
                //printf ("error in zmq_recvmsg(Main Massage): %s\n", zmq_strerror (errno));

        }

    }
    emit finished();
    qDebug() << "bsread ZMQ Receiver terminate";

}
size_t bsread_Decode::getMessage_size() const
{
    return message_size;
}
QString bsread_Decode::getMainHeader() const
{
    return MainHeader;
}

bool bsread_Decode::setMainHeader(char *value,size_t size)
{
    JSONObject jsonobj;
    QString RawData=QString(value);
    MainHeader = RawData.left((int)size);

    JSONValue *MainMessageJ = JSON::Parse(MainHeader.toStdString().c_str());
    if (MainMessageJ!=NULL){
        if(!MainMessageJ->IsObject()) {
            delete(MainMessageJ);
        } else {
            jsonobj=MainMessageJ->AsObject();
            if (jsonobj.find(L"hash") != jsonobj.end() && jsonobj[L"hash"]->IsString()) {
                hash=QString::fromWCharArray(jsonobj[L"hash"]->AsString().c_str());
                //qDebug() << "hType :" << hash.toLatin1().constData();
            }

            if (jsonobj.find(L"pulse_id") != jsonobj.end() && jsonobj[L"pulse_id"]->IsNumber()) {
                pulse_id=jsonobj[L"pulse_id"]->AsNumber();
            }
            if (jsonobj.find(L"htype") != jsonobj.end() && jsonobj[L"htype"]->IsString()) {
                main_htype=QString::fromWCharArray(jsonobj[L"htype"]->AsString().c_str());
            }
            if (jsonobj.find(L"global_timestamp") != jsonobj.end() && jsonobj[L"global_timestamp"]->IsObject())
            {
                JSONObject jsonobj2=jsonobj[L"global_timestamp"]->AsObject();
                if (jsonobj2.find(L"epoch") != jsonobj2.end() && jsonobj2[L"epoch"]->IsNumber()) {
                    global_timestamp_epoch=jsonobj2[L"epoch"]->AsNumber();
                }
                if (jsonobj2.find(L"ns") != jsonobj2.end() && jsonobj2[L"ns"]->IsNumber()) {
                    global_timestamp_ns=jsonobj2[L"ns"]->AsNumber();
                }
            }
            delete(MainMessageJ);

        }
    }
    return true;
}
void bsread_Decode::setHeader(char *value,size_t size){
    QMutexLocker locker(&mutex);
    JSONValue *HeaderMessageJ;
    QString RawData=QString(value);
    ChannelHeader = RawData.left((int)size);

    for (int i=0;i<Channels.size();i++){
        delete(Channels.at(i));
    }


    Channels.clear();
    ChannelSearch.clear();
    try{
        HeaderMessageJ = JSON::Parse(ChannelHeader.toStdString().c_str());
    }
    catch (...) {
        qDebug() << "bsreadPlugin: Header Error :"<< value;
        HeaderMessageJ=NULL;
    }

    if (HeaderMessageJ!=NULL){
        if(!HeaderMessageJ->IsObject()) {
            delete(HeaderMessageJ);
        } else {
            JSONObject jsonobj=HeaderMessageJ->AsObject();
            if (jsonobj.find(L"channels") != jsonobj.end() && jsonobj[L"channels"]->IsArray()) {


                JSONArray jsonobj2=jsonobj[L"channels"]->AsArray();

                for (unsigned int i = 0; i < jsonobj2.size(); i++)
                {

                    bsread_channeldata *chdata=new bsread_channeldata();
                    Channels.append(chdata);
                    JSONObject jsonobj3=jsonobj2[i]->AsObject();
                    if (jsonobj3.find(L"type") != jsonobj3.end() && jsonobj3[L"type"]->IsString()) {

                        QString value=QString::fromWCharArray(jsonobj3[L"type"]->AsString().c_str());
                        if (value=="double"){
                            chdata->type=bs_double;
                        }else if(value=="string"){
                            chdata->type=bs_string;
                        }else if(value=="integer"){
                            chdata->type=bs_integer;
                        }else if(value=="long"){
                            chdata->type=bs_long;
                        }else if(value=="short"){
                            chdata->type=bs_short;
                        }


                    }
                    if (jsonobj3.find(L"name") != jsonobj3.end() && jsonobj3[L"name"]->IsString()) {
                        chdata->name=QString::fromWCharArray(jsonobj3[L"name"]->AsString().c_str());
                        ChannelSearch.insert(chdata->name, chdata);
                        // printf("Ch-Name :%s\n",chdata->name.toLatin1().constData());
                    }
                    if (jsonobj3.find(L"offset") != jsonobj3.end() && jsonobj3[L"offset"]->IsNumber()) {
                        chdata->offset=jsonobj3[L"offset"]->AsNumber();
                    }
                    if (jsonobj3.find(L"modulo") != jsonobj3.end() && jsonobj3[L"modulo"]->IsNumber()) {
                        chdata->modulo=jsonobj3[L"modulo"]->AsNumber();
                    }

                    if (jsonobj3.find(L"encoding") != jsonobj3.end() && jsonobj3[L"encoding"]->IsString()) {
                        if (QString::fromWCharArray(jsonobj3[L"encoding"]->AsString().c_str())=="big"){
                            chdata->endianess=bs_big;
                        }

                        if (jsonobj3.find(L"shape") != jsonobj3.end() && jsonobj3[L"shape"]->IsArray()) {
                            chdata->shape.clear();
                            JSONArray jsonobj4=jsonobj3[L"shape"]->AsArray();
                            for (unsigned int j = 0; j < jsonobj4.size(); j++){
                                int value=(int)jsonobj4[j]->AsNumber();
                                chdata->shape.append(value);
                            }

                        }
                    }

                }

                if (jsonobj.find(L"pulse_id") != jsonobj.end() && jsonobj[L"pulse_id"]->IsNumber()) {
                    pulse_id=jsonobj[L"pulse_id"]->AsNumber();
                }
                if (jsonobj.find(L"htype") != jsonobj.end() && jsonobj[L"htype"]->IsNumber()) {
                    main_htype=QString::fromWCharArray(jsonobj[L"htype"]->AsString().c_str());
                }
                if (jsonobj.find(L"global_timestamp") != jsonobj.end() && jsonobj[L"global_timestamp"]->IsObject())
                {
                    JSONObject jsonobj2=jsonobj[L"global_timestamp"]->AsObject();
                    if (jsonobj2.find(L"epoch") != jsonobj2.end() && jsonobj2[L"epoch"]->IsNumber()) {
                        global_timestamp_epoch=jsonobj2[L"epoch"]->AsNumber();
                    }
                    if (jsonobj2.find(L"ns") != jsonobj2.end() && jsonobj2[L"ns"]->IsNumber()) {
                        global_timestamp_ns=jsonobj2[L"ns"]->AsNumber();
                    }
                }
                delete(HeaderMessageJ);

            }
        }



    }
}


void bsread_Decode::bsread_SetChannelData(void *message)
{
    if ((message)&&(Channels.size()>channelcounter)){
        switch (Channels.at(channelcounter)->type){
        case bs_double:{
            Channels.at(channelcounter)->bsdata.bs_double=*(double*) message;
            //printf("Data :%s %f\n",Channels.at(channelcounter)->name.toLatin1().constData(),Channels.at(channelcounter)->bsdata.bs_double);
            break;
        }
        case bs_string:{
            Channels.at(channelcounter)->bsdata.bs_string=QString((char*)message);
            //qDebug() << "bsstring :" << message;
            break;
        }
        case bs_integer:{
            Channels.at(channelcounter)->bsdata.bs_double=*(int*) message;
            break;
        }
        case bs_long:{
            Channels.at(channelcounter)->bsdata.bs_double=*(long*) message;
            break;
        }
        case bs_short:{
            Channels.at(channelcounter)->bsdata.bs_double=*(short*) message;
            break;
        }
        }
    }
}

void bsread_Decode::bsread_SetChannelTimeStamp(void * timestamp)
{
    if ((timestamp)&&(Channels.size()>channelcounter)){
        Channels.at(channelcounter)->timestamp=*(double*) timestamp;
        channelcounter++;
    }
}

void bsread_Decode::bsread_EndofData()
{
    QMutexLocker locker(&mutex);
    bsread_channeldata * bsreadPV;
    channelcounter=0;
    //Update Knobdata
    //qDebug() << "bsreadPlugin:Update Knobdata";
    if (listOfIndexes.size()>0){
        foreach(int index, listOfIndexes) {
        knobData* kData = bsread_KnobDataP->GetMutexKnobDataPtr(index);
        if((kData != (knobData *) 0) && (kData->index != -1)) {
            QString key = kData->pv;

            // find this pv in our internal values list 
            // and update its value
            bsreadPV=NULL;
            QMap<QString,bsread_channeldata*>::iterator i = ChannelSearch.find(key);
            while (i !=ChannelSearch.end() && i.key() == key) {
                bsreadPV = i.value();
                break;//?????
            }
            // update some data

            if (bsreadPV){
                switch (bsreadPV->type){
                case bs_double:{
                    kData->edata.rvalue=bsreadPV->bsdata.bs_double;
                    kData->edata.fieldtype = caDOUBLE;
                    //qDebug() << "Double :"<< kData->edata.rvalue << key << index;
                    kData->edata.connected = true;
                    break;
                }
                case bs_string:{
                    kData->edata.fieldtype = caSTRING;
                    
                    //qDebug() << "String length :" << bsreadPV->bsdata.bs_string.length();
                    if (bsreadPV->bsdata.bs_string.length()!=0){
                        if (!kData->edata.dataB){
                            kData->edata.dataSize = bsreadPV->bsdata.bs_string.length();
                            kData->edata.dataB = (void*)malloc((size_t)kData->edata.dataSize);
                        }
                        if (kData->edata.dataSize!= bsreadPV->bsdata.bs_string.length()){
                            free(kData->edata.dataB);
                            kData->edata.dataSize = bsreadPV->bsdata.bs_string.length();
                            kData->edata.dataB = (void*)malloc((size_t)kData->edata.dataSize);
                        }

                        memcpy(kData->edata.dataB, (char*) bsreadPV->bsdata.bs_string.toLatin1().constData()
                               , (size_t)kData->edata.dataSize);

                    }
                    kData->edata.connected = true;
                    break;
                }
                case bs_integer:{
                    kData->edata.ivalue=bsreadPV->bsdata.bs_integer;
                    kData->edata.fieldtype = caINT;
                    kData->edata.connected = true;
                    break;
                }
                case bs_long:{
                    kData->edata.ivalue=bsreadPV->bsdata.bs_long;
                    kData->edata.fieldtype = caLONG;
                    kData->edata.connected = true;
                    break;
                }
                case bs_short:{

                    kData->edata.ivalue=bsreadPV->bsdata.bs_short;
                    kData->edata.fieldtype = caINT;
                    kData->edata.connected = true;
                    break;
                }
                default:{
                    kData->edata.connected = false;

                    break;
                }

                }


            }else{
                kData->edata.connected = false;
            }

            kData->edata.accessR = true;
            kData->edata.accessW = false;
            kData->edata.monitorCount++;
            bsread_KnobDataP->SetMutexKnobData(kData->index, *kData);
            bsread_KnobDataP->SetMutexKnobDataReceived(kData);
        }
    }
}


}

void bsread_Decode::setTerminate()
{
    terminate = true;
}

void bsread_Decode::bsread_DataTimeOut(){
    QMutexLocker locker(&mutex);
    hash="Data Time Out";
    channelcounter=0;
    foreach(int index, listOfIndexes) {
        knobData* kData = bsread_KnobDataP->GetMutexKnobDataPtr(index);
        kData->edata.connected = false;
        bsread_KnobDataP->SetMutexKnobData(kData->index, *kData);
        bsread_KnobDataP->SetMutexKnobDataReceived(kData);
    }
}
void bsread_Decode::bsread_Delay(){
 // This only works Qt5
    //QThread::msleep(5);
#ifdef linux
    usleep(5000);
#else
    //Sleep::msleep(5);
   QThread::msleep(5);
#endif

}
bool bsread_Decode::bsread_DataMonitorConnection(QString channel,int index){
    QMutexLocker locker(&mutex);

    listOfIndexes.append(index);
    listOfRequestedChannels.append(channel);
    //qDebug() << "Index :" << channel << index;

    return true;
}


bool bsread_Decode::bsread_DataMonitorConnection(knobData *kData){
    return bsread_DataMonitorConnection(kData->pv,kData->index);
}

bool bsread_Decode::bsread_DataMonitorUnConnect(knobData *kData){
    QMutexLocker locker(&mutex);
    listOfIndexes.removeAll(kData->index);
    listOfRequestedChannels.removeAll(kData->pv);
    hash="";
    return true;
}

void *bsread_Decode::getZmqsocket() const
{
    return zmqsocket;
}
QString bsread_Decode::getConnectionPoint() const
{
    return ConnectionPoint;
}
bool bsread_Decode::getRunning_decode() const
{
    return running_decode;
}
MutexKnobData *bsread_Decode::getKnobData() const
{
    return bsread_KnobDataP;
}

void bsread_Decode::setKnobData(MutexKnobData *value)
{
    bsread_KnobDataP = value;
}





