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
#include "QtControls"

MutexKnobData::MutexKnobData()
{
    KnobDataArraySize=500;
    KnobData = (knobData*) malloc(KnobDataArraySize * sizeof(knobData));
    for(int i=0; i < KnobDataArraySize; i++){
        KnobData[i].index  = -1;
        KnobData[i].thisW = (void*) 0;
    }

    // start a timer with 50Hz
    timerId = startTimer(20);
}


MutexKnobData:: ~MutexKnobData()
{

}

/**
 * this routine (re)allocates memory and copies the old data to the new memory
 */
void MutexKnobData::ReAllocate(int oldsize, int newsize, void **ptr)
{
    void *tmp;
    //printf("reallocate for %d size\n", newsize);
    tmp = (void *) malloc(newsize);
    if(oldsize > 0) {
        memcpy(tmp, *ptr, oldsize);
        free(*ptr);
    }
    *ptr = tmp;
}

/**
 * insert the softpv into the list
 */
void MutexKnobData::InsertSoftPV(QString pv, int num, QWidget *w)
{
    int indx;
    char asc[MAXPVLEN+20];
    sprintf(asc, "%s_%p", pv.toAscii().constData(),  w);
    if(!getSoftPV(pv, &indx, (QWidget*) w)) variableList.insert(asc, num);
}

/**
 * remove the softpv into the list
 */
void MutexKnobData::RemoveSoftPV(QString pv, QWidget *w)
{
    char asc[MAXPVLEN+20];
    sprintf(asc, "%s_%p", pv.toAscii().constData(),  w);
    variableList.remove(asc);
}

/**
 * update the data for the caCalc softpv
 */
void MutexKnobData::UpdateSoftPV(QString pv, double value, QWidget *w)
{
    char asc[MAXPVLEN+20];
    sprintf(asc, "%s_%p", pv.toAscii().constData(),  w);
    QMap<QString, int>::const_iterator name = variableList.find(asc);
    if(name != variableList.end()) {
        knobData *ptr = GetMutexKnobDataPtr(name.value());
        ptr->edata.rvalue = value;
        ptr->edata.fieldtype = caDOUBLE;
        ptr->edata.precision = 3;
        ptr->edata.connected = true;
        ptr->edata.upper_disp_limit=0.0;
        ptr->edata.lower_disp_limit=0.0;
    }
}

/**
 * get the index to the data for the softpv
 */
bool MutexKnobData::getSoftPV(QString pv, int *indx, QWidget *w)
{
    QMapIterator<QString, int> i(variableList);
/*
    qDebug() << "list start";
    while (i.hasNext()) {
        i.next();
        qDebug() <<  "list item=" << i.key();
    }
    qDebug() << "list end";
*/
    char asc[MAXPVLEN+20];
    sprintf(asc, "%s_%p", pv.toAscii().constData(),  w);
    QMap<QString, int>::const_iterator name = variableList.find(asc);
    if(name != variableList.end()) {
        //qDebug() << "found softpv" << pv << "list item=" << name.key();
        *indx = name.value();
        return true;
    }
    //qDebug() << "did not find (yet) softpv" << pv;
    return false;
}

/**
 * get a copy for a knob
 */
