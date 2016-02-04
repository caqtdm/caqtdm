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
#include <QThread>
#include <QDebug>
#include <QAtomicInt>
#include <QBuffer>
#include <QByteArray>
#include <QDataStream>
#include "zmq.h"
#include <exception>
#include "bsread_decode.h"
#include "knobData.h"
#include "JSON.h"
#include "JSONValue.h"
#include "bsread_channeldata.h"
#include "bsread_wfhandling.h"

bsread_Decode::bsread_Decode(void * Context,QString ConnectionPoint)
{
   StreamConnectionPoint=ConnectionPoint;
   context=Context;
}
bsread_Decode::~bsread_Decode()
{
    QMutexLocker locker(&mutex);
    setTerminate();
//    while (zmq_term(zmqsocket)==-1){
//        if(zmq_errno()==EFAULT) {
//            break;
//        }else{
//            qDebug() << "bsreadPlugin: Terminaion ZMQ failed";
//        }
//    }
    bsread_Delay();


}



void bsread_Decode::bsread_createConnection(int rc)
{
    int value;
    zmqsocket=zmq_socket(context, ZMQ_PULL);
    if (!zmqsocket) {
        printf ("error in zmq_socket: %s\n", zmq_strerror (errno));
    }
    value=0;
    rc=zmq_setsockopt(zmqsocket,ZMQ_LINGER,&value,sizeof(value));
    if (rc != 0) {
        printf ("error in zmq_setsockopt: %s(%s)\n", zmq_strerror (errno),StreamConnectionPoint.toLatin1().constData());

    }
    rc = zmq_connect (zmqsocket, StreamConnectionPoint.toLatin1().constData());
}

