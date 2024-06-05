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
 *  Copyright (c) 2010 - 2014
 *
 *  Author:
 *    Anton Mezger
 *  Contact details:
 *    anton.mezger@psi.ch
 */
#include "archiverCommon.h"
#include <QApplication>
#include <QDebug>
#include <QThread>

#define SECONDSSLEEP 3600   // 1 hour
#define SECONDSTIMEOUT 60.5 // 1 minute

// constructor
ArchiverCommon::ArchiverCommon()
{
    //QDebug() << "ArchivePlugin: Create";
    mutexP = new QMutex;
}

ArchiverCommon::~ArchiverCommon(){
    delete mutexP;
    delete timer;
}

void ArchiverCommon::stopUpdateInterface()
{
    timer->stop();
    //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << "timer stop";
    QApplication::processEvents();
}

QMutex* ArchiverCommon::globalMutex()
{
    // This mutex is used for other parts of caQtDM which asynchronously want to process data that is also update in here
    return &m_globalMutex;
}

void ArchiverCommon::updateInterface()
{
    double diff;
    struct timeb now;
    QMap<QString, indexes> listOfIndexesToBeExecuted;
    listOfIndexesToBeExecuted.clear();

    QMutexLocker locker(&m_globalMutex);

    // after first start, set timer to wanted period
    if (!timerRunning) {
        timer->stop();
        timer->start(1000);
        timerRunning = true;
    }

    // copy indexes to be executed when it is time
    ftime(&now);

    QMap<QString, indexes>::const_iterator i = listOfIndexes.constBegin();
    while (i != listOfIndexes.constEnd()) {
        indexes indexNew = i.value();
        diff = ((double) now.time + (double) now.millitm / (double) 1000)
               - ((double) indexNew.lastUpdateTime.time
                  + (double) indexNew.lastUpdateTime.millitm / (double) 1000);
        // is it time to update ?
        //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << i.key() << diff << indexNew.updateSeconds;
        if (diff >= indexNew.updateSeconds) {
            ftime(&indexNew.lastUpdateTime);
            listOfIndexes.insert(i.key(), indexNew);
            listOfIndexesToBeExecuted.insert(i.key(), indexNew);
        }
        ++i;
    }
    //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << "number of indexes to execute" << listOfIndexesToBeExecuted.count();

    // call user routine for updating data
    if (listOfIndexesToBeExecuted.count() > 0) {
        emit Signal_UpdateInterface(listOfIndexesToBeExecuted);

        // and set the init field of index to false
        QMap<QString, indexes>::const_iterator i = listOfIndexes.constBegin();
        while (i != listOfIndexes.constEnd()) {
            indexes indexNew = i.value();
            if (indexNew.init) {
                indexNew.init = false;
                listOfIndexes.insert(i.key(), indexNew);
            }
            ++i;
        }
    }
}

// initialize our communicationlayer with everything you need
int ArchiverCommon::initCommunicationLayer(MutexKnobData *data,
                                           MessageWindow *messageWindow,
                                           QMap<QString, QString> options)
{
    Q_UNUSED(options);
    //QDebug() << "ArchivePlugin: InitCommunicationLayer with options" << options;
    mutexknobdataP = data;
    messagewindowP = messageWindow;
    timerRunning = false;

    // start a timer in order to update the interface at the specified rate
    timer = new QTimer((QObject*)this);
    connect(timer, SIGNAL(timeout()), (QObject*)this, SLOT(updateInterface()));

    timer->start(100);

    return true;
}

