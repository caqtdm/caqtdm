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

#include "mutexKnobData.h"
#include <cadef.h>

#include <QObject>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QWidget>
#include <QDebug>
#include <QPair>
#include "QtControls"

/**
 * this routine (re)allocates memory and copies the old data to the new memory
 */
MutexKnobData::MutexKnobData()
{
    KnobDataArraySize=500;
    KnobData = (knobData*) malloc(KnobDataArraySize * sizeof(knobData));
    if (KnobData==Q_NULLPTR) {
        printf("caQtDM -- could not allocate memory -> exit\n");
        exit(1);
    }
    for(int i=0; i < KnobDataArraySize; i++){
        KnobData[i].index  = -1;
        KnobData[i].thisW = (void*) Q_NULLPTR;
        KnobData[i].mutex = (void*) Q_NULLPTR;
        KnobData[i].edata.dataB = (void*) Q_NULLPTR;
        KnobData[i].edata.dataPtr = (void*) Q_NULLPTR;
    }

    nbMonitorsPerSecond = 0;
    nbDisplayCountPerSecond = 0;
    nbMonitors = 0;
    displayCount = 0;
    highestCount = 0;
    highestIndex = 0;
    highestIndexPV = 0;
    highestCountPerSecond = 0;

    suppressUpdates = false;
    ftime(&last);
    ftime(&monitorTiming);

    // start a timer with 10Hz
    prvRepetitionRate = DEFAULTRATE;
    timerId = startTimer(1000/DEFAULTRATE);

    myUpdateType = UpdateTimed;

    // Initialize doDefaultUnitReplacements with env. var CAQTDM_DEFAULT_UNIT_REPLACEMENTS
    doDefaultUnitReplacements = true;
    if (qgetenv("CAQTDM_DEFAULT_UNIT_REPLACEMENTS").toLower().replace("\"","") == "false") doDefaultUnitReplacements = false;

    // Create unit replacement strings
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    QString defaultReplaceUnitString = QString( "muA=0x00b5,0x0041;"
                                               "uA=0x00b5,0x0041;"
                                               "?A=0x00b5,0x0041;"
                                               "muJ=0x00b5,0x004A;"
                                               "?J=0x00b5,0x004A;"
                                               "uJ=0x00b5,0x004A");
#else
    // Degree char coming from epics (0x00b0)    |||||    is replaced with two characters, one being the degree charachter itself (0x00b0), the other being an Â (0x00c2).
    // Without this Â char the degree character  ⌄⌄⌄⌄⌄  cannot be displayed correctly, the Â itself is not displayed. We don't know why though.
    QString defaultReplaceUnitString = QString( "0x00b0=0x00c2,0x00b0;"
                                                "0x00b5=0x00ce,0x00bc;"
                                                "muA=0x00ce,0x00bc,0x0041;"
                                                "uA=0x00ce,0x00bc,0x0041;"
                                                "?A=0x00ce,0x00bc,0x0041;"
                                                "muJ=0x00ce,0x00bc,0x004A;"
                                                "?J=0x00ce,0x00bc,0x004A;"
                                                "uJ=0x00ce,0x00bc,0x004A");
#endif
    // Convert the values from CAQTDM_CUSTOM_UNIT_REPLACEMENTS (from QString replaceUnits) in QStrings that can be inserted into a map to then easily replace all the strings.
    QStringList replaceUnitsList = createUnitReplacementList();

    // Initialize QMaps for default unit replacement and user defined unit replacement.
    defaultReplaceUnitsPairList = createUnitReplacementPairList(defaultReplaceUnitString.split(";"));
    replaceUnitsPairList = createUnitReplacementPairList(replaceUnitsList);
    //qDebug() << defaultReplaceUnitsPairList;
}

MutexKnobData:: ~MutexKnobData()
{
}

QStringList MutexKnobData::createUnitReplacementList()
{
    QString replaceUnits = (QString)  qgetenv("CAQTDM_CUSTOM_UNIT_REPLACEMENTS");
    replaceUnits = replaceUnits.trimmed().remove("\"");
    QStringList replaceUnitsList = replaceUnits.split(";");
    return replaceUnitsList;
}

