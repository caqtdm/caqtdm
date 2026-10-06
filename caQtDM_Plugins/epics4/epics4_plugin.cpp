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
#include <QDebug>
#include <QString>
#include <QApplication>
#include <db_access.h>
#include <cadef.h>

#include <pv/nt.h>
#include "epics4_plugin.h"
#include <epicsThread.h>
#include <ctime>
#include "caQtDM_Plugins_global.h"

Q_LOGGING_CATEGORY(epics4Log, "caqtdm.plugins.epics.4")

using namespace std;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::pvAccess::ca;
using namespace epics::nt;

namespace epics { namespace caqtdm { namespace epics4 {


class PVAInterface;
typedef std::shared_ptr<PVAInterface> PVAInterfacePtr;
typedef std::weak_ptr<PVAInterface> PVAInterfaceWPtr;

class PVAChannelRequester;
typedef std::shared_ptr<PVAChannelRequester> PVAChannelRequesterPtr;
typedef std::weak_ptr<PVAChannelRequester> PVAChannelRequesterWPtr;

class epicsShareClass PVAChannel :
        public std::enable_shared_from_this<PVAChannel>
{
private:
    string fullName;
    string mapName;
    Epics4RequesterPtr requester;
    CallbackThreadPtr callbackThread;
    PVAChannelProviderPtr providerN;
    PVAChannelRequesterPtr pvaChannelRequester;
    std::vector<PVAInterfaceWPtr> pvaInterfaceList;
    Channel::shared_pointer channel;
    Mutex mutex;

public:
    POINTER_DEFINITIONS(PVAChannel);
    PVAChannel(const string & fullName, const string & mapName,
               const Epics4RequesterPtr & requester,
               const CallbackThreadPtr & callbackThread, const PVAChannelProviderPtr &providerN);
    ~PVAChannel();
    void destroy();
    std::string getRequesterName();
    void message(string const & message,MessageType messageType);
    void channelCreated(const Status & status, Channel::shared_pointer const & channel);
    void channelStateChange(Channel::shared_pointer const & channel,Channel::ConnectionState connectionState);

    Channel::shared_pointer getChannel() {return channel;}
    string getFullName() { return fullName;}
    string getMapName() { return mapName;}
    bool connect(const string & channelName,const string &providerName);
    void addInterface(const PVAInterfacePtr & pvaInterface);
    // following returns (true,false) if no more interfaces
    bool removeInterface(const PVAInterfacePtr & pvaInterface);
};

class epicsShareClass PVAChannelRequester : public ChannelRequester
{
    PVAChannelWPtr pvaChannel;

public:

    PVAChannelRequester(PVAChannelPtr const & pvaChannel): pvaChannel(pvaChannel) {
    }

    virtual ~PVAChannelRequester() {
        if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "~PVAChannelRequester";
    }

    virtual std::string getRequesterName()
    {
        PVAChannelPtr chan(pvaChannel.lock());
        if(!chan) return string("pvaChannel is null");
        return chan->getRequesterName();
    }

    virtual void message(std::string const & message, epics::pvData::MessageType messageType)
    {
        PVAChannelPtr chan(pvaChannel.lock());
        if(!chan) return;
        chan->message(message,messageType);
    }

    virtual void channelCreated(const epics::pvData::Status& status, Channel::shared_pointer const & channel)
    {
        PVAChannelPtr chan(pvaChannel.lock());
        if(!chan) return;
        chan->channelCreated(status,channel);
    }

    virtual void channelStateChange(Channel::shared_pointer const & channel, Channel::ConnectionState connectionState)
    {
        PVAChannelPtr chan(pvaChannel.lock());
        if(!chan) return;
        chan->channelStateChange(channel,connectionState);
    }
};

class PVAGetFieldRequester;
typedef std::shared_ptr<PVAGetFieldRequester> PVAGetFieldRequesterPtr;
typedef std::weak_ptr<PVAGetFieldRequester> PVAGetFieldRequesterWPtr;

class PVAChannelGetRequester;
typedef std::shared_ptr<PVAChannelGetRequester> PVAChannelGetRequesterPtr;
typedef std::weak_ptr<PVAChannelGetRequester> PVAChannelGetRequesterWPtr;

class PVAChannelPutRequester;
typedef std::shared_ptr<PVAChannelPutRequester> PVAChannelPutRequesterPtr;
typedef std::weak_ptr<PVAChannelPutRequester> PVAChannelPutRequesterWPtr;

class PVAMonitorRequester;
typedef std::shared_ptr<PVAMonitorRequester> PVAMonitorRequesterPtr;
typedef std::weak_ptr<PVAMonitorRequester> PVAMonitorRequesterWPtr;

class epicsShareClass PVAInterface :
        public epics4_CallbackRequester,
        public std::enable_shared_from_this<PVAInterface>
{
private:
    template <typename pureData> void fillData(pureData const &array, size_t size, knobData* kPtr);
    void fillNtMetadata();

    enum NormativeType {
        ntunknown_t,
        ntscalar_t,
        ntscalararray_t,
        ntenum_t
    };

    enum CallbackType {
        unknown_t,
        interface_t,
        displayControl_t,
        enum_t,
        createMonitor_t
    };
    PVAChannelPtr pvaChannel;
    bool gotFirstConnection;
    bool putFinished;
    bool unlistenCalled;
    bool monitorStarted;
    CreateRequest::shared_pointer createRequest;
    MutexKnobData *mutexKnobData;
    int index;
    Epics4RequesterPtr requester;
    CallbackThreadPtr callbackThread;
    bool gotFirstConnect;
    NormativeType normativeType;
    int ntKind;                 // enum ntType from the structure id, NT_NONE for non NT structures
    int dimRows, dimCols;       // cached NTMatrix dim (0 = none received yet)
    bool hasDimField;
    CallbackType callbackType;
    StructureConstPtr structure;
    ChannelGet::shared_pointer pvaChannelGet;
    ChannelPut::shared_pointer pvaChannelPut;
    PVStructurePtr putPVStructure;
    Event waitForPutConnect;
    Monitor::shared_pointer monitor;
    ConvertPtr convert;
    knobData kData;
    shared_vector<const string> choices;
    TimeStamp timeStamp;
    bool gotTimeStamp;
    string description;
    Mutex mutex;
    PVAGetFieldRequesterPtr pvaGetFieldRequester;
    PVAChannelGetRequesterPtr pvaChannelGetRequester;
    PVAChannelPutRequesterPtr pvaChannelPutRequester;
    BitSetPtr putBitSet;
    PVAMonitorRequesterPtr pvaMonitorRequester;

public:
    POINTER_DEFINITIONS(PVAInterface);
    PVAInterface(PVAChannelPtr const & pvaChannel,
                 MutexKnobData *mutexKnobData,
                 int index,
                 Epics4RequesterPtr const & requester,
                 CallbackThreadPtr const & callbackThread);
    virtual ~PVAInterface();
    void destroy();
    void clearMonitor();
    void stopMonitor();
    void startMonitor();

    std::string getRequesterName();
    void message(std::string const & message,MessageType messageType);
    // for getField
    void getDone(
            const Status& status,
            FieldConstPtr const & field);
    // for pvaChannelGet
    void channelGetConnect(
            const Status& status,
            ChannelGet::shared_pointer const & channelGet,
            Structure::const_shared_pointer const & structure);
    void getDone(
            const Status& status,
            ChannelGet::shared_pointer const & channelGet,
            PVStructurePtr const & pvStructure,
            BitSet::shared_pointer const & bitSet);

    // for monitor
    void monitorConnect(
            const Status & status,
            Monitor::shared_pointer const & monitor,
            Structure::const_shared_pointer const & structure);
    void unlisten(MonitorPtr const & monitor);
    void monitorEvent(MonitorPtr const & monitor);
    // for pvaChannelPut
    void channelPutConnect(
            const Status& status,
            ChannelPut::shared_pointer const & channelPut,
            Structure::const_shared_pointer const & structure);
    void getDone(
            const Status& status,
            ChannelPut::shared_pointer const & channelPut,
            PVStructurePtr const & pvStructure,
            BitSet::shared_pointer const & bitSet);
    void putDone(
            const Status& status,
            ChannelPut::shared_pointer const & channelPut);

    virtual void callback();

    void channelStateChange(bool isConnected);

    PVAChannelPtr getPVAChannel();
    int reconnect();
    int disconnect();
    int terminateIO();

    void getInterface();
    void gotInterface();
    void getDisplayControl();
    void gotDisplayControl(PVStructurePtr const & pvStructure);
    void getEnum();
    void gotEnum(PVStructurePtr const & pvStructure);
    void createMonitor();
    void gotMonitor();
    void getScalarData(PVStructurePtr const & pvStructure);
    void getEnumData(PVStructurePtr const & pvStructure);
    void getScalarArrayData(PVStructurePtr const & pvStructure);
    bool setValue(double rdata, int32_t idata, char *sdata, int forceType);
    bool setArrayValue(
            float *fdata, double *ddata,
            int16_t *data16, int32_t *data32,
            char *sdata, int nelm);
    bool getTimeStamp(char *timestamp);
    bool getDescription(char *timestamp);
};

class epicsShareClass PVAGetFieldRequester : public GetFieldRequester
{
    PVAInterfaceWPtr pvaInterface;
public:
    PVAGetFieldRequester(PVAInterfacePtr const & pvaInterface) : pvaInterface(pvaInterface)
    {}
    virtual ~PVAGetFieldRequester() {
        if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "~PVAGetFieldRequester";
    }

