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

#ifndef MUTEXKNOBDATA_H
#define MUTEXKNOBDATA_H

#include <caQtDM_Lib_global.h>

#include "dbrString.h"
#include "knobDefines.h"
#include <QMutex>
#include <QObject>
#include <QVector>
#include <QMap>
#include <QWaitCondition>
#include "knobData.h"

class CAQTDM_LIBSHARED_EXPORT MutexKnobData: public QObject {
    Q_OBJECT

public:
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
    knobData *getMutexKnobDataPV(QString pv);

    void timerEvent(QTimerEvent *);

    void SetMutexKnobDataConnected(int indx, int connected);

    void UpdateWidget(int indx, QWidget* w,  char* units, char* fec, char* statusString, knobData knb);
    void UpdateTextLine(char *message, char *name);

    void InsertSoftPV(QString pv, int num, QWidget* w);
    void RemoveSoftPV(QString pv, QWidget *w, int indx);
    void UpdateSoftPV(QString pv, double value, QWidget *w);
    bool getSoftPV(QString pv, int *indx, QWidget *w);
    void BuildSoftPVList(QWidget *w);

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
};
#endif // MUTEXKNOBDATA_H
