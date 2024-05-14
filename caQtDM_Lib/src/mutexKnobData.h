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

#if defined(_MSC_VER)
 //to avoid macro redefinition
 #define _MATH_DEFINES_DEFINED
 #include <math.h>
#endif


#include "caQtDM_Lib_global.h"

#include "dbrString.h"
#include "knobDefines.h"
#include <QMutex>
#include <QObject>
#include <QVector>
#include <QMap>
#include <QPair>
#include <QWaitCondition>
#include "knobData.h"
#include "mutexKnobDataWrapper.h"

#define DEFAULTRATE 10

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
    void UpdateSoftPV(QString pv, double value, QWidget *w, int dataIndex, int dataCount);
    bool getSoftPV(QString pv, int *indx, QWidget *w);
    void BuildSoftPVList(QWidget *w);

    int getMonitorsPerSecond();
    int getDisplaysPerSecond();
    float getHighestCountPV(QString &pv);
    void initHighestCountPV();

    void UpdateMechanism(UpdateType Type);
    QString SoftPV_Name(QString pv, QWidget *w);

    bool getSuppressUpdates() const;
    void setSuppressUpdates(bool newSuppressUpdates);

signals:

    void Signal_UpdateWidget(int, QWidget*, const QString&, const QString&, const QString&, const knobData&);
    void Signal_QLineEdit(const QString&, const QString&);

private:

    typedef struct _softlist {
        QString pv;                       /* device process variable name */
        int  index;
       QWidget *w;
    } softlist;

    QMutex mutex;
    knobData *KnobData;
    int KnobDataArraySize;
    int timerId, prvRepetitionRate;
    QMap<QString, int> softPV_WidgetList;
    QMap<QString, softlist> softPV_List;

    int nbMonitorsPerSecond, nbMonitors;
    int highestCount, highestIndex, highestIndexPV;
    float highestCountPerSecond;
    struct timeb monitorTiming;

    int nbDisplayCountPerSecond, displayCount;
    struct timeb last;

    bool suppressUpdates;
    UpdateType myUpdateType;

    bool doDefaultUnitReplacements;
    QList<QPair<QString,QString> > createUnitReplacementPairList(QStringList replaceUnitsList);
    QList<QPair<QString,QString> > defaultReplaceUnitsPairList;
    QList<QPair<QString,QString> > replaceUnitsPairList;
    QStringList createUnitReplacementList();
};
#endif // MUTEXKNOBDATA_H
