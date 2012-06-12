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

#include "dbrString.h"
#include "knobDefines.h"
#include <QMutex>
#include <QObject>
#include <QVector>
#include <QWaitCondition>
#include "knobData.h"

#define USETIMER

class MutexKnobData: public QObject {
    Q_OBJECT

public:
    MutexKnobData();

    ~MutexKnobData();

    void ReAllocate(int Indx, int size, void **ptr);

    knobData GetMutexKnobData(int indx);
    knobData *GetMutexKnobDataPtr(int indx);
    void SetMutexKnobData(int indx, knobData data);
    int GetMutexKnobDataIndex();
    int GetMutexKnobDataSize();
    void SetMutexKnobDataReceived(knobData *kData);
    knobData *getMutexKnobDataPV(QString pv);

#ifdef USETIMER
    void timerEvent(QTimerEvent *);
#endif

    void SetMutexKnobDataConnected(int indx, int connected);

    void UpdateWidget(int indx, QWidget* w,  char* units, char* fec,
                      char* statusString, knobData knb, QVector<double> y);
    void UpdateTextLine(char *message, char *name);

signals:

    void Signal_UpdateWidget(int, QWidget*, const QString&, const QString&, const QString&, knobData, const QVector<double> &);
    void Signal_QLineEdit(const QString&, const QString&);

private:
    QMutex mutex;
    knobData *KnobData;
    int KnobDataArraySize;
    int timerId;
};
#endif // MUTEXKNOBDATA_H