    virtual std::string getRequesterName()
    {
        PVAInterfacePtr _interface(pvaInterface.lock());
        if(!_interface) return string("pvaInterface is null");
        return _interface->getRequesterName();
    }

    virtual void message(std::string const & message, epics::pvData::MessageType messageType)
    {
        PVAInterfacePtr _interface(pvaInterface.lock());
        if(!_interface) return;
        _interface->message(message,messageType);
    }

    virtual void getDone(const Status& status,  FieldConstPtr const & field)
    {
        PVAInterfacePtr _interface(pvaInterface.lock());
        if(!_interface) return;
        _interface->getDone(status,field);
    }
};

class epicsShareClass PVAChannelGetRequester : public ChannelGetRequester
{
    PVAInterfaceWPtr pvaInterface;

public:

    PVAChannelGetRequester(PVAInterfacePtr const & pvaInterface) : pvaInterface(pvaInterface) {
    }

    virtual ~PVAChannelGetRequester() {
        if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "~PVAChannelGetRequester";
    }

    virtual std::string getRequesterName()
    {
        PVAInterfacePtr _interface(pvaInterface.lock());
        if(!_interface) return string("pvaInterface is null");
        return _interface->getRequesterName();
    }

    virtual void message(std::string const & message, epics::pvData::MessageType messageType)
    {
        PVAInterfacePtr _interface(pvaInterface.lock());
        if(!_interface) return;
        _interface->message(message,messageType);
    }

    virtual void channelGetConnect(
            const Status& status,
            ChannelGet::shared_pointer const & channelGet,
            Structure::const_shared_pointer const & structure)
    {
        PVAInterfacePtr _interface(pvaInterface.lock());
        if(!_interface) return;
        _interface->channelGetConnect(status,channelGet,structure);
    }

    virtual void getDone(
            const Status& status,
            ChannelGet::shared_pointer const & channelGet,
            PVStructurePtr const & pvStructure,
            BitSet::shared_pointer const & bitSet)
    {
        PVAInterfacePtr _interface(pvaInterface.lock());
        if(!_interface) return;
        _interface->getDone(status,channelGet,pvStructure,bitSet);
    }
};

class epicsShareClass PVAChannelPutRequester : public ChannelPutRequester
{
    PVAInterfaceWPtr pvaInterface;

public:

    PVAChannelPutRequester(PVAInterfacePtr const & pvaInterface): pvaInterface(pvaInterface)
    {}

    virtual ~PVAChannelPutRequester() {
        if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "~PVAChannelPutRequester";
    }

    virtual std::string getRequesterName()
    {
        PVAInterfacePtr _interface(pvaInterface.lock());
        if(!_interface) return string("pvaInterface is null");
        return _interface->getRequesterName();
    }

    virtual void message(std::string const & message, epics::pvData::MessageType messageType)
    {
        PVAInterfacePtr _interface(pvaInterface.lock());
        if(!_interface) return;
        _interface->message(message,messageType);
    }

    virtual void channelPutConnect(
            const Status& status,
            ChannelPut::shared_pointer const & channelPut,
            Structure::const_shared_pointer const & structure)
    {
        PVAInterfacePtr _interface(pvaInterface.lock());
        if(!_interface) return;
        _interface->channelPutConnect(status,channelPut,structure);
    }

    virtual void getDone(
            const Status& status,
            ChannelPut::shared_pointer const & channelPut,
            PVStructurePtr const & pvStructure,
            BitSet::shared_pointer const & bitSet)
    {
        PVAInterfacePtr _interface(pvaInterface.lock());
        if(!_interface) return;
        _interface->getDone(status,channelPut,pvStructure,bitSet);
    }

    virtual void putDone(
            const Status& status,
            ChannelPut::shared_pointer const & channelPut)
    {
        PVAInterfacePtr _interface(pvaInterface.lock());
        if(!_interface) return;
        _interface->putDone(status,channelPut);
    }
};

class epicsShareClass PVAMonitorRequester : public MonitorRequester
{
    PVAInterfaceWPtr pvaInterface;

public:
    PVAMonitorRequester(PVAInterfacePtr const & pvaInterface): pvaInterface(pvaInterface)
    {}

    virtual ~PVAMonitorRequester() {
        if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "~PVAMonitorRequester";
    }

    virtual std::string getRequesterName()
    {
        PVAInterfacePtr _interface(pvaInterface.lock());
        if(!_interface) return string("pvaInterface is null");
        return _interface->getRequesterName();
    }

    virtual void message(std::string const & message, epics::pvData::MessageType messageType)
    {
        PVAInterfacePtr _interface(pvaInterface.lock());
        if(!_interface) return;
        _interface->message(message,messageType);
    }

    virtual void monitorConnect(
            const Status& status,
            Monitor::shared_pointer const & monitor,
            Structure::const_shared_pointer const & structure)
    {
        PVAInterfacePtr _interface(pvaInterface.lock());
        if(!_interface) return;
        _interface->monitorConnect(status,monitor,structure);
    }

    virtual void unlisten(MonitorPtr const & monitor)
    {
        PVAInterfacePtr _interface(pvaInterface.lock());
        if(!_interface) return;
        _interface->unlisten(monitor);
    }

