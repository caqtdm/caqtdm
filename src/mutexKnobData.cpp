//******************************************************************************
// Copyright (c) 2012 Paul Scherrer Institut PSI), Villigen, Switzerland
// Disclaimer: neither  PSI, nor any of their employees makes any warranty
// or assumes any legal liability or responsibility for the use of this software
//******************************************************************************
//******************************************************************************
//
//     Author : Anton Chr. Mezger
//
//******************************************************************************

#include "mutexKnobData.h"
#include <cadef.h>

#include <QObject>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QWidget>
#include <QDebug>

MutexKnobData::MutexKnobData()
{
    KnobDataArraySize=500;
    KnobData = (knobData*) malloc(KnobDataArraySize * sizeof(knobData));
    for(int i=0; i < KnobDataArraySize; i++){
        KnobData[i].index  = -1;
    }
#ifdef USETIMER
    // start a timer with 5Hz
    timerId = startTimer(200);
#endif
}


MutexKnobData:: ~MutexKnobData()
{
        free (KnobData);
}

/**
 * this routine (re)allocates memory and copies the old data to the new memory
 */
void MutexKnobData::ReAllocate(int oldsize, int newsize, void **ptr)
{
    void *tmp;

    tmp = (void *) malloc(newsize);
    if(oldsize > 0) {
        memcpy(tmp, *ptr, oldsize);
        free(*ptr);
    }
    *ptr = tmp;
}


/**
 * get a copy for a knob
 */
knobData MutexKnobData::GetMutexKnobData(int indx)
{
    QMutexLocker locker(&mutex);
    return KnobData[indx];
}

extern "C" MutexKnobData* C_GetMutexKnobData(MutexKnobData* p, int indx, knobData *data)
{
    *data = p->GetMutexKnobData(indx);
    return p;
}
//*********************************************************************************************************************

/**
 * get a slot for a knob
 */
int MutexKnobData::GetMutexKnobDataIndex()
{
    int oldsize, newsize;
    for(int i=0; i < KnobDataArraySize; i++) {
        if(KnobData[i].index == -1) {
            return i;
        }
    }
    oldsize=KnobDataArraySize;
    KnobDataArraySize+=200;
    newsize = KnobDataArraySize;
    ReAllocate(oldsize * sizeof(knobData), newsize * sizeof(knobData), (void**) &(KnobData));
    for(int i=oldsize; i < newsize; i++){
        KnobData[i].index  = -1;
    }
    return oldsize;
}
//*********************************************************************************************************************

/**
 * get size of knob array
 */
int MutexKnobData::GetMutexKnobDataSize()
{
    return KnobDataArraySize;
}
//*********************************************************************************************************************

/**
 * replace in array a knob
 */
void MutexKnobData::SetMutexKnobData(int index, knobData data)
{
    QMutexLocker locker(&mutex);
    memcpy(&KnobData[index], &data, sizeof(knobData));
    KnobData[index].index = index;
}

extern "C" MutexKnobData* C_SetMutexKnobData(MutexKnobData* p, int index, knobData data)
{
    p->SetMutexKnobData(index, data);
    return p;
}
//*********************************************************************************************************************

/**
 * get all data for a knob given by the pv
 */

knobData* MutexKnobData::getMutexKnobDataPV(QString pv)
{
     QMutexLocker locker(&mutex);
    for(int i=0; i < GetMutexKnobDataSize(); i++) {
        knobData *kPtr = (knobData*) &KnobData[i];
        if(kPtr->index != -1) {
            QString kpv(kPtr->pv);
            if(kpv == pv) {
                return kPtr;
            }
        }
    }
    return (knobData*) 0;
}
//*********************************************************************************************************************

/**
 * get pointer to the knob data
 */
knobData* MutexKnobData::GetMutexKnobDataPtr(int index)
{
    QMutexLocker locker(&mutex);
    return (knobData*) &KnobData[index];
}
//*********************************************************************************************************************

/**
 * update received data
 */
void MutexKnobData::SetMutexKnobDataReceived(knobData *kData){
    QVector<double> y;
    QMutexLocker locker(&mutex);
    int index = kData->index;
    memcpy(&KnobData[index], kData, sizeof(kData));
    memcpy(&KnobData[index].edata, &kData->edata, sizeof(epicsData));
}

extern "C" MutexKnobData* C_SetMutexKnobDataReceived(MutexKnobData* p, knobData *kData)
{
    p->SetMutexKnobDataReceived(kData);
    return p;
}
//*********************************************************************************************************************

/**
  * timer is running with 250 ms speed
  */
