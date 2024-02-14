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
 *  Copyright (c) 2024
 *
 *  Author:
 *    Erik Schwarz
 *  Contact details:
 *    erik.schwarz@psi.ch
 */
#include <QApplication>
#include <QDebug>

#include "archiveHTTP_plugin.h"
#include "archiverCommon.h"

#define qasc(x) x.toLatin1().constData()

// =======================================================================================================================================================
//  public:

ArchiveHTTP_Plugin::ArchiveHTTP_Plugin()
{
    suspend = false;
    qRegisterMetaType<indexes>("indexes");
    qRegisterMetaType<QVector<double> >("QVector<double>");

    //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << "Create (http-retrieval)";
    archiverCommon = new ArchiverCommon();

    connect(archiverCommon,
            SIGNAL(Signal_UpdateInterface(QMap<QString, indexes>)),
            this,
            SLOT(Callback_UpdateInterface(QMap<QString, indexes>)));
    connect(archiverCommon,
            SIGNAL(Signal_AbortOutstandingRequests(QString)),
            this,
            SLOT(Callback_AbortOutstandingRequests(QString)));
    connect(this, SIGNAL(Signal_StopUpdateInterface()), archiverCommon, SLOT(stopUpdateInterface()));
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(closeEvent()));
}

ArchiveHTTP_Plugin::~ArchiveHTTP_Plugin()
{
    delete archiverCommon;
    //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << "ArchiveHTTP_Plugin::~ArchiveHTTP_Plugin()";
}

// gives the plugin name back
QString ArchiveHTTP_Plugin::pluginName()
{
    return "archiveHTTP";
}

// init communication
int ArchiveHTTP_Plugin::initCommunicationLayer(MutexKnobData *data,
                                               MessageWindow *messageWindow,
                                               QMap<QString, QString> options)
{
    mutexknobdataP = data;
    messagewindowP = messageWindow;
    return archiverCommon->initCommunicationLayer(data, messageWindow, options);
}

// define data to be called
int ArchiveHTTP_Plugin::pvAddMonitor(int index, knobData *kData, int rate, int skip)
{
    return archiverCommon->pvAddMonitor(index, kData, rate, skip);
}

// clear routines
int ArchiveHTTP_Plugin::pvClearMonitor(knobData *kData)
{
    return archiverCommon->pvClearMonitor(kData);
}
int ArchiveHTTP_Plugin::pvFreeAllocatedData(knobData *kData)
{
    return archiverCommon->pvFreeAllocatedData(kData);
}

int ArchiveHTTP_Plugin::pvSetValue(
    char *pv, double rdata, int32_t idata, char *sdata, char *object, char *errmess, int forceType)
{
    Q_UNUSED(pv);
    Q_UNUSED(rdata);
    Q_UNUSED(idata);
    Q_UNUSED(sdata);
    Q_UNUSED(object);
    Q_UNUSED(errmess);
    Q_UNUSED(forceType);
    return true;
}
int ArchiveHTTP_Plugin::pvSetWave(char *pv,
                                  float *fdata,
                                  double *ddata,
                                  int16_t *data16,
                                  int32_t *data32,
                                  char *sdata,
                                  int nelm,
                                  char *object,
                                  char *errmess)
{
    Q_UNUSED(pv);
    Q_UNUSED(fdata);
    Q_UNUSED(ddata);
    Q_UNUSED(data16);
    Q_UNUSED(data32);
    Q_UNUSED(sdata);
    Q_UNUSED(nelm);
    Q_UNUSED(object);
    Q_UNUSED(errmess);
    return true;
}
int ArchiveHTTP_Plugin::pvGetTimeStamp(char *pv, char *timestamp)
{
    Q_UNUSED(pv);
    Q_UNUSED(timestamp);
    return true;
}
int ArchiveHTTP_Plugin::pvGetDescription(char *pv, char *description)
{
    Q_UNUSED(pv);
    Q_UNUSED(description);
    return true;
}
int ArchiveHTTP_Plugin::pvClearEvent(void *ptr)
{
    return archiverCommon->pvClearEvent(ptr);
}
int ArchiveHTTP_Plugin::pvAddEvent(void *ptr)
{
    return archiverCommon->pvAddEvent(ptr);
}
int ArchiveHTTP_Plugin::pvReconnect(knobData *kData)
{
    Q_UNUSED(kData);
    return true;
}
int ArchiveHTTP_Plugin::pvDisconnect(knobData *kData)
{
    Q_UNUSED(kData);
    return true;
}
int ArchiveHTTP_Plugin::FlushIO()
{
    return true;
}

int ArchiveHTTP_Plugin::TerminateIO()
{
    return archiverCommon->TerminateIO();
}

// =======================================================================================================================================================
//  public slots:

