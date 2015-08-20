#include <QThread>
#include "zmq.h"

#include "bsread_decode.h"
#include "knobData.h"

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
        printf("Message\n");
        rc = zmq_recvmsg (zmqsocket, &msg, 0);
        if (rc < 0) {
            printf ("error in zmq_recvmsg: %s\n", zmq_strerror (errno));

        }
//        if (zmq_msg_size (&msg) != message_size) {
//            printf ("message of incorrect size received\n");

//        }

        zmq_getsockopt (zmqsocket, ZMQ_RCVMORE, &more, &more_size);
        while (more){

            rc = zmq_recvmsg (zmqsocket, &msg, 0);
            if (rc < 0) {
                printf ("error in zmq_recvmsg: %s\n", zmq_strerror (errno));
            }

            zmq_getsockopt (zmqsocket, ZMQ_RCVMORE, &more, &more_size);

        }




    }


}
size_t bsread_Decode::getMessage_size() const
{
    return message_size;
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





