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
 *  Authors:
 *    Marty Kraimer, Anton Mezger
 *  Contact details:
 *    anton.mezger@psi.ch, mrkraimer@comcast.net
 */
#ifndef EPICS4PLUGIN_H
#define EPICS4PLUGIN_H

#include <map>
#include <caerr.h>

#include <list>
#include <iostream>
#include <compilerDependencies.h>
#include <pv/requester.h>
#include <pv/status.h>
#include <pv/event.h>
#include <pv/lock.h>
#include <pv/pvData.h>
#include <pv/pvCopy.h>
#include <pv/pvTimeStamp.h>
#include <pv/timeStamp.h>
#include <pv/pvAlarm.h>
#include <pv/alarm.h>
#include <pv/pvAccess.h>
#include <pv/clientFactory.h>
#include <pv/caProvider.h>
#include <pv/standardField.h>
#include <pv/standardPVField.h>
#include <pv/createRequest.h>
#include <pv/nt.h>
#include <pv/convert.h>


#include <QObject>
#include "controlsinterface.h"
#include "callbackThread.h"
#include "epics4Requester.h"



namespace epics { namespace caqtdm { namespace epics4 {



class PVAChannel;
typedef std::tr1::shared_ptr<PVAChannel> PVAChannelPtr;
typedef std::tr1::weak_ptr<PVAChannel> PVAChannelWPtr;


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
    int pvSetValue(char *pv, double rdata, int32_t idata, char *sdata, char *object, char *errmess, int forceType)
    {
        Q_UNUSED(pv); Q_UNUSED(rdata); Q_UNUSED(idata); Q_UNUSED(sdata); Q_UNUSED(object); Q_UNUSED(errmess); Q_UNUSED(forceType);
        return false;
    }
    int pvSetWave(char *pv, float *fdata, double *ddata, int16_t *data16, int32_t *data32, char *sdata, int nelm, char *object, char *errmess)
    {
        Q_UNUSED(pv); Q_UNUSED(fdata); Q_UNUSED(ddata); Q_UNUSED(data16); Q_UNUSED(data32); Q_UNUSED(sdata);
        Q_UNUSED(nelm); Q_UNUSED(object); Q_UNUSED(errmess);
        return false;
    }
    int pvGetTimeStamp(char *pv, char *timestamp)
    {
        Q_UNUSED(pv); Q_UNUSED(timestamp);
        return false;
    }
    int pvGetDescription(char *pv, char *description)
    {
        Q_UNUSED(pv); Q_UNUSED(description);
        return false;
    }
    bool pvSetValue(knobData *kData, double rdata, int32_t idata, char *sdata, char *object, char *errmess, int forceType);
    bool pvSetWave(knobData *kData, float *fdata, double *ddata, int16_t *data16, int32_t *data32, char *sdata, int nelm, char *object, char *errmess);
    bool pvGetTimeStamp(knobData *kData, char *timestamp);
    bool pvGetDescription(knobData *kData, char *description);
    int pvClearEvent(void * ptr);
    int pvAddEvent(void * ptr);
    int pvReconnect(knobData *kData);
    int pvDisconnect(knobData *kData);
    int FlushIO();
    int TerminateIO();
    static void setDebug(bool value) {debug = value;}
    static bool getDebug() {return debug;}


  private:
    static bool debug;
    std::map<std::string,epics::caqtdm::epics4::PVAChannelWPtr> pvaChannelMap;
    epics::caqtdm::epics4::Epics4RequesterPtr requester;
    epics::pvData::CallbackThreadPtr callbackThread;
    MutexKnobData * mutexKnobData;
};

#endif