QList<QPair<QString, QString> > MutexKnobData::createUnitReplacementPairList(QStringList replaceUnitsList)
{
    QList<QPair<QString, QString> > replaceUnitsPairList;
    if (replaceUnitsList.length() > 0) 
    {   
        QStringList::iterator replaceUnitsListIterator;
        for (replaceUnitsListIterator = replaceUnitsList.begin(); replaceUnitsListIterator != replaceUnitsList.end(); ++replaceUnitsListIterator){
            QStringList unitHalfs = replaceUnitsListIterator->split("=");
            if (unitHalfs.length()%2!=0) continue;

            QStringList unitKeyParts = unitHalfs[0].split(",");
            QStringList unitValueParts = unitHalfs[1].split(",");
            QString unitKey = "";
            QString unitValue = "";
            QStringList::iterator unitPartsIterator;
            for (unitPartsIterator = unitKeyParts.begin(); unitPartsIterator != unitKeyParts.end(); ++unitPartsIterator) {
                bool hexOk = true;
                bool decOk = true;
                quint16 parsedValueHex = unitPartsIterator->toInt(&hexOk, 16);
                quint16 parsedValueDez = unitPartsIterator->toInt(&decOk, 10);
                if (hexOk && unitPartsIterator->toLower().startsWith("0x")){
                    unitKey += QString(parsedValueHex);
                } else if (decOk) {
                    unitKey += QString(parsedValueDez);
                } else {
                    //qDebug() << "Argument from CAQTDM_CUSTOM_UNIT_REPLACEMENTS cannot be converted to UTF-8 Code, will be treated as string: " << QString(*unitPartsIterator);
                    unitKey += QString(*unitPartsIterator);
                }
            }
            for (unitPartsIterator = unitValueParts.begin(); unitPartsIterator != unitValueParts.end(); ++unitPartsIterator) {
                bool hexOk = true;
                bool decOk = true;
                quint16 parsedValueHex = unitPartsIterator->toInt(&hexOk, 16);
                quint16 parsedValueDez = unitPartsIterator->toInt(&decOk, 10);
                if (hexOk && unitPartsIterator->toLower().startsWith("0x")){
                    unitValue += QString(parsedValueHex);
                } else if (decOk) {
                    unitValue += QString(parsedValueDez);
                } else {
                    //qDebug() << "Argument from CAQTDM_CUSTOM_UNIT_REPLACEMENTS cannot be converted to UTF-8 Code, will be treated as string:  " << QString(*unitPartsIterator);
                    unitValue += QString(*unitPartsIterator);
                }
            }
            replaceUnitsPairList.append(QPair<QString, QString>(unitKey, unitValue));
        }
    }
    return replaceUnitsPairList;
    //qDebug() << "replaceUnitsMap: " << replaceUnitsMap;
}

/**
 * this routine (re)allocates memory and copies the old data to the new memory
 */
void MutexKnobData::ReAllocate(int oldsize, int newsize, void **ptr)
{
    void *tmp;
    //printf("reallocate for %d size\n", newsize);
    tmp = (void *) malloc((size_t) newsize);
    if (tmp==Q_NULLPTR) {
        printf("caQtDM -- could not allocate any more memory -> exit\n");
        exit (1);
    }
    if(oldsize > 0) {
        memcpy(tmp, *ptr, (size_t) oldsize);
        free(*ptr);
    }
    *ptr = tmp;
}

/**
 * this routine allows to change between direct updated and timed updates
 */

void MutexKnobData::UpdateMechanism(UpdateType Type)
{
    myUpdateType = Type;
}
/**
 * softpv naming
 */
QString MutexKnobData::SoftPV_Name(QString pv, QWidget *w)
{
    //printf("%s\n",asc.toUtf8().constData());
    //printf("%s_%p\n", qasc(pv),  w);
    //fflush(stdout);
    return QString("%1_%2").arg(pv).arg((quintptr)w,QT_POINTER_SIZE * 2, 16, QChar('0'));
}