    virtual void monitorEvent(MonitorPtr const & monitor)
    {
        PVAInterfacePtr _interface(pvaInterface.lock());
        if(!_interface) return;
        _interface->monitorEvent(monitor);
    }
};

PVAChannel::PVAChannel(const string & fullName, const string & mapName,
                       const Epics4RequesterPtr & requester,
                       const CallbackThreadPtr & callbackThread,
                       const PVAChannelProviderPtr &providerN): fullName(fullName), mapName(mapName),
    requester(requester),
    callbackThread(callbackThread),
    providerN(providerN)
{
    if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "PVAChannel::PVAChannel() fullName" << QString::fromStdString(fullName) << QString::fromStdString(mapName);
}

PVAChannel::~PVAChannel()
{
    if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "PVAChannel::~PVAChannel()";
}

void PVAChannel::destroy()
{
    if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "PVAChannel::destroy()";
    pvaChannelRequester.reset();
    if(channel) channel->destroy();
}

std::string PVAChannel::getRequesterName()
{
    return requester->getRequesterName();
}

void PVAChannel::message(string const & message,MessageType messageType)
{
    requester->message(fullName + " " + message,messageType);
}

void PVAChannel::channelCreated(const Status & status, Channel::shared_pointer const & channel)
{
    if(Epics4Plugin::getDebug()) {
        qCDebug(::epics4Log) << "PVAChannel::created"
             << "fullName" << QString::fromStdString(getFullName()) << "status.isOK()" << ( status.isOK() ? "true" : "false")
             << "channel->isConnected())" << ( channel->isConnected() ? "true" : "false");
    }
    if(!status.isOK()) {
        requester->message(channel->getChannelName() + " " + status.getMessage(),errorMessage);
        return;
    }
    if(channel->isConnected()) channelStateChange(channel,Channel::CONNECTED);
}

void PVAChannel::channelStateChange(
        Channel::shared_pointer const & channel,Channel::ConnectionState connectionState)
{
    Q_UNUSED(channel);
    if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "PVAChannel::channelStateChange";
    Lock xx(mutex);
    if(pvaInterfaceList.empty()) return;
    size_t num = pvaInterfaceList.size();
    bool value = (connectionState==Channel::CONNECTED ? true :  false);
    if(Epics4Plugin::getDebug()) {
        qCDebug(::epics4Log) << "PVAChannel::channelStateChange isConnected" << ( value ? "true" : "false")
             << "fullName" << QString::fromStdString(getFullName()) << "num" << num;
    }

    for(size_t ind = 0; ind<num; ++ind) {
        PVAInterfacePtr _interface = pvaInterfaceList[ind].lock();
        if(_interface) _interface->channelStateChange(value);
    }
}

bool PVAChannel::connect(const string & channelName,const string & providerName)
{
    if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << QString::fromStdString("PVAChannel::connect " + channelName);

    if(!providerN) {
        requester->message(channelName + " provider " + providerName + " not registered",errorMessage);
        return false;
    }
    if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << QString::fromStdString("PVAChannel::provider=" + providerN->getProviderName());

    pvaChannelRequester = PVAChannelRequesterPtr(new PVAChannelRequester(shared_from_this()));

    channel = providerN->createChannel(
                channelName,
                pvaChannelRequester,
                ChannelProvider::PRIORITY_DEFAULT);
    if(!channel) {
        requester->message(channelName + " channelCreate failed ",errorMessage);
        pvaChannelRequester.reset();
        return false;
    }

    if(Epics4Plugin::getDebug()) {
        qCDebug(::epics4Log) << "PVAChannel::connect" << channel.get()
             << "fullName" << QString::fromStdString(getFullName())
             << "channel->isConnected())" <<( channel->isConnected() ? "true" : "false");
    }
    return true;
}

void PVAChannel::addInterface(const PVAInterfacePtr & pvaInterface)
{
    if(Epics4Plugin::getDebug()) {
        qCDebug(::epics4Log) << "PVAChannel::addInterface"
             << "fullName" << QString::fromStdString(getFullName())
             << "channel->isConnected())" << ( channel->isConnected() ? "true" : "false");
    }
    Lock xx(mutex);
    pvaInterfaceList.push_back(PVAInterfaceWPtr(pvaInterface));
    if(channel->isConnected()) pvaInterface->channelStateChange(true);

}

bool PVAChannel::removeInterface(const PVAInterfacePtr & pvaInterface)
{
    Lock xx(mutex);
    size_t num = pvaInterfaceList.size();
    if(Epics4Plugin::getDebug()) {
        qCDebug(::epics4Log) << "PVAChannel::removeInterface"
             << "fullName" << QString::fromStdString(getFullName()) << "num" << num;
    }
    for(size_t ind = 0; ind<num; ++ind) {
        PVAInterfacePtr _interface = pvaInterfaceList[ind].lock();
        if(_interface==pvaInterface) {
            pvaInterfaceList.erase(pvaInterfaceList.begin() + ind);
            break;
        }
    }
    num = pvaInterfaceList.size();
    if(Epics4Plugin::getDebug()) {
        qCDebug(::epics4Log) << "after remove num" << pvaInterfaceList.size();
    }
    return ((num==0) ? true : false);
}


class epicsShareClass PVAInterfaceGlue
{
private:
    PVAInterfacePtr pvaInterface;
public:
    PVAInterfaceGlue(const PVAInterfacePtr & pvaInterface) :pvaInterface(pvaInterface)
    {}
    ~PVAInterfaceGlue()
    {
        if(Epics4Plugin::getDebug()) {
            qCDebug(::epics4Log) << "~PVAInterfaceGlue()"
                 << "pvaInterface use count" << pvaInterface.use_count();
        }
    }
    PVAInterfacePtr getPVAInterface()
    {
        return pvaInterface;
    }
};


PVAInterface::PVAInterface(
        PVAChannelPtr const & pvaChannel,
        MutexKnobData *mutexKnobData,
        int index,
        Epics4RequesterPtr const & requester,
        CallbackThreadPtr const & callbackThread)
    : pvaChannel(pvaChannel),
      gotFirstConnection(false),
      putFinished(true),
      unlistenCalled(false),
      monitorStarted(false),
      createRequest(CreateRequest::create()),
      mutexKnobData(mutexKnobData),
      index(index),
      requester(requester),
      callbackThread(callbackThread),
      gotFirstConnect(false),
      normativeType(ntunknown_t),
      ntKind(NT_NONE),
      dimRows(0),
      dimCols(0),
      hasDimField(false),
      callbackType(unknown_t),
      convert(getConvert()),
      gotTimeStamp(false)
{
    if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "PVAInterface::PVAInterface()";
}

// NT metadata, written on every publish (K6); an NTMatrix without a dim
// so far is published as a column vector [valueCount, 1]
void PVAInterface::fillNtMetadata()
{
    kData.edata.ntType = ntKind;
    if(ntKind == NT_MATRIX) {
        bool cached = (dimRows > 0 && dimCols > 0);
        kData.edata.dimCount = 2;
        kData.edata.dim[0] = cached ? dimRows : kData.edata.valueCount;
        kData.edata.dim[1] = cached ? dimCols : 1;
    } else {
        kData.edata.dimCount = 0;
        kData.edata.dim[0] = 0;
        kData.edata.dim[1] = 0;
    }
}

PVAInterface::~PVAInterface()
{
    if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "PVAInterface::~PVAInterface()";
}

void PVAInterface::clearMonitor()
{
    Lock lock(mutex);
    if(monitor) {
        if(monitorStarted) {
            monitor->stop();
            monitorStarted = false;
        }
        monitor->destroy();
    }
}

void PVAInterface::stopMonitor()
{
    Lock lock(mutex);
    if(monitor) {
        if(monitorStarted) {
            monitor->stop();
            monitorStarted = false;
        }
    }
}

void PVAInterface::startMonitor()
{
    Lock lock(mutex);
    if(monitor) {
        if(!monitorStarted) {
            monitor->start();
            monitorStarted = true;
        }
    }
}

void PVAInterface::destroy()
{
    if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "PVAInterface::destroy calling pvaChannelGet->destroy";
    if(pvaChannelGet) pvaChannelGet->destroy();
    if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "PVAInterface::destroy calling pvaChannelPut->destroy";
    if(pvaChannelPut) pvaChannelPut->destroy();
    if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "PVAInterface::destroy calling pvaChannel.reset";
    pvaChannel.reset();
    if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "PVAInterface::destroy return";
}

void PVAInterface::channelStateChange(bool isConnected)
{
    if(Epics4Plugin::getDebug()) {
        qCDebug(::epics4Log) << "PVAInterface::channelStateChange index" << index
             << "fullName" << QString::fromStdString(pvaChannel->getFullName())
             << "isConnected" << (isConnected ? "true" : "false");
    }
    if(gotFirstConnect) mutexKnobData->SetMutexKnobDataConnected(index, isConnected);
    if(!isConnected || gotFirstConnect) return;
    callbackType = interface_t;
    if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "queue request";
    callbackThread->queueRequest(shared_from_this());
}


string PVAInterface::getRequesterName()
{
    return requester->getRequesterName();
}

void PVAInterface::message(std::string const & message,MessageType messageType)
{
    requester->message(pvaChannel->getFullName() + " " + message,messageType);
}

void PVAInterface::getDone(
        const Status& status,
        FieldConstPtr const & yyy)
{
    structure =  std::dynamic_pointer_cast<const Structure>(yyy);
    if(!status.isOK()) {
        string mess(status.getMessage());
        mess += " getField failed";
        message(mess,errorMessage);
        return;
    }
    FieldConstPtr field = structure->getField<Field>(string("value"));
    if(!field) {
        message(" no value field",errorMessage);
        return;
    }
    Type type = field->getType();
    if(type==scalar) {
        normativeType = ntscalar_t;
    } else if(type==scalarArray) {
        normativeType = ntscalararray_t;
    } else if(NTEnum::is_a(structure)) {
        normativeType = ntenum_t;
    } else {
        message(" value is not a valid nttype",errorMessage);
        return;
    }

    // the NT kind comes from the structure id; an NTMatrix keeps its
    // ntscalararray_t data path (value is double[])
    ntKind = NT_NONE;
    hasDimField = false;
    if(NTMatrix::is_a(structure)) {
        ntKind = NT_MATRIX;
        hasDimField = structure->getField("dim") ? true : false;
    } else if(NTScalarArray::is_a(structure)) {
        ntKind = NT_SCALAR_ARRAY;
    } else if(NTScalar::is_a(structure)) {
        ntKind = NT_SCALAR;
    }
    qCDebug(::epics4Log) << "structure id" << structure->getID().c_str() << "ntType" << ntKind;
    gotInterface();
}

void PVAInterface::channelGetConnect(
        const Status& status,
        ChannelGet::shared_pointer const & channelGet,
        Structure::const_shared_pointer const & structure)
{
    Q_UNUSED(channelGet);
    Q_UNUSED(structure);
    if(!status.isOK()) {
        string mess(status.getMessage());
        mess += " channelGetConnect failed";
        message(mess,errorMessage);
        return;
    }
    channelGet->get();
}

void PVAInterface::getDone(
        const Status& status,
        ChannelGet::shared_pointer const & channelGet,
        PVStructurePtr const & pvStructure,
        BitSet::shared_pointer const & bitSet)
{
    Q_UNUSED(channelGet);
    Q_UNUSED(bitSet);
    if(!status.isOK()) {
        string mess(status.getMessage());
        mess += " channelGetDone failed";
        message(mess,errorMessage);
        return;
    }
    switch(normativeType) {
    case ntscalar_t : gotDisplayControl(pvStructure); break;
    case ntscalararray_t : gotDisplayControl(pvStructure); break;
    case ntenum_t : gotEnum(pvStructure); break;
    default: message("getDone logic error",errorMessage); break;
    }
}

void PVAInterface::monitorConnect(
        const Status & status,
        Monitor::shared_pointer const & monitor,
        Structure::const_shared_pointer const & structure)
{
    Q_UNUSED(structure);
    if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << " PVAInterface::monitorConnect";
    if(status.isOK()) {
        Lock lock(mutex);
        if(!monitorStarted) {
            monitor->start();
            monitorStarted = true;
        }
    } else {
        string mess(status.getMessage());
        mess += " monitorConnect failed";
        message(mess,errorMessage);
    }
}

void PVAInterface::unlisten(MonitorPtr const & monitor)
{
    Q_UNUSED(monitor);
    qCDebug(::epics4Log) << "PVAInterface::unlisten";
    unlistenCalled = true;
}

void PVAInterface::monitorEvent(MonitorPtr const & monitor)
{
    if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "PVAInterface::monitorEvent";
    while(true) {
        MonitorElementPtr monitorElement(monitor->poll());
        if(!monitorElement) break;
        PVStructurePtr pvStructure = monitorElement->pvStructurePtr;
        kData = mutexKnobData->GetMutexKnobData(index);
        if(kData.index == -1) {
            monitor->release(monitorElement);
            return;
        }
        mutexKnobData->DataLock(&kData);
        // nothing may escape into the pvAccess thread
        try {
            // from the monitor data, the ca introspection lacks timeStamp
            PVFieldPtr pvAlarmField = pvStructure->getSubField<PVStructure>("alarm");
            Alarm alarm;
            PVAlarm pvAlarm;
            if(pvAlarmField && pvAlarm.attach(pvAlarmField)) {
                pvAlarm.get(alarm);
                kData.edata.severity = alarm.getSeverity();
            }
            PVFieldPtr pvTimeField = pvStructure->getSubField<PVStructure>("timeStamp");
            PVTimeStamp pvTimeStamp;
            if(pvTimeField && pvTimeStamp.attach(pvTimeField)) {
                pvTimeStamp.get(timeStamp);
                gotTimeStamp = true;
            }

            switch (normativeType) {
                case ntscalar_t : getScalarData(pvStructure); break;
                case ntenum_t : getEnumData(pvStructure); break;
                case ntscalararray_t : getScalarArrayData(pvStructure); break;
                default: message("monitorEvent logic error",errorMessage); break;
            }
            if(ntKind == NT_MATRIX) {
                // value and dim may be marked separately, the last dim stays cached
                PVIntArrayPtr pvDim = pvStructure->getSubField<PVIntArray>("dim");
                if(pvDim && pvDim->getLength() == 2) {
                    shared_vector<const int32> dim(pvDim->view());
                    if(dim[0] > 0 && dim[1] > 0) {
                        dimRows = dim[0];
                        dimCols = dim[1];
                    }
                }
                if(dimRows * dimCols != kData.edata.valueCount && dimRows > 0)
                    qCDebug(::epics4Log) << kData.pv << "dim" << dimRows << "x" << dimCols << "does not match" << kData.edata.valueCount << "values";
            }
            fillNtMetadata();
            qCDebug(::epics4Log) << "update" << kData.pv << kData.index << kData.pluginFlavor << kData.dispName <<kData.edata.rvalue << kData.edata.ivalue;
            mutexKnobData->SetMutexKnobDataReceived(&kData);
        } catch (std::exception &e) {
            qCCritical(::epics4Log) << "PVAInterface::monitorEvent" << kData.pv << "exception" << e.what();
        }

        mutexKnobData->DataUnlock(&kData);
        monitor->release(monitorElement);
    }
}

void PVAInterface::channelPutConnect(
        const Status& status,
        ChannelPut::shared_pointer const & channelPut,
        Structure::const_shared_pointer const & structure)
{
    Q_UNUSED(channelPut);
    if(!status.isOK()) {
        requester->message(pvaChannel->getFullName() + " " + status.getMessage(),errorMessage);
        return;
    }
    putPVStructure = getPVDataCreate()->createPVStructure(structure);
    putBitSet = BitSetPtr(new BitSet(putPVStructure->getNumberFields()));
    waitForPutConnect.signal();
}

void PVAInterface::getDone(
        const Status& status,
        ChannelPut::shared_pointer const & channelPut,
        PVStructurePtr const & pvStructure,
        BitSet::shared_pointer const & bitSet)
{
    Q_UNUSED(channelPut);
    Q_UNUSED(pvStructure);
    Q_UNUSED(bitSet);
    if(!status.isOK()) {
        requester->message(pvaChannel->getFullName() + " " + status.getMessage(),errorMessage);
        return;
    }
}

void PVAInterface::putDone(
        const Status& status,
        ChannelPut::shared_pointer const & channelPut)
{
    Q_UNUSED(channelPut);
    if(!status.isOK()) {
        requester->message(pvaChannel->getFullName() + " " + status.getMessage(),errorMessage);
        return;
    }
    Lock lock(mutex);
    putFinished = true;
    if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "PVAInterface::putDone set putFinished = true";
}


void PVAInterface::callback()
{
    if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "PVAInterface::callback() index" << index << "callbacktype" << callbackType;
    try {
        switch(callbackType) {
            case interface_t : getInterface(); break;
            case displayControl_t : getDisplayControl(); break;
            case enum_t : getEnum(); break;
            case createMonitor_t : createMonitor(); break;
            default: throw std::runtime_error("PVAInterface::callback logic error");
        }
    } catch (std::runtime_error e) {
        cerr << "exception " << e.what() << endl;
        return;
    }
}

PVAChannelPtr PVAInterface::getPVAChannel()
{
    return pvaChannel;
}

int PVAInterface::reconnect()
{
    if(Epics4Plugin::getDebug()) qCWarning(::epics4Log) << "PVAInterface::reconnect() not implemented";
    return false;
}

int PVAInterface::disconnect()
{
    if(Epics4Plugin::getDebug()) qCWarning(::epics4Log) << "PVAInterface::disconnect()  not implemented";
    return false;
}

int PVAInterface::terminateIO()
{
    if(Epics4Plugin::getDebug()) qCWarning(::epics4Log) << "PVAInterface::terminateIO()  not implemented";
    return false;
}


void PVAInterface::getInterface()
{
    pvaGetFieldRequester = PVAGetFieldRequesterPtr(new PVAGetFieldRequester(shared_from_this()));
    pvaChannel->getChannel()->getField(pvaGetFieldRequester ,"");
}

void PVAInterface::gotInterface()
{
    if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "PVAInterface::gotInterface()";
    if(normativeType==ntenum_t) {
        callbackType = enum_t;
        callbackThread->queueRequest(shared_from_this());
        return;
    }
    bool gotControl = false;
    bool gotDisplay = false;
    if(structure->getField("control")) gotControl = true;
    if(structure->getField("display")) gotDisplay = true;
    if(!gotControl && !gotDisplay)
    {
        FieldConstPtr valueField = structure->getField("value");
        if(!valueField) {
            message("no support",errorMessage);
            return;
        }
        kData = mutexKnobData->GetMutexKnobData(index);
        if(kData.index == -1) return;
        mutexKnobData->DataLock(&kData);
        kData.edata.initialize = true;
        kData.edata.accessR = 1;
        kData.edata.accessW = 1;
        fillNtMetadata();
        mutexKnobData->SetMutexKnobDataReceived(&kData);

        mutexKnobData->DataUnlock(&kData);
        callbackType = createMonitor_t;
        callbackThread->queueRequest(shared_from_this());
        return;
    }
    callbackType = displayControl_t;
    callbackThread->queueRequest(shared_from_this());
}


void PVAInterface::getDisplayControl()
{
    PVStructurePtr pvRequest = createRequest->createRequest("display,control");
    pvaChannelGetRequester = PVAChannelGetRequesterPtr(new PVAChannelGetRequester(shared_from_this()));
    pvaChannelGet = pvaChannel->getChannel()->createChannelGet(pvaChannelGetRequester,pvRequest);
}

// precision from a format like "F11.5" or "%.5f", 0 without a fraction
static short precisionFromFormat(const string & format)
{
    size_t pos = format.rfind('.');
    if(pos == string::npos) return 0;
    int prec = atoi(format.c_str() + pos + 1);
    return (prec < 0 || prec > 17) ? 0 : (short) prec;
}

void PVAInterface::gotDisplayControl(PVStructurePtr const & pvStructure)
{
    double controlLow = 0;
    double controlHigh = 0;
    double displayLow = 0;
    double displayHigh = 0;
    short precision = 0;
    string units = "";
    PVFieldPtr pvField = pvStructure->getSubField<PVStructure>(string("control"));
    if(pvField) {
        Control control;
        PVControl pvControl;
        if(pvControl.attach(pvField)) {
            pvControl.get(control);
            controlLow = control.getLow();
            controlHigh = control.getHigh();
        }
    }

    PVStructurePtr pvDisplay(pvStructure->getSubField<PVStructure>("display"));
    if(pvDisplay) {
        // limitlow
        PVDoublePtr pvDouble = pvDisplay->getSubField<PVDouble>("limitLow");
        if(pvDouble) displayLow = pvDouble->get();
        // limithigh
        pvDouble = pvDisplay->getSubField<PVDouble>("limitHigh");
        if(pvDouble)  displayHigh = pvDouble->get();
        // precision
        PVIntPtr pvInt  = pvDisplay->getSubField<PVInt>("precision");
        if(pvInt) {
            precision =  pvInt->get();
        } else {
            // the ca provider has a format string instead
            PVStringPtr pvFormat = pvDisplay->getSubField<PVString>("format");
            if(pvFormat) precision = precisionFromFormat(pvFormat->get());
        }
        // units
        PVStringPtr pvString = pvDisplay->getSubField<PVString>("units");
        if(pvString) units = pvString->get();
        // description
        pvString = pvDisplay->getSubField<PVString>("description");
        if(pvString)  description = pvString->get();
    }

    kData = mutexKnobData->GetMutexKnobData(index);
    if(kData.index == -1) return;
    mutexKnobData->DataLock(&kData);
    kData.edata.initialize = true;
    kData.edata.upper_disp_limit = displayHigh;
    kData.edata.lower_disp_limit = displayLow;
    kData.edata.upper_ctrl_limit = controlHigh;
    kData.edata.lower_ctrl_limit = controlLow;
    kData.edata.precision = precision;

    qstrncpy(kData.edata.fec, pvaChannel->getChannel()->getRemoteAddress().c_str(),caqtdm_string_t_length);
    AccessRights accessr = pvaChannel->getChannel()->getAccessRights(pvField);
    kData.edata.accessR = 1;
    kData.edata.accessW = 1;
    switch (accessr) {
        case AccessRights::none:
            {
                kData.edata.accessR = 0;
                kData.edata.accessW = 0;
            }
            break;
        case AccessRights::read:
            {
                kData.edata.accessR = 1;
                kData.edata.accessW = 0;
            }
            break;
        case AccessRights::readWrite:
            {
                kData.edata.accessR = 1;
                kData.edata.accessW = 1;
            }
            break;
    }

    int len = units.length();
    if(len>39) len = 39;    // kData.edata.units is 40 bytes
    if(len<1) {
        kData.edata.units[0] = '\0';
    } else {
        const char * from = units.c_str();
        for(int i=0; i< len; ++i) kData.edata.units[i] = from[i];
        kData.edata.units[len] = '\0';
    }
    fillNtMetadata();
    mutexKnobData->SetMutexKnobDataReceived(&kData);

    mutexKnobData->DataUnlock(&kData);
    callbackType = createMonitor_t;
    callbackThread->queueRequest(shared_from_this());
    pvaChannelGet.reset();
}

void PVAInterface::getEnum()
{
    PVStructurePtr pvRequest = createRequest->createRequest("value.choices");
    pvaChannelGetRequester = PVAChannelGetRequesterPtr(new PVAChannelGetRequester(shared_from_this()));
    pvaChannelGet = pvaChannel->getChannel()->createChannelGet(pvaChannelGetRequester,pvRequest);
}

void PVAInterface::gotEnum(PVStructurePtr const & pvStructure)
{
    PVStringArrayPtr pvChoices = pvStructure->getSubField<PVStringArray>(string("value.choices"));
    if(!pvChoices) {
        message("gotEnum no choices",errorMessage);
        return;
    }
    choices = pvChoices->view();
    int enumCount = choices.size();
    if(enumCount<=0) {
        message("gotEnum no choices",errorMessage);
        return;
    }
    kData = mutexKnobData->GetMutexKnobData(index);
    if(kData.index == -1) return;
    mutexKnobData->DataLock(&kData);
    kData.edata.precision = 0;
    kData.edata.units[0] = '\0';
    int dataSize = 0;
    for(int ind = 0; ind < enumCount; ++ind) {
        const string val = choices[ind];
        dataSize += val.length() + 1;
    }
    if(dataSize==0) {
        message("gotEnum no choices",errorMessage);
        mutexKnobData->DataUnlock(&kData);
        return;
    }
    if(dataSize > kData.edata.dataSize) {
        if(kData.edata.dataSize>0 && (kData.edata.dataB != (void*) Q_NULLPTR)) free(kData.edata.dataB);
        kData.edata.dataB = (void*) malloc((size_t) dataSize);
        kData.edata.dataSize = dataSize;
    }
    char * ptr = static_cast<char*>(kData.edata.dataB);
    for(int ind = 0; ind < enumCount; ++ind) {
        const string val = choices[ind];
        int len = val.length();
        memcpy(ptr, val.data(),len);
        ptr += len;
        ptr[0] = '\033';
        ptr++;
    }
    ptr--;
    ptr[0] = '\0';
    kData.edata.initialize = true;
    kData.edata.enumCount = enumCount;
    kData.edata.fieldtype  = DBF_ENUM;
    kData.edata.accessR = 1;
    kData.edata.accessW = 1;
    fillNtMetadata();

    mutexKnobData->SetMutexKnobDataReceived(&kData);

    mutexKnobData->DataUnlock(&kData);
    callbackType = createMonitor_t;
    callbackThread->queueRequest(shared_from_this());
    pvaChannelGet.reset();
}

void PVAInterface::createMonitor()
{
    if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "PVAInterface::createMonitor()";
    try {
        if(normativeType==ntunknown_t) return;
        string request("value,alarm,timeStamp");
        if(normativeType==ntenum_t) request = "alarm,timeStamp,value.index";
        if(ntKind == NT_MATRIX && hasDimField) request += ",dim";
        PVStructurePtr pvRequest = createRequest->createRequest(request);
        pvaMonitorRequester = PVAMonitorRequesterPtr(new PVAMonitorRequester(shared_from_this()));
        monitor = pvaChannel->getChannel()->createMonitor(pvaMonitorRequester,pvRequest);
        gotFirstConnect = true;
        mutexKnobData->SetMutexKnobDataConnected(index,true);
    } catch (std::runtime_error e) {
        cerr << "exception " << e.what() << endl;
        return;
    }
}

void PVAInterface::gotMonitor()
{
    if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "PVAInterface::gotMonitor()";
}


void PVAInterface::getScalarData(PVStructurePtr const & pvStructure)
{
    if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "getScalarData " << kData.pv;
    PVScalarPtr pvScalar = pvStructure->getSubField<PVScalar>("value");
    if(!pvScalar) {
        qCDebug(::epics4Log) << "PVAInterface::getScalarData pvStructure \n" << pvStructure.get();
        return;
    }

    PVFieldPtr pvAlarmField = pvStructure->getSubField<PVStructure>("alarm");
    if(pvAlarmField) {
        Alarm alarm;
        PVAlarm pvAlarm;
        bool result;
        result = pvAlarm.attach(pvAlarmField);
        if(result) {
            pvAlarm.get(alarm);
            int sev = alarm.getSeverity();
            kData.edata.severity = sev;
            kData.edata.status = (sev==0 ? 0 : 17);
        }
    }
    ScalarType scalarType = pvScalar->getScalar()->getScalarType();

    switch (scalarType) {
    case pvBoolean:
    {
        //cout << "boolean "<< endl ;
        PVBooleanPtr pvBoolean = std::dynamic_pointer_cast<PVBoolean>(pvScalar);
        bool value  = pvBoolean->get();
        kData.edata.ivalue = (value ? 1 : 0);
        kData.edata.rvalue = (float) kData.edata.ivalue;
        kData.edata.fieldtype  = DBF_LONG;
        kData.edata.valueCount = 1;
        kData.edata.monitorCount++;
    }
        break;
    case pvByte:
    {
        //cout  << "byte " << endl ;
        char value = convert->toByte(pvScalar);
        kData.edata.ivalue = value;
        kData.edata.rvalue = value;
        kData.edata.fieldtype  = DBF_CHAR;
        kData.edata.valueCount = 1;
        kData.edata.monitorCount++;
    }
        break;
    case pvShort:
    case pvInt:
    case pvLong:
    case pvUByte:
    case pvUShort:
    case pvUInt:
    case pvULong:
    {
        //cout  << "all others " << convert->toInt(pvScalar) << endl;
        int32 value = convert->toInt(pvScalar);
        kData.edata.ivalue = value;
        kData.edata.rvalue = value;
        kData.edata.fieldtype  = DBF_LONG;
        kData.edata.valueCount = 1;
        kData.edata.monitorCount++;
    }
        break;
    case pvFloat:
    case pvDouble:
    {
        //cout  << "float & double " << convert->toDouble(pvScalar) << endl;
        double value = convert->toDouble(pvScalar);
        kData.edata.rvalue = value;
        kData.edata.ivalue = value;
        kData.edata.fieldtype  = DBF_DOUBLE;
        kData.edata.valueCount = 1;
        kData.edata.monitorCount++;
    }
        break;
    case pvString:
    {
        //cout  << "string " << endl;
        PVStringPtr pvString = std::dynamic_pointer_cast<PVString>(pvScalar);
        string value = pvString->get();
        int len = value.length();
        const char * data = value.data();
        int length = len + 1;
        if(length>kData.edata.dataSize)
        {
            if(kData.edata.dataSize!=0 && (kData.edata.dataB != (void*) Q_NULLPTR)) free(kData.edata.dataB);
            kData.edata.dataB = (void*) malloc((size_t) length);
        }
        memcpy(kData.edata.dataB,data,len);
        char *yyy = (char *)(kData.edata.dataB);
        yyy[len] = '\0';
        kData.edata.dataSize = length;
        kData.edata.fieldtype  = DBF_STRING;
        kData.edata.valueCount = 1;
        kData.edata.monitorCount++;
    }
        break;
    }

}

void PVAInterface::getEnumData(PVStructurePtr const & pvStructure)
{
    PVFieldPtr pvAlarmField = pvStructure->getSubField<PVStructure>("alarm");
    if(pvAlarmField) {
        Alarm alarm;
        PVAlarm pvAlarm;
        bool result;
        result = pvAlarm.attach(pvAlarmField);
        if(result) {
            pvAlarm.get(alarm);
            int sev = alarm.getSeverity();
            kData.edata.severity = sev;
            kData.edata.status = (sev==0 ? 0 : 17);
        }
    }
    PVIntPtr pvIndex = pvStructure->getSubField<PVInt>("value.index");
    if(!pvIndex) return;
    int32 index = pvIndex->get();
    kData.edata.ivalue = index;
    kData.edata.rvalue = index;
    kData.edata.valueCount = 1;
    kData.edata.monitorCount++;
}

template <typename pureData>
void PVAInterface::fillData(pureData const &array, size_t length, knobData* kPtr) {
    if(length>0) {
        int size = sizeof(array[0]);
        if((length * size) > (size_t) kPtr->edata.dataSize) {
            if(kPtr->edata.dataB != (void*) Q_NULLPTR) free(kPtr->edata.dataB);
            kPtr->edata.dataB = (void*) malloc(length * size);
            kPtr->edata.dataSize = length * size;
        }
        memcpy(kPtr->edata.dataB, &array[0],  length * size);
    }
    kData.edata.valueCount = length;
    kData.edata.monitorCount++;
}


void PVAInterface::getScalarArrayData(PVStructurePtr const & pvStructure)
{
    PVScalarArrayPtr pva = pvStructure->getSubField<PVScalarArray>("value");
    if(!pva) return;
    ScalarArrayConstPtr scalar = pva->getScalarArray();
    ScalarType scalarType = scalar->getElementType();
    int length = pva->getLength();

    switch(scalarType) {

    case pvBoolean:
        message("array of pvBoolean not yet supported", errorMessage);
        break;

    case pvString: {
        if(length<1) {
            kData.edata.fieldtype = DBF_STRING;
            kData.edata.valueCount = 0;
            kData.edata.monitorCount++;
            return;
        }
        PVStringArrayPtr ArrayData = std::static_pointer_cast<PVStringArray> (pva);
        shared_vector<const string> array(ArrayData->view());
        kData.edata.fieldtype = DBF_STRING;
        int numBytes = 0;
        for(int i=0; i<length; ++i)
        {
            numBytes += array[i].length();
        }
        numBytes += length + 1;
        if(numBytes>kData.edata.dataSize) {
            if(kData.edata.dataB != (void*) Q_NULLPTR) free(kData.edata.dataB);
            kData.edata.dataB = (void*) malloc(numBytes);
            kData.edata.dataSize = numBytes;
        }
        char * ptr = static_cast<char*>(kData.edata.dataB);
        for(int i=0; i<length; ++i)
        {
            const string value = array[i];
            int len = value.length();
            memcpy(ptr,value.data(),len);
            ptr += len;
            ptr[0] = '\033'; ++ptr;
        }
        ptr--;
        ptr[0] = '\0';
        kData.edata.valueCount = length;
        kData.edata.monitorCount++;
        return;
    }
    case pvByte: {
        if(length<1) {
            kData.edata.fieldtype = DBF_CHAR;
            kData.edata.valueCount = 0;
            kData.edata.monitorCount++;
            return;
        }
        PVByteArrayPtr ArrayData = std::static_pointer_cast<PVByteArray> (pva);
        shared_vector<const int8> xxx(ArrayData->view());
        kData.edata.fieldtype = DBF_CHAR;
        fillData(xxx, length, &kData);
        return;
    }

    case pvUByte: {
        if(length<1) {
            kData.edata.fieldtype = DBF_CHAR;
            kData.edata.valueCount = 0;
            kData.edata.monitorCount++;
            return;
        }
        PVUByteArrayPtr ArrayData = std::static_pointer_cast<PVUByteArray> (pva);
        shared_vector<const uint8> xxx(ArrayData->view());
        kData.edata.fieldtype = DBF_CHAR;
        fillData(xxx, length, &kData);
        return;
    }

    case pvShort: {
        if(length<1) {
            kData.edata.fieldtype = DBF_INT;
            kData.edata.valueCount = 0;
            kData.edata.monitorCount++;
            return;
        }
        PVShortArrayPtr ArrayData = std::static_pointer_cast<PVShortArray> (pva);
        shared_vector<const int16> xxx(ArrayData->view());
        kData.edata.fieldtype = DBF_INT;
        fillData(xxx, length, &kData);
        return;
    }

    case pvUShort: {
        if(length<1) {
            kData.edata.fieldtype = DBF_INT;
            kData.edata.valueCount = 0;
            kData.edata.monitorCount++;
            return;
        }
        PVUShortArrayPtr ArrayData = std::static_pointer_cast<PVUShortArray> (pva);
        shared_vector<const uint16> xxx(ArrayData->view());
        kData.edata.fieldtype = DBF_INT;
        fillData(xxx, length, &kData);
        return;
    }

    case pvInt: {
        if(length<1) {
            kData.edata.fieldtype = DBF_LONG;
            kData.edata.valueCount = 0;
            kData.edata.monitorCount++;
            return;
        }
        PVIntArrayPtr ArrayData = std::static_pointer_cast<PVIntArray> (pva);
        shared_vector<const int32> xxx(ArrayData->view());
        kData.edata.fieldtype = DBF_LONG;
        fillData(xxx, length, &kData);
        return;
    }

    case pvUInt: {
        if(length<1) {
            kData.edata.fieldtype = DBF_LONG;
            kData.edata.valueCount = 0;
            kData.edata.monitorCount++;
            return;
        }
        PVUIntArrayPtr ArrayData = std::static_pointer_cast<PVUIntArray> (pva);
        shared_vector<const uint32> xxx(ArrayData->view());
        kData.edata.fieldtype = DBF_LONG;
        fillData(xxx, length, &kData);
        return;
    }

    case pvLong:
    case pvULong:
        requester->message("array of pvLong not yet supported", errorMessage);
        return;

    case pvFloat: {
        if(length<1) {
            kData.edata.fieldtype = DBF_FLOAT;
            kData.edata.valueCount = 0;
            kData.edata.monitorCount++;
            return;
        }
        PVFloatArrayPtr ArrayData = std::static_pointer_cast<PVFloatArray> (pva);
        shared_vector<const float> xxx(ArrayData->view());
        kData.edata.fieldtype = DBF_FLOAT;
        fillData(xxx, length, &kData);
        return;
    }


    case pvDouble: {
        if(length<1) {
            kData.edata.fieldtype = DBF_DOUBLE;
            kData.edata.valueCount = 0;
            kData.edata.monitorCount++;
            return;
        }
        PVDoubleArrayPtr ArrayData = std::static_pointer_cast<PVDoubleArray> (pva);
        shared_vector<const double> xxx(ArrayData->view());
        kData.edata.fieldtype = DBF_DOUBLE;
        fillData(xxx, length, &kData);
    }
    }
}

bool PVAInterface::setValue(double rdata, int32_t idata, char *sdata, int forceType)
{
    if(!pvaChannel->getChannel()->isConnected()) {
        string mess("channel ");
        mess += pvaChannel->getFullName() += " not connected";
        requester->message(mess, warningMessage);
        return false;
    }
    string request("value");
    if(normativeType==ntenum_t) request= "value.index";
    if(!pvaChannelPut) {
        PVStructurePtr pvRequest = createRequest->createRequest(request);
        pvaChannelPutRequester = PVAChannelPutRequesterPtr(new PVAChannelPutRequester(shared_from_this()));
        pvaChannelPut = pvaChannel->getChannel()->createChannelPut(pvaChannelPutRequester,pvRequest);
        bool signaled = waitForPutConnect.wait(5.0);
        if (!signaled)
        {
            requester->message("timeout on createChannelPut", errorMessage);
            return false;
        }
    }
    {
        Lock lock(mutex);
        if(Epics4Plugin::getDebug())
            qCDebug(::epics4Log) << "PVAInterface::setValue putFinished" << (putFinished ? "true" : "false");
        if(!putFinished) {
            requester->message("previous put did not complete", errorMessage);
            return false;
        }
        putFinished = false;
    }
    size_t index = putPVStructure->getStructure()->getFieldIndex("value");
    putBitSet->set(index);

    if(normativeType==ntenum_t){
        int enumCount = choices.size();
        for(int32 ind = 0; ind < enumCount; ++ind) {
            const string val = choices[ind];
            if(val.compare(sdata)==0) {
                PVIntPtr pvInt = putPVStructure->getSubField<PVInt>(request);
                pvInt->put(ind);
                pvaChannelPut->put(putPVStructure,putBitSet);
                return true;
            }
        }
        int ind = atoi(sdata);
        if(ind>=0 && ind<enumCount) {
            PVIntPtr pvInt = putPVStructure->getSubField<PVInt>(request);
            size_t index = pvInt->getFieldOffset();
            putBitSet->set(index);
            pvInt->put(ind);
            pvaChannelPut->put(putPVStructure,putBitSet);
            return true;
        }
        putFinished = true;
        return false;
    }

    PVScalarPtr pvScalar = putPVStructure->getSubField<PVScalar>("value");
    ScalarType scalarType = pvScalar->getScalar()->getScalarType();

    if(forceType == 1) scalarType = pvDouble;
    else if(forceType == 2) scalarType = pvInt;

    switch (scalarType) {
    case pvBoolean:
    {
        PVBooleanPtr pvBoolean = std::dynamic_pointer_cast<PVBoolean>(pvScalar);
        bool value  = (idata==0) ? false : true;
        pvBoolean->put(value);
    }
        break;
    case pvByte:
    case pvShort:
    case pvInt:
    case pvLong:
    case pvUByte:
    case pvUShort:
    case pvUInt:
    case pvULong:
    {
        convert->fromInt(pvScalar,idata);
    }
        break;
    case pvFloat:
    case pvDouble:
    {
        convert->fromDouble(pvScalar,rdata);
    }
        break;
    case pvString:
    {
        PVStringPtr pvString = std::dynamic_pointer_cast<PVString>(pvScalar);
        pvString->put(sdata);
    }
        break;
    }
    pvaChannelPut->put(putPVStructure,putBitSet);
    return true;
}

bool PVAInterface::setArrayValue(
        float *fdata, double *ddata,
        int16_t *data16, int32_t *data32,
        char *sdata, int nelm)
{
    if(!pvaChannel->getChannel()->isConnected()) {
        string mess("channel ");
        mess += pvaChannel->getFullName() += " not connected";
        requester->message(mess, warningMessage);
        return false;
    }
    if(!pvaChannelPut) {
        PVStructurePtr pvRequest = createRequest->createRequest("value");
        pvaChannelPutRequester = PVAChannelPutRequesterPtr(new PVAChannelPutRequester(shared_from_this()));
        pvaChannelPut =
                pvaChannel->getChannel()->createChannelPut(pvaChannelPutRequester,pvRequest);
        bool signaled = waitForPutConnect.wait(5.0);
        if (!signaled)
        {
            requester->message("timeout on createChannelPut", errorMessage);
            return false;
        }
    }
    {
        Lock lock(mutex);
        if(Epics4Plugin::getDebug())
            qCDebug(::epics4Log) << "PVAInterface::setArrayValue putFinished" << (putFinished ? "true" : "false");
        if(!putFinished) {
            requester->message("previous put did not complete", errorMessage);
            return false;
        }
        putFinished = false;
    }
    size_t index = putPVStructure->getStructure()->getFieldIndex("value");
    putBitSet->set(index);
    PVScalarArrayPtr pvScalarArray = putPVStructure->getSubField<PVScalarArray>("value");
    ScalarType scalarType = pvScalarArray->getScalarArray()->getElementType();

    switch (scalarType) {
    case pvBoolean:
    {
        requester->message("setArrayValue not supported for elementType pvBoolean",errorMessage);
        return false;
    }
        break;
    case pvByte:
    {
        shared_vector<int8> values(nelm);
        for(int i=0; i<nelm; ++i) values[i] = sdata[i];
        pvScalarArray->putFrom(freeze(values));
    }
        break;
    case pvShort:
    {
        shared_vector<int16> values(nelm);
        for(int i=0; i<nelm; ++i) values[i] = data16[i];
        pvScalarArray->putFrom(freeze(values));
    }
        break;
    case pvInt:
    {
        shared_vector<int32> values(nelm);
        for(int i=0; i<nelm; ++i) values[i] = data32[i];
        pvScalarArray->putFrom(freeze(values));
    }
        break;
    case pvLong:
    {
        shared_vector<int64> values(nelm);
        for(int i=0; i<nelm; ++i) values[i] = data32[i];
        pvScalarArray->putFrom(freeze(values));
    }
        break;
    case pvUByte:
    {
        shared_vector<uint8> values(nelm);
        for(int i=0; i<nelm; ++i) values[i] = sdata[i];
        pvScalarArray->putFrom(freeze(values));
    }
        break;
    case pvUShort:
    {
        shared_vector<uint16> values(nelm);
        for(int i=0; i<nelm; ++i) values[i] = data16[i];
        pvScalarArray->putFrom(freeze(values));
    }
        break;
    case pvUInt:
    {
        shared_vector<uint32> values(nelm);
        for(int i=0; i<nelm; ++i) values[i] = data32[i];
        pvScalarArray->putFrom(freeze(values));
    }
        break;
    case pvULong:
    {
        shared_vector<uint64> values(nelm);
        for(int i=0; i<nelm; ++i) values[i] = data32[i];
        pvScalarArray->putFrom(freeze(values));
    }
        break;
    case pvFloat:
    {
        shared_vector<float> values(nelm);
        for(int i=0; i<nelm; ++i) values[i] = fdata[i];
        pvScalarArray->putFrom(freeze(values));
    }
        break;
    case pvDouble:
    {
        shared_vector<double> values(nelm);
        for(int i=0; i<nelm; ++i) values[i] = ddata[i];
        pvScalarArray->putFrom(freeze(values));
    }
        break;
    case pvString:
    {
        requester->message("setArrayValue not supported for elementType pvString",errorMessage);
        return false;
    }
        break;
    }
    pvaChannelPut->put(putPVStructure,putBitSet);
    return true;
}

bool PVAInterface::getTimeStamp(char *buf)
{
    if(!gotTimeStamp) return false;
    // never processed record, same text as the epics3 plugin
    if(timeStamp.getSecondsPastEpoch() <= (int64) POSIX_TIME_AT_EPICS_EPOCH) {
        qstrncpy(buf, "TimeStamp: <undefined>", TIMESTAMP_STRING_LENGTH);
        return true;
    }
    time_t tt;
    timeStamp.toTime_t(tt);
    struct tm ctm;
    memcpy(&ctm,localtime(&tt),sizeof(struct tm));
    char tsString[32];
    strftime(tsString,32,"%b %d, %Y %H:%M:%S",&ctm);
    // same line format as the epics3 plugin
    int len = snprintf(buf,TIMESTAMP_STRING_LENGTH,"TimeStamp: %s.%09d",tsString,timeStamp.getNanoseconds());
    if(timeStamp.getUserTag() != 0 && len > 0 && len < TIMESTAMP_STRING_LENGTH)
        snprintf(buf + len,TIMESTAMP_STRING_LENGTH - len," tag %d",timeStamp.getUserTag());
    return true;
}

bool PVAInterface::getDescription(char *buf)
{
    qstrncpy(buf, description.c_str(), MAX_STRING_LENGTH);
    return true;
}
}}}

