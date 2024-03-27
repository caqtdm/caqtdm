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
#include "httpretrieval.h"
#include "urlhandlerhttp.h"

class Q_DECL_EXPORT WorkerHTTP : public QObject
{
    Q_OBJECT

public:
    WorkerHTTP()
    {
        //qDebug() << "WorkerHTTP::WorkerHTTP()";
        qRegisterMetaType<indexes>("indexes");
        qRegisterMetaType<QVector<double> >("QVector<double>");
        m_httpRetrieval = (HttpRetrieval *) 0;
    }

    ~WorkerHTTP()
    {
        //qDebug() << "WorkerHTTP::~WorkerHTTP()";
        PRINTFLUSH("WorkerHTTP::~WorkerHTTP()");
    }

private:
    QVector<double> XValsN, YValsN;

public slots:

    void workerFinish() { deleteLater(); }

    HttpRetrieval *getArchive() { return m_httpRetrieval; }

    void getFromArchive(QWidget *w,
                        indexes indexNew,
                        QString index_name,
                        MessageWindow *messageWindow)
    {
        PRINTFLUSH("getFromArchive\n");
        Q_UNUSED(w);
        struct timeb now;
        bool isBinned;

        QString key = indexNew.pv;
        int nbVal = 0;

        ftime(&now);
        double endSeconds = (double) now.time + (double) now.millitm / (double) 1000;
        double startSeconds = endSeconds - indexNew.secondsPast;
        if (indexNew.nrOfBins != -1) {
            isBinned = true;
        } else {
            isBinned = false;
        }

        UrlHandlerHttp *urlHandler = new UrlHandlerHttp();
        urlHandler->setUrl(index_name);
        urlHandler->setBackend(indexNew.backend);
        urlHandler->setChannelName(indexNew.pv);
        urlHandler->setBeginTime(QDateTime::fromSecsSinceEpoch(startSeconds));
        urlHandler->setEndTime(QDateTime::fromSecsSinceEpoch(endSeconds));
        urlHandler->setBinned(isBinned);
        urlHandler->setBinCount(indexNew.nrOfBins);

        m_httpRetrieval = new HttpRetrieval();

        //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << "fromArchive pointer=" << fromArchive << indexNew.timeAxis;
        bool readdata_ok = m_httpRetrieval->requestUrl(urlHandler,
                                                   indexNew.secondsPast,
                                                   isBinned,
                                                   indexNew.timeAxis,
                                                   key);

        if (m_httpRetrieval->is_Redirected()) {
            QUrl url = QUrl(m_httpRetrieval->getRedirected_Url());
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
            m_httpRetrieval->deleteLater();
            m_httpRetrieval = new HttpRetrieval();
            UrlHandlerHttp *urlHandler = new UrlHandlerHttp();
            urlHandler->setUrl(url);
            readdata_ok = m_httpRetrieval->requestUrl(urlHandler,
                                                  indexNew.secondsPast,
                                                  isBinned,
                                                  indexNew.timeAxis,
                                                  key);
        }

        if (readdata_ok) {
            if ((nbVal = m_httpRetrieval->getCount()) > 0) {
                //qDebug() << nbVal << total;
                XValsN.resize(m_httpRetrieval->getCount());
                YValsN.resize(m_httpRetrieval->getCount());
                m_httpRetrieval->getData(XValsN, YValsN);
            }

        } else {
            if (messageWindow != (MessageWindow *) Q_NULLPTR) {
                QString mess("ArchiveHTTP plugin -- lastError: ");
                mess.append(m_httpRetrieval->lastError());
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

        emit resultReady(indexNew, nbVal, XValsN, YValsN, m_httpRetrieval->getBackend());

        m_httpRetrieval->deleteLater();
    }

signals:
    void resultReady(indexes indexNew,
                     int nbVal,
                     QVector<double> TimerN,
                     QVector<double> YValsN,
                     QString backend);

public:
private:
    HttpRetrieval *m_httpRetrieval;
};

#endif // WORKERHTTP_H
