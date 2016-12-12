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
#include "archiverCommon.h"

// constructor
ArchiverCommon::ArchiverCommon()
{
    qDebug() << "ArchivePlugin: Create";
}

void ArchiverCommon::updateInterface()
{
    QMutexLocker locker(&mutex);

    // after first start, set timer to wanted period
    if(!timerRunning) {
        timer->stop();
        timer->start(10000);
        timerRunning = true;
    }

    // call user routine for updating data
    emit Signal_UpdateInterface(listOfIndexes);
}

// initialize our communicationlayer with everything you need
int ArchiverCommon::initCommunicationLayer(MutexKnobData *data, MessageWindow *messageWindow, QMap<QString, QString> options)
{
    //qDebug() << "ArchivePlugin: InitCommunicationLayer with options" << options;
    mutexknobdataP = data;
    messagewindowP = messageWindow;
    timerRunning = false;

    // we want to update the interface every 10 seconds
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateInterface()));

    timer->start(1000);

    return true;
}

// caQtDM_Lib will call this routine for defining a monitor
int ArchiverCommon::pvAddMonitor(int index, knobData *kData, int rate, int skip) {
    Q_UNUSED(index);
    Q_UNUSED(rate);
    Q_UNUSED(skip);
    //QString archive;
    int secondsPast;

    QMutexLocker locker(&mutex);

    //qDebug() << "ArchivePlugin:pvAddMonitor" << kData->pv << kData->index << kData->dispName;

    if(caCartesianPlot* w = qobject_cast<caCartesianPlot *>((QWidget*) kData->dispW)) {
        char asc[50];
        sprintf(asc, "%s_%p", kData->pv, kData->dispW);
        QString key = QString(asc);
        key = key.replace(".X", "");
        key = key.replace(".Y", "");

        QVariant var = w->property("secondspast");

        if(!var.isNull()) {
            secondsPast = var.toInt();
        } else {
            messagewindowP->postMsgEvent(QtDebugMsg, (char*) "no secondspast defined as dynamic property in widget, default to 1 hour");
            secondsPast = 3600;
        }

        indexes index;
        index.pv = QString(kData->pv);
        index.pv = index.pv.replace(".X", "");
        index.pv = index.pv.replace(".Y", "");
        index.indexX = index.indexY = 0;
        if(kData->specData[2] == caCartesianPlot::CH_X) index.indexX = kData->index;        // x
        else if(kData->specData[2] == caCartesianPlot::CH_Y) index.indexY = kData->index;   // y

        index.secondspast = secondsPast;
        if(!listOfIndexes.contains(key)) {
            listOfIndexes.insert(key, index);
        } else {
            QMap<QString, indexes>::iterator i = listOfIndexes.find(key);
            while (i !=listOfIndexes.end() && i.key() == key) {
                indexes indexNew = i.value();
                if(kData->specData[2] == caCartesianPlot::CH_X) indexNew.indexX = kData->index;
                else if(kData->specData[2] == caCartesianPlot::CH_Y) indexNew.indexY = kData->index;
                //qDebug() << indexNew.indexX << indexNew.indexY;
                listOfIndexes.insert(key, indexNew);
                break;
            }
        }

    } else {
        messagewindowP->postMsgEvent(QtDebugMsg, "archivedata can only be used in a cartesianplot");
    }

    return true;
}

void ArchiverCommon::updateCartesian(int nbVal, indexes indexNew, QVector<double> TimerN, QVector<double> YValsN)
{
    if(nbVal > 0) {
        //qDebug() << "update plot" << indexNew.indexX <<indexNew.indexY << mutexknobdataP;
        knobData* kData = mutexknobdataP->GetMutexKnobDataPtr(indexNew.indexX);
        //qDebug() << indexNew.indexX;
        if((kData != (knobData *) 0) && (kData->index != -1)) {
            mutexknobdataP->DataLock(kData);
            kData->edata.fieldtype = caDOUBLE;
            kData->edata.connected = true;
            kData->edata.accessR = kData->edata.accessW = true;
            kData->edata.monitorCount++;

            if((nbVal * sizeof(double)) > (size_t) kData->edata.dataSize) {
                if(kData->edata.dataB != (void*) 0) free(kData->edata.dataB);
                kData->edata.dataB = (void*) malloc(nbVal * sizeof(double));
                kData->edata.dataSize = nbVal * sizeof(double);

                memcpy(kData->edata.dataB, &TimerN[0],  nbVal * sizeof(double));
            }
            kData->edata.valueCount = nbVal;

            mutexknobdataP->SetMutexKnobData(kData->index, *kData);
            mutexknobdataP->DataUnlock(kData);
        }

        kData = mutexknobdataP->GetMutexKnobDataPtr(indexNew.indexY);
        //qDebug() << indexNew.indexY;
        if((kData != (knobData *) 0) && (kData->index != -1)) {
            mutexknobdataP->DataLock(kData);
            kData->edata.fieldtype = caDOUBLE;
            kData->edata.connected = true;
            kData->edata.accessR = kData->edata.accessW = true;
            kData->edata.monitorCount++;


            if((nbVal * sizeof(double)) > (size_t) kData->edata.dataSize) {
                if(kData->edata.dataB != (void*) 0) free(kData->edata.dataB);
                kData->edata.dataB = (void*) malloc(nbVal * sizeof(double));
                kData->edata.dataSize = nbVal * sizeof(double);
            }
            memcpy(kData->edata.dataB, &YValsN[0],  nbVal * sizeof(double));

            kData->edata.valueCount = nbVal;

            mutexknobdataP->SetMutexKnobDataReceived(kData);
            mutexknobdataP->DataUnlock(kData);
        }
    }
}

// caQtDM_Lib will call this routine for getting rid of a monitor
int ArchiverCommon::pvClearMonitor(knobData *kData) {
    QMutexLocker locker(&mutex);

    if (kData->index == -1) return true;

    if(caCartesianPlot* w = qobject_cast<caCartesianPlot *>((QWidget*) kData->dispW)) {
        Q_UNUSED(w);
        char asc[50];
        sprintf(asc, "%s_%p", kData->pv, kData->dispW);
        QString key = QString(asc);
        key = key.replace(".X", "");
        key = key.replace(".Y", "");

        //qDebug() << "ArchivePlugin:pvClearMonitor" << key;

        QMap<QString, indexes>::iterator i = listOfIndexes.find(key);
        while (i !=listOfIndexes.end() && i.key() == key) {
            //indexes indexNew = i.value();
            //qDebug() << indexNew.indexX << indexNew.indexY;
            listOfIndexes.remove(key);
            ++i;
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



