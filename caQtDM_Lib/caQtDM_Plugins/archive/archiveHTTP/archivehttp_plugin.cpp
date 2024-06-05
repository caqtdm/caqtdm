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
#include "archiverGeneral.h"
#include <QMetaType>

#define CURVE_IDENTIFIER "\\b[0-7]_"

#define qasc(x) x.toLatin1().constData()

ArchiveHTTP_Plugin::ArchiveHTTP_Plugin()
{
    m_IsSuspended = false;
    qRegisterMetaType<indexes>("indexes");
    qRegisterMetaType<QVector<double> >("QVector<double>");
    qRegisterMetaType<QSharedPointer<HttpPerformanceData> >("QSharedPointer<HttpPerformanceData>");
    m_archiverGeneral = new ArchiverGeneral();

    // This signal is used to update the specified channels, it is triggered regularly for all channels.
    connect(m_archiverGeneral,
            SIGNAL(Signal_UpdateInterface(QMap<QString, indexes>)),
            this,
            SLOT(Callback_UpdateInterface(QMap<QString, indexes>)));
    // This signal is used to abort all channels currently being requested, usually by a reload of the widget.
    connect(m_archiverGeneral,
            SIGNAL(Signal_AbortOutstandingRequests(QString)),
            this,
            SLOT(Callback_AbortOutstandingRequests(QString)));
    connect(this, SIGNAL(Signal_StopUpdateInterface()), m_archiverGeneral, SLOT(stopUpdateInterface()));
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(closeEvent()));
}

ArchiveHTTP_Plugin::~ArchiveHTTP_Plugin()
{
    delete m_archiverGeneral;
}

QString ArchiveHTTP_Plugin::pluginName()
{
    return "archiveHTTP";
}

int ArchiveHTTP_Plugin::initCommunicationLayer(MutexKnobData *data,
                                               MessageWindow *messageWindow,
                                               QMap<QString, QString> options)
{
    m_mutexKnobDataP = data;
    m_messageWindowP = messageWindow;
    return m_archiverGeneral->initCommunicationLayer(data, messageWindow, options);
}

int ArchiveHTTP_Plugin::pvAddMonitor(int index, knobData *kData, int rate, int skip)
{
    return m_archiverGeneral->pvAddMonitor(index, kData, rate, skip);
}

int ArchiveHTTP_Plugin::pvClearMonitor(knobData *kData)
{
    // Get rid of data to track redundancy, is needed for reload, because otherwise all channels (which are re-added on reload) will be seen as redundant,
    // resulting in none actually being updated.
    QString keyInCheck = kData->pv;
    // Remove extension
    keyInCheck.replace(".X", "", Qt::CaseInsensitive);
    keyInCheck.replace(".Y", "", Qt::CaseInsensitive);
    // We need that address, because it was added to the key in order to prevent mix ups from different widgets...
    keyInCheck += QString("_%1").arg(reinterpret_cast<quintptr>(kData->dispW), sizeof(void*) * 2, 16, QChar('0'));
    QList<QString> removeKeys;
    // Make sure all corresponding indexes are added to a list which we will remove.
    for (QMap<QString, indexes>::const_iterator tempI = m_IndexesToUpdate.constBegin();
         tempI != m_IndexesToUpdate.constEnd();
         tempI++) {
        QString keyStored = tempI.key();
        m_regexStr.setPattern(CURVE_IDENTIFIER);
        keyStored.replace(m_regexStr, "");
        if (keyStored == keyInCheck) {
            removeKeys.append(tempI.key());
        }
    }

    // Remove all found indexes. This is done after to not mess up our iterator.
    for (int i = 0; i < removeKeys.count(); i++) {
        // Remove entry for updating the data
        m_IndexesToUpdate.remove(removeKeys[i]);
        // Also remove the perfomance data
        if (m_retrievalPerformancePerPV.contains(removeKeys[i])) {
            m_retrievalPerformancePerPV.remove(removeKeys[i]);
        }
    }

    // Now just let archiverCommon do it's part.
    return m_archiverGeneral->pvClearMonitor(kData);
}