using namespace epics::caqtdm::epics4;

// interface behind edata.info, empty when no monitor was added
static PVAInterfacePtr interfaceOf(void *info)
{
    PVAInterfaceGlue *pvaInterfaceGlue = static_cast<PVAInterfaceGlue *>(info);
    if(!pvaInterfaceGlue) return PVAInterfacePtr();
    return pvaInterfaceGlue->getPVAInterface();
}

// channel name of a map key "provider://name_index"
static string channelNameOf(const string &mapname)
{
    size_t start = mapname.find("://");
    start = (start == string::npos) ? 0 : start + 3;
    size_t end = mapname.rfind('_');
    if(end == string::npos || end < start) end = mapname.length();
    return mapname.substr(start, end - start);
}

bool Epics4Plugin::debug = false;

QString Epics4Plugin::pluginName()
{
    return "epics4";
}

Epics4Plugin::Epics4Plugin()
{
    if(Epics4Plugin::getDebug()) qCInfo(::epics4Log) << "Epics4: Create";
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(closeEvent()));
}

Epics4Plugin::~Epics4Plugin()
{
    if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "Epics4Plugin:~Epics4Plugin";
}

int Epics4Plugin::initCommunicationLayer(MutexKnobData *mutexKnobDataP, MessageWindow *messageWindow, QMap<QString, QString> options)
{
    Q_UNUSED(options);
    if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "Epics4Plugin::initCommunicationLayer";
    QString msg=QString("Epics4Plugin: epics version: %1").arg(EPICS_VERSION_STRING);
    if(messageWindow != (MessageWindow *) Q_NULLPTR) messageWindow->postMsgEvent(QtDebugMsg,(char*) msg.toLatin1().constData());
    mutexKnobData = mutexKnobDataP;
    ClientFactory::start();
    CAClientFactory::start();
    epics4_callbackThread = epics4_CallbackThread::create();
    requester = Epics4RequesterPtr(new Epics4Requester(messageWindow));
    if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "Epics4Plugin::initCommunicationLayer return true";
    return true;

}

