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

#ifndef MUTEXKNOBDATA_H
#define MUTEXKNOBDATA_H


#include "caQtDM_Lib_global.h"

#include "dbrString.h"
#include "knobDefines.h"
#include <QMutex>
#include <QObject>
#include <QVector>
#include <QMap>
#include <QWaitCondition>
#include "knobData.h"
#include "mutexKnobDataWrapper.h"



class CAQTDM_LIBSHARED_EXPORT MutexKnobData: public QObject {
    Q_OBJECT

public:

    enum UpdateType {UpdateTimed=0, UpdateDirect};

    MutexKnobData();

    ~MutexKnobData();

    void ReAllocate(int Indx, int size, void **ptr);

    void DataLock( knobData *kData);
    void DataUnlock(knobData *kData);

    knobData GetMutexKnobData(int indx);
    knobData *GetMutexKnobDataPtr(int indx);
    void SetMutexKnobData(int indx, knobData data);
    int GetMutexKnobDataIndex();
    int GetMutexKnobDataSize();
    void SetMutexKnobDataReceived(knobData *kData);
    knobData *getMutexKnobDataPV(QWidget *widget, QString pv);

    void timerEvent(QTimerEvent *);

    void SetMutexKnobDataConnected(int indx, int connected);

    void UpdateWidget(int indx, QWidget* w,  char* units, char* fec, char* statusString, knobData knb);
    void UpdateTextLine(char *message, char *name);

    void InsertSoftPV(QString pv, int num, QWidget* w);
    void RemoveSoftPV(QString pv, QWidget *w, int indx);
    void UpdateSoftPV(QString pv, double value, QWidget *w);
    bool getSoftPV(QString pv, int *indx, QWidget *w);
    void BuildSoftPVList(QWidget *w);

    int getMonitorsPerSecond();
    int getDisplaysPerSecond();
    float getHighestCountPV(QString &pv);
    void initHighestCountPV();

    void UpdateMechanism(UpdateType Type);

    void BlockProcessing(bool block) { blockProcess= block;}

signals:

    void Signal_UpdateWidget(int, QWidget*, const QString&, const QString&, const QString&, const knobData&);
    void Signal_QLineEdit(const QString&, const QString&);

private:

    QMutex mutex;
    knobData *KnobData;
    int KnobDataArraySize;
    int timerId;
    QMap<QString, int> softPV_WidgetList;
    QMap<QString, int> softPV_List;

    int nbMonitorsPerSecond, nbMonitors;
    int highestCount, highestIndex, highestIndexPV;
    float highestCountPerSecond;
    struct timeb monitorTiming;

    int nbDisplayCountPerSecond, displayCount;
    struct timeb last;

    bool blockProcess;

    UpdateType myUpdateType;
};
#endif // MUTEXKNOBDATA_H
