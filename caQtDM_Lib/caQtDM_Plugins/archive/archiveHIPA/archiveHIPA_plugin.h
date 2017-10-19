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
#ifndef ArchiveHIPA_Plugin_H
#define ArchiveHIPA_Plugin_H

#include <QObject>
#include <QMap>
#include <QMutex>
#include <QList>
#include <QTimer>
#include <QThread>
#include <qwt.h>
#include "cacartesianplot.h"
#include "controlsinterface.h"
#include "archiveHIPA_plugin.h"
#include "archiverCommon.h"
#include "hipaRetrieval.h"

class Q_DECL_EXPORT WorkerHIPA : public QObject
{
    Q_OBJECT

public:
    WorkerHIPA() {
        qRegisterMetaType<indexes>("indexes");
        qRegisterMetaType<QVector<double> >("QVector<double>");
    }

private:
    QVector<double>  TimerN, YValsN;

public slots:

    void workerFinish() {
        //qDebug() << "worker finish";
        deleteLater();
    }

    void getFromArchive(QWidget *w, indexes indexNew) {

        Q_UNUSED(w);

        QMutex *mutex = indexNew.mutexP;
        mutex->lock();

        QString key = indexNew.pv;

        int nbVal = 0;

        char dev[40];
        float *Timer, *YVals;

        QString loggingServer = (QString)  qgetenv("LOGGINGSERVER");
        loggingServer = loggingServer.toUpper();
        if(loggingServer.isEmpty() || !loggingServer.contains("HIPA")) setenv("LOGGINGSERVER", "hipa-lgexp.psi.ch", 1);

        //qDebug() << "get from archive at " << "hipa-lgexp.psi.ch";
        int startHours = indexNew.secondsPast / 3600;
        int day =  startHours/24 + 1;
        int arraySize =  3600/5 * 24 * (day+1);

        Timer = (float*) malloc(arraySize * sizeof(float));
        YVals = (float*) malloc(arraySize * sizeof(float));

        strcpy(dev, qasc(key));
        int ret = GetLogShift(indexNew.secondsPast, dev, &nbVal, Timer, YVals);
        if(!ret) nbVal = 0;

        // resize arrays
        TimerN.clear();
        YValsN.clear();
        TimerN.resize(nbVal);
        YValsN.resize(nbVal);

        int k=0;
        for(int j=0; j<nbVal; j++) {
            if(Timer[j] >= -startHours) {
                TimerN[k] = Timer[j];
                YValsN[k] = YVals[j];
                ++k;
            }
        }
        nbVal = k;
        free(Timer);
        free(YVals);

        //qDebug() << ">>>> hipa nbval=" << nbVal << TimerN.count() << k;

        emit resultReady(indexNew, nbVal, TimerN, YValsN, "");
        mutex->unlock();
    }

signals:
    void resultReady(indexes indexNew, int nbVal, QVector<double> TimerN, QVector<double> YValsN, QString backend);

public:

};


class Q_DECL_EXPORT ArchiveHIPA_Plugin : public QObject, ControlsInterface
{
    Q_OBJECT

    Q_INTERFACES(ControlsInterface)
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "ch.psi.caqtdm.Plugin.ControlsInterface/1.0.democontrols")
#endif

public:
    QString pluginName();
    ArchiveHIPA_Plugin();

    int initCommunicationLayer(MutexKnobData *data, MessageWindow *messageWindow, QMap<QString, QString> options);
    int pvAddMonitor(int index, knobData *kData, int rate, int skip);
    int pvClearMonitor(knobData *kData);
    int pvFreeAllocatedData(knobData *kData);
    int pvSetValue(char *pv, double rdata, int32_t idata, char *sdata, char *object, char *errmess, int forceType);
    int pvSetWave(char *pv, float *fdata, double *ddata, int16_t *data16, int32_t *data32, char *sdata, int nelm, char *object, char *errmess);
    int pvGetTimeStamp(char *pv, char *timestamp);
    int pvGetDescription(char *pv, char *description);
    int pvClearEvent(void * ptr);
    int pvAddEvent(void * ptr);
    int pvReconnect(knobData *kData);
    int pvDisconnect(knobData *kData);
    int FlushIO();
    int TerminateIO();

public slots:
    void handleResults(indexes, int, QVector<double>, QVector<double>, QString);
    void closeEvent();

signals:
    void operate(QWidget*, const indexes);
    void Signal_StopUpdateInterface();

private slots:
    void updateValues();
    void Callback_UpdateInterface( QMap<QString, indexes> listOfIndexes);

private:
    QMutex mutex;
    MutexKnobData *mutexknobdataP;
    MessageWindow *messagewindowP;
    ArchiverCommon *archiverCommon;
    QMap<QString, QThread*> listOfThreads;
};

#endif