int Epics4Plugin::pvAddMonitor(int index, knobData *kData, int rate, int skip)
{
    Q_UNUSED(rate);
    Q_UNUSED(skip);
    if(Epics4Plugin::getDebug()) {
        qCDebug(::epics4Log) << "Epics4Plugin::pvAddMonitor"
             << "pv" << kData->pv << "index" << kData->index;
    }

    PVAInterfaceGlue *pvaInterfaceGlue = static_cast<PVAInterfaceGlue *>(kData->edata.info);
    if(pvaInterfaceGlue) {
        qCCritical(::epics4Log) << "Epics4Plugin::pvAddMonitor already added" << kData->pv;
        return false;
    }
    string channelName(kData->pv);
    string providerName(kData->pluginFlavor);
    string fullname(providerName+"://"+channelName);
    stringstream out;
    out << kData->index;
    string mapname = fullname + "_" + out.str();

    if(Epics4Plugin::getDebug()) {
        qCDebug(::epics4Log) << "providerName" << QString::fromStdString(providerName)
             << "channelName" << QString::fromStdString(channelName)
             << "fullname" << QString::fromStdString(fullname);
    }

#if  EPICS_VERSION > 6
    providerN = ChannelProviderRegistry::clients()->getProvider(providerName);
#else
    providerN = getChannelProviderRegistry();
#endif
    if(!providerN) {
        requester->message(fullname + " provider " + providerName + " not registered",errorMessage);
        return false;
    }

    PVAChannelPtr pvaChannel;
    bool foundit(false);
    /*
    std::map<string,PVAChannelWPtr>::iterator it = pvaChannelMap.find(fullname);
    if(it != pvaChannelMap.end()) {
        pvaChannel = it->second.lock();
        if(pvaChannel) {
            foundit = true;
            if(Epics4Plugin::getDebug()) cout << "found it\n";
        } else {
            pvaChannelMap.erase(it);
            if(Epics4Plugin::getDebug()) cout << "erase it\n";
        }
    }*/
    foundit = false;
    if(!foundit) {
        pvaChannel = PVAChannelPtr(new PVAChannel(fullname, mapname, requester,epics4_callbackThread, providerN));
        if(!pvaChannel->connect(channelName,providerName)) return false;
        pvaChannelMap.insert(std::pair<string,PVAChannelWPtr>(mapname,pvaChannel));
        pvMap.insert(std::pair<string,int>(mapname, kData->index));
        if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "created new and called connect";
    }
    PVAInterfacePtr pvaInterface(new PVAInterface(pvaChannel, mutexKnobData,index,requester,epics4_callbackThread));
    pvaInterfaceGlue = new PVAInterfaceGlue(pvaInterface);
    kData->edata.info = pvaInterfaceGlue;
    C_SetMutexKnobData(mutexKnobData, index, *kData);
    if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "calling addInterface";
    pvaChannel->addInterface(pvaInterface);
    return true;
}

