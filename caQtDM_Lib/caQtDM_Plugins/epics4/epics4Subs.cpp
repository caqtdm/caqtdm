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

#include <QDebug>

#include <QString>
#include <QStringList>

#include "epics4Subs.h"
#include "pvAccessImpl.h"



#include <epicsStdlib.h>
#include <epicsGetopt.h>
#include <epicsThread.h>
#include <epicsExit.h>


#define DEFAULT_TIMEOUT 3.0
#define DEFAULT_REQUEST "field(value)"
#define DEFAULT_PROVIDER "pva"

string request(DEFAULT_REQUEST);
double timeOut = DEFAULT_TIMEOUT;

class RequesterImpl : public Requester
{
public:
    RequesterImpl(){}
    string getRequesterName()
    {
        static string name("RequesterImpl");
        return name;
    }
    void message(
            string const & message,
            MessageType messageType)
    {
        qDebug("message: '%s'", message.c_str());
    }
    void destroy() {}
};


epics4Subs::epics4Subs(MutexKnobData* mutexKnobData)
{
    pvRequest =  CreateRequest::create()->createRequest(request);
    if(pvRequest.get()==NULL) printf("failed to parse request string\n");

    qDebug() << "client factory start and get provider";
    ClientFactory::start();
    provider = getChannelProviderRegistry()->getProvider("pva");
    m_mutexKnobData = mutexKnobData;

    pvaClient = PvaClient::get("pva ca");
    RequesterPtr requester(new RequesterImpl());
    pvaClient->setRequester(requester);
    pvaClient->message(" epics4Subs::this is a test",infoMessage);

}

epics4Subs:: ~epics4Subs()
{
    foreach (Monitor::shared_pointer monitorGet, monitorArray) {
        qDebug() << "get rid of monitor";
        monitorGet->stop();
    }
    foreach (Channel::shared_pointer channel, channelArray) {
        qDebug() << "get rid of channel";
        channel->destroy();
    }
}

void epics4Subs::CreateAndConnect4(int num, QString pv)
{
    bool allOK = true;
    // create channel
    qDebug() << "epics4Subs::CreateAndConnect4" << pv;
    shared_ptr<MonitorPVRequesterImpl> channelRequesterImpl(new MonitorPVRequesterImpl());
    Channel::shared_pointer channel = provider->createChannel(qasc(pv), channelRequesterImpl);
    channelArray.append(channel);

    channelRequesterImpl = dynamic_pointer_cast<MonitorPVRequesterImpl>(channel->getChannelRequester());
    channelRequesterImpl->defineIndexForKnobData(num, m_mutexKnobData);

    if (channelRequesterImpl->waitUntilConnected(timeOut)) {
        shared_ptr<GetFieldRequesterImpl> getFieldRequesterImpl;

        getFieldRequesterImpl.reset(new GetFieldRequesterImpl(channel));
        channel->getField(getFieldRequesterImpl, "");

        if (getFieldRequesterImpl.get() == 0 || getFieldRequesterImpl->waitUntilFieldGet(timeOut)) {
            shared_ptr<DataMonitorRequesterImpl> monitorRequesterImpl(new DataMonitorRequesterImpl(channel->getChannelName()));
            monitorRequesterImpl->defineIndexForKnobData(num, m_mutexKnobData);
            Monitor::shared_pointer monitorGet = channel->createMonitor(monitorRequesterImpl, pvRequest);
            monitorArray.append(monitorGet);
            allOK &= true;
        } else {
            allOK = false;
            channel->destroy();
            std::cerr << "[" << channel->getChannelName() << "] failed to get channel introspection data" << std::endl;
        }

    } else  {
        allOK = false;
        channel->destroy();
        std::cerr << "[" << channel->getChannelName() << "] connection timeout" << std::endl;
    }
}

void  epics4Subs::Epics4SetValue(char* pv, double rdata, int32_t idata, char *sdata, int forceType)
{
    qDebug() << " epics4Subs::Epics4SetValue " << pv << " with values" << rdata << idata << sdata << forceType;
    try {
        PvaClientChannelPtr pvaChannel = pvaClient->createChannel(pv);
        pvaChannel->connect();
        PvaClientPutPtr pvaPut = pvaChannel->createPut();
        pvaPut->connect();
        PvaClientPutDataPtr pvaPutData = pvaPut->getData();
        PVStructurePtr arg = pvaPutData->getPVStructure();
        ParsePVStructure(arg, rdata, idata, sdata, forceType);
        pvaPut->put();
        pvaPut->destroy();
        pvaChannel->destroy();

    } catch (std::runtime_error e) {
        qDebug() << "channel example exception" << e.what();
    }
}

