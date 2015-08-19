#ifndef BSREAD_DECODE_H
#define BSREAD_DECODE_H


class bsread_Decode : public QThread
{
public:
    bsread_Decode(void * Context,QString ConnectionPoint);


    void *getZmqsocket() const;

    QString getConnectionPoint() const;

private:
    void * zmqsocket;

    QString ConnectionPoint;

};

#endif // BSREAD_DECODE_H