int Epics4Plugin::pvClearMonitor(knobData *kData) {
    if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "Epics4Plugin:pvClearMonitor";
    PVAInterfacePtr pvaInterface = interfaceOf(kData->edata.info);
    if(!pvaInterface) {
        qCDebug(::epics4Log) << "Epics4Plugin::pvClearMonitor no interface for" << kData->pv;
        return false;
    }
    pvaInterface->clearMonitor();
    return true;
}

int Epics4Plugin::pvFreeAllocatedData(knobData *kData)
{
    if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "Epics4Plugin:pvFreeAllocatedData";
    PVAInterfaceGlue *pvaInterfaceGlue  = static_cast<PVAInterfaceGlue *>(kData->edata.info);
    PVAInterfacePtr pvaInterface = interfaceOf(kData->edata.info);
    if(pvaInterface) {
        PVAChannelPtr pvaChannel(pvaInterface->getPVAChannel());
        if(pvaChannel && pvaChannel->removeInterface(pvaInterface)) {
            string mapName = pvaChannel->getMapName();
            pvaChannelMap.erase(mapName);
            pvMap.erase(mapName);
            pvaChannel->destroy();
        }
        pvaInterface->destroy();
    }
    kData->edata.info = Q_NULLPTR;
    delete pvaInterfaceGlue;
    if(kData->edata.dataB != (void*) Q_NULLPTR) {
        if(kData->edata.dataB != (void*) Q_NULLPTR) free(kData->edata.dataB);
        kData->edata.dataB = (void*) Q_NULLPTR;
    }
    return true;
}

