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
using namespace std::tr1;
using namespace epics::pvData;
using namespace epics::pvAccess;

/// terse mode functions
void convertStructure(epics::pvData::StringBuilder buffer, epics::pvData::PVStructure *data, int notFirst);
void convertArray(epics::pvData::StringBuilder buffer, epics::pvData::PVScalarArray * pv, int notFirst);
void convertStructureArray(epics::pvData::StringBuilder buffer, epics::pvData::PVStructureArray * pvdata, int notFirst);

void terseSeparator(char c);
void terseArrayCount(bool flag);
std::ostream& terse(std::ostream& o, epics::pvData::PVField::shared_pointer const & pv);
std::ostream& terseStructure(std::ostream& o, epics::pvData::PVStructure::shared_pointer const & pvStructure);
std::ostream& terseScalarArray(std::ostream& o, epics::pvData::PVScalarArray::shared_pointer const & pvArray);
std::ostream& terseStructureArray(std::ostream& o, epics::pvData::PVStructureArray::shared_pointer const & pvArray);


/*
  Requester implementation:
  Allows messages to be sent to a requester. For error and message logging etc.
  */

class RequesterImpl : public epics::pvData::Requester
{
public:
    RequesterImpl(epics::pvData::String const & requesterName);
    virtual epics::pvData::String getRequesterName();
    virtual void message(epics::pvData::String const & message, epics::pvData::MessageType messageType);

private:
    epics::pvData::String m_requesterName;
};

/*
  ChannelRequester implementation.
  (Each type of access (get, put, putget, needs its own requester)
*/
class ChannelRequesterImpl : public epics::pvAccess::ChannelRequester
{
private:
    epics::pvData::Event m_event;
    int m_channelIndex;
    MutexKnobData* m_mutexData;

public:
    virtual epics::pvData::String getRequesterName();
    virtual void message(String const & message,MessageType messageType);
    virtual void channelCreated(const epics::pvData::Status& status, Channel::shared_pointer const & channel);
    virtual void channelStateChange(epics::pvAccess::Channel::shared_pointer const & channel, epics::pvAccess::Channel::ConnectionState connectionState);
    bool waitUntilConnected(double timeOut);
    void setMyData(int num, MutexKnobData* mutexKnobData);
};

class ChannelPutRequesterImpl : public epics::pvAccess::ChannelPutRequester
{
    private:
    ChannelPut::shared_pointer m_channelPut;
    PVStructure::shared_pointer m_pvStructure;
    BitSet::shared_pointer m_bitSet;
    Mutex m_pointerMutex;
    Mutex m_eventMutex;
    auto_ptr<Event> m_event;
    String m_channelName;

    public:
    ChannelPutRequesterImpl(String channelName);
    virtual String getRequesterName();
    virtual void message(String const & message, MessageType messageType);
    virtual void channelPutConnect(const epics::pvData::Status& status,
                                   ChannelPut::shared_pointer const & channelPut,
                                   epics::pvData::PVStructure::shared_pointer const & pvStructure,
                                   epics::pvData::BitSet::shared_pointer const & bitSet);

    virtual void getDone(const epics::pvData::Status& status);
    virtual void putDone(const epics::pvData::Status& status);
    PVStructure::shared_pointer getStructure();
    void resetEvent();
    bool waitUntilDone(double timeOut);
    void supressGetValue(bool flag);
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
    virtual epics::pvData::String getRequesterName();
    virtual void message(epics::pvData::String const & message, epics::pvData::MessageType messageType);
    virtual void getDone(const epics::pvData::Status& status, epics::pvData::FieldConstPtr const & field);
    epics::pvData::FieldConstPtr getField();
    bool waitUntilFieldGet(double timeOut);
};

/*
  MonitorRequester implementation.
*/
class MonitorRequesterImpl : public MonitorRequester
{
private:
    String m_channelName;
    int m_channelIndex;
    MutexKnobData* m_mutexData;

public:
    MonitorRequesterImpl(String channelName);
    virtual String getRequesterName();
    virtual void message(String const & message,MessageType messageType);
    virtual void monitorConnect(const epics::pvData::Status& status, Monitor::shared_pointer const & monitor, StructureConstPtr const & );
    virtual void monitorEvent(Monitor::shared_pointer const & monitor);
    virtual void unlisten(Monitor::shared_pointer const & );
    void ParsePVStructure(epics::pvData::PVStructure::shared_pointer const & pv, knobData* kData, int notFirst);
    void ParseScalar(QString fieldName, PVScalarPtr const & pvs, knobData* kData);
    void ParseScalarArray(PVScalarArrayPtr const & pvs, knobData* kData);
    void setMyData(int num, MutexKnobData* mutexKnobData);
};
#endif