knobData MutexKnobData::GetMutexKnobData(int index)
{
    knobData kData;
    QMutexLocker locker(&mutex);

    memcpy(&kData, &KnobData[index], sizeof(knobData));
    memcpy(&kData.edata, &KnobData[index].edata, sizeof(epicsData));
    return kData;
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
    void *p = &KnobData;
    //ReAllocate(oldsize * sizeof(knobData), newsize * sizeof(knobData), (void**) &KnobData);
    ReAllocate(oldsize * sizeof(knobData), newsize * sizeof(knobData), (void**) p);
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
 * update array with the received data
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
  * timer is running with 50 ms speed
  */
void MutexKnobData::timerEvent(QTimerEvent *)
{
    double diff, repRate;
    char units[40];
    char fec[40];
    char dataString[1024];
    struct timeb now;
    ftime(&now);

    static int displayCount;
    static struct timeb last;

    for(int i=0; i < GetMutexKnobDataSize(); i++) {
        knobData *kPtr = (knobData*) &KnobData[i];

        if(kPtr->index != -1) {
           diff = ((double) now.time + (double) now.millitm / (double)1000) -
                  ((double) kPtr->edata.lastTime.time + (double) kPtr->edata.lastTime.millitm / (double)1000);
           if(kPtr->edata.repRate < 1) repRate = 1;
           else repRate = kPtr->edata.repRate;
        }
        if(kPtr->index != -1 && kPtr->soft && diff >= 0.1) {
            int indx;
             //qDebug() << "I am a soft channel" << kPtr->pv << kPtr->dispName << kPtr->edata.rvalue << kPtr->index;
            // get for this soft pv the index of the corresponding caCalc into the knobData array where the data were updated
            if(getSoftPV(kPtr->pv, &indx, (QWidget*) kPtr->thisW)) {
                // get value from (updated) QMap variable list
                knobData *ptr = (knobData*) &KnobData[indx];
                //qDebug() << "will be updated with value=" << ptr->edata.rvalue << "from" << ptr->pv << "index=" << ptr->index;
                kPtr->edata.rvalue = ptr->edata.rvalue;
                kPtr->edata.fieldtype = caDOUBLE;
                kPtr->edata.connected = true;
                kPtr->edata.accessW = true;
                kPtr->edata.accessR = true;
                //update also the count when value changed
                if(kPtr->edata.oldsoftvalue != ptr->edata.rvalue) {
                    //qDebug() << kPtr->pv << "will be updated with value=" << ptr->edata.rvalue << "from" << ptr->pv << "index=" << ptr->index << "oldvalue=" << kPtr->edata.oldsoftvalue;
                    kPtr->edata.monitorCount++;
                }
                kPtr->edata.oldsoftvalue = ptr->edata.rvalue;
                QWidget *ww = (QWidget *)kPtr->dispW;
                if (caTextEntry *widget = qobject_cast<caTextEntry *>(ww)) {
                    widget->setAccessW(kPtr->edata.accessW);
                }
            }
        }

        // use specified repetition rate (normally 5Hz)
        if( ((kPtr->index != -1) && (kPtr->edata.monitorCount > kPtr->edata.displayCount) && (diff >= (1.0/(double)repRate)))){
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
            int caFieldType= kPtr->edata.fieldtype;

            if((caFieldType == DBF_STRING || caFieldType == DBF_ENUM || caFieldType == DBF_CHAR) && kPtr->edata.dataB != (void*) 0) {
                if(kPtr->edata.dataSize < 1024) {
                    memcpy(dataString, (char*) kPtr->edata.dataB, kPtr->edata.dataSize);
                    dataString[kPtr->edata.dataSize] = '\0';
                }
            }

            kPtr->edata.displayCount = kPtr->edata.monitorCount;
            locker.unlock();
            UpdateWidget(index, dispW, units, fec, dataString, KnobData[index]);
            kPtr->edata.lastTime = now;
            kPtr->edata.initialize = false;
            displayCount++;

        } else if ((kPtr->index != -1)  && (diff >= (1.0/(double)repRate))){
            if( (!kPtr->edata.connected)) {
                QMutexLocker locker(&mutex);
                bool displayIt = false;
                units[0] = '\0';
                fec[0] = '\0';
                dataString[0] = '\0';
                int index = kPtr->index;
                // brake unconnected displays
                if(kPtr->edata.unconnectCount == 0) {
                  kPtr->edata.displayCount = kPtr->edata.monitorCount;
                  kPtr->edata.lastTime = now;
                  displayIt = true;
                }
                kPtr->edata.unconnectCount++;
                if(kPtr->edata.unconnectCount == 10) kPtr->edata.unconnectCount=0;
                locker.unlock();
                if(displayIt) UpdateWidget(index, (QWidget*) kPtr->dispW, units, fec, dataString, KnobData[index]);
            }
        }
    }

    ftime(&now);
    diff = ((double) now.time + (double) now.millitm / (double)1000) -
           ((double) last.time + (double) last.millitm / (double)1000);

    if(diff >= 10.0) {
        ftime(&last);
        //printf("displayed=%.1f/s %f\n", (displayCount/diff), diff);
        displayCount = 0;
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
        UpdateWidget(index, (QWidget*)KnobData[index].dispW, (char*) " ", (char*) " ",  (char*) " ", KnobData[index]);
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

void MutexKnobData::UpdateWidget(int index, QWidget* w,char *units, char *fec, char *dataString, knobData knb)
{
    //QString StringUnits = QString::fromLatin1(units);
    //StringUnits.remove(QRegExp("[\\n\\t\\r]"));
    emit Signal_UpdateWidget(index, w, units, fec, dataString, knb);
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


