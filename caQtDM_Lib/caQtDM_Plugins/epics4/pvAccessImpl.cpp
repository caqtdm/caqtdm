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

#include "pvAccessImpl.h"
#include <QDebug>
#include <QString>
#include <QStringList>

using namespace std;
using namespace std::tr1;
using namespace epics::pvData;
using namespace epics::pvAccess;

#define DEFAULT_TIMEOUT 3.0

// Requester.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RequesterImpl::RequesterImpl(String const & requesterName) : m_requesterName(requesterName)
{
}

String RequesterImpl::getRequesterName()
{
    return "RequesterImpl";
}

void RequesterImpl::message(String const & message, MessageType messageType)
{
    std::cerr << "[" << getRequesterName() << "] message(" << message << ", " << getMessageTypeName(messageType) << ")" << std::endl;
}

// ChannelRequester.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
String ChannelRequesterImpl::getRequesterName()
{
    return "ChannelRequesterImpl";
}

void ChannelRequesterImpl::message(String const & message, MessageType messageType)
{
    std::cout << "[" << getRequesterName() << "] message(" << message << ", " << getMessageTypeName[messageType] << ")"  << std::endl;
}

void ChannelRequesterImpl::channelCreated(const epics::pvData::Status& status, Channel::shared_pointer const & channel)
{
    qDebug() << "ChannelRequesterImpl channelCreated";
    if (status.isSuccess()) {
        // show warning
        if (!status.isOK()) {
             std::cout << "[" << channel->getChannelName() << "] channel create: " << status.toString()  << std::endl;
        }
    } else {
         std::cout << "[" << channel->getChannelName() << "] failed to create a channel: " << status.toString()  << std::endl;
    }
}

void ChannelRequesterImpl::channelStateChange(Channel::shared_pointer const & channel, Channel::ConnectionState connectionState)
{
    Q_UNUSED(channel);
    qDebug() << "ChannelRequesterImpl channelStateChange " << m_channelIndex;
    if (connectionState == Channel::CONNECTED) {
         std::cout << channel->getChannelName() << " CONNECTED: " << std::endl;
        m_event.signal();
        m_mutexData->SetMutexKnobDataConnected(m_channelIndex, true);
    } else if (connectionState == Channel::DISCONNECTED) {
         std::cout << channel->getChannelName() << " DISCONNECTED: " <<  std::endl;
        m_event.signal();
        m_mutexData->SetMutexKnobDataConnected(m_channelIndex, false);
    } else {
         std::cout << channel->getChannelName() << " " << Channel::ConnectionStateNames[connectionState] <<  std::endl;
    }
}

bool ChannelRequesterImpl::waitUntilConnected(double timeOut)
{
    return m_event.wait(timeOut);
}

