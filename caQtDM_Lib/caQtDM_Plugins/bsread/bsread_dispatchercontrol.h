#ifndef BSREAD_DISPATCHERCONTROL_H
#define BSREAD_DISPATCHERCONTROL_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>


class bsread_dispatchercontrol : public QObject
{
Q_OBJECT

public:
    explicit bsread_dispatchercontrol(QObject *parent=0);
    int set_Dispatcher(QString *dispatcher);

signals:
   void requestFinished();

private slots:
   void finishReply();
   void httpReadyRead();

protected:
  QNetworkAccessManager manager;
  QList<QString> streams;

};

#endif // BSREAD_DISPATCHERCONTROL_H
