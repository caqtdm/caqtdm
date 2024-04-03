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
#include <QThreadPool>

#include "archivehttp_plugin.h"
#include "archiverCommon.h"

#define qasc(x) x.toLatin1().constData()

// =======================================================================================================================================================
//  public:

ArchiveHTTP_Plugin::ArchiveHTTP_Plugin()
{
    qDebug() << "Slept for 10000 ms";
    suspend = false;
    qRegisterMetaType<indexes>("indexes");
    qRegisterMetaType<QVector<double> >("QVector<double>");
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
}

QString ArchiveHTTP_Plugin::pluginName()
{
    return "archiveHTTP";
}

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
    // get rid of data to track redundancy, is needed for reload, because otherwise all channels (which are re-added on reload) will be seen as redundant, resulting in none actually being updated.
    QString keyInCheck = kData->pv;
    keyInCheck.replace(".X", "", Qt::CaseInsensitive);
    keyInCheck.replace(".Y", "", Qt::CaseInsensitive);
    // We need that address, because it was added to the key in order to prevent mix ups from different widgets...
    char dispWAddress[CHAR_ARRAY_LENGTH];
    sprintf(dispWAddress, "_%p",kData->dispW);
    keyInCheck += QString(dispWAddress);
    for (QMap<QString, indexes>::const_iterator tempI = m_IndexesToUpdate.constBegin();
         tempI != m_IndexesToUpdate.constEnd();
         tempI++) {
        QString keyStored = tempI.key();
        regexStr.setPattern("\\b[0-7]_");
        keyStored.replace(regexStr, "");

        if (keyStored == keyInCheck) {
            m_IndexesToUpdate.remove(tempI.key());
            break;
        }
    }

    // now just let archiverCommon do the usual stuff
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
    indexes indexNew, int nbVal, QVector<double> XValsN, QVector<double> YValsN, QString backend, bool isFinalIteration)
{
    QMutexLocker mutexLocker(&mutex);
    QMap<QString, WorkerHttpThread*>::const_iterator listOfThreadsEntry = listOfThreads.constFind(indexNew.key);
    if (listOfThreadsEntry == listOfThreads.constEnd()) {
        // This should never happen
        return;
    }
    const bool isActive =  listOfThreadsEntry.value()->isActive();

    XValsN.resize(nbVal);
    YValsN.resize(nbVal);

    // set data for other indexes with same channel
    indexes indexInCheck = indexNew;
    regexStr.setPattern("\\b[0-7]_");
    indexInCheck.key.replace(regexStr, "");
    indexInCheck.key.replace(regexStr, "");
    if (nbVal > 0) {
        for (QMap<QString, indexes>::const_iterator tempI = m_IndexesToUpdate.constBegin();
             tempI != m_IndexesToUpdate.constEnd();
             tempI++) {
            QString keyStored = tempI.key();
            keyStored.replace(regexStr, "");
            keyStored.replace(regexStr, "");

            if (keyStored == indexInCheck.key) {
                if (isActive) {
                    archiverCommon->updateCartesian(nbVal, tempI.value(), XValsN, YValsN, backend);
                }
            }
        }
    }
    XValsN.resize(0);
    YValsN.resize(0);
    if (isFinalIteration) {
        WorkerHttpThread *finishedThread = (WorkerHttpThread *) listOfThreadsEntry.value();
        if (finishedThread != Q_NULLPTR) {
            finishedThread->quit();
            finishedThread->wait();
        }
        listOfThreads.remove(indexNew.key);


        QList<QString> removeKeys;
        regexStr.setPattern("\\b[0-7]_");
        for (QMap<QString, indexes>::const_iterator indexesToUpdateIterator = m_IndexesToUpdate.constBegin();
             indexesToUpdateIterator != m_IndexesToUpdate.constEnd();
             indexesToUpdateIterator++) {
            QString keyStored = indexesToUpdateIterator.key();
            keyStored.replace(regexStr, "");
            keyStored.replace(regexStr, "");
            if (keyStored == indexInCheck.key) {
                if (!isActive) {
                    archiverCommon->updateSecondsPast(indexesToUpdateIterator.value(), nbVal != 0);
                }
                removeKeys.append(indexesToUpdateIterator.key());;
            }
        }

        for (int i = 0; i < removeKeys.count(); i++) {
            m_IndexesToUpdate.remove(removeKeys.at(i));
        }
    }
}

// =======================================================================================================================================================
//  private slots:

// this routine will be called now every 10 seconds to update the cartesianplot
// however with much data it may take much longer, then  suppress any new request
void ArchiveHTTP_Plugin::Callback_UpdateInterface(QMap<QString, indexes> listOfIndexes)
{
    QMutexLocker mutexLocker(&mutex);
    if (suspend) {
        return;
    }

    // Index name (url)
    QString index_name = "https://data-api.psi.ch/";

    // remove the curve index that seperates indexes from different curve,
    // so we can avoid requesting the same index multiple times.
    regexStr.setPattern("\\b[0-7]_");
    QMap<QString, indexes>::const_iterator i = listOfIndexes.constBegin();
    while (i != listOfIndexes.constEnd()) {
        // Don't retrieve data twice
        QString keyInCheck = i.key();
        keyInCheck.replace(regexStr, "");
        keyInCheck.replace(regexStr, "");
        indexes indexInCheck = i.value();
        indexInCheck.key.replace(regexStr, "");
        indexInCheck.key.replace(regexStr, "");
        bool keyAlreadyPresent = false;
        for (QMap<QString, indexes>::const_iterator tempI = m_IndexesToUpdate.constBegin();
             tempI != m_IndexesToUpdate.constEnd();
             tempI++) {
            QString keyStored = tempI.key();
            keyStored.replace(regexStr, "");
            keyStored.replace(regexStr, "");

            if (keyStored == keyInCheck) {
                m_IndexesToUpdate.insert(i.key(), i.value());
                keyAlreadyPresent = true;
                break;
            }
        }
        if (keyAlreadyPresent) {
            i++;
            continue;
        }
        m_IndexesToUpdate.insert(i.key(), i.value());

        // Now initiate the retrieval
        indexes indexNew = i.value();

        // Get Index name if specified for this widget
        indexNew.nrOfBins = -1;
        indexNew.backend = "";
        if (caCartesianPlot *w = qobject_cast<caCartesianPlot *>((QWidget *) indexNew.w)) {
            QVariant var = w->property("nrOfBins");
            if (!var.isNull()) {
                bool ok;
                indexNew.nrOfBins = var.toInt(&ok);
                if (!ok) {
                    indexNew.nrOfBins = -1;
                }
            } else if (indexNew.init) {
                QString mess(
                    "ArchiveHTTP plugin -- no nrOfBins defined as dynamic property in widget "
                    + w->objectName() + ", defaulting to maximum number of points");
                if (messagewindowP != (MessageWindow *) Q_NULLPTR) {
                    messagewindowP->postMsgEvent(QtWarningMsg, (char *) qasc(mess));
                }
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
                    if (messagewindowP != (MessageWindow *) Q_NULLPTR) {
                        messagewindowP->postMsgEvent(QtWarningMsg, (char *) qasc(mess));
                    }
                }
            } else if (indexNew.init) {
                QString mess(
                    "ArchiveHTTP plugin -- no backend defined as dynamic property in widget "
                    + w->objectName()
                    + ", it is defined by the server e.g.(sf-archiverappliance,sf-databuffer)");
                if (messagewindowP != (MessageWindow *) Q_NULLPTR) {
                    messagewindowP->postMsgEvent(QtWarningMsg, (char *) qasc(mess));
                }
            }

            // first look if an environment variable is set for the url
            QString url = (QString) qgetenv("CAQTDM_ARCHIVERHTTP_URL");
            if (url.size() == 0 || (!w->property("archiverIndex").toString().isEmpty())) {
                var = w->property("archiverIndex");
                if (!var.isNull()) {
                    QString indexName = var.toString();
                    index_name = qasc(indexName);
                    if (indexNew.init) {
                        QString mess("ArchiveHTTP plugin -- archiverIndex defined as dynamic "
                                     "property in widget "
                                     + w->objectName() + ", set to " + index_name);
                        if (messagewindowP != (MessageWindow *) Q_NULLPTR) {
                            messagewindowP->postMsgEvent(QtWarningMsg, (char *) qasc(mess));
                        }
                    }
                } else if (indexNew.init) {
                    QString mess(
                        "ArchiveHTTP plugin -- no environment variable CAQTDM_ARCHIVERSF_URL "
                        "set and no archiverIndex defined as dynamic property in widget "
                        + w->objectName() + ", defaulting to " + index_name);
                    if (messagewindowP != (MessageWindow *) Q_NULLPTR) {
                        messagewindowP->postMsgEvent(QtWarningMsg, (char *) qasc(mess));
                    }
                }
            } else {
                if (indexNew.init) {
                    QString mess("ArchiveHTTP plugin -- archiver URL defined as " + url
                                 + " from environment variable CAQTDM_ARCHIVERSF_URL");
                    if (messagewindowP != (MessageWindow *) Q_NULLPTR) {
                        messagewindowP->postMsgEvent(QtWarningMsg, (char *) qasc(mess));
                    }
                }
                index_name = url;
            }
        }
        WorkerHTTP *newWorker = new WorkerHTTP;
        WorkerHttpThread *newWorkerThread = new WorkerHttpThread(newWorker);
        listOfThreads.insert(i.key(), newWorkerThread);

        newWorker->moveToThread(newWorkerThread);

        connect(newWorkerThread, SIGNAL(finished()), newWorker, SLOT(workerFinish()));
        connect(newWorkerThread, SIGNAL(finished()), newWorkerThread, SLOT(deleteLater()));
        connect(this,
                SIGNAL(operate(QWidget *, indexes, QString, MessageWindow *, MutexKnobData *)),
                newWorker,
                SLOT(getFromArchive(QWidget *, indexes, QString, MessageWindow *, MutexKnobData *)));
        connect(newWorker,
                SIGNAL(resultReady(indexes, int, QVector<double>, QVector<double>, QString, bool)),
                this,
                SLOT(handleResults(indexes, int, QVector<double>, QVector<double>, QString, bool)));
        newWorkerThread->start();

        emit operate((QWidget *) messagewindowP, indexNew, index_name, messagewindowP, mutexknobdataP);
        disconnect(newWorker);
        ++i;
    }
}

void ArchiveHTTP_Plugin::Callback_AbortOutstandingRequests(QString key)
{
    QMutexLocker mutexLocker(&mutex);
    suspend = true;

    QMap<QString, WorkerHttpThread*>::iterator listOfThreadsEntry = listOfThreads.find(key);
    if (listOfThreadsEntry != listOfThreads.end()) {
        listOfThreadsEntry.value()->setIsActive(false);
        //listOfThreadsEntry.value()->getHttpRetrieval()->cancelDownload();
        QMetaObject::invokeMethod(listOfThreadsEntry.value()->getHttpRetrieval(), "cancelDownload");
    }

    suspend = false;
}

void ArchiveHTTP_Plugin::closeEvent()
{
    emit Signal_StopUpdateInterface();
}
// =======================================================================================================================================================

