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
 *  Copyright (c) 2010 - 2015
 *
 *  Author:
 *    Anton Mezger
 *  Contact details:
 *    anton.mezger@psi.ch
 */
#ifndef CONTROLSINTERFACE_H
#define CONTROLSINTERFACE_H
#include <QString>
#include <QtPlugin>
#include "MessageWindow.h"
#include "mutexKnobData.h"
#include <stdint.h>

class ControlsInterface

{
public:
    virtual ~ControlsInterface() {}
    virtual QString pluginName() = 0;
    virtual int initCommunicationLayer(MutexKnobData *data,
                                       MessageWindow *messageWindow,
                                       QMap<QString, QString> options)
        = 0;
    virtual int pvAddMonitor(int index, knobData *kData, int rate, int skip) = 0; // skip is not used
    virtual int pvClearMonitor(knobData *kData) = 0;
    virtual int pvFreeAllocatedData(knobData *kData) = 0;

    virtual int pvSetValue(char *pv,
                           double rdata,
                           int32_t idata,
                           char *sdata,
                           char *object,
                           char *errmess,
                           int forceType)
        = 0;
    virtual bool pvSetValue(knobData *kData,
                            double rdata,
                            int32_t idata,
                            char *sdata,
                            char *object,
                            char *errmess,
                            int forceType)
    {
        Q_UNUSED(kData);
        Q_UNUSED(rdata);
        Q_UNUSED(idata);
        Q_UNUSED(sdata);
        Q_UNUSED(object);
        Q_UNUSED(errmess);
        Q_UNUSED(forceType);
        return false;
    }

    virtual int pvSetWave(char *pv,
                          float *fdata,
                          double *ddata,
                          int16_t *data16,
                          int32_t *data32,
                          char *sdata,
                          int nelm,
                          char *object,
                          char *errmess)
        = 0;
    virtual bool pvSetWave(knobData *kData,
                           float *fdata,
                           double *ddata,
                           int16_t *data16,
                           int32_t *data32,
                           char *sdata,
                           int nelm,
                           char *object,
                           char *errmess)
    {
        Q_UNUSED(kData);
        Q_UNUSED(fdata);
        Q_UNUSED(ddata);
        Q_UNUSED(data16);
        Q_UNUSED(data32);
        Q_UNUSED(sdata);
        Q_UNUSED(nelm);
        Q_UNUSED(object);
        Q_UNUSED(errmess);
        return false;
    }

    virtual int pvGetTimeStamp(char *pv, char *timestamp) = 0;
    virtual bool pvGetTimeStamp(knobData *kData, char *timestamp)
    {
        Q_UNUSED(kData);
        Q_UNUSED(timestamp);
        return false;
    }

    virtual int pvGetDescription(char *pv, char *description) = 0;
    virtual bool pvGetDescription(knobData *kData, char *description)
    {
        Q_UNUSED(kData);
        Q_UNUSED(description);
        return false;
    }

    virtual int pvClearEvent(void *ptr) = 0;
    virtual int pvAddEvent(void *ptr) = 0;
    virtual int pvReconnect(knobData *kData) = 0;
    virtual int pvDisconnect(knobData *kData) = 0;
    virtual int FlushIO() = 0;
    virtual int TerminateIO() = 0;
};

QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(ControlsInterface, "ch.psi.caqtdm.Plugin.ControlsInterface/1.0")
QT_END_NAMESPACE

#endif
