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

#include "epics4Subs.h"
#include <epicsStdlib.h>
#include <epicsGetopt.h>
#include <epicsThread.h>
#include <epicsExit.h>

#define DEFAULT_TIMEOUT 1.0
double timeOut = DEFAULT_TIMEOUT;

epics4Subs::epics4Subs(MutexKnobData* mutexKnobData)
{
    pvRequest =  CreateRequest::create()->createRequest("field(value,alarm.timestamp,display,control,valueAlarm)");
    if(pvRequest.get()==NULL) printf("failed to parse request string\n");

    ClientFactory::start();
    provider = getChannelProviderRegistry()->getProvider("pva");
    m_mutexKnobData = mutexKnobData;

    pvaClient = PvaClient::get("pva");
}

epics4Subs:: ~epics4Subs()
{
    foreach (Monitor::shared_pointer monitorGet, monitorArray) {
        std::cout << "get rid of monitor" << std::endl;
        monitorGet->stop();
    }
    foreach (Channel::shared_pointer channel, channelArray) {
        std::cout << "get rid of channel" << std::endl;
        channel->destroy();
    }
}

void epics4Subs::CreateAndConnect4(int num, char *pv)
{
    bool allOK = true;
    // create channel
    //std::cout << "epics4Subs::CreateAndConnect4 " << pv << std::endl;
    shared_ptr<MonitorPVRequesterImpl> channelRequesterImpl(new MonitorPVRequesterImpl());
    Channel::shared_pointer channel = provider->createChannel(pv, channelRequesterImpl);
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

int  epics4Subs::EpicsGetDescription(char *pv, char *description)
{
        PvaClientChannelPtr pvaChannel = pvaClient->createChannel(pv);

        pvaChannel->issueConnect();
        Status status = pvaChannel->waitConnect(2.0);
        if(!status.isOK()) {cout << " connect failed\n"; return false;}
        PvaClientGetPtr pvaGet = pvaChannel->createGet("field(display)");
        pvaGet->issueConnect();
        status = pvaGet->waitConnect();
        if(!status.isOK()) {cout << " createGet failed\n"; return false;}

        PvaClientGetDataPtr pvaData = pvaGet->getData();
        PVStructurePtr pvStructure = pvaData->getPVStructure();
        PVFieldPtrArray const & fieldsData = pvStructure->getPVFields();
        if (fieldsData.size() != 0) {
            size_t length = pvStructure->getStructure()->getNumberFields();
            for(size_t i = 0; i < length; i++) {
                PVFieldPtr fieldField = fieldsData[i];
                Type type = fieldField->getField()->getType();
                if(type == structure) {
                    PVStructurePtr pv = TR1::static_pointer_cast<PVStructure>(fieldField);
                    PVStringPtr pvUnits = static_pointer_cast<PVStructure>(pv)->getSubField<PVString>("units");
                    PVStringPtr pvFormat = static_pointer_cast<PVStructure>(pv)->getSubField<PVString>("format");
                    PVStringPtr pvDescription = static_pointer_cast<PVStructure>(pv)->getSubField<PVString>("description");
                    string strng = (string) pvDescription->get();
                    strcpy(description, strng.c_str());
                }
            }
        }
}

void  epics4Subs::Epics4SetValue(char* pv, double rdata, int32_t idata, char *sdata, int forceType)
{
    PvaClientPutPtr pvaPut;
    PVStructurePtr arg;
    try {

        if(QString(pv) != lastWrittenPV) {
            lastWrittenPV = QString(pv);
            std::cout << " epics4Subs::Epics4SetValue 1 " << pv << " with value " << rdata << " " << idata << " " << sdata << " " << forceType << std::endl;
            pvaChannel = pvaClient->createChannel(pv);
            pvaPut = pvaChannel->createPut();
            pvaPut->get();
            PvaClientPutDataPtr pvaPutData = pvaPut->getData();
            arg = pvaPutData->getPVStructure();
        } else {
            pvaChannel->waitConnect(timeOut);
            std::cout << " epics4Subs::Epics4SetValue 2 " << pv << " with value " << rdata << " " << idata << " " << sdata << " " << forceType << std::endl;
            pvaPut = pvaChannel->createPut();
            pvaPut->get();
            PvaClientPutDataPtr pvaPutData = pvaPut->getData();
            arg = pvaPutData->getPVStructure();
        }

        Epics4SetData(arg, rdata, idata, sdata, forceType);
        pvaPut->put();

    } catch (std::runtime_error e) {
        std::cout << "channel example exception " << e.what() << std::endl;
    }
}

void epics4Subs::Epics4SetData(PVStructurePtr const & pvStructure, double rdata, int32_t idata, char *sdata, int forceType)
{   
    //std::cout << "epics4Subs::Epics4SetData " << std::endl;

    PVFieldPtrArray const & fieldsData = pvStructure->getPVFields();
    if (fieldsData.size() != 0) {
        size_t length = pvStructure->getStructure()->getNumberFields();
        for(size_t i = 0; i < length; i++) {
            PVFieldPtr fieldField = fieldsData[i];
            Type type = fieldField->getField()->getType();

            if(type == structure) {

                // treat enum structure
                PVStructurePtr pv = TR1::static_pointer_cast<PVStructure>(fieldField);
                string id = pv->getStructure()->getID();
                if (id == "enum_t") {
                    int32 index = -1;

                    PVInt::shared_pointer pvIndex = pv->getSubField<PVInt>("index");
                    if (!pvIndex) throw std::runtime_error("enum_t structure does not have 'int index' field");

                    PVStringArray::shared_pointer pvChoices = pv->getSubField<PVStringArray>("choices");
                    if (!pvChoices) throw std::runtime_error("enum_t structure does not have 'string choices[]' field");
                    PVStringArray::const_svector choices(pvChoices->view());

                    shared_vector<string>::const_iterator it = std::find(choices.begin(), choices.end(), sdata);
                    if (it != choices.end())index = static_cast<int32>(it - choices.begin());
                    if(index != -1) pvIndex->put(index);
                }


            } else if(type==scalar) {
                PVScalarPtr pvScalar = static_pointer_cast<PVScalar>(fieldField);
                QString thisFieldName  = QString::fromStdString(pvScalar->getFieldName());
                if(thisFieldName.contains("value")) {
                    setScalarData(pvScalar, rdata, idata, sdata, forceType);
                    break;
                }
            }  else if(type==scalarArray) {
                std::cout << "DataMonitorRequesterImpl::ParsePVStructure scalarArray" << std::endl;
                //convertArray(static_cast<PVScalarArray *>(pv),notFirst);
                return;
            }
            else if(type==structureArray) {
                std::cout << "DataMonitorRequesterImpl::ParsePVStructure structureArray" << std::endl;
                //convertStructureArray(static_cast<PVStructureArray*>(pv),notFirst);
                return;
            }
        }
    }
}

void epics4Subs::setScalarData(PVScalarPtr const & pvScalar, double rdata, int32_t idata, char *sdata, int forceType)
{
    //std::cout << "epics4Subs::setScalarData " << std::endl;
    ScalarConstPtr scalar = pvScalar->getScalar();
    ScalarType scalarType = scalar->getScalarType();
    switch(scalarType) {
    case pvBoolean: {
        //std::cout << "pvboolean " << idata << std::endl;
        PVBooleanPtr pv = static_pointer_cast<PVBoolean>(pvScalar);
        pv->put(idata);
        return;
    }
    case pvByte : {
        //std::cout << "pvbyte " << idata << std::endl;
        PVBytePtr pv = static_pointer_cast<PVByte>(pvScalar);
        int8 value = idata;
        pv->put(value);
        return;
    }
    case pvShort : {
        //std::cout << "pvshort " << idata << std::endl;
        PVShortPtr pv = static_pointer_cast<PVShort>(pvScalar);
        int16 value = idata;
        pv->put(value);
        return;
    }
    case pvInt : {
        //std::cout << "pvint " << idata << std::endl;
        PVIntPtr pv = static_pointer_cast<PVInt>(pvScalar);
        int32 value = idata;
        pv->put(value);
        return;
    }
    case pvLong : {
        //std::cout << "pvlong " << idata << std::endl;
        PVLongPtr pv = static_pointer_cast<PVLong>(pvScalar);
        int64 value = idata;
        pv->put(value);
        return;
    }
    case pvUByte : {
        //std::cout << "pvubyte " << idata << std::endl;
        PVUBytePtr pv = static_pointer_cast<PVUByte>(pvScalar);
        uint8 value = idata;
        pv->put(value);
        return;
    }
    case pvUShort : {
        //std::cout << "pvushort " << idata << std::endl;
        PVUShortPtr pv = static_pointer_cast<PVUShort>(pvScalar);
        uint16 value = idata;
        pv->put(value);
        return;
    }
    case pvUInt : {
        //std::cout << "pvuint " << idata << std::endl;
        PVUIntPtr pv = static_pointer_cast<PVUInt>(pvScalar);
        uint32 value = idata;
        pv->put(value);
        return;
    }
    case pvULong : {
        //std::cout << "pvulong " << idata << std::endl;
        PVULongPtr pv = static_pointer_cast<PVULong>(pvScalar);
        uint64 value = idata;
        pv->put(value);
        return;
    }
    case pvFloat : {
        //std::cout << "pvfloat " << rdata << std::endl;
        PVFloatPtr pv = static_pointer_cast<PVFloat>(pvScalar);
        pv->put((float)rdata);
        return;
    }
    case pvDouble : {
        //std::cout << "pvdouble " << rdata << std::endl;
        PVDoublePtr pv = static_pointer_cast<PVDouble>(pvScalar);
        pv->put((double)rdata);
        return;
    }
    case pvString: {
        //std::cout << "pvstring " << sdata << std::endl;
        PVStringPtr value = static_pointer_cast<PVString>(pvScalar);
        value->put(sdata);
        return;
    }
    }
    string message("fromString unknown scalarType ");
    throw std::logic_error(message);
}

