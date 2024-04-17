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
#include <QtCore>
#include <QThread>
#include <QDebug>
#include <QAtomicInt>
#include <QBuffer>
#include <QByteArray>
#include <QDataStream>
#include "zmq.h"
#include "dbrString.h"
#include <exception>
#include "bsread_decode.h"
#include "knobData.h"
#include "JSON.h"
#include "JSONValue.h"
#include "bsread_channeldata.h"
#include "bsread_wfhandling.h"

enum Alarms {NO_ALARM=0, MINOR_ALARM, MAJOR_ALARM, INVALID_ALARM, NOTCONNECTED=99};


bsread_Decode::bsread_Decode(void * Context,QString ConnectionPoint)
{
   StreamConnectionPoint=ConnectionPoint;
   StreamConnectionType="push_pull";
   context=Context;
   UpdaterPool=Q_NULLPTR;
   BlockPool=Q_NULLPTR;
}
bsread_Decode::bsread_Decode(void * Context,QString ConnectionPoint,QString ConnectionType)
{
   StreamConnectionPoint=ConnectionPoint;
   StreamConnectionType=ConnectionType;
   context=Context;
   UpdaterPool=Q_NULLPTR;
   BlockPool=Q_NULLPTR;
}





bsread_Decode::~bsread_Decode()
{
    QMutexLocker locker(&mutex);
    setTerminate();
    bsread_Delay();
    //delete(UpdaterPool);
    //delete(BlockPool);
}