// caQtDM_Lib will call this routine for defining a monitor
int ArchiverCommon::pvAddMonitor(int index, knobData *kData, int rate, int skip)
{
    Q_UNUSED(index);
    Q_UNUSED(rate);
    Q_UNUSED(skip);

    QMutexLocker locker(&m_globalMutex);

    //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << "ArchivePlugin:pvAddMonitor" << kData->pv << kData->index << kData->dispName;

    if (caCartesianPlot *w = qobject_cast<caCartesianPlot *>((QWidget *) kData->dispW)) {
        indexes index;

        // Generate key to distinguish curves with the same pv but from different curves or plots.
        QString key = QString("%1_%2_%3").arg(kData->specData[0]).arg(kData->pv).arg(reinterpret_cast<quintptr>(kData->dispW), sizeof(void*) * 2, 16, QChar('0'));

        // We need to construct new, temporary QString objects, else we modify the actual string, which is unintended
        QString possibleXKeyForMinY = QString(key).replace(".minY", "");
        QString possibleXKeyForMaxY = QString(key).replace(".maxY", "");
        key = key.replace(".X", "");
        key = key.replace(".Y", "");

        QVariant var = w->property("secondsPast");
        if (!var.isNull()) {
            bool ok;
            index.secondsPast = var.toInt(&ok);
            if (!ok) {
                index.secondsPast = 3600;
            }
        } else {
            QString mess("Archive plugin -- no secondsPast defined as dynamic property in widget "
                         + QString(kData->dispName) + ", default to 1 hour back");
            if (messagewindowP != (MessageWindow *) Q_NULLPTR && !QString(kData->pv).contains(".Y")) {
                messagewindowP->postMsgEvent(QtWarningMsg, (char *) qasc(mess));
            }
            index.secondsPast = 3600;
        }

        var = w->property("secondsUpdate");
        if (!var.isNull()) {
            bool ok;
            index.updateSeconds = var.toInt(&ok);
            if (!ok) {
                index.updateSeconds = SECONDSTIMEOUT;
            }

            // override the user specification if too many data are going to be requested
            if (index.secondsPast > 7200) {
                index.updateSeconds = 60;
            } else if (index.secondsPast > 3600) {
                index.updateSeconds = 30;
            }

        } else {
            QString mess("Archive plugin -- no secondsUpdate defined as dynamic property in widget "
                         + QString(kData->dispName) + ", default to 60 seconds update");
            if (messagewindowP != (MessageWindow *) Q_NULLPTR && !QString(kData->pv).contains(".Y"))
                messagewindowP->postMsgEvent(QtWarningMsg, (char *) qasc(mess));
            index.updateSeconds = SECONDSTIMEOUT;
        }
        if (index.updateSeconds < 10)
            index.updateSeconds = 10;
        index.updateSecondsOrig = index.updateSeconds;

        index.init = true;
        index.key = key;
        index.mutexP = mutexP;
        index.pv = QString(kData->pv);
        index.pv = index.pv.replace(".X", "");
        index.pv = index.pv.replace(".Y", "");
        index.w = (QWidget *) kData->dispW;
        index.indexX = index.indexY = 0;
        if (kData->specData[2] == caCartesianPlot::CH_X) {
            index.indexX = kData->index; // x
        } else if (kData->specData[2] == caCartesianPlot::CH_Y) {
            index.indexY = kData->index; // y
        }

        if (caCartesianPlot *ww = qobject_cast<caCartesianPlot *>((QWidget *) index.w)) {
            if (ww->getXaxisType() == caCartesianPlot::time) {
                index.timeAxis = true;
            } else {
                index.timeAxis = false;
            }
        }
        if (!alreadyProcessedIndexes.contains(key) && !alreadyProcessedIndexes.contains(possibleXKeyForMinY) && !alreadyProcessedIndexes.contains(possibleXKeyForMaxY)) {
            alreadyProcessedIndexes.insert(key, index);
        } else if (!listOfIndexes.contains(key) && !listOfIndexes.contains(possibleXKeyForMinY) && !listOfIndexes.contains(possibleXKeyForMaxY)) {
            QMap<QString, indexes>::iterator i;
            QVector<QMap<QString, indexes>::iterator > listOfIterators;
            listOfIterators.append(alreadyProcessedIndexes.find(key));
            listOfIterators.append(alreadyProcessedIndexes.find(possibleXKeyForMinY));
            listOfIterators.append(alreadyProcessedIndexes.find(possibleXKeyForMaxY));
            // Iterate through all possible forms of an already processed index to find indexX or indexY, depending on what we already have.
            for (int j = 0; j < listOfIterators.count(); j++) {
                i = listOfIterators[j];
                while (i != alreadyProcessedIndexes.end()) {
                    indexes indexNew = i.value();
                    // Get the already processed Index and extend it by the current index for this axis.
                    // So if we currently have the Y axis, then the already processed Index must contain the X axis
                    // and should therefore be extended by the index for the Y axis.
                    if (kData->specData[2] == caCartesianPlot::CH_Y) {
                        indexNew.indexY = kData->index;
                    } else if (kData->specData[2] == caCartesianPlot::CH_X) { // This case usually doesn't happen, as X axis is typically processed first and therefore in alreadyProcessedIndexes.
                        indexNew.indexX = kData->index;
                    }
                    if (kData->edata.info != (void *) Q_NULLPTR) {
                        free(kData->edata.info);
                    }
                    kData->edata.info = (char *) malloc(key.length());
                    qstrncpy((char*)kData->edata.info, qasc(key), key.length());
                    indexNew.lastUpdateTime.time = 0;
                    // Make sure that the key of indexNew contains ".minY" or ".maxY", if it exists in the current index
                    if (key.contains(".maxY") || key.contains(".minY")) {
                        indexNew.key = key;
                    }
                    listOfIndexes.insert(key, indexNew);
                    // This list is only used to find all indexes that have to be removed again upon reload
                    alreadyProcessedIndexes.insert(key, indexNew);
                    break;
                }
            }
        }

    } else {
        QString mess("archivedata can only be used in a cartesianplot");
        if (messagewindowP != (MessageWindow *) Q_NULLPTR) {
            messagewindowP->postMsgEvent(QtDebugMsg, (char *) qasc(mess));
        }
    }
    return true;
}