int ArchiveHTTP_Plugin::pvFreeAllocatedData(knobData *kData)
{
    return m_archiverGeneral->pvFreeAllocatedData(kData);
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
    QString report = "<br>Performance data for last request to this pv: <br>";
    QString keyInCheck = pv;
    // Get rid of the suffix
    keyInCheck.replace(".X", "", Qt::CaseInsensitive);
    keyInCheck.replace(".Y", "", Qt::CaseInsensitive);
    keyInCheck.replace(".minY", "", Qt::CaseInsensitive);
    keyInCheck.replace(".maxY", "", Qt::CaseInsensitive);
    // Get rid of curve number
    m_regexStr.setPattern(CURVE_IDENTIFIER);
    keyInCheck.replace(m_regexStr, "");

    // Now, the only thing remaining besides the pv name is the plot identifier, which we need, as different plots have different performance data.

    // Search for the entry that contains our pv and plot
    bool foundAnEntry = false;
    for (auto entry = m_retrievalPerformancePerPV.constBegin(); entry != m_retrievalPerformancePerPV.constEnd(); ++entry) {
        if (entry.key().contains(keyInCheck)) {
            foundAnEntry = true;
            // Generate a nice report and append it to our string
            report.append(entry.value()->generateReport());
            // This should only return one result either way, however we don't want useless iterations.
            break;
        }
    }
    if (!foundAnEntry) {
        // Something isn't right...
        return false;
    }
    // Write back our return value
    qstrncpy(description, report.toUtf8().constData(), MAX_STRING_LENGTH);
    return true;
}

int ArchiveHTTP_Plugin::pvClearEvent(void *ptr)
{
    Q_UNUSED(ptr);
    return true;
}
int ArchiveHTTP_Plugin::pvAddEvent(void *ptr)
{
    Q_UNUSED(ptr);
    return true;
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
    return true;
}

void ArchiveHTTP_Plugin::updateCartesianAppended(int numberOfValues,
                                                 indexes indexNew,
                                                 QVector<double> XValues,
                                                 QVector<double> YValues,
                                                 QString backend)
{
    // We have to make sure that we are synchronized with the archiverCommon
    QMutexLocker locker(m_archiverGeneral->globalMutex());

    if (numberOfValues > 0) {
        // Check for a valid index
        knobData kDataX = m_mutexKnobDataP->GetMutexKnobData(indexNew.indexX);
        knobData kDataY = m_mutexKnobDataP->GetMutexKnobData(indexNew.indexY);
        if (kDataX.index == -1 || kDataY.index == -1) {
            return;
        }

        // Initialize values used amongst both indexes
        QVector<double> alreadyStoredValues;
        timeb now;
        ftime(&now);
        double endSeconds = (double) now.time + (double) now.millitm / (double) 1000;
        double startSeconds = endSeconds - indexNew.secondsPast;

        // Lock both indexes so the corresponding data isn't updated while we process it.
        m_mutexKnobDataP->DataLock(&kDataX);
        m_mutexKnobDataP->DataLock(&kDataY);

        // Start updating the X index
        kDataX.edata.fieldtype = caDOUBLE;
        kDataX.edata.connected = true;
        kDataX.edata.accessR = true;
        kDataX.edata.accessW = false;
        kDataX.edata.monitorCount++;
        qstrncpy(kDataX.edata.fec, qasc(backend), sizeof(caqtdm_string_t));
        int offsetOfFirstNeededValue = -1;

        // If there is already useful data in the knobData, reuse it
        if (kDataX.edata.valueCount > 0) {
            alreadyStoredValues.resize(kDataX.edata.valueCount);
            memcpy(alreadyStoredValues.data(), kDataX.edata.dataB, kDataX.edata.dataSize);
            QVector<double>::iterator firstNeededX = std::upper_bound(alreadyStoredValues.begin(), alreadyStoredValues.end(), startSeconds * 1000);
            offsetOfFirstNeededValue = std::distance(alreadyStoredValues.begin(), firstNeededX);
            QVector<double> stillUsedValues(firstNeededX, alreadyStoredValues.end());
            // Update the number of values according to the amount of reused values
            numberOfValues += stillUsedValues.count();
            // Add the new data to the reused data
            stillUsedValues.append(XValues);
            XValues = std::move(stillUsedValues);
        }

        // reallocate the memory to fit the new data
        kDataX.edata.dataSize = numberOfValues * sizeof(double);
        kDataX.edata.dataB = (void *) realloc(kDataX.edata.dataB, kDataX.edata.dataSize);
        if (kDataX.edata.dataB == NULL) {
            // Uhhhhhm, no this should not happen
            printf("Realloc failed to allocate memory, maybe the system ran out of memory...\n");
            throw;
        }

        // Now copy the new data to be plotted into the knobData
        memcpy(kDataX.edata.dataB, XValues.data(), kDataX.edata.dataSize);
        kDataX.edata.valueCount = numberOfValues;
        m_mutexKnobDataP->SetMutexKnobDataReceived(&kDataX);

        // Do the same thing with the Y Index
        kDataY.edata.fieldtype = caDOUBLE;
        kDataY.edata.connected = true;
        kDataY.edata.accessR = true;
        kDataY.edata.accessW = false;
        kDataY.edata.monitorCount++;
        qstrncpy(kDataY.edata.fec, qasc(backend), sizeof(caqtdm_string_t));

        // Here, we have to modify the reusing part slightly
        if (kDataY.edata.valueCount > 0 && offsetOfFirstNeededValue != -1) {
            alreadyStoredValues.resize(kDataY.edata.valueCount);
            memcpy(alreadyStoredValues.data(), kDataY.edata.dataB, kDataY.edata.dataSize);

            // Because this data does not contain a timestamp we can use, we just use the same offset as used in the X axis to ensure we get the corresponding elements.
            QVector<double>::iterator firstNeededY = alreadyStoredValues.begin() + offsetOfFirstNeededValue;
            QVector<double> stillUsedValues(firstNeededY, alreadyStoredValues.end());
            stillUsedValues.append(YValues);
            YValues = std::move(stillUsedValues);
            // We don't have to calculate the number of values again as they are equal amongst both axes.
        }

        kDataY.edata.dataSize = numberOfValues * sizeof(double);
        kDataY.edata.dataB = (void *) realloc(kDataY.edata.dataB, kDataY.edata.dataSize);
        if (kDataY.edata.dataB == NULL) {
            // Uhhhhhm, no this should not happen
            printf("Realloc failed to allocate memory, maybe the system ran out of memory...\n");
            throw;
        }

        memcpy(kDataY.edata.dataB, &YValues[0], kDataY.edata.dataSize);
        kDataY.edata.valueCount = numberOfValues;
        m_mutexKnobDataP->SetMutexKnobDataReceived(&kDataY);

        // Unlock both indexes again
        m_mutexKnobDataP->DataUnlock(&kDataX);
        m_mutexKnobDataP->DataUnlock(&kDataY);
    }
}