void MutexKnobData::timerEvent(QTimerEvent *)
{
    char units[40];
    char fec[40];
    char dataString[1024];
    QVector<double> y;
    struct timeb now;
    ftime(&now);

    for(int i=0; i < GetMutexKnobDataSize(); i++) {
        knobData *kPtr = (knobData*) &KnobData[i];


        if( ((kPtr->index != -1) && (kPtr->edata.monitorCount > kPtr->edata.displayCount))){
/*
            printf("<%s> index=%d mcount=%d dcount=%d value=%f datasize=%d valuecount=%d\n", kPtr->pv, kPtr->index, kPtr->edata.monitorCount,
                                                                      kPtr->edata.displayCount, kPtr->edata.rvalue,
                                                                      kPtr->edata.dataSize, kPtr->edata.valueCount);

*/
            QMutexLocker locker(&mutex);
            int index = kPtr->index;
            QWidget *dispW = (QWidget*) kPtr->dispW;
            dataString[0] = '\0';
            strcpy(units, kPtr->edata.units);
            strcpy(fec, kPtr->edata.fec);
            int valueCount= kPtr->edata.valueCount;
            int caFieldType= kPtr->edata.fieldtype;
            y.clear();

            if((caFieldType == DBF_STRING || caFieldType == DBF_ENUM || caFieldType == DBF_CHAR) && kPtr->edata.dataB != (void*) 0) {
                memcpy(dataString, (char*) kPtr->edata.dataB, kPtr->edata.dataSize);
                dataString[kPtr->edata.dataSize] = '\0';

            } else if(kPtr->edata.dataB != (void*) 0) {
                if(valueCount > 0) {
                    y.reserve(valueCount);
                    switch(caFieldType) {
                    case DBF_FLOAT: {
                        float* P = (float*) kPtr->edata.dataB;
                        for(int i=0; i< valueCount; i++) y.append(P[i]);
                    }
                    break;

                    case DBF_DOUBLE:
                    {
                        double* P = (double*) kPtr->edata.dataB;
                        for(int i=0; i< valueCount; i++) y.append(P[i]);
                    }
                    break;

                    case DBF_LONG:
                    {
                        long* P = (long*) kPtr->edata.dataB;
                        for(int i=0; i< valueCount; i++) y.append(P[i]);
                    }
                    break;

                    case DBF_INT:
                    {
                        int* P = (int*) kPtr->edata.dataB;
                        for(int i=0; i< valueCount; i++) y.append(P[i]);
                    }
                    break;
                    }
                }
            }

            kPtr->edata.displayCount = kPtr->edata.monitorCount;
            locker.unlock();

            UpdateWidget(index, dispW, units, fec, dataString, KnobData[index], y);
            kPtr->edata.initialize = false;

        } else if (kPtr->index != -1) {
            if( (!kPtr->edata.connected)) {
                units[0] = '\0';
                fec[0] = '\0';
                dataString[0] = '\0';
                //qDebug() << kPtr->pv << "not connected" << (QWidget*) kPtr->dispW;
                int index = kPtr->index;
                y.clear();
                kPtr->edata.displayCount = kPtr->edata.monitorCount;
                UpdateWidget(index, (QWidget*) kPtr->dispW, units, fec, dataString, KnobData[index], y);
            }
        }
    }

}

//*********************************************************************************************************************

/**
 * replace connected field
 */
void MutexKnobData::SetMutexKnobDataConnected(int index, int connected)
{
    QVector<double> y;
    y.clear();
    QMutexLocker locker(&mutex);
    KnobData[index].edata.connected = connected;

    if(!connected) {
        UpdateWidget(index, (QWidget*)KnobData[index].dispW, (char*) " ", (char*) " ",  (char*) " ", KnobData[index], y);
    }

}
extern "C" MutexKnobData* C_SetMutexKnobDataConnected(MutexKnobData* p, int index, int connected)
{
    p->SetMutexKnobDataConnected(index,connected);
    return p;
}
//*********************************************************************************************************************

/**
 * update display data
 */

void MutexKnobData::UpdateWidget(int index, QWidget* w,char *units, char *fec,
                                 char *dataString, knobData knb, QVector<double> y)
{
    //QString StringUnits = QString::fromLatin1(units);
    //StringUnits.remove(QRegExp("[\\n\\t\\r]"));
    emit Signal_UpdateWidget(index, w, units, fec, dataString, knb, y);
}
//*********************************************************************************************************************

void MutexKnobData::UpdateTextLine(char *message, char *name)
{
    QString String = QString::fromLatin1(message);
    //String.remove(QRegExp("[\\n\\t\\r]"));
    emit Signal_QLineEdit(String, name);
}

extern "C" MutexKnobData* C_UpdateTextLine(MutexKnobData* p, char *message, char *name) // wrapper function
{
    p->UpdateTextLine(message, name);
    return p;
}