void ArchiveHTTP_Plugin::handleResults(
    indexes indexNew, int nbVal, QVector<double> TimerN, QVector<double> YValsN, QString backend)
{
    //QThread *thread = QThread::currentThread();
    //qDebug() << "in sf handle results" << nbVal << TimerN.count() << indexNew.indexX << indexNew.indexY << thread;

    TimerN.resize(nbVal);
    YValsN.resize(nbVal);

    //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << "handle cartesian";
    if (nbVal > 0)
        archiverCommon->updateCartesian(nbVal, indexNew, TimerN, YValsN, backend);
    TimerN.resize(0);
    YValsN.resize(0);

    //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << "handle cartesian fisnished";
    QList<QString> removeKeys;
    removeKeys.clear();

    QMap<QString, WorkerHttpThread *>::iterator j = listOfThreads.find(indexNew.key);
    while (j != listOfThreads.end() && j.key() == indexNew.key) {
        WorkerHttpThread *tmpThread = (WorkerHttpThread *) j.value();
        tmpThread->quit();
        removeKeys.append(indexNew.key);
        //qDebug() << tmpThread << "sf quit";
        ++j;
    }

    for (int i = 0; i < removeKeys.count(); i++) {
        listOfThreads.remove(removeKeys.at(i));
    }

    if (nbVal == 0)
        archiverCommon->updateSecondsPast(indexNew, false);
    else
        archiverCommon->updateSecondsPast(indexNew, true);

    //qDebug() << "in sf handle results finished";
}

// =======================================================================================================================================================
//  private slots:

// this routine will be called now every 10 seconds to update the cartesianplot
// however with much data it may take much longer, then  suppress any new request
void ArchiveHTTP_Plugin::Callback_UpdateInterface(QMap<QString, indexes> listOfIndexes)
{
    if (suspend)
        return;

    // Index name (url)
    QString index_name = "https://data-api.psi.ch/sf/query";

    //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << "====================== ArchiveHTTP_Plugin::Callback_UpdateInterface";

    QMap<QString, indexes>::const_iterator i = listOfIndexes.constBegin();

    while (i != listOfIndexes.constEnd()) {
        WorkerHttpThread *tmpThread = (WorkerHttpThread *) Q_NULLPTR;
        indexes indexNew = i.value();
        //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << " -------------" << i.key() << ": " << indexNew.indexX << indexNew.indexY << indexNew.pv << indexNew.w;

        QMap<QString, WorkerHttpThread *>::iterator j = listOfThreads.find(indexNew.key);
        while (j != listOfThreads.end() && j.key() == indexNew.key) {
            tmpThread = (WorkerHttpThread *) j.value();
            ++j;
        }

        if ((tmpThread != (WorkerHttpThread *) Q_NULLPTR) && tmpThread->isRunning()) {
            //QDebug() << (__FILE__) << ":" << (__LINE__) << "|"            << "thread is running" << tmpThread << tmpThread->isRunning();

        } else {
            // Get Index name if specified for this widget
            indexNew.nrOfBins = -1;
            indexNew.backend = "";
            if (caCartesianPlot *w = qobject_cast<caCartesianPlot *>((QWidget *) indexNew.w)) {
                QVariant var = w->property("nrOfBins");
                if (!var.isNull()) {
                    bool ok;
                    indexNew.nrOfBins = var.toInt(&ok);
                    if (!ok)
                        indexNew.nrOfBins = -1;
                } else if (indexNew.init) {
                    QString mess(
                        "ArchiveHTTP plugin -- no nrOfBins defined as dynamic property in widget "
                        + w->objectName() + ", defaulting to maximum number of points");
                    if (messagewindowP != (MessageWindow *) Q_NULLPTR)
                        messagewindowP->postMsgEvent(QtWarningMsg, (char *) qasc(mess));
                }

                var = w->property("backend");
                if (!var.isNull()) {
                    QString backend = var.toString().trimmed().toLower();
                    if (QString::compare(backend, "sf-archiverappliance") == 0) {
                        indexNew.backend = var.toString();
                    } else if (QString::compare(backend, "sf-databuffer") == 0) {
                        indexNew.backend = var.toString();
                    } else {
                        QString mess(
                            "ArchiveHTTP plugin -- backend defined as dynamic property in widget "
                            "but not known (use sf-archiverappliance or sf-databuffer) in widget "
                            + w->objectName());
                        indexNew.backend = var.toString();
                        if (messagewindowP != (MessageWindow *) Q_NULLPTR)
                            messagewindowP->postMsgEvent(QtWarningMsg, (char *) qasc(mess));
                    }
                } else if (indexNew.init) {
                    QString mess(
                        "ArchiveHTTP plugin -- no backend defined as dynamic property in widget "
                        + w->objectName()
                        + ", it is defined by the server e.g.(sf-archiverappliance,sf-databuffer)");
                    if (messagewindowP != (MessageWindow *) Q_NULLPTR)
                        messagewindowP->postMsgEvent(QtWarningMsg, (char *) qasc(mess));
                }

                // first look if an environment variable is set for the url
                QString url = (QString) qgetenv("CAQTDM_ARCHIVERSF_URL");
                if (url.size() == 0 || (!w->property("archiverIndex").toString().isEmpty())) {
                    var = w->property("archiverIndex");
                    //QDebug() << (__FILE__) << ":" << (__LINE__) << "|"<< "Check URL: " << var;
                    if (!var.isNull()) {
                        QString indexName = var.toString();
                        index_name = qasc(indexName);
                        if (indexNew.init) {
                            QString mess("ArchiveHTTP plugin -- archiverIndex defined as dynamic "
                                         "property in widget "
                                         + w->objectName() + ", set to " + index_name);
                            if (messagewindowP != (MessageWindow *) Q_NULLPTR)
                                messagewindowP->postMsgEvent(QtWarningMsg, (char *) qasc(mess));
                        }
                    } else if (indexNew.init) {
                        QString mess(
                            "ArchiveHTTP plugin -- no environment variable CAQTDM_ARCHIVERSF_URL "
                            "set and no archiverIndex defined as dynamic property in widget "
                            + w->objectName() + ", defaulting to " + index_name);
                        if (messagewindowP != (MessageWindow *) Q_NULLPTR)
                            messagewindowP->postMsgEvent(QtWarningMsg, (char *) qasc(mess));
                    }
                } else {
                    if (indexNew.init) {
                        QString mess("ArchiveHTTP plugin -- archiver URL defined as " + url
                                     + " from environment variable CAQTDM_ARCHIVERSF_URL");
                        if (messagewindowP != (MessageWindow *) Q_NULLPTR)
                            messagewindowP->postMsgEvent(QtWarningMsg, (char *) qasc(mess));
                    }
                    index_name = url;
                }
            }
            WorkerHTTP *worker = new WorkerHTTP;
            WorkerHttpThread *tmpWorkerThread = new WorkerHttpThread(worker);
            //qDebug() << "tmpThread new" << tmpThread;
            listOfThreads.insert(i.key(), tmpWorkerThread);
            ;

            worker->moveToThread(tmpWorkerThread);

            connect(tmpWorkerThread, SIGNAL(finished()), worker, SLOT(workerFinish()));
            connect(tmpWorkerThread, SIGNAL(finished()), tmpWorkerThread, SLOT(deleteLater()));
            connect(this,
                    SIGNAL(operate(QWidget *, indexes, QString, MessageWindow *)),
                    worker,
                    SLOT(getFromArchive(QWidget *, indexes, QString, MessageWindow *)));
            connect(worker,
                    SIGNAL(resultReady(indexes, int, QVector<double>, QVector<double>, QString)),
                    this,
                    SLOT(handleResults(indexes, int, QVector<double>, QVector<double>, QString)));
            tmpWorkerThread->start();

            emit operate((QWidget *) messagewindowP, indexNew, index_name, messagewindowP);

            disconnect(worker);
        }

        ++i;
    }
    //QDebug() << (__FILE__) << ":" << (__LINE__) << "|"   << "====================== ArchiveHTTP_Plugin::Callback_UpdateInterface finished";
}

