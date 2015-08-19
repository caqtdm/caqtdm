#include "bsread_decode.h"
#include "zmq.h"

bsread_Decode::bsread_Decode(void * Context,QString ConnectionPoint)
{

    zmqsocket=zmq_socket(Context, ZMQ_PULL);
    if (!zmqsocket) {
        printf ("error in zmq_socket: %s\n", zmq_strerror (errno));
    }
    rc = zmq_connect (zmqsocket, ConnectionPoint.toLatin1().constData());
    if (rc != 0) {
        printf ("error in zmq_bind: %s(%s)\n", zmq_strerror (errno),ConnectionPoint.toLatin1().constData());

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



