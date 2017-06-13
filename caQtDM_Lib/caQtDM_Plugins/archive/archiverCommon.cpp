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
#include <QDebug>
#include <QThread>
#include <QApplication>
#include "archiverCommon.h"

// constructor
ArchiverCommon::ArchiverCommon()
{
    qDebug() << "ArchivePlugin: Create";
    mutexP = new QMutex;
}

void ArchiverCommon::stopUpdateInterface()
{
    timer->stop();
    //qDebug() << "timer stop";
    QApplication::processEvents();
}

void ArchiverCommon::updateInterface()
{
    double diff;
    struct timeb now;
    QMap<QString, indexes> listOfIndexesToBeExecuted;
    listOfIndexesToBeExecuted.clear();

    //qDebug() << "ArchiverCommon::updateInterface";

    QMutexLocker locker(&mutex);

    // after first start, set timer to wanted period
    if(!timerRunning) {
        timer->stop();
        timer->start(1000);
        timerRunning = true;
    }

    // copy indexes to be executed when it is time
    ftime(&now);

    QMap<QString, indexes>::const_iterator i = listOfIndexes.constBegin();
    while (i != listOfIndexes.constEnd()) {
        indexes indexNew = i.value();

        diff = ((double) now.time + (double) now.millitm / (double)1000) -
               ((double) indexNew.lastUpdateTime.time + (double) indexNew.lastUpdateTime.millitm / (double)1000);
        // is it time to update ?
        if(diff >= indexNew.updateSeconds) {
            ftime(&indexNew.lastUpdateTime);
            listOfIndexes.insert(i.key(), indexNew);
            listOfIndexesToBeExecuted.insert(i.key(), indexNew);
        }
        ++i;
    }
    //qDebug() << "number of indexes to execute" << listOfIndexesToBeExecuted.count();

    // call user routine for updating data
    if(listOfIndexesToBeExecuted.count() > 0) {
        emit Signal_UpdateInterface(listOfIndexesToBeExecuted);

        // and set the init field of index to false
        QMap<QString, indexes>::const_iterator i = listOfIndexes.constBegin();
        while (i != listOfIndexes.constEnd()) {
            indexes indexNew = i.value();
            if(indexNew.init) {
               indexNew.init = false;
               listOfIndexes.insert(i.key(), indexNew);
            }
             ++i;
        }
    }
}

// initialize our communicationlayer with everything you need
int ArchiverCommon::initCommunicationLayer(MutexKnobData *data, MessageWindow *messageWindow, QMap<QString, QString> options)
{
    Q_UNUSED(options);
    qDebug() << "ArchivePlugin: InitCommunicationLayer with options" << options;
    mutexknobdataP = data;
    messagewindowP = messageWindow;
    timerRunning = false;

    // start a timer in order to update the interface at the specified rate
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateInterface()));

    timer->start(100);

    return true;
}

// caQtDM_Lib will call this routine for defining a monitor
int ArchiverCommon::pvAddMonitor(int index, knobData *kData, int rate, int skip) {
    Q_UNUSED(index);
    Q_UNUSED(rate);
    Q_UNUSED(skip);

    QMutexLocker locker(&mutex);

    //qDebug() << "ArchivePlugin:pvAddMonitor" << kData->pv << kData->index << kData->dispName;

    if(caCartesianPlot* w = qobject_cast<caCartesianPlot *>((QWidget*) kData->dispW)) {

        char asc[CHAR_ARRAY_LENGTH];
        indexes index;

        sprintf(asc, "%d_%s_%p",kData->specData[0], kData->pv, kData->dispW);
        QString key = QString(asc);
        key = key.replace(".X", "");
        key = key.replace(".Y", "");

        QVariant var = w->property("secondsPast");
        if(!var.isNull()) {
            index.secondsPast = var.toInt();
        } else {
            QString mess("Archive plugin -- no secondsPast defined as dynamic property in widget " + QString(kData->dispName) + ", default to 1 hour back");
            if(messagewindowP != (MessageWindow *) 0 && !QString(kData->pv).contains(".Y")) messagewindowP->postMsgEvent(QtWarningMsg, (char*) qasc(mess));
            index.secondsPast = 3600;
        }

        var = w->property("secondsUpdate");
        if(!var.isNull()) {
            index.updateSeconds = var.toInt();
        } else{
            QString mess("Archive plugin -- no secondsUpdate defined as dynamic property in widget " + QString(kData->dispName) + ", default to 10 seconds update");
            if(messagewindowP != (MessageWindow *) 0 && !QString(kData->pv).contains(".Y")) messagewindowP->postMsgEvent(QtWarningMsg, (char*) qasc(mess));
            index.updateSeconds = 10;
        }

        index.init = true;
        index.key = key;
        index.mutexP = mutexP;
        index.pv = QString(kData->pv);
        index.pv = index.pv.replace(".X", "");
        index.pv = index.pv.replace(".Y", "");
        index.w = (QWidget*) kData->dispW;
        index.indexX = index.indexY = 0;
        if(kData->specData[2] == caCartesianPlot::CH_X) index.indexX = kData->index;        // x
        else if(kData->specData[2] == caCartesianPlot::CH_Y) index.indexY = kData->index;   // y

        if(!listOfIndexes.contains(key)) {
            listOfIndexes.insert(key, index);
        } else {
            QMap<QString, indexes>::iterator i = listOfIndexes.find(key);
            while (i !=listOfIndexes.end() && i.key() == key) {
                indexes indexNew = i.value();
                if(kData->specData[2] == caCartesianPlot::CH_X) indexNew.indexX = kData->index;
                else if(kData->specData[2] == caCartesianPlot::CH_Y) indexNew.indexY = kData->index;
                //qDebug() << "indexes x and y" << indexNew.indexX << indexNew.indexY;
                indexNew.lastUpdateTime.time = 0;
                listOfIndexes.insert(key, indexNew);
                break;
            }
        }

    } else {
        QString mess("archivedata can only be used in a cartesianplot");
        if(messagewindowP != (MessageWindow *) 0) messagewindowP->postMsgEvent(QtDebugMsg, (char*) qasc(mess));
    }

    return true;
}