void ArchiverCommon::updateSecondsPast(indexes indexNew, bool original)
{
    QMutexLocker locker(&m_globalMutex);
    QString key = indexNew.key;
    QMap<QString, indexes>::iterator i = listOfIndexes.find(key);
    while (i != listOfIndexes.end() && i.key() == key) {
        indexes indexNew = i.value();
        if (original) {
            if (indexNew.updateSeconds != indexNew.updateSecondsOrig) {
                //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << "resume original timing " << indexNew.updateSecondsOrig << " for" << indexNew.pv;
                indexNew.updateSeconds = indexNew.updateSecondsOrig;
                listOfIndexes.insert(key, indexNew);
            }
        } else if (indexNew.updateSeconds < SECONDSTIMEOUT) {
            //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << "set new timing " << SECONDSTIMEOUT << " for" << indexNew.pv;
            indexNew.updateSeconds = SECONDSTIMEOUT;
            ftime(&indexNew.lastUpdateTime);
            listOfIndexes.insert(key, indexNew);
        }
        break;
    }
}

void ArchiverCommon::updateCartesian(
    int nbVal, indexes indexNew, QVector<double> XValsN, QVector<double> YValsN, QString backend)
{
    QMutexLocker locker(&m_globalMutex);
    //qDebug() << (__FILE__) << ":" << (__LINE__) << "|" << "ArchiverCommon::updateCartesian";
    if (nbVal > 0) {
        knobData kData = mutexknobdataP->GetMutexKnobData(indexNew.indexX);
        if (kData.index == -1) {
            return;
        }
        mutexknobdataP->DataLock(&kData);
        kData.edata.fieldtype = caDOUBLE;
        kData.edata.connected = true;
        kData.edata.accessR = kData.edata.accessW = true;
        kData.edata.monitorCount++;
        strcpy(kData.edata.fec, qasc(backend));

        if ((nbVal * sizeof(double)) > (size_t) kData.edata.dataSize) {
            if (kData.edata.dataB != (void *) Q_NULLPTR) {
                free(kData.edata.dataB);
            }
            kData.edata.dataB = (void *) malloc(nbVal * sizeof(double));
            kData.edata.dataSize = nbVal * sizeof(double);
        }
        memcpy(kData.edata.dataB, &XValsN[0], nbVal * sizeof(double));
        kData.edata.valueCount = nbVal;
        mutexknobdataP->SetMutexKnobDataReceived(&kData);
        mutexknobdataP->DataUnlock(&kData);
        kData = mutexknobdataP->GetMutexKnobData(indexNew.indexY);
        if (kData.index == -1) {
            return;
        }
        mutexknobdataP->DataLock(&kData);
        kData.edata.fieldtype = caDOUBLE;
        kData.edata.connected = true;
        kData.edata.accessR = kData.edata.accessW = true;
        kData.edata.monitorCount++;
        strcpy(kData.edata.fec, qasc(backend));

        if ((nbVal * sizeof(double)) > (size_t) kData.edata.dataSize) {
            if (kData.edata.dataB != (void *) Q_NULLPTR) {
                free(kData.edata.dataB);
            }
            kData.edata.dataB = (void *) malloc(nbVal * sizeof(double));
            kData.edata.dataSize = nbVal * sizeof(double);
        }
        memcpy(kData.edata.dataB, &YValsN[0], nbVal * sizeof(double));
        kData.edata.valueCount = nbVal;
        mutexknobdataP->SetMutexKnobDataReceived(&kData);
        mutexknobdataP->DataUnlock(&kData);
    }
}