void ArchiveHTTP_Plugin::handleResults(
    indexes indexNew, int valueCount, QVector<double> XVals, QVector<double> YVals, QVector<double> YMinVals, QVector<double> YMaxVals, QString backend, bool isFinalIteration)
{
    QMutexLocker mutexLocker(&m_globalMutex);
    QMap<QString, WorkerHttpThread*>::const_iterator listOfThreadsEntry = m_listOfThreads.constFind(indexNew.key);
    if (listOfThreadsEntry == m_listOfThreads.constEnd()) {
        // This should never happen
        return;
    }
    const bool isActive =  listOfThreadsEntry.value()->isActive();

    XVals.resize(valueCount);
    YVals.resize(valueCount);

    // set data for other indexes with same channel (& same widget because different widgets might have different time spans etc.)
    indexes indexInCheck = indexNew;
    m_regexStr.setPattern(CURVE_IDENTIFIER);
    indexInCheck.key.replace(m_regexStr, "");
    indexInCheck.key.replace(m_regexStr, "");
    indexInCheck.key.replace(".minY", "");
    indexInCheck.key.replace(".maxY", "");
    if (valueCount > 0) {
        for (QMap<QString, indexes>::const_iterator tempI = m_IndexesToUpdate.constBegin();
             tempI != m_IndexesToUpdate.constEnd();
             tempI++) {
            QString keyStored = tempI.key();
            keyStored.replace(m_regexStr, "");
            keyStored.replace(m_regexStr, "");
            keyStored.replace(".minY", "");
            keyStored.replace(".maxY", "");
            if (keyStored == indexInCheck.key) {
                if (isActive) {
                    // If we have binned data and the channel contains min/max then use the according values.
                    if (tempI.key().contains(".minY") && indexNew.nrOfBins > 0) {
                        updateCartesianAppended(valueCount, tempI.value(), XVals, YMinVals, backend);
                    } else if (tempI.key().contains(".maxY") && indexNew.nrOfBins > 0) {
                        updateCartesianAppended(valueCount, tempI.value(), XVals, YMaxVals, backend);
                    } else {
                        updateCartesianAppended(valueCount, tempI.value(), XVals, YVals, backend);
                    }
                }
            }
        }
    }
    XVals.resize(0);
    YVals.resize(0);
    if (isFinalIteration) {
        WorkerHttpThread *finishedThread = (WorkerHttpThread *) listOfThreadsEntry.value();
        if (finishedThread != Q_NULLPTR) {
            finishedThread->quit();
            finishedThread->wait();
        }
        m_listOfThreads.remove(indexNew.key);

        QList<QString> removeKeys;
        m_regexStr.setPattern(CURVE_IDENTIFIER);
        for (QMap<QString, indexes>::const_iterator indexesToUpdateIterator = m_IndexesToUpdate.constBegin();
             indexesToUpdateIterator != m_IndexesToUpdate.constEnd();
             indexesToUpdateIterator++) {
            QString keyStored = indexesToUpdateIterator.key();
            keyStored.replace(m_regexStr, "");
            keyStored.replace(m_regexStr, "");
            keyStored.replace(".minY", "");
            keyStored.replace(".maxY", "");
            if (keyStored == indexInCheck.key) {
                if (!isActive) {
                    m_archiverGeneral->updateSecondsPast(indexesToUpdateIterator.value(), valueCount != 0);
                }
                removeKeys.append(indexesToUpdateIterator.key());;
            }
        }

        for (int i = 0; i < removeKeys.count(); i++) {
            m_IndexesToUpdate.remove(removeKeys.at(i));
        }
    }
}

