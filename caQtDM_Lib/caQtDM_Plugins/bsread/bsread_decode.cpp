#include <QThread>
#include "zmq.h"

#include "bsread_decode.h"
#include "knobData.h"
#include "JSON.h"
#include "JSONValue.h"


bsread_Decode::bsread_Decode(void * Context,QString ConnectionPoint)
{
    int rc;
    zmqsocket=zmq_socket(Context, ZMQ_PULL);
    if (!zmqsocket) {
        printf ("error in zmq_socket: %s\n", zmq_strerror (errno));
    }
    rc = zmq_connect (zmqsocket, ConnectionPoint.toLatin1().constData());
    if (rc != 0) {
        printf ("error in zmq_bind: %s(%s)\n", zmq_strerror (errno),ConnectionPoint.toLatin1().constData());
        running_decode=false;
    }else{
        running_decode=true;
    }


}
void bsread_Decode::run()
{
    int rc;
    zmq_msg_t msg;
    int64_t more;
    size_t more_size = sizeof (more);
    rc = zmq_msg_init (&msg);

    while (1){
        //printf("Message\n");
        rc = zmq_recvmsg (zmqsocket, &msg, 0);
        if (rc < 0) {
            printf ("error in zmq_recvmsg: %s\n", zmq_strerror (errno));

        }
        //        if (zmq_msg_size (&msg) != message_size) {
        //            printf ("message of incorrect size received\n");

        //        }
        setMainHeader((char*)zmq_msg_data(&msg));

        zmq_getsockopt (zmqsocket, ZMQ_RCVMORE, &more, &more_size);
        while (more){

            rc = zmq_recvmsg (zmqsocket, &msg, 0);
            if (rc < 0) {
                printf ("error in zmq_recvmsg: %s\n", zmq_strerror (errno));
            }
            Header=QString((char*)&msg);
            zmq_getsockopt (zmqsocket, ZMQ_RCVMORE, &more, &more_size);

        }




    }


}
size_t bsread_Decode::getMessage_size() const
{
    return message_size;
}
QString bsread_Decode::getMainHeader() const
{
    return MainHeader;
}

void bsread_Decode::setMainHeader(char *value)
{
    wchar_t * wcData;
    JSONObject jsonobj;
    MainHeader = QString(value);

    JSONValue *MainMessageJ = JSON::Parse(value);
    if (MainMessageJ!=NULL){
        if(!MainMessageJ->IsObject()) {
            delete(MainMessageJ);
        } else {
            jsonobj=MainMessageJ->AsObject();
            if (jsonobj.find(L"hash") != jsonobj.end() && jsonobj[L"hash"]->IsString()) {
                hash=QString::fromWCharArray(jsonobj[L"hash"]->AsString().c_str());
                printf("hType :%s\n",hash.toLatin1().constData());
            }

            if (jsonobj.find(L"pulse_id") != jsonobj.end() && jsonobj[L"pulse_id"]->IsNumber()) {
                pulse_id=jsonobj[L"pulse_id"]->AsNumber();
            }
            if (jsonobj.find(L"htype") != jsonobj.end() && jsonobj[L"htype"]->IsNumber()) {
                htype=QString::fromWCharArray(jsonobj[L"htype"]->AsString().c_str());
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
    return KnobData;
}

void bsread_Decode::setKnobData(MutexKnobData *value)
{
    KnobData = value;
}