void  ChannelRequesterImpl::setMyData(int num, MutexKnobData* mutexKnobData)
{
    m_channelIndex = num;
    m_mutexData =  mutexKnobData;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
GetFieldRequesterImpl::GetFieldRequesterImpl(epics::pvAccess::Channel::shared_pointer channel) : m_channel(channel)
{
}

String GetFieldRequesterImpl::getRequesterName()
{
    return "GetFieldRequesterImpl";
}

void GetFieldRequesterImpl::message(String const & message, MessageType messageType)
{
    std::cerr << "[" << getRequesterName() << "] message(" << message << ", " << getMessageTypeName(messageType) << ")" << std::endl;
}

void GetFieldRequesterImpl::getDone(const epics::pvData::Status& status, epics::pvData::FieldConstPtr const & field)
{
    qDebug() << "GetFieldRequesterImpl getDone";
    if (status.isSuccess()) {
        // show warning
        if (!status.isOK()) {
            std::cerr << "[" << m_channel->getChannelName() << "] getField create: " << status.toString() << std::endl;
        }
        // assign smart pointers
        {
            Lock lock(m_pointerMutex);
            m_field = field;
        }
    } else {
        // do not complain about missing field
        //std::cerr << "[" << m_channel->getChannelName() << "] failed to get channel introspection data: " << status.toString() << std::endl;
    }
    m_event.signal();
}

bool GetFieldRequesterImpl::waitUntilFieldGet(double timeOut)
{
    return m_event.wait(timeOut);
}

epics::pvData::FieldConstPtr GetFieldRequesterImpl::getField()
{
    Lock lock(m_pointerMutex);
    return m_field;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ChannelPutRequesterImpl::ChannelPutRequesterImpl(String channelName) : m_channelName(channelName)
{
    resetEvent();
}

String ChannelPutRequesterImpl::getRequesterName()
{
    return "ChannelPutRequesterImpl";
}

void ChannelPutRequesterImpl::message(String const & message, MessageType messageType)
{
    std::cerr << "[" << getRequesterName() << "] message(" << message << ", " << getMessageTypeName(messageType) << ")" << std::endl;
}

void ChannelPutRequesterImpl::channelPutConnect(const epics::pvData::Status& status,
                                                        ChannelPut::shared_pointer const & channelPut,
                                                        epics::pvData::PVStructure::shared_pointer const & pvStructure,
                                                        epics::pvData::BitSet::shared_pointer const & bitSet)
{
    if (status.isSuccess()) {
        // show warning
        if (!status.isOK()) {
            std::cerr << "[" << m_channelName << "] channel put create: " << status.toString() << std::endl;
        }

        // assign smart pointers
        {
            Lock lock(m_pointerMutex);
            m_channelPut = channelPut;
            m_pvStructure = pvStructure;
            m_bitSet = bitSet;
        }

        // we always put all
        m_bitSet->set(0);

        // get immediately old value
        channelPut->get();
    } else {
        std::cerr << "[" << m_channelName << "] failed to create channel put: " << status.toString() << std::endl;
    }
}

void ChannelPutRequesterImpl::getDone(const epics::pvData::Status& status)
{
     qDebug() << "ChannelPutRequesterImpl getDone";
    if (status.isSuccess()) {
        // show warning
        if (!status.isOK()) {
            std::cerr << "[" << m_channelName << "] channel get: " << status.toString() << std::endl;
        }

        // access smart pointers
        Lock lock(m_pointerMutex);
        {
            PVField::shared_pointer value = m_pvStructure->getSubField("value");
        }
        m_event->signal();
    } else {
        std::cerr << "[" << m_channelName << "] failed to get: " << status.toString() << std::endl;
    }

}

void ChannelPutRequesterImpl::putDone(const epics::pvData::Status& status)
{
    if (status.isSuccess()) {
        // show warning
        if (!status.isOK()) {
            std::cerr << "[" << m_channelName << "] channel put: " << status.toString() << std::endl;
        }
        m_event->signal();
    } else {
        std::cerr << "[" << m_channelName << "] failed to get: " << status.toString() << std::endl;
    }
}

PVStructure::shared_pointer ChannelPutRequesterImpl::getStructure()
{
    Lock lock(m_pointerMutex);
    return m_pvStructure;
}

void ChannelPutRequesterImpl::resetEvent()
{
    Lock lock(m_eventMutex);
    m_event.reset(new Event());
}

bool ChannelPutRequesterImpl::waitUntilDone(double timeOut)
{
    Event* event;
    {
        Lock lock(m_eventMutex);
        event = m_event.get();
    }
    return event->wait(timeOut);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MonitorRequesterImpl::MonitorRequesterImpl(String channelName) : m_channelName(channelName)
{
    qDebug() << "MonitorRequesterImpl";
}

String  MonitorRequesterImpl::getRequesterName()
{
    return "MonitorRequesterImpl";
}

void  MonitorRequesterImpl::message(String const & message,MessageType messageType)
{
    std::cerr << "[" << getRequesterName() << "] message(" << message << ", " << getMessageTypeName(messageType) << ")" << std::endl;
}

void  MonitorRequesterImpl::monitorConnect(const epics::pvData::Status& status, Monitor::shared_pointer const & monitor, StructureConstPtr const & /*structure*/)
{
    qDebug() << "monitorConnect";
    if (status.isSuccess()) {
        Status startStatus = monitor->start();
        // show error
        if (!startStatus.isSuccess()) {
            std::cerr << "[" << m_channelName << "] channel monitor start: " << startStatus.toString() << std::endl;
        }

    } else {
        std::cerr << "monitorConnect(" << status.toString() << ")" << std::endl;
    }
}

void  MonitorRequesterImpl::monitorEvent(Monitor::shared_pointer const & monitor)
{
    qDebug() << "monitorEvent " << m_channelIndex;
    MonitorElement::shared_pointer element;

    knobData *kPtr = m_mutexData->GetMutexKnobDataPtr(m_channelIndex);  // use pointer
    if(kPtr->index == -1) return;
    qDebug() << "connection state" << kPtr->edata.connected;

    while (element = monitor->poll()) {

        PVField::shared_pointer value = element->pvStructurePtr->getSubField("value");
        if (value.get() == 0) {
             std::cout <<  "no 'value' field" << m_channelName  << std::endl;
             std::cout << *(element->pvStructurePtr.get())  << std::endl;
        } else {
            Type valueType = value->getField()->getType();

            // we have a structure
            if (valueType != scalar && valueType != scalarArray) {
                 std::cout << "structure " <<  m_channelName << std::endl;
                 std::cout << *(element->pvStructurePtr.get()) << std::endl;

                // we have a scalar value or scalar array
            } else {
                if (value->getField()->getType() == scalar) {
                     std::cout << "scalar "<<  m_channelName << std::endl;

                    ParseScalar("", static_pointer_cast<PVScalar>(value), kPtr);

                    PVField::shared_pointer alarm = element->pvStructurePtr->getSubField("alarm");
                    if (alarm.get() != 0) ParsePVStructure(static_pointer_cast<PVStructure>(alarm), kPtr, false);

                    PVField::shared_pointer display = element->pvStructurePtr->getSubField("display");
                    if (display.get() != 0) ParsePVStructure(static_pointer_cast<PVStructure>(display), kPtr, false);

                    PVField::shared_pointer control = element->pvStructurePtr->getSubField("control");
                    if (control.get() != 0) ParsePVStructure(static_pointer_cast<PVStructure>(control), kPtr, false);

                    PVField::shared_pointer valueAlarm = element->pvStructurePtr->getSubField("valueAlarm");
                    if (valueAlarm.get() != 0) ParsePVStructure(static_pointer_cast<PVStructure>(valueAlarm), kPtr, false);

                } else {
                     std::cout <<  "scalar array " << m_channelName << " values: "  << std::endl;
                    ParseScalarArray(static_pointer_cast<PVScalarArray>(value), kPtr);
                }
            }
        }
    }

    kPtr->edata.accessW = true;
    kPtr->edata.accessR = true;
    kPtr->edata.monitorCount++;
    strcpy(kPtr->edata.fec, "EPICS4");
    m_mutexData->SetMutexKnobDataReceived(kPtr);

    monitor->release(element);
}

void  MonitorRequesterImpl::unlisten(Monitor::shared_pointer const & /*monitor*/)
{
    std::cerr << "unlisten" << std::endl;
}

void  MonitorRequesterImpl::setMyData(int num, MutexKnobData* mutexKnobData)
{
    m_channelIndex = num;
    m_mutexData =  mutexKnobData;
}

void MonitorRequesterImpl::ParsePVStructure(epics::pvData::PVStructure::shared_pointer const & pv, knobData* kPtr, int notFirst)
{
    Type type = pv->getField()->getType();
    QString tstr = QString(pv->getField()->getID().c_str());

    if(type==structure)
    {
        QString fieldName  = QString::fromStdString(pv->getFieldName());
        FieldConstPtrArray fieldsData = pv->getStructure()->getFields();
        PVFieldPtrArray const & fieldsArr = pv->getPVFields();

        if (!fieldsData.empty()) {
            int length = pv->getStructure()->getNumberFields();
            StringArray const & fieldNames = pv->getStructure()->getFieldNames();
            for(int i=0; i<length; i++) {
                //qDebug() << "ParsePVStructure (" << notFirst << ") field " << i << " name " << fieldNames[i];
                epics::pvData::Type subtype =pv->getStructure()->getField(fieldNames[i])->getType();

                if(subtype==structure) {
                    //qDebug() << " type structure";
                    PVStructurePtr subpv = pv->getStructureField(fieldNames[i]);
                    ParsePVStructure(subpv, kPtr, notFirst + 1);
                }

                if(subtype==structureArray) {
                    //qDebug() << " type structureArray";
                    //PVStructureArrayPtr subpv = pv->getStructureArrayField(fieldNames[i]);
                    //ParsePVStructure(subpv, kPtr, notFirst + 1);
                }

                if(subtype==scalar) {
                    //qDebug() << " type scalar";
                    PVScalarPtr pvs = static_pointer_cast<PVScalar>(fieldsArr[i]);
                    ParseScalar(fieldName, pvs, kPtr);
                }
                if(subtype==scalarArray) {
                    //qDebug() << " type scalarArray";
                    //PVScalarArrayPtr subpv = pv->getScalarArrayField(fieldNames[i]);
                    //subpv->getField()->getType();
                    //ParsePVStructure(subpv, kPtr ,notFirst + 1);
                }
            }
        }
        return;
    }
    if(type==scalarArray) {
        qDebug() << "scalarArray";
        //convertArray(static_cast<PVScalarArray *>(pv),notFirst);
        return;
    }
    if(type==structureArray) {
        qDebug() << "structureArray";
        //convertStructureArray(static_cast<PVStructureArray*>(pv),notFirst);
        return;
    }
}

void MonitorRequesterImpl::ParseScalarArray(PVScalarArray::shared_pointer const & pvArray,  knobData* kPtr)
{
    size_t length = pvArray->getLength();
    qDebug() << "lenght of array " << length;
    PVDoubleArrayPtr ArrayData = static_pointer_cast<epics::pvData::PVDoubleArray> (pvArray);
    DoubleArrayData valuesArrayData;
    int valuesLength = ArrayData->get(0, ArrayData->getLength(), valuesArrayData);

    if((double) (valuesLength * sizeof(double)) != kPtr->edata.dataSize) {
        free(kPtr->edata.dataB);
        kPtr->edata.dataB = (void*) malloc(valuesLength * sizeof(double));
        kPtr->edata.dataSize = valuesLength * sizeof(double);
    }
    kPtr->edata.valueCount = valuesLength;
    kPtr->edata.precision = 3;  // for the time beeing, set to 3
    kPtr->edata.fieldtype = caDOUBLE;
    memcpy(kPtr->edata.dataB, &valuesArrayData.data[0],  valuesLength * sizeof(double));

}

void MonitorRequesterImpl::ParseScalar(QString fieldName, PVScalarPtr const & pvs, knobData* kPtr)
{
    QString thisFieldName  = QString::fromStdString(pvs->getFieldName());
    //qDebug() << "ParseScalar -- field/subfield" << fieldName << thisFieldName;
    ScalarConstPtr scalar = pvs->getScalar();
    ScalarType scalarType = scalar->getScalarType();

    switch(scalarType) {
    case pvBoolean: {
        PVBooleanPtr data = static_pointer_cast<PVBoolean>(pvs);

        if(thisFieldName.contains("value")) {
            kPtr->edata.fieldtype = caINT;
            kPtr->edata.ivalue = (int)data->get();
        }
    }
        break;
    case pvByte: {
        PVBytePtr data = static_pointer_cast<PVByte>(pvs);

        if(thisFieldName.contains("value")) {
            kPtr->edata.fieldtype = caCHAR;
            kPtr->edata.ivalue = (int)data->get();
        }
    }
        break;
    case pvShort: {
        PVShortPtr data = static_pointer_cast<PVShort>(pvs);

        if(thisFieldName.contains("value")) {
            kPtr->edata.fieldtype = caINT;
            kPtr->edata.rvalue = data->get();
            kPtr->edata.ivalue = (int)data->get();
        }
    }
        break;
    case pvInt: {
        PVIntPtr data = static_pointer_cast<PVInt>(pvs);

        if(fieldName.contains("alarm") && thisFieldName.contains("severity")) {
            kPtr->edata.severity =(int) data->get();
        } else if(fieldName.contains("alarm") && thisFieldName.contains("status")) {
            kPtr->edata.status =  (int) data->get();


        } if(thisFieldName.contains("value")) {
            kPtr->edata.fieldtype = caINT;
            kPtr->edata.rvalue = data->get();
            kPtr->edata.ivalue = (int)data->get();
        }
    }
        break;
    case pvLong: {
        PVLongPtr data = static_pointer_cast<PVLong>(pvs);

        if(thisFieldName.contains("value")) {
            kPtr->edata.fieldtype = caLONG;
            kPtr->edata.rvalue = data->get();
            kPtr->edata.ivalue = (int64)data->get();
        }
    }
        break;
    case pvFloat: {
        PVFloatPtr data = static_pointer_cast<PVFloat>(pvs);

        if(thisFieldName.contains("value")) {
            kPtr->edata.fieldtype = caFLOAT;
            kPtr->edata.rvalue = data->get();
        }
    }
        break;
    case pvDouble: {
        PVDoublePtr data = static_pointer_cast<PVDouble>(pvs);

        if(fieldName.contains("display") && thisFieldName.contains("limitLow")) {
            kPtr->edata.lower_disp_limit = data->get();
        } else if(fieldName.contains("display") && thisFieldName.contains("limitHigh")) {
            kPtr->edata.upper_disp_limit = data->get();

        } else if(fieldName.contains("control") && thisFieldName.contains("limitLow")) {
            kPtr->edata.lower_ctrl_limit = data->get();
        } else if(fieldName.contains("control") && thisFieldName.contains("limitHigh")) {
            kPtr->edata.upper_ctrl_limit = data->get();

        } else if(fieldName.contains("valueAlarm") && thisFieldName.contains("lowAlarmLimit")) {
            kPtr->edata.lower_alarm_limit = data->get();
        } else if(fieldName.contains("valueAlarm") && thisFieldName.contains("highAlarmLimit")) {
            kPtr->edata.upper_alarm_limit = data->get();

        } else if(fieldName.contains("valueAlarm") && thisFieldName.contains("lowWarningLimit")) {
            kPtr->edata.lower_warning_limit = data->get();
        } else if(fieldName.contains("valueAlarm") && thisFieldName.contains("highWarningLimit")) {
            kPtr->edata.upper_warning_limit = data->get();

            // fill value and type
        } else if(thisFieldName.contains("value")) {
            kPtr->edata.precision = 3;  // for the time beeing, set to 3
            kPtr->edata.fieldtype = caDOUBLE;
            kPtr->edata.rvalue = data->get();
        }
    }
        break;
    case pvString: {
        PVStringPtr data = static_pointer_cast<PVString>(pvs);

        // fill units
        if(fieldName.contains("display") && thisFieldName.contains("units")) {
            strcpy(kPtr->edata.units, data->get().c_str());
        } else if(fieldName.contains("display") && thisFieldName.contains("description")) {
        } else if(fieldName.contains("display") && thisFieldName.contains("format")) {  // should use now precision

            // fill value and type
        } else if(thisFieldName.contains("value")) {
            qDebug() << "String :" << data->get().c_str();
        }
    }
        break;
    default:
        qDebug() << "(unknown ScalarType)" << endl;
    }


}