void ArchiveHTTP_Plugin::Callback_AbortOutstandingRequests(QString key)
{
    suspend = true;
    //QDebug() << (__FILE__) << ":" << (__LINE__) << "|"<< "Callback_AbortOutstandingRequests for key" << key;

    WorkerHttpThread *tmpThread = (WorkerHttpThread *) Q_NULLPTR;
    QMap<QString, WorkerHttpThread *>::iterator j = listOfThreads.find(key);
    while (j != listOfThreads.end() && j.key() == key) {
        tmpThread = (WorkerHttpThread *) j.value();
        if (tmpThread != (WorkerHttpThread *) Q_NULLPTR) {
            httpRetrieval *retrieval = tmpThread->getArchive();
            tmpThread->quit();
            if (retrieval != (httpRetrieval *) Q_NULLPTR) {
                //QDebug() << (__FILE__) << ":" << (__LINE__) << "|"<< "retrieval->cancelDownload()" << retrieval;
                retrieval->cancelDownload();
                retrieval->deleteLater();
            }
        }
        ++j;
    }

    QApplication::processEvents();
    suspend = false;
}

void ArchiveHTTP_Plugin::closeEvent()
{
    //QDebug() << (__FILE__) << ":" << (__LINE__) << "|"  << "ArchiveHTTP_Plugin::closeEvent ";
    emit Signal_StopUpdateInterface();
}
// =======================================================================================================================================================

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#else
Q_EXPORT_PLUGIN2(ArchiveHTTP_Plugin, ArchiveHTTP_Plugin)
#endif