bool MutexKnobData::getSuppressUpdates() const
{
    return suppressUpdates;
}

void MutexKnobData::setSuppressUpdates(bool newSuppressUpdates)
{
    suppressUpdates = newSuppressUpdates;
}

/**
 * insert the softpv into the list of softpv's with the corresponding widget
 */
void MutexKnobData::InsertSoftPV(QString pv, int num, QWidget *w)
{
    int indx;
    //char asc[MAXPVLEN+20];
    //sprintf(asc, "%s_%p", qasc(pv),  w);
    QMutexLocker locker(&mutex);
    QString asc=SoftPV_Name(pv, w);
    if(!getSoftPV(pv, &indx, (QWidget*) w)) {
        softPV_WidgetList.insert(asc, num);
        //qDebug() << "insert softpv_widgetList" << asc;
    }
}

/**
 * insert the softpv into the global list for keeping where they are used (using data pointer and main widget)
 */
void  MutexKnobData::BuildSoftPVList(QWidget *w)
{
    char asc[MAXPVLEN+20];
    softlist softstruct;
    QMutexLocker locker(&mutex);
    softPV_List.clear();
    // go through all our monitors
    for(int i=0; i < KnobDataArraySize; i++) {
        if(KnobData[i].index != -1 && KnobData[i].soft) {
            QWidget *w1 = (QWidget*) KnobData[i].thisW;
            // when the main widget corresponds keep it
            if(w == w1) {
                sprintf(asc, "%s_%d_%p", KnobData[i].pv, KnobData[i].index, w);
                softstruct.pv = QString(KnobData[i].pv);
                softstruct.index = KnobData[i].index;
                softstruct.w = w;
                softPV_List.insert(asc, softstruct);
                //qDebug() << "insert softpv_list" << asc << KnobData[i].dispName ;
            }
        }
        // for softpvs that were not yet known as soft pv, add them
        if(KnobData[i].index != -1) {
            int index;
            if(getSoftPV(KnobData[i].pv, &index, (QWidget *) KnobData[i].thisW)) {
                mutex.unlock();
                sprintf(asc, "%s_%d_%p", KnobData[i].pv, KnobData[i].index, w);
                softstruct.pv = QString(KnobData[i].pv);
                softstruct.index = KnobData[i].index;
                softstruct.w = w;
                softPV_List.insert(asc, softstruct);
                InsertSoftPV(KnobData[i].pv, KnobData[i].index, (QWidget *) KnobData[i].thisW);
                //qDebug() << "insert untill now unknown pv" << asc << KnobData[i].index;
                mutex.lock();
            }
        }
    }

/*
    QMapIterator<QString, int> i(softPV_List);
    int number = 0;
    while (i.hasNext()) {
        i.next();
        number++;
    }
    qDebug() << "buildsoftpvlist=" << number;
*/
}

/**
 * remove the softpv from both lists
 */
void MutexKnobData::RemoveSoftPV(QString pv, QWidget *w, int indx)
{
    //char asc[MAXPVLEN+20];
    QMutexLocker locker(&mutex);
    // remove from the softpv list
    //sprintf(asc, "%s_%p", qasc(pv),  w);
    QString asc=SoftPV_Name(pv, w);
    softPV_WidgetList.remove(asc);

    // and remove from the global list
    char asc1[MAXPVLEN+20];
    QWidget *w1 = (QWidget*) KnobData[indx].thisW;
    sprintf(asc1, "%s_%d_%p",  KnobData[indx].pv, KnobData[indx].index,  w1);
    softPV_List.remove(asc1);

/*
     QMapIterator<QString, int> i(softPV_List);
     qDebug() << "list start";
     while (i.hasNext()) {
         i.next();
         qDebug() <<  "list item=" << i.key() << i.value();
     }
     qDebug() << "list end";
*/
}

/**
 * update the data for the caCalc softpv
 */
