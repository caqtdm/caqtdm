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
#ifndef EPICS4PLUGIN_H
#define EPICS4PLUGIN_H

#include <map>
#include <caerr.h>
#include <pv/pvData.h>
#include <pv/pvaClient.h>
#include <pv/convert.h>
#include <pv/standardField.h>
#include <pv/standardPVField.h>
#include <pv/alarm.h>
#include <pv/timeStamp.h>

#include <QObject>
#include "controlsinterface.h"
#include "callbackThread.h"
#include "epics4Requester.h"

namespace epics { namespace caqtdm { namespace epics4 {


class PvaInterface;
typedef std::tr1::shared_ptr<PvaInterface> PvaInterfacePtr;
typedef std::tr1::weak_ptr<PvaInterface> PvaInterfaceWPtr;


}}}


class Q_DECL_EXPORT Epics4Plugin : public QObject, ControlsInterface,
     public std::tr1::enable_shared_from_this<Epics4Plugin>
{
    Q_OBJECT
    Q_INTERFACES(ControlsInterface)
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "ch.psi.caqtdm.Plugin.ControlsInterface/1.0.epics4controls")
#endif

public:
    QString pluginName();
    Epics4Plugin();
    ~Epics4Plugin();

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

  private:
    epics::pvaClient::PvaClientPtr pvaClient;
    epics::caqtdm::epics4::Epics4RequesterPtr requester;
    epics::pvData::CallbackThreadPtr callbackThread;
    MutexKnobData * mutexKnobData;
    std::map<std::string,epics::caqtdm::epics4::PvaInterfacePtr> pvaInterfaceMap;
};

#endif
