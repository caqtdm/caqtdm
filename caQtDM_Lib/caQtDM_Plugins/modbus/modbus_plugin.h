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
 *  Copyright (c) 2010 - 2020
 *
 *  Author:
 *    Anton Mezger
 *  Contact details:
 *    anton.mezger@psi.ch
 */
#ifndef MODBUSPLUGIN_H
#define MODBUSPLUGIN_H

#ifdef _MSC_VER
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#endif
#include <QObject>
#include <QMap>
#include <QMutex>
#include <QList>
#include <QTimer>
#include "controlsinterface.h"
#include "modbus_decode.h"

class Q_DECL_EXPORT modbusPlugin : public QObject, ControlsInterface
{
    Q_OBJECT
    Q_INTERFACES(ControlsInterface)
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "ch.psi.caqtdm.Plugin.ControlsInterface/1.0.modbus")
#endif

public:
    QString pluginName();
    modbusPlugin();
    ~modbusPlugin();


    int initCommunicationLayer(MutexKnobData *data, MessageWindow *messageWindow,QMap<QString, QString> options);

    QString removeEPICSExtensions(QString pv);
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

private slots:
    void closeEvent();
signals:
    void closeSignal();

private:
    QMutex mutex;
    MutexKnobData *mutexknobdataP;
    MessageWindow *messagewindowP;
    QMap<QString, QString> optionsP;
    double initValue;
    //QTimer *timer, *timerValues;

    QMap<QString,QPointer<modbus_decode>> modbusconnections;
    QList<QThread*> modbusThreads;
    QMap<QString, QString> modbus_translation_map;
};

#endif