void ArchiveHTTP_Plugin::Callback_UpdateInterface(QMap<QString, indexes> listOfIndexes)
{
    QMutexLocker mutexLocker(&m_globalMutex);
    if (m_IsSuspended) {
        return;
    }

    // Index name (url), will later be redefined if dynamic property or environment variable is set
    QString index_name = "data-api.psi.ch";

    // remove the curve index that seperates indexes from different curve,
    // so we can avoid requesting the same index multiple times.
    m_regexStr.setPattern(CURVE_IDENTIFIER);
    QMap<QString, indexes>::const_iterator i = listOfIndexes.constBegin();
    while (i != listOfIndexes.constEnd()) {
        // Don't retrieve data twice
        QString keyInCheck = i.key();
        // Account for different curve Numbers
        keyInCheck.replace(m_regexStr, "");
        keyInCheck.replace(m_regexStr, "");
        // Account for .minY and .maxY
        keyInCheck.replace(".minY", "");
        keyInCheck.replace(".maxY", "");
        bool keyAlreadyPresent = false;
        for (QMap<QString, indexes>::const_iterator tempI = m_IndexesToUpdate.constBegin();
             tempI != m_IndexesToUpdate.constEnd();
             tempI++) {
            QString keyStored = tempI.key();
            keyStored.replace(m_regexStr, "");
            keyStored.replace(m_regexStr, "");
            keyStored.replace(".minY", "");
            keyStored.replace(".maxY", "");

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

        // If it doesn't already exist, create an Object to measure performance
        if (!m_retrievalPerformancePerPV.contains(i.key())) {
            HttpPerformanceData *newHttpPerformanceData = new HttpPerformanceData;
            m_retrievalPerformancePerPV.insert(i.key(), QSharedPointer<HttpPerformanceData>(newHttpPerformanceData));
        }

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
                if (m_messageWindowP != (MessageWindow *) Q_NULLPTR) {
                    m_messageWindowP->postMsgEvent(QtWarningMsg, (char *) qasc(mess));
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
                    if (m_messageWindowP != (MessageWindow *) Q_NULLPTR) {
                        m_messageWindowP->postMsgEvent(QtWarningMsg, (char *) qasc(mess));
                    }
                }
            } else if (indexNew.init) {
                QString mess(
                    "ArchiveHTTP plugin -- no backend defined as dynamic property in widget "
                    + w->objectName()
                    + ", it is defined by the server e.g.(sf-archiverappliance,sf-databuffer)");
                if (m_messageWindowP != (MessageWindow *) Q_NULLPTR) {
                    m_messageWindowP->postMsgEvent(QtWarningMsg, (char *) qasc(mess));
                }
            }

            // If we previously got a redirect, the dynamic property should now be set
            // with the redirection target, so take that if it exists. Else, if the user defined an environment variable,
            // take that. If neither a dynamic property or an environment variable is set, we use the hardcoded default value.
            // Therefore, dynamic property comes first, then environment variable, then predefined url.
            QString url = (QString) qgetenv("CAQTDM_ARCHIVERHTTP_URL");
            // Do this if dynamic property is set
            // Also do this if no environment variable is defined
            if (url.size() == 0 || (!w->property("archiverIndex").toString().isEmpty())) {
                var = w->property("archiverIndex");
                // Do this if dynamic property is set
                if (!var.isNull()) {
                    QString indexName = var.toString();
                    index_name = qasc(indexName);
                    if (indexNew.init) {
                        QString mess("ArchiveHTTP plugin -- archiverIndex defined as dynamic "
                                     "property in widget "
                                     + w->objectName() + ", set to " + index_name);
                        if (m_messageWindowP != (MessageWindow *) Q_NULLPTR) {
                            m_messageWindowP->postMsgEvent(QtWarningMsg, (char *) qasc(mess));
                        }
                    }
                } else if (indexNew.init) { // In this case nothing is set, use predefined url
                    QString mess(
                        "ArchiveHTTP plugin -- no environment variable CAQTDM_ARCHIVERHTTP_URL "
                        "set and no archiverIndex defined as dynamic property in widget "
                        + w->objectName() + ", defaulting to " + index_name);
                    if (m_messageWindowP != (MessageWindow *) Q_NULLPTR) {
                        m_messageWindowP->postMsgEvent(QtWarningMsg, (char *) qasc(mess));
                    }
                }
            } else { // Environment variable is set but no dynamic property exists
                if (indexNew.init) {
                    QString mess("ArchiveHTTP plugin -- archiver URL defined as " + url
                                 + " from environment variable CAQTDM_ARCHIVERHTTP_URL");
                    if (m_messageWindowP != (MessageWindow *) Q_NULLPTR) {
                        m_messageWindowP->postMsgEvent(QtWarningMsg, (char *) qasc(mess));
                    }
                }
                index_name = url;
            }
        }
        WorkerHTTP *newWorker = new WorkerHTTP;
        WorkerHttpThread *newWorkerThread = new WorkerHttpThread(newWorker);
        m_listOfThreads.insert(i.key(), newWorkerThread);

        newWorker->moveToThread(newWorkerThread);

        connect(newWorkerThread, SIGNAL(finished()), newWorker, SLOT(workerFinish()));
        connect(newWorkerThread, SIGNAL(finished()), newWorkerThread, SLOT(deleteLater()));
        connect(this,
                SIGNAL(operate(QWidget *, indexes, QString, MessageWindow *, MutexKnobData *, QSharedPointer<HttpPerformanceData>)),
                newWorker,
                SLOT(getFromArchive(QWidget *, indexes, QString, MessageWindow *, MutexKnobData *, QSharedPointer<HttpPerformanceData>)));
        connect(newWorker,
                SIGNAL(resultReady(indexes, int, QVector<double>, QVector<double>, QVector<double>, QVector<double>, QString, bool)),
                this,
                SLOT(handleResults(indexes, int, QVector<double>, QVector<double>, QVector<double>, QVector<double>, QString, bool)));
        newWorkerThread->start();

        emit operate((QWidget *) m_messageWindowP, indexNew, index_name, m_messageWindowP, m_mutexKnobDataP, m_retrievalPerformancePerPV.value(i.key()));
        disconnect(newWorker);
        ++i;
    }
}

void ArchiveHTTP_Plugin::Callback_AbortOutstandingRequests(QString key)
{
    QMutexLocker mutexLocker(&m_globalMutex);
    m_IsSuspended = true;

    QMap<QString, WorkerHttpThread*>::iterator listOfThreadsEntry = m_listOfThreads.find(key);
    if (listOfThreadsEntry != m_listOfThreads.end()) {
        listOfThreadsEntry.value()->setIsActive(false);
        HttpRetrieval *retrieval = listOfThreadsEntry.value()->getHttpRetrieval();
        if (retrieval != Q_NULLPTR) {
            retrieval->cancelDownload();
        }
    }

    m_IsSuspended = false;
}

void ArchiveHTTP_Plugin::closeEvent()
{
    emit Signal_StopUpdateInterface();
}