void MutexKnobData::UpdateSoftPV(QString pv, double value, QWidget *w, int dataIndex, int dataCount)
{
    // update the right data
    //char asc[MAXPVLEN+20];
    //sprintf(asc, "%s_%p", qasc(pv),  w);
    QString asc=SoftPV_Name(pv, w);
    QMap<QString, int>::const_iterator name = softPV_WidgetList.find(asc);
    if(name != softPV_WidgetList.end()) {
        knobData *ptr = GetMutexKnobDataPtr(name.value());
        ptr->edata.fieldtype = caDOUBLE;
        ptr->edata.precision = 3;

        ptr->edata.upper_disp_limit=0.0;
        ptr->edata.lower_disp_limit=0.0;

        // single value
        if(dataCount <= 1) {
            //qDebug() << "updateSoftPV --  single" << ptr->index << "for name" << ptr->pv << "with value=" << value << "dataIndex=" << dataIndex << "dataCount=" << dataCount ;
            ptr->edata.rvalue = value;
            ptr->edata.connected = true;

        // waveform
        } else if(dataIndex < dataCount) {
            // initialize data to nan and update the correct index
            if((int) (dataCount * sizeof(double)) != ptr->edata.dataSize) {
                if(ptr->edata.dataB != (void*) Q_NULLPTR) free(ptr->edata.dataB);
                ptr->edata.dataB = (void*) malloc(dataCount * sizeof(double));
                double *data = (double *) ptr->edata.dataB;
                for(int i=0; i<dataCount; i++) data[i] = qQNaN();
            }
            ptr->edata.dataSize = dataCount * (int) sizeof(double);
            ptr->edata.valueCount = dataCount;
            //qDebug() << "updateSoftPV -- pv" << pv << "wave dataindex" << dataIndex << "with value " << value << dataCount;
            double *data = (double *) ptr->edata.dataB;
            data[dataIndex] = value;
        }
    }

    // and update everywhere where this soft channel is also used on this main window
    QMapIterator<QString, softlist> i(softPV_List);
    softlist softstruct;
    while (i.hasNext()) {
        i.next();
        softstruct = i.value();
        if(pv == softstruct.pv) {
            int indx = softstruct.index;
            if(KnobData[indx].index != -1 && KnobData[indx].pv == pv && softstruct.w == w) {
                //qDebug() <<  "     update index=" << softstruct.index << i.key() <<  w << "with" << value;

                // simple double
                if(dataCount <= 1) {
                    KnobData[indx].edata.rvalue = value;

                // waveform
                } else {
                    // allocate and initialize data to nan
                    if((int) (dataCount * sizeof(double)) !=  KnobData[indx].edata.dataSize) {
                        if( KnobData[indx].edata.dataB != (void*) Q_NULLPTR) free( KnobData[indx].edata.dataB);
                        KnobData[indx].edata.dataB = (void*) malloc(dataCount * sizeof(double));
                        double *data = (double *) KnobData[indx].edata.dataB;
                        for(int i=0; i<dataCount; i++) data[i] = qQNaN();
                    }
                    KnobData[indx].edata.dataSize = dataCount * sizeof(double);
                    KnobData[indx].edata.valueCount = dataCount;
                    KnobData[indx].edata.rvalue = value;
                }
                KnobData[indx].edata.fieldtype = caDOUBLE;
                KnobData[indx].edata.precision = 3;
                KnobData[indx].edata.connected = true;
                KnobData[indx].edata.upper_disp_limit=0.0;
                KnobData[indx].edata.lower_disp_limit=0.0;
                KnobData[indx].edata.connected = true;
            }
        }
    }

}

/**
 * get the index to the data for the softpv
 */