bool Epics4Plugin::pvSetValue(knobData *kData,
                              double rdata, int32_t idata,
                              char *sdata, char *object, char *errmess, int forceType)
{
    Q_UNUSED(object);
    Q_UNUSED(errmess);
    Q_UNUSED(forceType);
    if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "Epics4Plugin:pvSetValue";
    PVAInterfacePtr pvaInterface = interfaceOf(kData->edata.info);
    if(!pvaInterface) {
        requester->message(string(kData->pv) + " has no channel, value not written",errorMessage);
        return false;
    }
    return pvaInterface->setValue(rdata,idata,sdata,forceType);
}

bool Epics4Plugin::pvSetWave(knobData *kData,
                             float *fdata, double *ddata,
                             int16_t *data16, int32_t *data32, char *sdata, int nelm,
                             char *object, char *errmess)
{
    Q_UNUSED(object);
    Q_UNUSED(errmess);
    if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "Epics4Plugin:pvSetWave";
    PVAInterfacePtr pvaInterface = interfaceOf(kData->edata.info);
    if(!pvaInterface) {
        requester->message(string(kData->pv) + " has no channel, waveform not written",errorMessage);
        return false;
    }
    return pvaInterface->setArrayValue(fdata,ddata,data16,data32,sdata,nelm);
}

