#ifndef WORKERHTTP_H
#define WORKERHTTP_H

#include <QList>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QThread>
#include <QTimer>
#include <qwt.h>

#include "archiverCommon.h"
#include "httpRetrieval.h"

class Q_DECL_EXPORT WorkerHTTP : public QObject
{
    Q_OBJECT

public:
    WorkerHTTP()
    {
        //qDebug() << "WorkerHTTP::WorkerHTTP()";
        qRegisterMetaType<indexes>("indexes");
        qRegisterMetaType<QVector<double> >("QVector<double>");
        fromArchive = (httpRetrieval *) 0;
    }

    ~WorkerHTTP()
    {
        //qDebug() << "WorkerHTTP::~WorkerHTTP()";
    }

private:
    QVector<double> TimerN, YValsN;

public slots:

    void workerFinish() { deleteLater(); }

    httpRetrieval *getArchive() { return fromArchive; }

    void getFromArchive(QWidget *w,
                        indexes indexNew,
                        QString index_name,
                        MessageWindow *messageWindow)
    {
        Q_UNUSED(w);

        QMutex *mutex = indexNew.mutexP;
        mutex->lock();

        struct timeb now;
        QUrl url = QUrl(index_name);
        QString fields, agg;
        bool isBinned;

        QString key = indexNew.pv;
        int nbVal = 0;

        ftime(&now);
        double endSeconds = (double) now.time + (double) now.millitm / (double) 1000;
        double startSeconds = endSeconds - indexNew.secondsPast;
#ifdef CSV
        QString response = "'response':{'format':'csv'}";
#else
        QString response = "'response':{'format':'json'}";
#endif
        QString channels;
        if (indexNew.backend.size() > 0) {
            channels = "'channels': [ {'name':'" + key + "', 'backend' : '" + indexNew.backend
                       + "' }]";
        } else {
            channels = "'channels': [ {'name':'" + key + "' }]";
        }

        QString range = "'range': { 'startSeconds' : '" + QString::number(startSeconds, 'g', 10)
                        + "', 'endSeconds' : '" + QString::number(endSeconds, 'g', 10) + "'}";
        fields = "'fields':['channel','globalSeconds','value']";

        if (indexNew.nrOfBins != -1) {
            isBinned = true;
            agg = tr(", 'aggregation': {'aggregationType':'value', "
                     "'aggregations':['min','mean','max'], 'nrOfBins' : %1}")
                      .arg(indexNew.nrOfBins);
        } else {
            isBinned = true;
            agg = ", 'aggregation': {'aggregationType':'value', "
                  "'aggregations':['min','mean','max'], 'durationPerBin' : 'PT1S'}";
            //agg = "";
        }
        QString total = "{" + response + "," + range + "," + channels + "," + fields + agg + "}";
        total = total.replace("'", "\"");
        QByteArray json_str = total.toUtf8();

        fromArchive = new httpRetrieval();

       //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << "fromArchive pointer=" << fromArchive << indexNew.timeAxis;
        bool readdata_ok = fromArchive->requestUrl(url,
                                                   json_str,
                                                   indexNew.secondsPast,
                                                   isBinned,
                                                   indexNew.timeAxis,
                                                   key);

        if (fromArchive->is_Redirected()) {
            url = QUrl(fromArchive->getRedirected_Url());
            // Messages in case of a redirect and set the widget to the correct location
            // with a reload of the panel this information get lost.
            // the url storage location is the dynamic property of the widget
            QString mess("ArchiveHTTP plugin -- redirect: ");
            mess.append(key);
            mess.append(" to ");
            mess.append(url.toString());
            messageWindow->postMsgEvent(QtDebugMsg, (char *) qasc(mess));
            indexNew.w->setProperty("archiverIndex", QVariant(url.toString()));
           //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << "archiv PV" << indexNew.pv;
            fromArchive->deleteLater();
            fromArchive = new httpRetrieval();
            readdata_ok = fromArchive->requestUrl(url,
                                                  json_str,
                                                  indexNew.secondsPast,
                                                  isBinned,
                                                  indexNew.timeAxis,
                                                  key);
        }

        if (readdata_ok) {
            if ((nbVal = fromArchive->getCount()) > 0) {
                //qDebug() << nbVal << total;
                TimerN.resize(fromArchive->getCount());
                YValsN.resize(fromArchive->getCount());
                fromArchive->getData(TimerN, YValsN);
            }

        } else {
            if (messageWindow != (MessageWindow *) Q_NULLPTR) {
                QString mess("ArchiveHTTP plugin -- lastError: ");
                mess.append(fromArchive->lastError());
                mess.append(" for pv: ");
                mess.append(key);
#if QT_VERSION > 0x050000
                mess = QString(mess.toHtmlEscaped());
#else
                mess = (Qt::escape(mess));
#endif
                messageWindow->postMsgEvent(QtFatalMsg, (char *) qasc(mess));
            }
        }

       //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << QTime::currentTime().toString() << "number of values received" << nbVal << fromArchive << "for" << key;

        emit resultReady(indexNew, nbVal, TimerN, YValsN, fromArchive->getBackend());

        mutex->unlock();
        fromArchive->deleteLater();
    }

signals:
    void resultReady(indexes indexNew,
                     int nbVal,
                     QVector<double> TimerN,
                     QVector<double> YValsN,
                     QString backend);

public:
private:
    httpRetrieval *fromArchive;
};

#endif // WORKERHTTP_H