void bsread_Decode::bsread_createConnection(int rc)
{
    int value;
    qDebug()<< "StreamConnectionType: "<<StreamConnectionType;
    if (QString::compare(StreamConnectionType,"pub_sub",Qt::CaseInsensitive)==0){
        zmqsocket=zmq_socket(context, ZMQ_SUB);
    }else{
        zmqsocket=zmq_socket(context, ZMQ_PULL);
    }


    if (!zmqsocket) {
        printf ("error in zmq_socket: %s\n", zmq_strerror (errno));
    }
    value=1;
    rc=zmq_setsockopt(zmqsocket,ZMQ_LINGER,&value,sizeof(value));
    if (rc != 0) {
        printf ("error in zmq_setsockopt: %s(%s)\n", zmq_strerror (errno),StreamConnectionPoint.toLatin1().constData());

    }
    value=2;
    rc=zmq_setsockopt(zmqsocket,ZMQ_RCVHWM,&value,sizeof(value));
    if (rc != 0) {
        printf ("error in zmq_setsockopt: %s(%s)\n", zmq_strerror (errno),StreamConnectionPoint.toLatin1().constData());

    }

    rc = zmq_connect (zmqsocket, StreamConnectionPoint.toLatin1().constData());

    if (QString::compare(StreamConnectionType,"pub_sub",Qt::CaseInsensitive)==0){
        rc=zmq_setsockopt( zmqsocket, ZMQ_SUBSCRIBE, "", 0 );
        if (rc != 0) {
            printf ("error in zmq_setsockopt: %s(%s)\n", zmq_strerror (errno),StreamConnectionPoint.toLatin1().constData());

        }
    }

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


    //qDebug() << "bsreadDecode: ConnectionPoint :"<< StreamConnectionPoint << StreamConnectionType ;
    //qDebug() << "bsreadDecode: start ThreadID" << QThread::currentThreadId();

    bsread_createConnection(rc);
    if (rc != 0) {
        printf ("error in zmq_connect: %s(%s)\n", zmq_strerror (errno),StreamConnectionPoint.toLatin1().constData());
        //qDebug() << "bsreadPlugin: ConnectionPoint faild";
        running_decode=false;
    }else{
        running_decode=true;
        channelcounter=0;

        while (!terminate){
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
                        //qDebug() <<"---------------------------";
                        bsread_EndofData();
                    }else{
                     if (main_htype.contains("bsr_reconnect")){
                         //StreamConnectionPoint=main_reconnect_adress;
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
                        //qDebug() << "bsread ZMQ Data Timeout";
                        //bsread_DataTimeOut();
                        notReceivedCounter=0;
                    }
                }

                //printf ("error in zmq_recvmsg(Main Massage): %s\n", zmq_strerror (errno));

            }

        }

        bsread_DataTimeOut();
        zmq_msg_close(&msg);
        zmq_close(zmqsocket);


    }

    emit finished();
    //qDebug() << "bsreadDecode: finished ThreadID" << QThread::currentThreadId();
    qDebug() << "bsread ZMQ Receiver terminate";

}
QString bsread_Decode::getStreamConnectionPoint() const
{
    return StreamConnectionPoint;
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
    if (MainMessageJ!=Q_NULLPTR){
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
                //qDebug() << "pulse_id :" << pulse_id;
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
                if (jsonobj2.find(L"sec") != jsonobj2.end() && jsonobj2[L"sec"]->IsNumber()) {
                    global_timestamp_sec=jsonobj2[L"sec"]->AsNumber();
                }
                if (jsonobj2.find(L"ns_offset") != jsonobj2.end() && jsonobj2[L"ns_offset"]->IsNumber()) {
                    global_timestamp_ns_offset=jsonobj2[L"ns_offset"]->AsNumber();
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
    //qDebug() << "Integer :" << ChannelHeader.toStdString().c_str();
    try{
        HeaderMessageJ = JSON::Parse(ChannelHeader.toStdString().c_str());
    }
    catch (...) {
        qDebug() << "bsreadPlugin: Header Error :"<< value;
        HeaderMessageJ=Q_NULLPTR;
    }

    if (HeaderMessageJ!=Q_NULLPTR){
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
                        }else{
                            chdata->type=bs_none;
                        }


                    }
                    if (jsonobj3.find(L"name") != jsonobj3.end() && jsonobj3[L"name"]->IsString()) {
                        chdata->name=QString::fromWCharArray(jsonobj3[L"name"]->AsString().c_str());
                        ChannelSearch.insert(chdata->name, chdata);
                        //printf("Ch-Name :%s\n",chdata->name.toLatin1().constData());
                    }
                    if (jsonobj3.find(L"offset") != jsonobj3.end() && jsonobj3[L"offset"]->IsNumber()) {
                        chdata->offset=jsonobj3[L"offset"]->AsNumber();
                    }
                    if (jsonobj3.find(L"modulo") != jsonobj3.end() && jsonobj3[L"modulo"]->IsNumber()) {
                        chdata->modulo=jsonobj3[L"modulo"]->AsNumber();
                    }

                    if (jsonobj3.find(L"encoding") != jsonobj3.end() && jsonobj3[L"encoding"]->IsString()) {
                        QString encoding=QString::fromWCharArray(jsonobj3[L"encoding"]->AsString().c_str());

                        if (encoding=="big"){
                            chdata->endianess=bs_big;
                        }else if(encoding!="little"){
                           chdata->endianess=bs_other;
                        }
                        QStringList enc_values=encoding.split(",");
                        bsread_channeldata *encoding_chdata;
                        if (enc_values.count()>0){
                            QString EncodingChannel=chdata->name;
                            encoding_chdata=new bsread_channeldata();
                            Channels.append(encoding_chdata);
                            EncodingChannel.append(".ENC_GROUP");
                            encoding_chdata->type=bs_string;
                            encoding_chdata->name=EncodingChannel;
                            encoding_chdata->bsdata.bs_string=enc_values.at(0);
                            encoding_chdata->valid=true;
                            ChannelSearch.insert(EncodingChannel, encoding_chdata);
                        }
                        if (enc_values.count()>1){
                            QString EncodingChannel=chdata->name;
                            encoding_chdata=new bsread_channeldata();
                            Channels.append(encoding_chdata);
                            EncodingChannel.append(".ENC_TYPE");
                            encoding_chdata->type=bs_string;
                            encoding_chdata->name=EncodingChannel;
                            encoding_chdata->bsdata.bs_string=enc_values.at(1);
                            encoding_chdata->valid=true;
                            ChannelSearch.insert(EncodingChannel, encoding_chdata);
                        }
                        if (enc_values.count()>2){
                            QString EncodingChannel=chdata->name;
                            encoding_chdata=new bsread_channeldata();
                            Channels.append(encoding_chdata);
                            EncodingChannel.append(".ENC_SUBTYPE");
                            encoding_chdata->type=bs_string;
                            encoding_chdata->name=EncodingChannel;
                            encoding_chdata->bsdata.bs_string=enc_values.at(2);
                            encoding_chdata->valid=true;
                            ChannelSearch.insert(EncodingChannel, encoding_chdata);
                        }

                    }
                    if (jsonobj3.find(L"shape") != jsonobj3.end() && jsonobj3[L"shape"]->IsArray()) {
                        chdata->shape.clear();
                        JSONArray jsonobj4=jsonobj3[L"shape"]->AsArray();
                        for (unsigned int j = 0; j < jsonobj4.size(); j++){
                            int value=(int)jsonobj4[j]->AsNumber();
                            chdata->shape.append(value);
                            if ((jsonobj4.size()>1)&&(value>1)){
                                QString ShapeChannel=chdata->name;
                                bsread_channeldata *shape_chdata=new bsread_channeldata();
                                Channels.append(shape_chdata);
                                ShapeChannel.append(".BSREADSHAPE");
                                ShapeChannel.append(QString::number(j));
                                shape_chdata->type=bs_float32;
                                shape_chdata->name=ShapeChannel;
                                shape_chdata->bsdata.bs_float32=value;
                                shape_chdata->valid=true;
                                ChannelSearch.insert(ShapeChannel, shape_chdata);
                                qDebug()<< "shape["<<j<<"]:" << value << ShapeChannel;
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
                    if (jsonobj2.find(L"sec") != jsonobj2.end() && jsonobj2[L"sec"]->IsNumber()) {
                        global_timestamp_sec=jsonobj2[L"sec"]->AsNumber();
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

void bsread_Decode::bsdata_assign_single(bsread_channeldata* Data, void *message,int * datatypesize)
{
    switch (Data->type){
        case bs_float64:{
            switch (Data->endianess){
             case bs_big:{
                QByteArray data = QByteArray::fromRawData((const char *)message,sizeof(double));
                QDataStream stream(data);
                stream.setFloatingPointPrecision(QDataStream::DoublePrecision);
                stream.setByteOrder(QDataStream::BigEndian);
                stream>>Data->bsdata.bs_float64;

                break;
             }
             default:Data->bsdata.bs_float64=*(double*) message;
            }
            //qDebug() << "Double :" << Data->bsdata.bs_float64 << *(double*) message ;
            *datatypesize=sizeof(double);
            break;
        }
        case bs_float32:{
            switch (Data->endianess){
             case bs_big:{
                QByteArray data = QByteArray::fromRawData((const char *)message,sizeof(float));
                QDataStream stream(data);
                stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
                stream.setByteOrder(QDataStream::BigEndian);
                stream>>Data->bsdata.bs_float32;

                break;
             }
             default:Data->bsdata.bs_float32=*(float*) message;
            }
//            if (Data->name.endsWith("MINSB04-RACC200-PUP20:SIG-AMPLT-AVG")){
//              qDebug() << "Float :" << Data->bsdata.bs_float32 << *(float*) message<< *(long*) message;
//            }
            *datatypesize=sizeof(float);
            break;
        }
        case bs_int64:{
            Data->bsdata.bs_int64=*(qint64*) message;
            *datatypesize=sizeof(qint64);
            break;
        }
        case bs_int32:{
            switch (Data->endianess){
             case bs_big:{
                QByteArray data = QByteArray::fromRawData((const char *)message,sizeof(qint32));
                QDataStream stream(data);
                stream.setByteOrder(QDataStream::BigEndian);
                stream>>Data->bsdata.bs_int32;
                break;
             }
             default:Data->bsdata.bs_int32=*(qint32*) message;
            }



            *datatypesize=sizeof(qint32);
            break;
        }
        case bs_uint64:{
            Data->bsdata.bs_uint64=*(quint64*) message;
            *datatypesize=sizeof(quint64);
            break;
        }
        case bs_uint32:{
            switch (Data->endianess){
             case bs_big:{
                QByteArray data = QByteArray::fromRawData((const char *)message,sizeof(quint32));
                QDataStream stream(data);
                stream.setByteOrder(QDataStream::BigEndian);
                stream>>Data->bsdata.bs_uint32;
                break;
             }
             default:Data->bsdata.bs_uint32=*(quint32*) message;
            }


            *datatypesize=sizeof(quint32);
            break;
        }
        case bs_int16:{
            Data->bsdata.bs_int16=*(qint16*) message;
            *datatypesize=sizeof(qint16);
            break;
        }

        case bs_uint16:{
            Data->bsdata.bs_uint16=*(quint16*) message;
            *datatypesize=sizeof(qint16);
            break;
        }
        case bs_int8:{
            Data->bsdata.bs_int8=*(qint8*) message;
            *datatypesize=sizeof(qint16);
            break;
        }
        case bs_uint8:{
            Data->bsdata.bs_uint8=*(quint8*) message;
            *datatypesize=sizeof(qint16);
            break;
        }
        case bs_bool:{
            Data->bsdata.bs_bool=*(bool*) message;
            *datatypesize=sizeof(qint16);
            break;
        }

        case bs_string:{
            Data->bsdata.bs_string=QString((char*)message);
            *datatypesize=sizeof(char);
            break;
        }
    }
}

void bsread_Decode::bsread_SetData(bsread_channeldata* Data,void *message,size_t size){

    int datatypesize;
    switch(Data->shape.count()){
    case 0:{
        bsdata_assign_single(Data, message,&datatypesize);
        break;
    }
    case 1:{
        int datasize=Data->shape.at(0);
        //qDebug()<< "Datasize:" << datasize << "ZMQ Size:" << size <<" "<<Data->name ;
        if (datasize==1){
            if (size>0){
              bsdata_assign_single(Data, message,&datatypesize);
              channelcounter++;
              Data->valid=true;
            }else{
              Data->valid=false;
            }
        }else{
            if (datasize>1){

                bsdata_assign_single(Data, message,&datatypesize);
                if(Data->bsdata.wf_data_size!=(ulong)(datasize*datatypesize)){
                    if (Data->bsdata.wf_data!=Q_NULLPTR){
                        free(Data->bsdata.wf_data);
                    }
                    Data->bsdata.wf_data=malloc(datasize*datatypesize);

                }
                if (size<((ulong)(datasize*datatypesize))){
                    memcpy(Data->bsdata.wf_data,message,size);
                    Data->bsdata.wf_data_size=(ulong)(size/datatypesize);
                }else{
                    memcpy(Data->bsdata.wf_data,message,datasize*datatypesize);
                    Data->bsdata.wf_data_size=datasize;
                }
                Data->valid=true;

                channelcounter++;
                //qDebug() << "Data->bsdata.wf_data_size :" << Data->bsdata.wf_data_size << "  " <<size <<"  " <<datasize <<"  " << datatypesize;
            }else{
                Data->valid=false;
            }
        }
        break;
    }
    case 2:{
        int datasize=Data->shape.at(0)*Data->shape.at(1);
        if (datasize==1){
            if (size>0){
              bsdata_assign_single(Data, message,&datatypesize);
              Data->valid=true;
            }else{
              Data->valid=false;
            }
        }else{
            if (datasize>1){
                // handle Image Color data as 16 bit
                if (Data->endianess==bs_other){
                  Data->type=bs_uint16;

                }

                bsdata_assign_single(Data, message,&datatypesize);
                if(Data->bsdata.wf_data_size!=(ulong)(datasize*datatypesize)){
                    if (Data->bsdata.wf_data!=Q_NULLPTR){
                        free(Data->bsdata.wf_data);
                    }
                    Data->bsdata.wf_data=malloc(datasize*datatypesize);

                }
                if (size<((ulong)(datasize*datatypesize))){
                    memcpy(Data->bsdata.wf_data,message,size);
                    Data->bsdata.wf_data_size=(ulong)(size/datatypesize);
                }else{
                    memcpy(Data->bsdata.wf_data,message,datasize*datatypesize);
                    Data->bsdata.wf_data_size=datasize;
                }
                Data->valid=true;
                channelcounter++;
                channelcounter++;
                //qDebug() << "Data->bsdata.wf_data_size :" << Data->bsdata.wf_data_size << "  " <<size <<"  " <<datasize <<"  " << datatypesize;
            }else{
                Data->valid=false;
            }
        }

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
    }
    // After the timestamp the next channel is coming, even when the timestamp is NULL
    channelcounter++;

}

void bsread_Decode::bsread_InitHeaderChannels()
{
    bsread_channeldata *chdata;

    chdata=new bsread_channeldata();
    Channels.append(chdata);
    chdata->type=bs_string;
    chdata->name="bsread:hash";
    chdata->valid=true;
    ChannelSearch.insert(chdata->name, chdata);

    chdata=new bsread_channeldata();
    Channels.append(chdata);
    chdata->type=bs_float64;
    chdata->name="bsread:pulse_id";
    chdata->valid=true;
    ChannelSearch.insert(chdata->name, chdata);

    chdata=new bsread_channeldata();
    Channels.append(chdata);
    chdata->type=bs_string;
    chdata->name="bsread:htype";
    chdata->valid=true;
    ChannelSearch.insert(chdata->name, chdata);
/*
    chdata=new bsread_channeldata();
    Channels.append(chdata);
    chdata->type=bs_float64;
    chdata->name="bsread:global_timestamp_epoch";
    ChannelSearch.insert(chdata->name, chdata);
*/
    chdata=new bsread_channeldata();
    Channels.append(chdata);
    chdata->type=bs_float64;
    chdata->name="bsread:global_timestamp_ns";
    chdata->valid=true;
    ChannelSearch.insert(chdata->name, chdata);

    chdata=new bsread_channeldata();
    Channels.append(chdata);
    chdata->type=bs_float64;
    chdata->name="bsread:global_timestamp_sec";
    chdata->valid=true;
    ChannelSearch.insert(chdata->name, chdata);

/*
    chdata=new bsread_channeldata();
    Channels.append(chdata);
    chdata->type=bs_float64;
    chdata->name="bsread:global_timestamp_ns_offset";
    ChannelSearch.insert(chdata->name, chdata);
*/
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
        //Channels.at(channelcounter)->bsdata.bs_float64=global_timestamp_epoch;
        //channelcounter++;
        Channels.at(channelcounter)->bsdata.bs_float64=global_timestamp_ns;
        channelcounter++;
        Channels.at(channelcounter)->bsdata.bs_float64=global_timestamp_sec;
        channelcounter++;
        //Channels.at(channelcounter)->bsdata.bs_float64=global_timestamp_ns_offset;
        //channelcounter++;

    }
}


void bsread_Decode::WaveformManagment(knobData* kData,bsread_channeldata * bsreadPV){
    bsread_wfhandling *transfer=new bsread_wfhandling(kData,bsreadPV,BlockPool);
    transfer->process();
    delete(transfer);
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
            if((kData != (knobData *) Q_NULLPTR) && (kData->index != -1)) {
                QString key = kData->pv;
                //qDebug() << kData->pv;
                QString ioc_string=StreamConnectionPoint.leftJustified(39, ' ');
                qstrncpy(kData->edata.fec,ioc_string.toLatin1().constData(),caqtdm_string_t_length);
                // find this pv in our internal values list
                // and update its value
                bsreadPV=Q_NULLPTR;
                QMap<QString,bsread_channeldata*>::iterator i = ChannelSearch.find(key);
                while (i !=ChannelSearch.end() && i.key() == key) {
                    bsreadPV = i.value();
                    break;//?????
                }
                // update some data
                // bs_string,bs_float64,bs_float32,bs_int64,bs_int32,bs_uint64,bs_uint32,bs_int16,bs_uint16,bs_int8,bs_uint8

                if (bsreadPV){
                    if (!bsreadPV->valid){
                        kData->edata.severity=INVALID_ALARM;
                    }else{
                        kData->edata.severity=NO_ALARM;
                    }
                    switch (bsreadPV->type){
                    case bs_float64:{
                        kData->edata.fieldtype = caDOUBLE;
                        if(bsreadPV->bsdata.wf_data_size!=0){
                             WaveformManagment(kData,bsreadPV);
                             MonitorList->append(kData);
                        }else{
                            kData->edata.rvalue=bsreadPV->bsdata.bs_float64;
                            kData->edata.precision=bsreadPV->precision;
                            //qDebug() << "double: "<< kData->edata.rvalue;
                            MonitorList->insert(0,kData);
                        }
                        kData->edata.connected = true;
                        break;
                    }
                    case bs_float32:{
                        kData->edata.fieldtype = caFLOAT;
                        if(bsreadPV->bsdata.wf_data_size!=0){
                            WaveformManagment(kData,bsreadPV);
                            MonitorList->append(kData);
                        }else{
                            kData->edata.rvalue=bsreadPV->bsdata.bs_float32;
                            kData->edata.precision=bsreadPV->precision;
                            //qDebug() << "float: "<< kData->edata.rvalue;
                            MonitorList->insert(0,kData);
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
                        kData->edata.fieldtype = caDOUBLE;
                        if(bsreadPV->bsdata.wf_data_size!=0){
                            WaveformManagment(kData,bsreadPV);
                            MonitorList->append(kData);
                        }else{
                            kData->edata.rvalue=bsreadPV->bsdata.bs_int64;
                            MonitorList->insert(0,kData);
                        }
                        kData->edata.connected = true;
                        break;
                    }
                    case bs_int32:{
                        kData->edata.fieldtype = caLONG;
                        if(bsreadPV->bsdata.wf_data_size!=0){
                            WaveformManagment(kData,bsreadPV);
                            MonitorList->append(kData);
                        }else{
                            kData->edata.ivalue=bsreadPV->bsdata.bs_int32;
                            MonitorList->insert(0,kData);
                        }
                        kData->edata.connected = true;
                        break;
                    }
                    case bs_uint64:{
                        kData->edata.fieldtype = caDOUBLE;
                        if(bsreadPV->bsdata.wf_data_size!=0){
                            WaveformManagment(kData,bsreadPV);
                            MonitorList->append(kData);
                        }else{
                            kData->edata.rvalue=bsreadPV->bsdata.bs_uint64;
                            MonitorList->insert(0,kData);
                        }
                        kData->edata.connected = true;
                        break;
                    }
                    case bs_uint32:{
                        kData->edata.fieldtype = caDOUBLE;
                        if(bsreadPV->bsdata.wf_data_size!=0){
                            WaveformManagment(kData,bsreadPV);
                            MonitorList->append(kData);
                        }else{
                            kData->edata.ivalue=bsreadPV->bsdata.bs_uint32;
                            MonitorList->insert(0,kData);
                        }
                        kData->edata.connected = true;
                        break;
                    }
                    case bs_int16:{
                        kData->edata.fieldtype = caINT;
                        if(bsreadPV->bsdata.wf_data_size!=0){
                            WaveformManagment(kData,bsreadPV);
                            MonitorList->append(kData);
                        }else{
                            kData->edata.ivalue=bsreadPV->bsdata.bs_int16;
                            MonitorList->insert(0,kData);
                        }
                        kData->edata.connected = true;
                        break;
                    }
                    case bs_uint16:{
                        kData->edata.fieldtype = caINT;
                        if(bsreadPV->bsdata.wf_data_size!=0){
                            WaveformManagment(kData,bsreadPV);
                            MonitorList->append(kData);
                        }else{
                            kData->edata.ivalue=bsreadPV->bsdata.bs_uint16;
                            MonitorList->insert(0,kData);
                        }

                        kData->edata.connected = true;
                        break;
                    }
                    case bs_int8:{
                        if(bsreadPV->bsdata.wf_data_size!=0){
                            WaveformManagment(kData,bsreadPV);
                            MonitorList->append(kData);
                        }else{
                            kData->edata.ivalue=bsreadPV->bsdata.bs_int8;
                            MonitorList->insert(0,kData);
                        }
                        kData->edata.fieldtype = caINT;
                        kData->edata.connected = true;
                        break;
                    }
                    case bs_uint8:{
                        kData->edata.fieldtype = caINT;
                        if(bsreadPV->bsdata.wf_data_size!=0){
                            WaveformManagment(kData,bsreadPV);
                            MonitorList->append(kData);
                        }else{
                            kData->edata.ivalue=bsreadPV->bsdata.bs_uint8;
                            MonitorList->insert(0,kData);
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
       //qDebug() << "ActiveThreads: "<< QThreadPool::globalInstance()->activeThreadCount();
       // UpdaterPool->waitForDone(-1);
      // while(WfDataHandlerQueue.count()!=0){
       //    qDebug() << "WfDataHandlerQueue Wait ";

      // }



//        foreach(int index, listOfIndexes) {
          for (int i=0;i<MonitorList->count();i++){
            knobData* kData = MonitorList->at(i);
            if((kData != (knobData *) Q_NULLPTR) && (kData->index != -1)) {
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
    if (bsread_KnobDataP){
        foreach(int index, listOfIndexes) {
            knobData* kData = bsread_KnobDataP->GetMutexKnobDataPtr(index);
            //qDebug() << "Index :" << kData->pv << kData->index;
            if (kData->index>=0){
                bsread_KnobDataP->DataLock(kData);
                kData->edata.connected = false;
                bsread_KnobDataP->SetMutexKnobData(kData->index, *kData);
                bsread_KnobDataP->SetMutexKnobDataReceived(kData);
                bsread_KnobDataP->DataUnlock(kData);
            }
        }
    }
}
void bsread_Decode::bsread_Delay(){
    QMutex LocalLocker;
    QWaitCondition LocalDelay;
    LocalLocker.lock();
    LocalDelay.wait(&LocalLocker,5);
    LocalLocker.unlock();
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
    kData->edata.dataB=Q_NULLPTR;
    kData->edata.dataSize=0;
    kData->edata.valueCount=0;
    datamutex->unlock();

    //qDebug() << "Index :" << kData->pv << kData->index;
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
    qDebug() << "StreamConnectionPoint:" <<StreamConnectionPoint;
    return StreamConnectionPoint;
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