int Epics4Plugin::pvGetTimeStamp(char *pv, char *timestamp)
{
    if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "Epics4Plugin:pvGetTimeStamp";
    map<std::string, int>::iterator it = pvMap.begin();
    while (it != pvMap.end()) {
        if (channelNameOf(it->first) == pv) {
            knobData kData = mutexKnobData->GetMutexKnobData(it->second);
            pvGetTimeStamp(&kData, timestamp);
            break;
        }
        it++;
    }
    return true;
}

bool Epics4Plugin::pvGetTimeStamp(knobData *kData, char *timestamp) {
    if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "Epics4Plugin:pvGetTimeStamp";
    PVAInterfacePtr pvaInterface = interfaceOf(kData->edata.info);
    if(!pvaInterface) {
        qCDebug(::epics4Log) << "Epics4Plugin::pvGetTimeStamp no interface for" << kData->pv;
        return false;
    }
    pvaInterface->getTimeStamp(timestamp);
    return true;
}

int Epics4Plugin::pvGetDescription(char *pv, char *description)
{
    if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "Epics4Plugin:pvGetDescription";
    map<std::string, int>::iterator it = pvMap.begin();
    while (it != pvMap.end()) {
        if (channelNameOf(it->first) == pv) {
            knobData kData = mutexKnobData->GetMutexKnobData(it->second);
            pvGetDescription(&kData, description);
            break;
        }
        it++;
    }
    return true;
}

bool Epics4Plugin::pvGetDescription(knobData *kData, char *description) {
    if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "Epics4Plugin:pvGetDescription";
    PVAInterfacePtr pvaInterface = interfaceOf(kData->edata.info);
    if(!pvaInterface) {
        qCDebug(::epics4Log) << "Epics4Plugin::pvGetDescription no interface for" << kData->pv;
        return false;
    }
    pvaInterface->getDescription(description);
    return true;
}

int Epics4Plugin::pvClearEvent(void * ptr) {
    if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "Epics4Plugin:pvClearEvent";
    PVAInterfacePtr pvaInterface = interfaceOf(ptr);
    if(!pvaInterface) {
        qCDebug(::epics4Log) << "Epics4Plugin::pvClearEvent no interface";
        return false;
    }
    pvaInterface->stopMonitor();

    return true;
}

int Epics4Plugin::pvAddEvent(void * ptr) {
    if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "Epics4Plugin:pvAddEvent";
    PVAInterfacePtr pvaInterface = interfaceOf(ptr);
    if(!pvaInterface) {
        qCDebug(::epics4Log) << "Epics4Plugin::pvAddEvent no interface";
        return false;
    }
    pvaInterface->startMonitor();
    return true;
}

int Epics4Plugin::pvReconnect(knobData *kData) {
    if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "Epics4Plugin:pvReconnect";
    PVAInterfacePtr pvaInterface = interfaceOf(kData->edata.info);
    if(!pvaInterface) {
        qCDebug(::epics4Log) << "Epics4Plugin::pvReconnect no interface for" << kData->pv;
        return false;
    }
    return pvaInterface->reconnect();
}

int Epics4Plugin::pvDisconnect(knobData *kData) {
    if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "Epics4Plugin:pvDisconnect";
    PVAInterfacePtr pvaInterface = interfaceOf(kData->edata.info);
    if(!pvaInterface) {
        qCDebug(::epics4Log) << "Epics4Plugin::pvDisconnect no interface for" << kData->pv;
        return false;
    }
    return pvaInterface->disconnect();
}

int Epics4Plugin::FlushIO() {
    if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "Epics4Plugin:FlushIO";
    ca_flush_io();
    return true;
}

void Epics4Plugin::closeEvent(){
   TerminateIO();
   if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "Epics4Plugin::closeEvent calling ClientFactory::stop();";
   ClientFactory::stop();
   if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "Epics4Plugin::closeEvent calling CAClientFactory::stop();";
   CAClientFactory::stop();
   if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "Epics4Plugin::closeEvent calling epics4_callbackThread->stop();";
   epics4_callbackThread->stop();
   providerN=Q_NULLPTR;

}

int Epics4Plugin::TerminateIO() {
    if(Epics4Plugin::getDebug()) qCDebug(::epics4Log) << "Epics4Plugin::TerminateIO returning true;";
    return true;
}


#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#else
Q_EXPORT_PLUGIN2(Epics4Plugin, Epics4Plugin)
#endif