void epics4Subs::ParsePVStructure(PVStructurePtr const & pvStructure, double rdata, int32_t idata, char *sdata, int forceType)
{
    qDebug() << "epics4Subs::parse structure";
    PVFieldPtrArray const & fieldsData = pvStructure->getPVFields();
    if (fieldsData.size() != 0) {
        size_t length = pvStructure->getStructure()->getNumberFields();
        for(size_t i = 0; i < length; i++) {
            PVFieldPtr fieldField = fieldsData[i];
            Type type = fieldField->getField()->getType();
            if(type==scalar) {
                PVScalarPtr pvScalar = static_pointer_cast<PVScalar>(fieldField);
                QString thisFieldName  = QString::fromStdString(pvScalar->getFieldName());
                if(thisFieldName.contains("value")) {
                    setScalarData(pvScalar, rdata, idata, sdata, forceType);
                    break;
                }
            }
        }
    }
}

void epics4Subs::setScalarData(PVScalarPtr const & pvScalar, double rdata, int32_t idata, char *sdata, int forceType)
{
    qDebug() << "epics4Subs::fromstring";
    ScalarConstPtr scalar = pvScalar->getScalar();
    ScalarType scalarType = scalar->getScalarType();
    switch(scalarType) {
    case pvBoolean: {
        qDebug() << "pvboolean" << idata;
        PVBooleanPtr pv = static_pointer_cast<PVBoolean>(pvScalar);
        pv->put(idata);
        return;
    }
    case pvByte : {
        qDebug() << "pvbyte" << idata;
        PVBytePtr pv = static_pointer_cast<PVByte>(pvScalar);
        int8 value = idata;
        pv->put(value);
        return;
    }
    case pvShort : {
        qDebug() << "pvshort" << idata;
        PVShortPtr pv = static_pointer_cast<PVShort>(pvScalar);
        int16 value = idata;
        pv->put(value);
        return;
    }
    case pvInt : {
        qDebug() << "pvint" << idata;
        PVIntPtr pv = static_pointer_cast<PVInt>(pvScalar);
        pv->put(idata);
        return;
    }
    case pvLong : {
        qDebug() << "pvlong" << idata;
        PVLongPtr pv = static_pointer_cast<PVLong>(pvScalar);
        int64 value = idata;
        pv->put(value);
        return;
    }
    case pvUByte : {
        qDebug() << "pvubyte" << idata;
        PVUBytePtr pv = static_pointer_cast<PVUByte>(pvScalar);
        uint8 value = idata;
        pv->put(value);
        return;
    }
    case pvUShort : {
        qDebug() << "pvushort" << idata;
        PVUShortPtr pv = static_pointer_cast<PVUShort>(pvScalar);
        uint16 value = idata;
        pv->put(value);
        return;
    }
    case pvUInt : {
        qDebug() << "pvuint" << idata;
        PVUIntPtr pv = static_pointer_cast<PVUInt>(pvScalar);
        uint32 value = idata;
        pv->put(value);
        return;
    }
    case pvULong : {
        qDebug() << "pvulong" << idata;
        PVULongPtr pv = static_pointer_cast<PVULong>(pvScalar);
        uint64 value = idata;
        pv->put(value);
        return;
    }
    case pvFloat : {
        qDebug() << "pvfloat" << rdata;
        PVFloatPtr pv = static_pointer_cast<PVFloat>(pvScalar);
        pv->put((float)rdata);
        return;
    }
    case pvDouble : {
        qDebug() << "pvdouble" << rdata;
        PVDoublePtr pv = static_pointer_cast<PVDouble>(pvScalar);
        pv->put((double)rdata);
        return;
    }
    case pvString: {
        qDebug() << "pvstring" << sdata;
        PVStringPtr value = static_pointer_cast<PVString>(pvScalar);
        value->put(sdata);
        return;
    }
    }
    string message("fromString unknown scalarType ");
    throw std::logic_error(message);
}