void bsread_Decode::process()
{
    int rc;
    zmq_msg_t msg;
    int64_t more;
    int64_t notReceivedCounter=0;
    QString last_hash="This will never be seen";
    size_t more_size = sizeof (more);
    size_t msg_size;

    rc = zmq_msg_init (&msg);
    terminate=false;


    //qDebug() << "bsreadPlugin: ConnectionPoint :"<< ConnectionPoint;
    bsread_createConnection(rc);
    if (rc != 0) {
        printf ("error in zmq_connect: %s(%s)\n", zmq_strerror (errno),StreamConnectionPoint.toLatin1().constData());
        //qDebug() << "bsreadPlugin: ConnectionPoint faild";
        running_decode=false;
    }else{

        running_decode=true;
        channelcounter=0;

        while (!terminate){
            //printf("Message\n");
            rc = zmq_msg_recv (&msg,zmqsocket,ZMQ_DONTWAIT);
            if (rc > 0) {
                notReceivedCounter=0;
                setMainHeader((char*)zmq_msg_data(&msg),zmq_msg_size (&msg));

                if (main_htype.contains("bsr_m")){
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
                        bsread_TransferHeaderData();
                        zmq_getsockopt (zmqsocket, ZMQ_RCVMORE, &more, &more_size);
                        while(more){
                            rc = zmq_msg_recv (&msg,zmqsocket,0);
                            if (rc < 0) {
                                printf ("error in zmq_recvmsg(Data): %s\n", zmq_strerror (errno));
                            }
                            msg_size=zmq_msg_size(&msg);
                            bsread_SetChannelData(zmq_msg_data(&msg),msg_size);

                            //qDebug() <<msg_size;
                            zmq_getsockopt (zmqsocket, ZMQ_RCVMORE, &more, &more_size);

                            if (more){
                                rc = zmq_msg_recv (&msg,zmqsocket,0);
                                if (rc < 0) {
                                    printf ("error in zmq_recvmsg(Timestamp): %s\n", zmq_strerror (errno));
                                }
                                msg_size=zmq_msg_size(&msg);
                                bsread_SetChannelTimeStamp(zmq_msg_data(&msg));
                                zmq_getsockopt (zmqsocket, ZMQ_RCVMORE, &more, &more_size);
                                //qDebug() <<msg_size;
                            }

                        }
                        // qDebug() <<"---------------------------";
                        bsread_EndofData();
                    }else{
                     if (main_htype.contains("bsr_reconnect")){
                         StreamConnectionPoint=main_reconnect_adress;
                         bsread_createConnection(rc);
                         if (rc != 0) {
                             printf ("error in bsr_reconnect: %s(%s)\n", zmq_strerror (errno),StreamConnectionPoint.toLatin1().constData());
                             terminate=true;
                         }

                     }
                     if (main_htype.contains("bsr_stop")){
                        terminate=true;
                     }





                    }


                }
            }else{
                if (terminate){
                    break;
                }else{
                    bsread_Delay();
                }
                if (zmq_errno()==EAGAIN){
                    bsread_Delay();
                    notReceivedCounter++;
                    if (notReceivedCounter>200){
                       qDebug() << "bsread ZMQ Data Timeout";
                        bsread_DataTimeOut();
                        notReceivedCounter=0;
                    }
                }

                //printf ("error in zmq_recvmsg(Main Massage): %s\n", zmq_strerror (errno));

            }

        }
    }
    zmq_msg_close(&msg);
    zmq_close(zmqsocket);
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
    channelcounter=0;
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
    //Header Channel
    bsread_InitHeaderChannels();
    qDebug() << "Integer :" << ChannelHeader.toStdString().c_str();
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

                        if (value=="float64"){
                            chdata->type=bs_float64;
                        }else if(value=="float32"){
                            chdata->type=bs_float32;
                        }else if(value=="int64"){
                            chdata->type=bs_int64;
                        }else if(value=="int32"){
                            chdata->type=bs_int32;
                        }else if(value=="uint64"){
                            chdata->type=bs_uint64;
                        }else if(value=="uint32"){
                            chdata->type=bs_uint32;
                        }else if(value=="int16"){
                            chdata->type=bs_int16;
                        }else if(value=="uint16"){
                            chdata->type=bs_uint16;
                        }else if(value=="int8"){
                            chdata->type=bs_int8;
                        }else if(value=="uint8"){
                            chdata->type=bs_uint8;
                        }else if(value=="bool"){
                            chdata->type=bs_bool;
                        }else if(value=="string"){
                            chdata->type=bs_string;
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
                    }
                    if (jsonobj3.find(L"shape") != jsonobj3.end() && jsonobj3[L"shape"]->IsArray()) {
                        chdata->shape.clear();
                        JSONArray jsonobj4=jsonobj3[L"shape"]->AsArray();
                        for (unsigned int j = 0; j < jsonobj4.size(); j++){
                            int value=(int)jsonobj4[j]->AsNumber();
                            chdata->shape.append(value);
                            //qDebug()<< "shape:" << value;
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

void bsread_Decode::bsread_SetData(bsread_channeldata* Data,void *message,size_t size){


    switch(Data->shape.count()){
    case 0:{
        switch (Data->type){
            case bs_float64:{
                Data->bsdata.bs_float64=*(double*) message;
                break;
            }
            case bs_float32:{
                Data->bsdata.bs_float32=*(float*) message;
                break;
            }
            case bs_int64:{
                Data->bsdata.bs_int64=*(qint64*) message;
                break;
            }
            case bs_int32:{
                Data->bsdata.bs_int32=*(qint32*) message;
                break;
            }
            case bs_uint64:{
                Data->bsdata.bs_uint64=*(quint64*) message;
                break;
            }
            case bs_uint32:{
                Data->bsdata.bs_uint32=*(quint32*) message;
                break;
            }
            case bs_int16:{
                Data->bsdata.bs_int16=*(qint16*) message;
                break;
            }

            case bs_uint16:{
                Data->bsdata.bs_uint16=*(quint16*) message;
                break;
            }
            case bs_int8:{
                Data->bsdata.bs_int8=*(qint8*) message;
                break;
            }
            case bs_uint8:{
                Data->bsdata.bs_uint8=*(quint8*) message;
                break;
            }
            case bs_bool:{
                Data->bsdata.bs_bool=*(bool*) message;
                break;
            }

            case bs_string:{
                Data->bsdata.bs_string=QString((char*)message);
                break;
            }
        }





        break;
    }
    case 1:{
        int datasize=Data->shape.at(0);
        //qDebug()<< "Datasize:" << datasize << "ZMQ Size:" << size <<" "<<Data->name ;
        if (datasize==1){
            switch (Data->type){
                case bs_float64:{
                    Data->bsdata.bs_float64=*(double*) message;
                    break;
                }
                case bs_float32:{
                    Data->bsdata.bs_float32=*(float*) message;
                    break;
                }
                case bs_int64:{
                    Data->bsdata.bs_int64=*(qint64*) message;
                    break;
                }
                case bs_int32:{
                    Data->bsdata.bs_int32=*(qint32*) message;
                    break;
                }
                case bs_uint64:{
                    Data->bsdata.bs_uint64=*(quint64*) message;
                    //qDebug() << "Integer :"<< Data->bsdata.bs_long;
                    break;
                }
                case bs_uint32:{
                    Data->bsdata.bs_uint32=*(quint32*) message;
                    break;
                }
                case bs_int16:{
                    Data->bsdata.bs_int16=*(qint16*) message;
                    break;
                }

                case bs_uint16:{
                    Data->bsdata.bs_uint16=*(quint16*) message;
                    break;
                }
                case bs_int8:{
                    Data->bsdata.bs_int8=*(qint8*) message;
                    break;
                }
                case bs_uint8:{
                    Data->bsdata.bs_uint8=*(quint8*) message;
                    break;
                }
                case bs_bool:{
                    Data->bsdata.bs_bool=*(bool*) message;
                    break;
                }

                case bs_string:{
                    Data->bsdata.bs_string=QString((char*)message);
                    break;
                }
            }
        }else{
            if (datasize>1){
                int datatypesize;
                switch (Data->type){
                    case bs_float64:{
                        datatypesize=sizeof(double);
                        break;
                    }
                    case bs_float32:{
                        datatypesize=sizeof(float);
                        break;
                    }
                    case bs_int64:{
                        datatypesize=sizeof(qint64);
                        break;
                    }
                    case bs_uint64:{
                        datatypesize=sizeof(quint64);
                        break;
                    }
                    case bs_int32:{
                        datatypesize=sizeof(qint32);
                        break;
                    }
                    case bs_uint32:{
                        datatypesize=sizeof(quint32);
                        break;
                    }
                    case bs_int16:{
                        datatypesize=sizeof(qint16);
                        break;
                    }
                    case bs_uint16:{
                        datatypesize=sizeof(quint16);
                        break;
                    }
                    case bs_int8:{
                        datatypesize=sizeof(qint8);
                        break;
                    }
                    case bs_uint8:{
                        datatypesize=sizeof(quint8);
                        break;
                    }
                    case bs_bool:{
                         datatypesize=sizeof(bool);
                        break;
                    }

                    case bs_string:{
                        Data->bsdata.bs_string=QString((char*)message);
                        break;
                    }

                }

                if(Data->bsdata.wf_data_size!=(datasize*datatypesize)){
                    if (Data->bsdata.wf_data!=NULL){
                        free(Data->bsdata.wf_data);
                    }
                    Data->bsdata.wf_data=malloc(datasize*datatypesize);

                }
                if (size<(datasize*datatypesize)){
                    memcpy(Data->bsdata.wf_data,message,size);
                    Data->bsdata.wf_data_size=(size/datatypesize);
                }else{
                    memcpy(Data->bsdata.wf_data,message,datasize*datatypesize);
                    Data->bsdata.wf_data_size=datasize;
                }
                //qDebug() << "Data->bsdata.wf_data_size :" << Data->bsdata.wf_data_size << "  " <<size <<"  " <<datasize <<"  " << datatypesize;
            }
        }
        break;
    }
    case 2:{

        break;
    }
    default:{

        break;
    }
    }

}

void bsread_Decode::bsread_SetChannelData(void *message,size_t size)
{
    if ((message)&&(Channels.size()>channelcounter)){
      bsread_SetData(Channels.at(channelcounter),message,size);
    }
}

void bsread_Decode::bsread_SetChannelTimeStamp(void * timestamp)
{
    if ((timestamp)&&(Channels.size()>channelcounter)){
        Channels.at(channelcounter)->timestamp=*(double*) timestamp;
        channelcounter++;
    }
}

void bsread_Decode::bsread_InitHeaderChannels()
{
    bsread_channeldata *chdata;

    chdata=new bsread_channeldata();
    Channels.append(chdata);
    chdata->type=bs_string;
    chdata->name="hash";
    ChannelSearch.insert(chdata->name, chdata);

    chdata=new bsread_channeldata();
    Channels.append(chdata);
    chdata->type=bs_float64;
    chdata->name="pulse_id";
    ChannelSearch.insert(chdata->name, chdata);

    chdata=new bsread_channeldata();
    Channels.append(chdata);
    chdata->type=bs_string;
    chdata->name="htype";
    ChannelSearch.insert(chdata->name, chdata);

    chdata=new bsread_channeldata();
    Channels.append(chdata);
    chdata->type=bs_float64;
    chdata->name="global_timestamp_epoch";
    ChannelSearch.insert(chdata->name, chdata);

    chdata=new bsread_channeldata();
    Channels.append(chdata);
    chdata->type=bs_float64;
    chdata->name="global_timestamp_ns";
    ChannelSearch.insert(chdata->name, chdata);


}

void bsread_Decode::bsread_TransferHeaderData()
{
    if (Channels.size()>4){
        Channels.at(channelcounter)->bsdata.bs_string=hash;
        channelcounter++;
        Channels.at(channelcounter)->bsdata.bs_float64=pulse_id;
        channelcounter++;
        Channels.at(channelcounter)->bsdata.bs_string=main_htype;
        channelcounter++;
        Channels.at(channelcounter)->bsdata.bs_float64=global_timestamp_epoch;
        channelcounter++;
        Channels.at(channelcounter)->bsdata.bs_float64=global_timestamp_ns;
        channelcounter++;

    }
}


void bsread_Decode::WaveformManagment(knobData* kData,bsread_channeldata * bsreadPV){

    bsread_wfhandling *transfer=new bsread_wfhandling(kData,bsreadPV);
    QThreadPool::globalInstance()->start(transfer);
}



void bsread_Decode::bsread_EndofData()
{
    QMutexLocker locker(&mutex);
    bsread_channeldata * bsreadPV;
    QList<knobData*> * MonitorList=new QList<knobData*>;

    //Update Knobdata
    //qDebug() << "bsreadPlugin:Update Knobdata";
    if (listOfIndexes.size()>0){
        foreach(int index, listOfIndexes) {
            knobData* kData = bsread_KnobDataP->GetMutexKnobDataPtr(index);
            if((kData != (knobData *) 0) && (kData->index != -1)) {
                QString key = kData->pv;
                QString ioc_string=StreamConnectionPoint.leftJustified(39, ' ');
                strcpy(kData->edata.fec,ioc_string.toLatin1().constData());
                // find this pv in our internal values list
                // and update its value
                bsreadPV=NULL;
                QMap<QString,bsread_channeldata*>::iterator i = ChannelSearch.find(key);
                while (i !=ChannelSearch.end() && i.key() == key) {
                    bsreadPV = i.value();
                    break;//?????
                }
                // update some data
                // bs_string,bs_float64,bs_float32,bs_int64,bs_int32,bs_uint64,bs_uint32,bs_int16,bs_uint16,bs_int8,bs_uint8

                if (bsreadPV){
                    switch (bsreadPV->type){
                    case bs_float64:{
                        MonitorList->append(kData);
                        kData->edata.fieldtype = caDOUBLE;
                        if(bsreadPV->bsdata.wf_data_size!=0){
                             WaveformManagment(kData,bsreadPV);
                        }else{
                            kData->edata.rvalue=bsreadPV->bsdata.bs_float64;
                        }
                        kData->edata.connected = true;
                        break;
                    }
                    case bs_float32:{
                        MonitorList->append(kData);
                        kData->edata.fieldtype = caDOUBLE;
                        if(bsreadPV->bsdata.wf_data_size!=0){
                            WaveformManagment(kData,bsreadPV);
                        }else{
                            kData->edata.rvalue=bsreadPV->bsdata.bs_float32;
                        }
                        kData->edata.connected = true;
                        break;
                    }
                    case bs_string:{
                        MonitorList->append(kData);
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
                    case bs_int64:{
                        MonitorList->append(kData);
                        kData->edata.fieldtype = caDOUBLE;
                        if(bsreadPV->bsdata.wf_data_size!=0){
                            WaveformManagment(kData,bsreadPV);
                        }else{
                            kData->edata.rvalue=bsreadPV->bsdata.bs_int64;
                        }
                        kData->edata.connected = true;
                        break;
                    }
                    case bs_int32:{
                        MonitorList->append(kData);
                        kData->edata.fieldtype = caLONG;
                        if(bsreadPV->bsdata.wf_data_size!=0){
                            WaveformManagment(kData,bsreadPV);
                        }else{
                            kData->edata.ivalue=bsreadPV->bsdata.bs_int32;
                        }
                        kData->edata.connected = true;
                        break;
                    }
                    case bs_uint64:{
                        MonitorList->append(kData);
                        kData->edata.fieldtype = caDOUBLE;
                        if(bsreadPV->bsdata.wf_data_size!=0){
                            WaveformManagment(kData,bsreadPV);
                        }else{
                            kData->edata.rvalue=bsreadPV->bsdata.bs_uint64;
                        }
                        kData->edata.connected = true;
                        break;
                    }
                    case bs_uint32:{
                        MonitorList->append(kData);
                        kData->edata.fieldtype = caDOUBLE;
                        if(bsreadPV->bsdata.wf_data_size!=0){
                            WaveformManagment(kData,bsreadPV);
                        }else{
                            kData->edata.ivalue=bsreadPV->bsdata.bs_uint32;
                        }
                        kData->edata.connected = true;
                        break;
                    }
                    case bs_int16:{
                        MonitorList->append(kData);
                        kData->edata.fieldtype = caINT;
                        if(bsreadPV->bsdata.wf_data_size!=0){
                            WaveformManagment(kData,bsreadPV);
                        }else{
                            kData->edata.ivalue=bsreadPV->bsdata.bs_int16;
                        }
                        kData->edata.connected = true;
                        break;
                    }
                    case bs_uint16:{
                        MonitorList->append(kData);
                        kData->edata.fieldtype = caINT;
                        if(bsreadPV->bsdata.wf_data_size!=0){
                            WaveformManagment(kData,bsreadPV);
                        }else{
                            kData->edata.ivalue=bsreadPV->bsdata.bs_uint16;
                        }

                        kData->edata.connected = true;
                        break;
                    }
                    case bs_int8:{
                        MonitorList->append(kData);
                        if(bsreadPV->bsdata.wf_data_size!=0){
                            WaveformManagment(kData,bsreadPV);
                        }else{
                            kData->edata.ivalue=bsreadPV->bsdata.bs_int8;
                        }
                        kData->edata.fieldtype = caINT;
                        kData->edata.connected = true;
                        break;
                    }
                    case bs_uint8:{
                        MonitorList->append(kData);
                        kData->edata.fieldtype = caINT;
                        if(bsreadPV->bsdata.wf_data_size!=0){
                            WaveformManagment(kData,bsreadPV);
                        }else{
                            kData->edata.ivalue=bsreadPV->bsdata.bs_uint8;
                        }

                        kData->edata.connected = true;
                        break;
                    }
                    case bs_bool:{
                        MonitorList->append(kData);
                        kData->edata.ivalue=bsreadPV->bsdata.bs_bool;
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
            }
        }

        QThreadPool::globalInstance()->waitForDone(2);
//        foreach(int index, listOfIndexes) {
          for (int i=0;i<MonitorList->count();i++){
            knobData* kData = MonitorList->at(i);
            if((kData != (knobData *) 0) && (kData->index != -1)) {
                kData->edata.monitorCount++;
                bsread_KnobDataP->SetMutexKnobData(kData->index, *kData);
                bsread_KnobDataP->SetMutexKnobDataReceived(kData);

            }
        }
        delete MonitorList;


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
    //Sleep::msleep(1);

    QMutex LocalLocker;
    QWaitCondition LocalDelay;
    LocalLocker.lock();
    LocalDelay.wait(&LocalLocker,5);
    LocalLocker.unlock();

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
    QMutex *datamutex;
    datamutex = (QMutex*) kData->mutex;
    datamutex->lock();
    kData->edata.dataB=NULL;
    kData->edata.dataSize=0;
    kData->edata.valueCount=0;
    datamutex->unlock();

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