void ArchiverCommon::updateCartesian(int nbVal, indexes indexNew, QVector<double> TimerN, QVector<double> YValsN, QString backend)
{
    QMutexLocker locker(&mutex);
    //qDebug() << "ArchiverCommon::updateCartesian";
    if(nbVal > 0) {
        knobData kData = mutexknobdataP->GetMutexKnobData(indexNew.indexX);
        if(kData.index == -1) return;
        mutexknobdataP->DataLock(&kData);
        kData.edata.fieldtype = caDOUBLE;
        kData.edata.connected = true;
        kData.edata.accessR = kData.edata.accessW = true;
        kData.edata.monitorCount++;
        strcpy(kData.edata.fec, qasc(backend));

        if((nbVal * sizeof(double)) > (size_t) kData.edata.dataSize) {
            if(kData.edata.dataB != (void*) 0) free(kData.edata.dataB);
            kData.edata.dataB = (void*) malloc(nbVal * sizeof(double));
            kData.edata.dataSize = nbVal * sizeof(double);
        }
        memcpy(kData.edata.dataB, &TimerN[0],  nbVal * sizeof(double));
        kData.edata.valueCount = nbVal;
        mutexknobdataP->SetMutexKnobDataReceived(&kData);
        mutexknobdataP->DataUnlock(&kData);

        kData = mutexknobdataP->GetMutexKnobData(indexNew.indexY);
        if(kData.index == -1) return;
        mutexknobdataP->DataLock(&kData);
        kData.edata.fieldtype = caDOUBLE;
        kData.edata.connected = true;
        kData.edata.accessR = kData.edata.accessW = true;
        kData.edata.monitorCount++;
        strcpy(kData.edata.fec, qasc(backend));

        if((nbVal * sizeof(double)) > (size_t) kData.edata.dataSize) {
            if(kData.edata.dataB != (void*) 0) free(kData.edata.dataB);
            kData.edata.dataB = (void*) malloc(nbVal * sizeof(double));
            kData.edata.dataSize = nbVal * sizeof(double);
        }
        memcpy(kData.edata.dataB, &YValsN[0],  nbVal * sizeof(double));

        kData.edata.valueCount = nbVal;
        mutexknobdataP->SetMutexKnobDataReceived(&kData);
        mutexknobdataP->DataUnlock(&kData);
    }
}

// caQtDM_Lib will call this routine for getting rid of a monitor
int ArchiverCommon::pvClearMonitor(knobData *kData) {


    if (kData->index == -1) return true;
    //qDebug() << "clearmonitor" << kData->index << kData->pv;

    if(caCartesianPlot* w = qobject_cast<caCartesianPlot *>((QWidget*) kData->dispW)) {
        Q_UNUSED(w);
        char asc[CHAR_ARRAY_LENGTH];
        sprintf(asc, "%d_%s_%p",kData->specData[0], kData->pv, kData->dispW);
        QString key = QString(asc);
        key = key.replace(".X", "");
        key = key.replace(".Y", "");

        // already removed ?
        bool found = false;
        QMap<QString, indexes>::iterator i = listOfIndexes.find(key);
        while (i !=listOfIndexes.end() && i.key() == key) {
            found = true;
            ++i;
        }

        if(found) {
            QList<QString> removeKeys;
            removeKeys.clear();

            QMap<QString, indexes>::iterator i = listOfIndexes.find(key);
            while (i !=listOfIndexes.end() && i.key() == key) {
                removeKeys.append(key);
                ++i;
            }

            for(int i=0; i< removeKeys.count(); i++) {
                listOfIndexes.remove(removeKeys.at(i));
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
    if (kData->edata.info != (void *) 0) {
        free(kData->edata.info);
        kData->edata.info = (void*) 0;
    }
    if(kData->edata.dataB != (void*) 0) {
        free(kData->edata.dataB);
        kData->edata.dataB = (void*) 0;
    }
    return true;
}



