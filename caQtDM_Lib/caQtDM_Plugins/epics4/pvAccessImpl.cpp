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

// ChannelRequester.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::string MonitorPVRequesterImpl::getRequesterName()
{
    qDebug() << "MonitorPVRequesterImpl::getRequesterName";
    return "MonitorPVRequesterImpl";
}

void MonitorPVRequesterImpl::message(std::string const & message, MessageType messageType)
{
    std::cout << "[" << getRequesterName() << "] message(" << message << ", " << getMessageTypeName[messageType] << ")"  << std::endl;
}

void MonitorPVRequesterImpl::channelCreated(const epics::pvData::Status& status, Channel::shared_pointer const & channel)
{
    qDebug() << "MonitorPVRequesterImpl::channelCreated";
    if (status.isSuccess()) {
        // show warning
        if (!status.isOK()) {
             std::cout << "[" << channel->getChannelName() << "] channel create: " << status.getMessage()  << std::endl;
        }
    } else {
         std::cout << "[" << channel->getChannelName() << "] failed to create a channel: " << status.getMessage()  << std::endl;
    }
}

void MonitorPVRequesterImpl::channelStateChange(Channel::shared_pointer const & channel, Channel::ConnectionState connectionState)
{
    Q_UNUSED(channel);
    qDebug() << "MonitorPVRequesterImpl::channelStateChange " << m_channelIndex;
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

bool MonitorPVRequesterImpl::waitUntilConnected(double timeOut)
{
     qDebug() << "MonitorPVRequesterImpl::waitUntilConnected";
    return m_event.wait(timeOut);
}

void  MonitorPVRequesterImpl::defineIndexForKnobData(int num, MutexKnobData* mutexKnobData)
{
    qDebug() << "MonitorPVRequesterImpl::defineIndexForKnobData";
    m_channelIndex = num;
    m_mutexData =  mutexKnobData;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
GetFieldRequesterImpl::GetFieldRequesterImpl(epics::pvAccess::Channel::shared_pointer channel) : m_channel(channel)
{
    qDebug() << "GetFieldRequesterImpl::GetFieldRequesterImpl";
}

string GetFieldRequesterImpl::getRequesterName()
{
    return "GetFieldRequesterImpl";
}

void GetFieldRequesterImpl::message(string const & message, MessageType messageType)
{
    std::cerr << "[" << getRequesterName() << "] message(" << message << ", " << getMessageTypeName(messageType) << ")" << std::endl;
}

void GetFieldRequesterImpl::getDone(const epics::pvData::Status& status, epics::pvData::FieldConstPtr const & field)
{
    qDebug() << "GetFieldRequesterImpl getDone";
    if (status.isSuccess()) {
        // show warning
        if (!status.isOK()) {
            std::cerr << "[" << m_channel->getChannelName() << "] getField create: " << status.getMessage() << std::endl;
        }
        // assign smart pointers
        {
            Lock lock(m_pointerMutex);
            m_field = field;
        }
    } else {
        // do not complain about missing field
        std::cerr << "[" << m_channel->getChannelName() << "] failed to get channel introspection data: " << status.getMessage() << std::endl;
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
DataMonitorRequesterImpl::DataMonitorRequesterImpl(string channelName) : m_channelName(QString::fromStdString(channelName))
{
    qDebug() << "DataMonitorRequesterImpl::MonitorRequesterImpl";
}

string  DataMonitorRequesterImpl::getRequesterName()
{
    return "DataMonitorRequesterImpl";
}

void  DataMonitorRequesterImpl::message(string const & message, MessageType messageType)
{
    std::cerr << "[" << getRequesterName() << "] message(" << message << ", " << getMessageTypeName(messageType) << ")" << std::endl;
}

void  DataMonitorRequesterImpl::monitorConnect(const epics::pvData::Status& status, Monitor::shared_pointer const & monitor, StructureConstPtr const & /*structure*/)
{
    qDebug() <<  "DataMonitorRequesterImpl::monitorConnect : monitorConnect";
    if (status.isSuccess()) {
        Status startStatus = monitor->start();
        // show error
        if (!startStatus.isSuccess()) {
            std::cerr << "[" << qasc(m_channelName) << "] channel monitor start: " << startStatus.getMessage() << std::endl;
        }

    } else {
        std::cerr << "DataMonitorRequesterImpl::monitorConnect : monitorConnect(" << status.getMessage() << ")" << std::endl;
    }
}

void  DataMonitorRequesterImpl::monitorEvent(Monitor::shared_pointer const & monitor)
{
    qDebug() << "DataMonitorRequesterImpl::monitorEvent" << m_channelIndex;
    MonitorElement::shared_pointer element;

    knobData *kPtr = m_mutexData->GetMutexKnobDataPtr(m_channelIndex);  // use pointer
    if(kPtr->index == -1) return;
    qDebug() << "DataMonitorRequesterImpl::monitorEvent : connection state" << kPtr->edata.connected;

    while (element = monitor->poll()) {

        PVField::shared_pointer value = element->pvStructurePtr->getSubField("value");
        if (value.get() == 0) {
             std::cout << "DataMonitorRequesterImpl::monitorEvent : no 'value' field" << qasc(m_channelName)  << std::endl;
             std::cout << "DataMonitorRequesterImpl::monitorEvent : " << *(element->pvStructurePtr.get())  << std::endl;
        } else {
            Type valueType = value->getField()->getType();

            // we have a structure
            if (valueType != scalar && valueType != scalarArray) {
                 std::cout << "DataMonitorRequesterImpl::monitorEvent : structure " <<  qasc(m_channelName) << std::endl;
                 std::cout << "DataMonitorRequesterImpl::monitorEvent : " << *(element->pvStructurePtr.get()) << std::endl;

                 // a structure, can be an enum
                 if (valueType == structure) {
                     string id = TR1::static_pointer_cast<PVStructure>(value)->getStructure()->getID();
                     qDebug() << QString::fromStdString(id);

                     if (id == "enum_t") {
                         qDebug() << "I am an enum";

                         PVInt::shared_pointer pvIndex = TR1::static_pointer_cast<PVStructure>(value)->getSubField<PVInt>("index");
                         if (!pvIndex) throw std::runtime_error("enum_t structure does not have 'int index' field");

                         PVStringArray::shared_pointer pvChoices = TR1::static_pointer_cast<PVStructure>(value)->getSubField<PVStringArray>("choices");
                         if (!pvChoices) throw std::runtime_error("enum_t structure does not have 'string choices[]' field");

                         int32 ix = pvIndex->get();
                         int32 len = static_cast<int32>(pvChoices->getLength());

                         if ((ix < 0) || (ix >= len)) {
                             kPtr->edata.rvalue = ix;
                             kPtr->edata.ivalue = ix;

                         } else {
                             // copy the enum value and strings to our array
                             const PVStringArray::const_svector& data(pvChoices->view());
                             QString enumArray;
                             for (int i = 0; i < len; i++) {
                                 QString strng = QString::fromStdString(data[i]);
                                 enumArray.append(strng);
                                 if(i < len-1) enumArray.append( "\033");
                             }

                             int dataSize = enumArray.size();
                             if(dataSize != kPtr->edata.dataSize) {
                                 free(kPtr->edata.dataB);
                                 kPtr->edata.dataB = (void*) malloc((size_t) dataSize);
                                 kPtr->edata.dataSize = dataSize;
                             }
                             strcpy( (char*) kPtr->edata.dataB, qasc(enumArray));
                             kPtr->edata.rvalue = ix;
                             kPtr->edata.ivalue = ix;
                             kPtr->edata.precision = 0;
                             kPtr->edata.units[0] = '\0';
                             kPtr->edata.valueCount = len;
                             kPtr->edata.fieldtype = caENUM;

                         }
                     } else {
                         qDebug() << "epics4 -- only enum_t yet treated";
                     }


                 } else {
                     std::cout << "DataMonitorRequesterImpl::monitorEvent : "  << *(element->pvStructurePtr.get());
                 }




                // we have a scalar value or scalar array
            } else {
                if (value->getField()->getType() == scalar) {
                     std::cout << "DataMonitorRequesterImpl::monitorEvent : scalar "<<  qasc(m_channelName) << std::endl;

                    ParseScalar("", static_pointer_cast<PVScalar>(value), kPtr);
                    qDebug() << "DataMonitorRequesterImpl::monitorEvent : after parsing";

                    PVField::shared_pointer alarm = element->pvStructurePtr->getSubField("alarm");
                    if (alarm.get() != 0) ParsePVStructure(m_channelName, static_pointer_cast<PVStructure>(alarm), kPtr, false);

                    PVField::shared_pointer display = element->pvStructurePtr->getSubField("display");
                    if (display.get() != 0) ParsePVStructure(m_channelName, static_pointer_cast<PVStructure>(display), kPtr, false);

                    PVField::shared_pointer control = element->pvStructurePtr->getSubField("control");
                    if (control.get() != 0) ParsePVStructure(m_channelName, static_pointer_cast<PVStructure>(control), kPtr, false);

                    PVField::shared_pointer valueAlarm = element->pvStructurePtr->getSubField("valueAlarm");
                    if (valueAlarm.get() != 0) ParsePVStructure(m_channelName, static_pointer_cast<PVStructure>(valueAlarm), kPtr, false);

                } else {
                     std::cout <<  "DataMonitorRequesterImpl::monitorEvent :scalar array " << qasc(m_channelName) << " values: "  << std::endl;
                    ParseScalarArray(static_pointer_cast<PVScalarArray>(value), kPtr);
                }
            }
        }
        monitor->release(element);
    }

    struct timeb now;
    ftime(&now);
    kPtr->edata.accessW = true;
    kPtr->edata.accessR = true;
    kPtr->edata.actTime = now;
    kPtr->edata.status = 0;
    kPtr->edata.monitorCount++;
    m_mutexData->SetMutexKnobDataReceived(kPtr);
}

void  DataMonitorRequesterImpl::unlisten(Monitor::shared_pointer const & /*monitor*/)
{
    qDebug() << "DataMonitorRequesterImpl::unlisten";
    std::cerr << "unlisten" << std::endl;
}

void  DataMonitorRequesterImpl::defineIndexForKnobData(int num, MutexKnobData* mutexKnobData)
{
    qDebug() << "DataMonitorRequesterImpl::defineIndexForKnobData" << num;
    m_channelIndex = num;
    m_mutexData =  mutexKnobData;
}

void DataMonitorRequesterImpl::ParsePVStructure(QString fieldName, PVStructure::shared_pointer const & pv, knobData* kPtr, int notFirst)
{
    qDebug() << "DataMonitorRequesterImpl::ParsePVStructure" << fieldName;
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
                qDebug() << "ParsePVStructure (" << notFirst << ") field " << i << " name " << QString::fromStdString(fieldNames[i]);
                epics::pvData::Type subtype =pv->getStructure()->getField(fieldNames[i])->getType();

                if(subtype==structure) {
                    qDebug() << "DataMonitorRequesterImpl::ParsePVStructure type structure";
                    //todoPVStructurePtr subpv = pv->getStructureField(fieldNames[i]);
                    //todoParsePVStructure(subpv, kPtr, notFirst + 1);
                }

                if(subtype==structureArray) {
                    qDebug() << "DataMonitorRequesterImpl::ParsePVStructure type structureArray";
                    //PVStructureArrayPtr subpv = pv->getStructureArrayField(fieldNames[i]);
                    //ParsePVStructure(subpv, kPtr, notFirst + 1);
                }

                if(subtype==scalar) {
                    qDebug() << "DataMonitorRequesterImpl::ParsePVStructure type scalar";
                    PVScalarPtr pvs = static_pointer_cast<PVScalar>(fieldsArr[i]);
                    ParseScalar(fieldName, pvs, kPtr);
                }
                if(subtype==scalarArray) {
                    qDebug() << "DataMonitorRequesterImpl::ParsePVStructure type scalarArray";
                    //PVScalarArrayPtr subpv = pv->getScalarArrayField(fieldNames[i]);
                    //subpv->getField()->getType();
                    //ParsePVStructure(subpv, kPtr ,notFirst + 1);
                }
            }
        }
        return;
    }
    if(type==scalarArray) {
        qDebug() << "DataMonitorRequesterImpl::ParsePVStructure scalarArray";
        //convertArray(static_cast<PVScalarArray *>(pv),notFirst);
        return;
    }
    if(type==structureArray) {
        qDebug() << "DataMonitorRequesterImpl::ParsePVStructure structureArray";
        //convertStructureArray(static_cast<PVStructureArray*>(pv),notFirst);
        return;
    }
}

void DataMonitorRequesterImpl::ParseScalarArray(PVScalarArray::shared_pointer const & pvArray,  knobData* kPtr)
{
    qDebug() << "DataMonitorRequesterImpl::ParseScalarArray";
    /*
    size_t length = pvArray->getLength();
    qDebug() << "lenght of array " << length;
    PVDoubleArrayPtr ArrayData = static_pointer_cast<epics::pvData::PVDoubleArray> (pvArray);
    //DoubleArrayData valuesArrayData;
    PVValueArray<double> valuesArrayData;
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
*/
}

void DataMonitorRequesterImpl::ParseScalar(QString fieldName, PVScalarPtr const & pvs, knobData* kPtr)
{
    QString thisFieldName  = QString::fromStdString(pvs->getFieldName());
    qDebug() << " DataMonitorRequesterImpl::ParseScalar -- field/subfield" << fieldName << thisFieldName;
    ScalarConstPtr scalar = pvs->getScalar();
    ScalarType scalarType = scalar->getScalarType();

    qDebug() << " DataMonitorRequesterImpl::ParseScalar -- value" << scalar;

    switch(scalarType) {
    case pvBoolean: {
        qDebug() << "DataMonitorRequesterImpl::ParseScalar -- is pvBoolean";
        PVBooleanPtr data = static_pointer_cast<PVBoolean>(pvs);

        if(thisFieldName.contains("value")) {
            kPtr->edata.fieldtype = caINT;
            kPtr->edata.ivalue = (int)data->get();
        }
    }
        break;
    case pvByte: {
        qDebug() << "DataMonitorRequesterImpl::ParseScalar -- is pvByte";
        PVBytePtr data = static_pointer_cast<PVByte>(pvs);

        if(thisFieldName.contains("value")) {
            kPtr->edata.fieldtype = caCHAR;
            kPtr->edata.ivalue = (int)data->get();
        }
    }
        break;
    case pvShort:
    case pvUShort: {
        qDebug() << "DataMonitorRequesterImpl::ParseScalar -- is pvShort";
        PVShortPtr data = static_pointer_cast<PVShort>(pvs);

        if(thisFieldName.contains("value")) {
            kPtr->edata.fieldtype = caINT;
            kPtr->edata.rvalue = data->get();
            kPtr->edata.ivalue = (int)data->get();
        }
    }
        break;

    case pvInt: {
        qDebug() << "DataMonitorRequesterImpl::ParseScalar -- is pvInt";
        PVIntPtr data = static_pointer_cast<PVInt>(pvs);

        if(fieldName.contains("alarm") && thisFieldName.contains("severity")) {
            kPtr->edata.severity =(int) data->get();
        } else if(fieldName.contains("alarm") && thisFieldName.contains("status")) {
            kPtr->edata.status =  (int) data->get();
        }

        if(thisFieldName.contains("value")) {
            kPtr->edata.fieldtype = caINT;
            kPtr->edata.rvalue = data->get();
            kPtr->edata.ivalue = (int)data->get();
        }
    }
        break;
    case pvUInt: {
        qDebug() << "DataMonitorRequesterImpl::ParseScalar -- is pvInt";
        PVUIntPtr data = static_pointer_cast<PVUInt>(pvs);

        if(fieldName.contains("alarm") && thisFieldName.contains("severity")) {
            kPtr->edata.severity =(int) data->get();
        } else if(fieldName.contains("alarm") && thisFieldName.contains("status")) {
            kPtr->edata.status =  (int) data->get();
        }

        if(thisFieldName.contains("value")) {
            kPtr->edata.fieldtype = caINT;
            kPtr->edata.rvalue = data->get();
            kPtr->edata.ivalue = (int)data->get();
        }
    }
        break;
    case pvLong:
    case pvULong: {
        qDebug() << "DataMonitorRequesterImpl::ParseScalar -- is pvLong";
        PVLongPtr data = static_pointer_cast<PVLong>(pvs);

        if(thisFieldName.contains("value")) {
            kPtr->edata.fieldtype = caLONG;
            kPtr->edata.rvalue = data->get();
            kPtr->edata.ivalue = (int64)data->get();
        }
    }
        break;
    case pvFloat: {
        qDebug() << "DataMonitorRequesterImpl::ParseScalar -- is pvFloat";
        PVFloatPtr data = static_pointer_cast<PVFloat>(pvs);

        if(thisFieldName.contains("value")) {
            kPtr->edata.fieldtype = caFLOAT;
            kPtr->edata.rvalue = data->get();
        }
    }
        break;
    case pvDouble: {
        qDebug() << "DataMonitorRequesterImpl::ParseScalar -- is pvDouble" << fieldName;
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
            qDebug() << "DataMonitorRequesterImpl::ParseScalar -- is pvDouble with fieldname=" << thisFieldName << data;
            kPtr->edata.precision = 3;  // for the time beeing, set to 3
            kPtr->edata.fieldtype = caDOUBLE;
            kPtr->edata.rvalue = data->get();
        } else {
           qDebug() << "DataMonitorRequesterImpl::ParseScalar -- is pvDouble but fielname is empty";
        }
    }
        break;
    case pvString: {
        qDebug() << "DataMonitorRequesterImpl::ParseScalar -- is pvString";
        PVStringPtr data = static_pointer_cast<PVString>(pvs);

        // fill units
        if(fieldName.contains("display") && thisFieldName.contains("units")) {
            strcpy(kPtr->edata.units, data->get().c_str());
        } else if(fieldName.contains("display") && thisFieldName.contains("description")) {
        } else if(fieldName.contains("display") && thisFieldName.contains("format")) {  // should use now precision

            // fill value and type
        } else if(thisFieldName.contains("value")) {
            qDebug() << "DataMonitorRequesterImpl::ParseScalar String :" << data->get().c_str();
        }
    }
        break;
    default:
        qDebug() << "DataMonitorRequesterImpl::ParseScalar (unknown ScalarType)" << endl;
    }


}

