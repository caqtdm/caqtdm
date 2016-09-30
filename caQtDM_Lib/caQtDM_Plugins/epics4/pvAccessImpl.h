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

#ifndef PVACCESSIMPL_H
#define  PVACCESSIMPL_H

#include <iostream>
#include <stdio.h>
#include <vector>
#include <string>

#include <epicsStdlib.h>
#include <epicsGetopt.h>

#include <pv/clientFactory.h>
#include <pv/pvData.h>
#include <pv/pvAccess.h>
#include <pv/logger.h>
#include <pv/monitor.h>
#include <pv/event.h>
#include <epicsExit.h>

#include "knobDefines.h"
#include "mutexKnobData.h"

using namespace std;
namespace TR1 = std::tr1;
using namespace epics::pvData;
using namespace epics::pvAccess;

#define qasc(x) x.toLatin1().constData()

class MonitorPVRequesterImpl : public ChannelRequester
{
private:
    epics::pvData::Event m_event;
    int m_channelIndex;
    MutexKnobData* m_mutexData;

public:
    virtual string getRequesterName();
    virtual void message(string const & message, MessageType messageType);
    virtual void channelCreated(const epics::pvData::Status& status, Channel::shared_pointer const & channel);
    virtual void channelStateChange(epics::pvAccess::Channel::shared_pointer const & channel, epics::pvAccess::Channel::ConnectionState connectionState);
    bool waitUntilConnected(double timeOut);
    void defineIndexForKnobData(int num, MutexKnobData* mutexKnobData);
};

/*
  FieldRequester implementation.
*/
class GetFieldRequesterImpl : public epics::pvAccess::GetFieldRequester
{
private:
    epics::pvAccess::Channel::shared_pointer m_channel;
    epics::pvData::FieldConstPtr m_field;
    epics::pvData::Event m_event;
    epics::pvData::Mutex m_pointerMutex;

public:
    GetFieldRequesterImpl(epics::pvAccess::Channel::shared_pointer channel);
    virtual std::string getRequesterName();
    virtual void message(std::string const & message, epics::pvData::MessageType messageType);
    virtual void getDone(const epics::pvData::Status& status, epics::pvData::FieldConstPtr const & field);
    epics::pvData::FieldConstPtr getField();
    bool waitUntilFieldGet(double timeOut);
};

/*
  MonitorRequester implementation.
*/
class DataMonitorRequesterImpl : public MonitorRequester
{
private:
    QString m_channelName;
    int m_channelIndex;
    MutexKnobData* m_mutexData;

public:
    DataMonitorRequesterImpl(std::string channelName);
    virtual std::string getRequesterName();
    virtual void message(std::string const & message, MessageType messageType);
    virtual void monitorConnect(const epics::pvData::Status& status, Monitor::shared_pointer const & monitor, StructureConstPtr const & );
    virtual void monitorEvent(Monitor::shared_pointer const & monitor);
    virtual void unlisten(Monitor::shared_pointer const & );
    void ParsePVStructure(QString fieldName, epics::pvData::PVStructure::shared_pointer const & pv, knobData* kData, int notFirst);
    void ParseScalar(QString fieldName, PVScalarPtr const & pvs, knobData* kData);
    void ParseScalarArray(PVScalarArrayPtr const & pvs, knobData* kData);
    void defineIndexForKnobData(int num, MutexKnobData* mutexKnobData);
};
#endif