// caQtDM_Lib will call this routine for getting rid of a monitor
int ArchiverCommon::pvClearMonitor(knobData *kData)
{
    if (kData->index == -1) {
        return true;
    }
    QMutexLocker locker(&m_globalMutex);

    if (caCartesianPlot *w = qobject_cast<caCartesianPlot *>((QWidget *) kData->dispW)) {
        // generate key used to distinguish knobDatas with the same pv but different widgets / curves
        QString key = QString("%1_%2_%3").arg(kData->specData[0]).arg(kData->pv).arg(reinterpret_cast<quintptr>(kData->dispW), sizeof(void*) * 2, 16, QChar('0'));
        QString possibleXKeyForMinY = QString(key).replace(".minY", "");
        QString possibleXKeyForMaxY = QString(key).replace(".maxY", "");
        key = key.replace(".X", "");
        key = key.replace(".Y", "");


        QMap<QString, indexes>::iterator i;
        QVector<QMap<QString, indexes>::iterator > listOfIterators;
        listOfIterators.append(alreadyProcessedIndexes.find(key));
        listOfIterators.append(alreadyProcessedIndexes.find(possibleXKeyForMinY));
        listOfIterators.append(alreadyProcessedIndexes.find(possibleXKeyForMaxY));

        // find keys to be removed
        QList<QString> removeKeys;
        removeKeys.clear();
        bool found = false;
        for (int j = 0; j < listOfIterators.count(); j++) {
            i = listOfIterators[j];
            while (i != alreadyProcessedIndexes.end()) {
                found = true;
                removeKeys.append(i.key());
                ++i;
            }
        }
        if (found) {
            for (int i = 0; i < removeKeys.count(); i++) {
                listOfIndexes.remove(removeKeys.at(i));
                alreadyProcessedIndexes.remove(removeKeys.at(i));
            }
            emit Signal_AbortOutstandingRequests(key);
        }
    }

    pvFreeAllocatedData(kData);
    kData->index = -1;
    return true;
}

int ArchiverCommon::pvFreeAllocatedData(knobData *kData)
{
    // Lock the data mutex first, so we don't free data that is still in use
    mutexknobdataP->DataLock(kData);
    if (kData->edata.info != (void *) Q_NULLPTR) {
        free(kData->edata.info);
        kData->edata.info = (void *) Q_NULLPTR;
    }
    if (kData->edata.dataB != (void *) Q_NULLPTR) {
        free(kData->edata.dataB);
        kData->edata.dataB = (void *) Q_NULLPTR;
    }
    mutexknobdataP->DataUnlock(kData);
    return true;
}

int ArchiverCommon::pvClearEvent(void *ptr)
{
    char asc[CHAR_ARRAY_LENGTH];
    //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << "clear event" << ptr;

    QMutexLocker locker(&m_globalMutex);

    memcpy(asc, ptr, sizeof(asc));
    QString key = QString(asc);

    QMap<QString, indexes>::iterator i = listOfIndexes.find(key);
    while (i != listOfIndexes.end() && i.key() == key) {
        indexes indexNew = i.value();
        if (indexNew.updateSeconds != SECONDSSLEEP) {
            //QDebug() << "archiverCommon.cpp:354 " << "update" << indexNew.pv << "to " << SECONDSSLEEP << "seconds";
            indexNew.updateSeconds = SECONDSSLEEP;
            ftime(&indexNew.lastUpdateTime);
            listOfIndexes.insert(key, indexNew);
        }
        break;
    }

    return true;
}

int ArchiverCommon::pvAddEvent(void *ptr)
{
    char asc[CHAR_ARRAY_LENGTH];
    //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << "add event" << ptr;
    QMutexLocker locker(&m_globalMutex);

    memcpy(asc, ptr, sizeof(asc));
    QString key = QString(asc);

    QMap<QString, indexes>::iterator i = listOfIndexes.find(key);
    while (i != listOfIndexes.end() && i.key() == key) {
        indexes indexNew = i.value();
        if (indexNew.updateSeconds != indexNew.updateSecondsOrig) {
            if (indexNew.updateSeconds != SECONDSTIMEOUT) {
                //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << "update" << indexNew.pv << "to" << indexNew.updateSecondsOrig << "seconds";
                indexNew.updateSeconds = indexNew.updateSecondsOrig;
                listOfIndexes.insert(key, indexNew);
            }
        }
        break;
    }
    return true;
}
