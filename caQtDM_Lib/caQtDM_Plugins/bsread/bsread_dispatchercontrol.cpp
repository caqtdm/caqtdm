#include <QDebug>
#include "bsread_dispatchercontrol.h"
#include <QThread>
#include "JSON.h"
#include "JSONValue.h"

bsread_dispatchercontrol::bsread_dispatchercontrol(QObject *parent) : QObject(parent)
{

}


int bsread_dispatchercontrol::set_Dispatcher(QString *dispatcher)
{
   QString StreamDispatcher=dispatcher->append("stream");
   QUrl url(StreamDispatcher);
   QNetworkReply* reply;
   QNetworkRequest request=QNetworkRequest(url);

   //reply = manager.get(QNetworkRequest(url));

   QString data="{\"channel:\"[";

   data.append("{\"name\":\"TESTChannel\",\"modulo\":1,\"offset\":0}");


   data.append("]},\sendIncompleteMessages\":true,} ");

    QByteArray transferdata;
    transferdata.append(data);

   qDebug() <<"Send TEst Data"<< *dispatcher << data;

   request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
   reply = manager.post(request,transferdata);




   connect(reply, SIGNAL(finished()), this, SLOT(finishReply()));
   connect(reply, SIGNAL(readyRead()),this, SLOT(httpReadyRead()));


   QThread::msleep(2500);
   qDebug() <<"Rec  finished";

/*
  QStringList ConnectionPart=dispatcher->split(":");
  QString Hostname=ConnectionPart.begin();
  quint16 Port = ConnectionPart.at(1).toInt();
  QHttp DispatcherConection=new QHttp(Hostname,Port,NULL);
*/

 return 0;
}


void bsread_dispatchercontrol::finishReply()
{

    JSONObject jsonobj;
    QString stream;
    QObject* obj = sender();
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(obj);

    streams.clear();

    qDebug() <<"Testdata received";


    JSONValue *MainMessageJ = JSON::Parse(reply->readAll());





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