bool MutexKnobData::getSoftPV(QString pv, int *indx, QWidget *w)
{
    //char asc[MAXPVLEN+20];
    //sprintf(asc, "%s_%p", qasc(pv),  w);
    QString asc=SoftPV_Name(pv, w);
    QMap<QString, int>::const_iterator name = softPV_WidgetList.find(asc);
    if(name != softPV_WidgetList.end()) {
        *indx = name.value();
        return true;
    }
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
    QMutexLocker locker(&mutex);
    for(int i=0; i < KnobDataArraySize; i++) {
        if(KnobData[i].index == -1) {
            return i;
        }
    }
    oldsize=KnobDataArraySize;
    newsize = KnobDataArraySize+200;
    void *p = &KnobData;
    ReAllocate(oldsize * (int) sizeof(knobData), newsize * (int) sizeof(knobData), (void**) p);
    for(int i=oldsize; i < newsize; i++){
        KnobData[i].index  = -1;
    }
    KnobDataArraySize=newsize;
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
    if (KnobData&&(index<KnobDataArraySize))
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
knobData* MutexKnobData::getMutexKnobDataPV(QWidget *widget, QString pv)
{
    int loop = 0;
    QMutexLocker locker(&mutex);
    while (loop < 2) {

        for(int i=0; i < GetMutexKnobDataSize(); i++) {
            knobData *kPtr = (knobData*) &KnobData[i];
            if(kPtr->index != -1) {
                QWidget *w = (QWidget *) kPtr->dispW;
                QString kpv(kPtr->pv);

                if(loop == 1) {
                    if(kpv == pv) {
                         //qDebug() << pv << "not exact match for" << widget;
                        return kPtr;
                    }

                } else {
                    if(kpv == pv && widget == w) {
                        //qDebug() << pv << "exact match for" << widget;
                        return kPtr;
                    }
                }
            }
        }
    loop++;
    }
    return (knobData*) Q_NULLPTR;
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

void MutexKnobData::DataLock(knobData *kData)
{
    QMutex *datamutex;
    datamutex = (QMutex*) kData->mutex;
    datamutex->lock();
}

void MutexKnobData::DataUnlock(knobData *kData)
{
    QMutex *datamutex;
    datamutex = (QMutex*) kData->mutex;
    datamutex->unlock();
}

extern "C" MutexKnobData* C_DataLock(MutexKnobData* p, knobData *kData) {
    p->DataLock(kData);
    return p;
}
extern "C" MutexKnobData* C_DataUnlock(MutexKnobData* p, knobData *kData) {
    p->DataUnlock(kData);
    return p;
}

/**
 * update array with the received data
 */
void MutexKnobData::SetMutexKnobDataReceived(knobData *kData) {
    char units[40];
    char fec[40];
    char dataString[STRING_EXCHANGE_SIZE];
    double diff;
    struct timeb now;
    QMutexLocker locker(&mutex);
    int index = kData->index;
    memcpy(&KnobData[index].edata, &kData->edata, sizeof(epicsData));

    /*****************************************************************************************/
    // Statistics
    /*****************************************************************************************/

    nbMonitors++;

    // find monitor with highest count since last time
    if((kData->edata.monitorCount-kData->edata.monitorCountPrev) > highestCount) {
        highestCount = kData->edata.monitorCount -kData->edata.monitorCountPrev ;
        highestIndex = index;
    }

    // calculate after 5 seconds our statistics
    ftime(&now);
    diff = ((double) now.time + (double) now.millitm / (double)1000) -
            ((double) monitorTiming.time + (double) monitorTiming.millitm / (double)1000);

    if(diff >= 5.0) {
        ftime(&monitorTiming);
        nbMonitorsPerSecond = (int) (nbMonitors/diff);
        nbMonitors = 0;
        // remember monitor count for all monitors
        for(int i=0; i < GetMutexKnobDataSize(); i++) {
            knobData *kPtr = (knobData*) &KnobData[i];
            if(kPtr->index != -1) kPtr->edata.monitorCountPrev = kPtr->edata.monitorCount;
        }

        highestCountPerSecond = highestCount / (float) diff;
        highestIndexPV = highestIndex;
        highestCount = 0;
        nbDisplayCountPerSecond =  (int) (displayCount/diff);
        displayCount = 0;
    }

    /*****************************************************************************************/

    // direct update without timing

    if(myUpdateType == UpdateDirect) {
        if (!suppressUpdates ) {
            QWidget *dispW = (QWidget*) kData->dispW;
            dataString[0] = '\0';
            qstrncpy(units, kData->edata.units,caqtdm_string_t_length);
            qstrncpy(fec, kData->edata.fec,caqtdm_string_t_length);
            int caFieldType= kData->edata.fieldtype;
            if((caFieldType == DBF_STRING || caFieldType == DBF_ENUM || caFieldType == DBF_CHAR) && kData->edata.dataB != (void*) Q_NULLPTR) {
                if(kData->edata.dataSize < STRING_EXCHANGE_SIZE) {
                    memcpy(dataString, (char*) kData->edata.dataB, (size_t) kData->edata.dataSize);
                    dataString[kData->edata.dataSize] = '\0';
                } else {
                    memcpy(dataString, (char*) kData->edata.dataB, STRING_EXCHANGE_SIZE);
                    dataString[STRING_EXCHANGE_SIZE-1] = '\0';
                }
            }

            kData->edata.displayCount = kData->edata.monitorCount;
            locker.unlock();
            UpdateWidget(index, dispW, units, fec, dataString, KnobData[index]);
            kData->edata.lastTime = now;
            kData->edata.initialize = false;
            displayCount++;
        }
    }
}

int MutexKnobData::getMonitorsPerSecond()
{
    QMutexLocker locker(&mutex);
    return nbMonitorsPerSecond;
}

int MutexKnobData::getDisplaysPerSecond()
{
    QMutexLocker locker(&mutex);
    return nbDisplayCountPerSecond;
}

float MutexKnobData::getHighestCountPV(QString &pv)
{
    QMutexLocker locker(&mutex);

    if(KnobData[highestIndexPV].index != -1) {
        pv = KnobData[highestIndexPV].pv;
        return highestCountPerSecond;
    } else {
        return 0.0;
    }
}

void MutexKnobData::initHighestCountPV()
{
    QMutexLocker locker(&mutex);
    ftime(&monitorTiming);
    highestCount = 0;
}

extern "C" MutexKnobData* C_SetMutexKnobDataReceived(MutexKnobData* p, knobData *kData)
{
    p->SetMutexKnobDataReceived(kData);
    return p;
}
//*********************************************************************************************************************

/**
  * timer is running with default (5 Hz) speed
  */
void MutexKnobData::timerEvent(QTimerEvent *)
{
    if (suppressUpdates) {
        return;
    }
    double diff=0.2, repRate=5.0;
    char units[40];
    char fec[40];
    char dataString[STRING_EXCHANGE_SIZE];
    struct timeb now;
    int repetitionRate = DEFAULTRATE;

    ftime(&now);

    // do we have something that should go faster then 5 Hz, then change timer, but change back when nothing fast requested
    for(int i=0; i < GetMutexKnobDataSize(); i++) {
        knobData *kPtr = (knobData*) &KnobData[i];
        if(kPtr->index != -1) {
          if(kPtr->edata.repRate > repetitionRate) repetitionRate = kPtr->edata.repRate;
          if(repetitionRate > 50) repetitionRate = 50;  // not more than 50Hz
        }
    }
    if(repetitionRate != prvRepetitionRate) {
        killTimer(timerId);
        timerId = startTimer(1000/repetitionRate);
        //qDebug() << repetitionRate << prvRepetitionRate << 1000/repetitionRate << "ms";
        prvRepetitionRate = repetitionRate;
    }

    //int number = 0;
    //qDebug() << "============================================";
    for(int i=0; i < GetMutexKnobDataSize(); i++) {
        knobData *kPtr = (knobData*) &KnobData[i];

        if(kPtr->index != -1) {
            diff = ((double) now.time + (double) now.millitm / (double)1000) -
                    ((double) kPtr->edata.lastTime.time + (double) kPtr->edata.lastTime.millitm / (double)1000);
            if(kPtr->edata.repRate < 1) repRate = 1;
            else repRate = kPtr->edata.repRate;
        }

        // update all graphical items for this soft pv when a value changes

        if(kPtr->index != -1 && kPtr->soft && (diff >= (2.0/(double)repRate))) {

            int indx;

            //qDebug() << "I am a soft channel" << "pv=" << kPtr->pv << "index" << kPtr->index << "object" << kPtr->dispName << "value" << kPtr->edata.rvalue ;
            // get for this soft pv the index of the corresponding caCalc into the knobData array where the data were updated
            if(getSoftPV(kPtr->pv, &indx, (QWidget*) kPtr->thisW)) {

                // we do not update when softpv is hidden
                bool update = false;
                bool treatit = false;
                QWidget *w1 =  (QWidget*) kPtr->dispW;
                QString className = w1->metaObject()->className();
                if(className.contains("caStripPlot") || className.contains("caWaterfallPlot")) treatit = true;
                else if(w1->property("hidden").value<bool>()) treatit = false;
                else treatit = true;
                if(caCalc* calcWidget = qobject_cast<caCalc *>(w1)) {
                   if(calcWidget->getEventSignal() != caCalc::Never) treatit = true;
                }

                if(treatit) {
                    // get value from (updated) QMap variable list
                    knobData *ptr = (knobData*) &KnobData[indx];
                    kPtr->edata.fieldtype = caDOUBLE;
                    kPtr->edata.accessW = true;
                    kPtr->edata.accessR = true;

                    // when waveform put first value into the normal value
                    if(ptr->edata.valueCount > 0) {
                        double *data = (double *) ptr->edata.dataB;
                        kPtr->edata.rvalue = data[0];
                        kPtr->edata.monitorCount++;
                        memcpy(kPtr->edata.dataB,  data, kPtr->edata.valueCount * sizeof(double));
                    } else {
                        kPtr->edata.rvalue = ptr->edata.rvalue;
                        kPtr->edata.ivalue = (int) ptr->edata.rvalue;
                        if(kPtr->edata.oldsoftvalue != ptr->edata.rvalue) {
                            update = true;
                            //qDebug() << "update" << kPtr->pv << kPtr->dispName << "old value" << kPtr->edata.oldsoftvalue << "new value" << ptr->edata.rvalue;
                        }
                    }

                    kPtr->edata.connected = true;

                    // when no update then when any monitors for calculation increase monitorcount when underlying pv changes or when its calculates on itsself
                    QWidget *w1 =  (QWidget*) kPtr->dispW;
                    if((!update) && (ptr->edata.valueCount) == 0) {
                        QVariant var = w1->property("MonitorList");
                        QVariantList list = var.toList();
                        if((list.size() > 0)) {
                            int nbMonitors = list.at(0).toInt();
                            if(nbMonitors > 0)  update = true;
                        }
                    }

                    if(update) kPtr->edata.monitorCount++;
                    kPtr->edata.oldsoftvalue = ptr->edata.rvalue;
                    QWidget *ww = (QWidget *)kPtr->dispW;
                    if (caTextEntry *widget = qobject_cast<caTextEntry *>(ww)) {
                        widget->setAccessW((bool) kPtr->edata.accessW);
                    }
                }
            }
        }

        // use specified repetition rate (normally 5Hz)
        if( ((kPtr->index != -1) && (kPtr->edata.monitorCount > kPtr->edata.displayCount) && (diff >= (1.0/(double)repRate)))){
/*
            printf("<%s> index=%d mcount=%d dcount=%d value=%f ivalue=%d datasize=%d valuecount=%d\n", kPtr->pv, kPtr->index, kPtr->edata.monitorCount,
                                                                      kPtr->edata.displayCount, kPtr->edata.rvalue, kPtr->edata.ivalue,
                                                                      kPtr->edata.dataSize, kPtr->edata.valueCount);
*/
            if((myUpdateType == UpdateTimed) || kPtr->soft) {
                QMutexLocker locker(&mutex);
                int index = kPtr->index;
                QWidget *dispW = (QWidget*) kPtr->dispW;
                dataString[0] = '\0';
                qstrncpy(units, kPtr->edata.units,caqtdm_string_t_length);
                qstrncpy(fec, kPtr->edata.fec,caqtdm_string_t_length);
                int caFieldType= kPtr->edata.fieldtype;

                if((caFieldType == DBF_STRING || caFieldType == DBF_ENUM || caFieldType == DBF_CHAR) && kPtr->edata.dataB != (void*) Q_NULLPTR) {
                    if(kPtr->edata.dataSize < STRING_EXCHANGE_SIZE) {
                        memcpy(dataString, (char*) kPtr->edata.dataB, (size_t) kPtr->edata.dataSize);
                        dataString[kPtr->edata.dataSize] = '\0';
                    } else {
                        memcpy(dataString, (char*) kPtr->edata.dataB, STRING_EXCHANGE_SIZE);
                        dataString[STRING_EXCHANGE_SIZE-1] = '\0';
                    }
                }

                kPtr->edata.displayCount = kPtr->edata.monitorCount;
                locker.unlock();
                UpdateWidget(index, dispW, units, fec, dataString, KnobData[index]);
                kPtr->edata.lastTime = now;
                kPtr->edata.initialize = false;
                displayCount++;
            }

        } else if ((kPtr->index != -1)  && (diff >= (1.0/(double)repRate))) {
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
}

//*********************************************************************************************************************

/**
 * replace connected field
 */
void MutexKnobData::SetMutexKnobDataConnected(int index, int connected)
{
    QMutexLocker locker(&mutex);

    if( KnobData[index].index == -1) return;

    KnobData[index].edata.connected = connected;

#ifdef epics4
    connectInfoShort *tmp = (connectInfoShort *) KnobData[index].edata.info;
    if (tmp != (connectInfoShort *) Q_NULLPTR) tmp->connected = connected;
#endif

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

QString getBufferAsHexStr(char* buf, int buffsize) {
    QString result;
    for(int i = 0; i < buffsize; ++i)
        result += "0x" + QString("%1:").arg(static_cast<unsigned char>(buf[i]), 2, 16, QChar('0')).toUpper();
    result.chop(1);
    return result;
}
void MutexKnobData::UpdateWidget(int index, QWidget* w, char *units, char *fec, char *dataString, knobData knb)
{
    QString unitsString;

    // Check whether this is specifically accessing the .EGU epics field
    bool isEguField = QString(knb.pv).endsWith(".EGU");
    if (isEguField) {
        // If it is, the unit is stored in the dataString
        unitsString = QString::fromLatin1(dataString);
    } else {
        // If not, it is stored in the units string
        unitsString = QString::fromLatin1(units);
    }


    // Replace known sequences of characters which are meant to represent special characters
    if(unitsString.size() > 0) {
        // iterator for both loops
        QList<QPair<QString, QString> >::iterator i;

        if (doDefaultUnitReplacements){
            // replace default QStrings
            for (i = defaultReplaceUnitsPairList.begin(); i != defaultReplaceUnitsPairList.end(); ++i){
                unitsString.replace(i->first, i->second);
            }
        }

        // replace QStrings defined in CAQTDM_REPLACE_UNITS
        for (i = replaceUnitsPairList.begin(); i != replaceUnitsPairList.end(); ++i){
            unitsString.replace(i->first, i->second);
        }
    }

    // This just reinterprets it as utf8
    unitsString = QString::fromUtf8(qasc(unitsString));

    // Send updated data to main thread
    if (isEguField) {
        emit Signal_UpdateWidget(index, w, units, fec, unitsString, knb);
    } else {
        emit Signal_UpdateWidget(index, w, unitsString, fec, dataString, knb);
    }
}
void MutexKnobData::UpdateTextLine(char *message, char *name)
{
    QString String = QString::fromLatin1(message);
    emit Signal_QLineEdit(String, name);
}

extern "C" MutexKnobData* C_UpdateTextLine(MutexKnobData* p, char *message, char *name) // wrapper function
{
    p->UpdateTextLine(message, name);
    return p;
}



