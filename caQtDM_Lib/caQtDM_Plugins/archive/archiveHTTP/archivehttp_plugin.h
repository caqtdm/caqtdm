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
 *  Copyright (c) 2024
 *
 *  Author:
 *    Erik Schwarz
 *  Contact details:
 *    erik.schwarz@psi.ch
 */
#ifndef ARCHIVEHTTP_PLUGIN_H
#define ARCHIVEHTTP_PLUGIN_H

#include <QList>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QThread>
#include <QTimer>
#include <qwt.h>

#include "archiverCommon.h"
#include "controlsinterface.h"
#include "workerHttpThread.h"

class Q_DECL_EXPORT ArchiveHTTP_Plugin : public QObject, ControlsInterface
{
    Q_OBJECT

    Q_INTERFACES(ControlsInterface)
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "ch.psi.caqtdm.Plugin.ControlsInterface/1.0.democontrols")
#endif

    // Everything in public: is present in ControlsInterface
public:
    ArchiveHTTP_Plugin();
    ~ArchiveHTTP_Plugin();
    QString pluginName();
    int initCommunicationLayer(MutexKnobData *data,
                               MessageWindow *messageWindow,
                               QMap<QString, QString> options);
    int pvAddMonitor(int index, knobData *kData, int rate, int skip);
    int pvClearMonitor(knobData *kData);
    int pvFreeAllocatedData(knobData *kData);
    int pvSetValue(char *pv,
                   double rdata,
                   int32_t idata,
                   char *sdata,
                   char *object,
                   char *errmess,
                   int forceType);
    int pvSetWave(char *pv,
                  float *fdata,
                  double *ddata,
                  int16_t *data16,
                  int32_t *data32,
                  char *sdata,
                  int nelm,
                  char *object,
                  char *errmess);
    int pvGetTimeStamp(char *pv, char *timestamp);
    int pvGetDescription(char *pv, char *description);
    int pvClearEvent(void *ptr);
    int pvAddEvent(void *ptr);
    int pvReconnect(knobData *kData);
    int pvDisconnect(knobData *kData);
    int FlushIO();
    int TerminateIO();

    // From hereon downwards everything is new, so not available through ControlsInterface (without cast)
public slots:
    void handleResults(indexes, int, QVector<double>, QVector<double>, QString);

signals:
    void operate(QWidget *, const indexes, const QString, MessageWindow *);
    void Signal_StopUpdateInterface();

private slots:
    void Callback_UpdateInterface(QMap<QString, indexes> listOfIndexes);
    void Callback_AbortOutstandingRequests(QString key);
    void closeEvent();

private:
    QMutex mutex;
    MutexKnobData *mutexknobdataP;
    MessageWindow *messagewindowP;
    ArchiverCommon *archiverCommon;
    QMap<QString, WorkerHttpThread *> listOfThreads;
    bool suspend;
};

#endif
