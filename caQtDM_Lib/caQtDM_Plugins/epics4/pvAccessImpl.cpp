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

using namespace std;
using namespace std::tr1;
using namespace epics::pvData;
using namespace epics::pvAccess;

#include <QDebug>

#define DEFAULT_TIMEOUT 3.0

// ChannelRequester.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string MonitorPVRequesterImpl::getRequesterName()
{
    std::cout << "MonitorPVRequesterImpl::getRequesterName" <<  std::endl;
    return "MonitorPVRequesterImpl";
}

void MonitorPVRequesterImpl::message(std::string const & message, MessageType messageType)
{
    std::cout << "[" << getRequesterName() << "] message(" << message << ", " << getMessageTypeName[messageType] << ")"  << std::endl;
}

void MonitorPVRequesterImpl::channelCreated(const epics::pvData::Status& status, Channel::shared_pointer const & channel)
{
    std::cout << "MonitorPVRequesterImpl::channelCreated ============================================" <<  std::endl;
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
    connected = false;
    std::cout << "MonitorPVRequesterImpl::channelStateChange " << m_channelIndex <<  std::endl;
    if (connectionState == Channel::CONNECTED) {
        std::cout << channel->getChannelName() << " CONNECTED: " << std::endl;
        m_event.signal();
        m_mutexData->SetMutexKnobDataConnected(m_channelIndex, true);
        connected = true;
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
    std::cout << "MonitorPVRequesterImpl::waitUntilConnected" <<  std::endl;
    return m_event.wait(timeOut);
}

void  MonitorPVRequesterImpl::defineIndexForKnobData(int num, MutexKnobData* mutexKnobData)
{
    std::cout << "MonitorPVRequesterImpl::defineIndexForKnobData" <<  std::endl;
    m_channelIndex = num;
    m_mutexData =  mutexKnobData;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
GetFieldRequesterImpl::GetFieldRequesterImpl(epics::pvAccess::Channel::shared_pointer channel) : m_channel(channel)
{
    std::cout << "GetFieldRequesterImpl::GetFieldRequesterImpl" <<  std::endl;
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
    std::cout << "GetFieldRequesterImpl getDone" <<  std::endl;
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
DataMonitorRequesterImpl::DataMonitorRequesterImpl(string channelName) : m_channelName(channelName)
{
    std::cout << "DataMonitorRequesterImpl::MonitorRequesterImpl" <<  std::endl;
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
    std::cout <<  "DataMonitorRequesterImpl::monitorConnect : monitorConnect" <<  std::endl;
    if (status.isSuccess()) {
        Status startStatus = monitor->start();
        // show error
        if (!startStatus.isSuccess()) {
            std::cerr << "[" << m_channelName << "] channel monitor start: " << startStatus.getMessage() << std::endl;
        }

    } else {
        std::cerr << "DataMonitorRequesterImpl::monitorConnect : monitorConnect(" << status.getMessage() << ")" << std::endl;
    }
}

void  DataMonitorRequesterImpl::monitorEvent(Monitor::shared_pointer const & monitor)
{
    //std::cout << "DataMonitorRequesterImpl::monitorEvent " << m_channelIndex <<  std::endl;
    MonitorElement::shared_pointer element;

    knobData *kPtr = m_mutexData->GetMutexKnobDataPtr(m_channelIndex);  // use pointer
    if(kPtr->index == -1) return;
    //std::cout << "DataMonitorRequesterImpl::monitorEvent : connection state " << kPtr->edata.connected <<  std::endl;

    while (element = monitor->poll()) {

        PVField::shared_pointer value = element->pvStructurePtr->getSubField("value");
        if (value.get() == 0) {
            //std::cout << "DataMonitorRequesterImpl::monitorEvent : no 'value' field " << m_channelName  << std::endl;
            //std::cout << "DataMonitorRequesterImpl::monitorEvent : " << *(element->pvStructurePtr.get())  << std::endl;
        } else {
            Type valueType = value->getField()->getType();

            // we have a structure
            if (valueType != scalar && valueType != scalarArray) {
                //std::cout << "DataMonitorRequesterImpl::monitorEvent : structure " <<  m_channelName << std::endl;
                //std::cout << "DataMonitorRequesterImpl::monitorEvent : " << *(element->pvStructurePtr.get()) << std::endl;

                // a structure, can be an enum
                if (valueType == structure) {
                    string id = TR1::static_pointer_cast<PVStructure>(value)->getStructure()->getID();

                    if (id == "enum_t") {
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
                            kPtr->edata.enumCount = len;
                            kPtr->edata.fieldtype = caENUM;

                        }
                    } else {
                        std::cout << "epics4 -- only enum_t yet treated" <<  std::endl;
                    }


                } else {
                    std::cout << "DataMonitorRequesterImpl::monitorEvent : "  << *(element->pvStructurePtr.get()) <<  std::endl;
                }




                // we have a scalar value or scalar array
            } else {
                if (value->getField()->getType() == scalar) {
                    //std::cout << "DataMonitorRequesterImpl::monitorEvent : scalar "<<  m_channelName << std::endl;

                    ParseScalar("", static_pointer_cast<PVScalar>(value), kPtr, _alarm);

                    PVField::shared_pointer alarm = element->pvStructurePtr->getSubField("alarm");
                    if (alarm.get() != 0) ParsePVStructure(m_channelName, static_pointer_cast<PVStructure>(alarm), kPtr, false, _alarm);

                    PVField::shared_pointer display = element->pvStructurePtr->getSubField("display");
                    if (display.get() != 0) ParsePVStructure(m_channelName, static_pointer_cast<PVStructure>(display), kPtr, false, _display);

                    PVField::shared_pointer control = element->pvStructurePtr->getSubField("control");
                    if (control.get() != 0) ParsePVStructure(m_channelName, static_pointer_cast<PVStructure>(control), kPtr, false, _control);

                    PVField::shared_pointer valueAlarm = element->pvStructurePtr->getSubField("valueAlarm");
                    if (valueAlarm.get() != 0) ParsePVStructure(m_channelName, static_pointer_cast<PVStructure>(valueAlarm), kPtr, false, _valuealarm);

                } else {
                    std::cout <<  "DataMonitorRequesterImpl::monitorEvent :scalar array " << m_channelName << " values: "  << std::endl;
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
    std::cout << "DataMonitorRequesterImpl::unlisten" <<  std::endl;
    std::cerr << "unlisten" << std::endl;
}

void  DataMonitorRequesterImpl::defineIndexForKnobData(int num, MutexKnobData* mutexKnobData)
{
    std::cout << "DataMonitorRequesterImpl::defineIndexForKnobData" << num <<  std::endl;
    m_channelIndex = num;
    m_mutexData =  mutexKnobData;
}

void DataMonitorRequesterImpl::ParsePVStructure(string fieldName, PVStructure::shared_pointer const & pv, knobData* kPtr, int notFirst, limitsType limits)
{
    //std::cout << "DataMonitorRequesterImpl::ParsePVStructure" << fieldName <<  std::endl;
    Type type = pv->getField()->getType();

    if(type==structure)
    {
        FieldConstPtrArray fieldsData = pv->getStructure()->getFields();
        PVFieldPtrArray const & fieldsArr = pv->getPVFields();

        if (!fieldsData.empty()) {
            int length = pv->getStructure()->getNumberFields();
            StringArray const & fieldNames = pv->getStructure()->getFieldNames();
            for(int i=0; i<length; i++) {
                //std::cout << "ParsePVStructure (" << notFirst << ") field " << i << " name " << fieldNames[i] <<  std::endl;
                epics::pvData::Type subtype =pv->getStructure()->getField(fieldNames[i])->getType();

                if(subtype==structure) {
                    std::cout << "DataMonitorRequesterImpl::ParsePVStructure type structure" <<  std::endl;
                    //todoPVStructurePtr subpv = pv->getStructureField(fieldNames[i]);
                    //todoParsePVStructure(subpv, kPtr, notFirst + 1);
                }

                if(subtype==structureArray) {
                    std::cout << "DataMonitorRequesterImpl::ParsePVStructure type structureArray" <<  std::endl;
                    //PVStructureArrayPtr subpv = pv->getStructureArrayField(fieldNames[i]);
                    //ParsePVStructure(subpv, kPtr, notFirst + 1);
                }

                if(subtype==scalar) {
                    //std::cout << "DataMonitorRequesterImpl::ParsePVStructure type scalar" <<  std::endl;
                    PVScalarPtr pvs = static_pointer_cast<PVScalar>(fieldsArr[i]);
                    ParseScalar(fieldName, pvs, kPtr, limits);
                }
                if(subtype==scalarArray) {
                    std::cout << "DataMonitorRequesterImpl::ParsePVStructure type scalarArray" <<  std::endl;
                    //PVScalarArrayPtr subpv = pv->getScalarArrayField(fieldNames[i]);
                    //subpv->getField()->getType();
                    //ParsePVStructure(subpv, kPtr ,notFirst + 1);
                }
            }
        }
        return;
    }
    if(type==scalarArray) {
        std::cout << "DataMonitorRequesterImpl::ParsePVStructure scalarArray" <<  std::endl;
        //convertArray(static_cast<PVScalarArray *>(pv),notFirst);
        return;
    }
    if(type==structureArray) {
        std::cout << "DataMonitorRequesterImpl::ParsePVStructure structureArray" <<  std::endl;
        //convertStructureArray(static_cast<PVStructureArray*>(pv),notFirst);
        return;
    }
}

void DataMonitorRequesterImpl::ParseScalarArray(PVScalarArray::shared_pointer const & pvs,  knobData* kPtr)
{
    std::cout << "DataMonitorRequesterImpl::ParseScalarArray" <<  std::endl;

    ScalarArrayConstPtr scalar = pvs->getScalarArray();
    ScalarType scalarType = scalar->getElementType();
    size_t length = pvs->getLength();
    qDebug() << "lenght of array " << length;

    switch(scalarType) {
    case pvByte: {

        break;
    }
    case pvUByte: {

        break;
    }
    case pvShort: {

        break;
    }
    case pvUShort: {

        break;
    }
    case pvInt: {

        break;
    }
    case pvUInt: {

        break;
    }

    case pvFloat: {

        cout << "i am float array\n";

        PVFloatArrayPtr ArrayData = static_pointer_cast<epics::pvData::PVFloatArray> (pvs);
        shared_vector<const float> xxx(ArrayData->view());
        for (size_t i = 0; i < length; i++) {
            cout << xxx[i] << " ";
        }
        cout  << std::endl;

        if((double) (length * sizeof(float)) != kPtr->edata.dataSize) {
            free(kPtr->edata.dataB);
            kPtr->edata.dataB = (void*) malloc(length * sizeof(float));
            kPtr->edata.dataSize = length * sizeof(float);
        }

        kPtr->edata.fieldtype = caFLOAT;
        memcpy(kPtr->edata.dataB, &xxx[0],  length * sizeof(float));
        break;
    }
    case pvDouble: {

        break;
    }
    case pvString: {

        break;
    }
    default:
        throw std::logic_error("Should never get here");
    }


    kPtr->edata.valueCount = length;
    kPtr->edata.precision = 3;  // for the time beeing, set to 3
    kPtr->edata.fieldtype = caFLOAT;

}

void DataMonitorRequesterImpl::ParseScalar(string fieldName, PVScalarPtr const & pvs, knobData* kPtr, limitsType limits)
{
    string pvFieldName  = pvs->getFieldName();
    ScalarConstPtr scalar = pvs->getScalar();
    ScalarType scalarType = scalar->getScalarType();

    switch(scalarType) {
    case pvBoolean: {
        PVBooleanPtr data = static_pointer_cast<PVBoolean>(pvs);

        if (limits == _valuealarm &&  pvFieldName.find("active") != string::npos) {
            //cout << "DataMonitorRequesterImpl::ParseScalar : found active for status " << data->get() << std::endl;
        }

        if (pvFieldName.find("value") != string::npos) {
            kPtr->edata.fieldtype = caINT;
            kPtr->edata.rvalue = (double) data->get();
            kPtr->edata.ivalue = (long) data->get();
        }
    }
        break;
    case pvByte: {
        PVBytePtr data = static_pointer_cast<PVByte>(pvs);

        if (pvFieldName.find("value") != string::npos) {
            kPtr->edata.fieldtype = caCHAR;
            kPtr->edata.rvalue = (double) data->get();
            kPtr->edata.ivalue = (long) data->get();
        }
    }
        break;
    case pvShort:
    case pvUShort: {
        PVShortPtr data = static_pointer_cast<PVShort>(pvs);
        if (pvFieldName.find("value") != string::npos) {
            kPtr->edata.fieldtype = caINT;
            kPtr->edata.rvalue = (double) data->get();
            kPtr->edata.ivalue = (long) data->get();
        }
    }
        break;

    case pvInt: {
        PVIntPtr data = static_pointer_cast<PVInt>(pvs);
        if (pvFieldName.find("value") != string::npos) {
            kPtr->edata.fieldtype = caINT;
            kPtr->edata.rvalue =(double)  data->get();
            kPtr->edata.ivalue = (long) data->get();
        }
    }
        break;

    case pvUInt: {
        PVUIntPtr data = static_pointer_cast<PVUInt>(pvs);
        if (pvFieldName.find("value") != string::npos) {
            kPtr->edata.fieldtype = caINT;
            kPtr->edata.rvalue = (double) data->get();
            kPtr->edata.ivalue = (long) data->get();
        }
    }
        break;

    case pvLong:
    case pvULong: {
        PVLongPtr data = static_pointer_cast<PVLong>(pvs);
        if (pvFieldName.find("value") != string::npos) {
            kPtr->edata.fieldtype = caLONG;
            kPtr->edata.rvalue = (double) data->get();
            kPtr->edata.ivalue = (long)data->get();
        }
    }
        break;

    case pvFloat: {
        PVFloatPtr data = static_pointer_cast<PVFloat>(pvs);
        if (limits == _display &&         pvFieldName.find("limitLow")         != string::npos) kPtr->edata.lower_disp_limit = data->get();
        else if (limits == _display &&    pvFieldName.find("limitHigh")        != string::npos) kPtr->edata.upper_disp_limit = data->get();
        else if (limits == _control &&    pvFieldName.find("limitLow")         != string::npos) kPtr->edata.lower_ctrl_limit = data->get();
        else if (limits == _control &&    pvFieldName.find("limitHigh")        != string::npos) kPtr->edata.upper_ctrl_limit = data->get();
        else if (limits == _valuealarm && pvFieldName.find("lowWarningLimit")  != string::npos) kPtr->edata.lower_warning_limit = data->get();
        else if (limits == _valuealarm && pvFieldName.find("highWarningLimit") != string::npos) kPtr->edata.upper_warning_limit = data->get();
        else if (limits == _valuealarm && pvFieldName.find("lowAlarmLimit")    != string::npos) kPtr->edata.lower_alarm_limit = data->get();
        else if (limits == _valuealarm && pvFieldName.find("highAlarmLimit")   != string::npos) kPtr->edata.upper_alarm_limit = data->get();

        // fill value and type
        else if (pvFieldName.find("value") != string::npos) {
            kPtr->edata.fieldtype = caFLOAT;
            kPtr->edata.rvalue = data->get();
            kPtr->edata.ivalue = (long) data->get();
        }

    }
        break;

    case pvDouble: {
        PVDoublePtr data = static_pointer_cast<PVDouble>(pvs);
        if (limits == _display &&         pvFieldName.find("limitLow")         != string::npos) kPtr->edata.lower_disp_limit = data->get();
        else if (limits == _display &&    pvFieldName.find("limitHigh")        != string::npos) kPtr->edata.upper_disp_limit = data->get();
        else if (limits == _control &&    pvFieldName.find("limitLow")         != string::npos) kPtr->edata.lower_ctrl_limit = data->get();
        else if (limits == _control &&    pvFieldName.find("limitHigh")        != string::npos) kPtr->edata.upper_ctrl_limit = data->get();
        else if (limits == _valuealarm && pvFieldName.find("lowWarningLimit")  != string::npos) kPtr->edata.lower_warning_limit = data->get();
        else if (limits == _valuealarm && pvFieldName.find("highWarningLimit") != string::npos) kPtr->edata.upper_warning_limit = data->get();
        else if (limits == _valuealarm && pvFieldName.find("lowAlarmLimit")    != string::npos)   kPtr->edata.lower_alarm_limit = data->get();
        else if (limits == _valuealarm && pvFieldName.find("highAlarmLimit")   != string::npos)   kPtr->edata.upper_alarm_limit = data->get();

        // fill value and type
        else if(pvFieldName.find("value") != string::npos) {
            kPtr->edata.precision = 3;  // for the time beeing, set to 3
            kPtr->edata.fieldtype = caDOUBLE;
            kPtr->edata.rvalue = data->get();
            kPtr->edata.ivalue = (long) data->get();
        } else {
            //std::cout << "DataMonitorRequesterImpl::ParseScalar -- is pvDouble but fielname is empty" <<  std::endl;
        }
    }
        break;

    case pvString: {
        PVStringPtr data = static_pointer_cast<PVString>(pvs);
        // fill units and precision
        if (limits == _display      && pvFieldName.find("units") != string::npos) strcpy(kPtr->edata.units, data->get().c_str());
        else if (limits == _display && pvFieldName.find("format") != string::npos) {
            int precision;
            QString format(data->get().c_str());
            scanFormat(format, precision);
            kPtr->edata.precision = precision;
        // fill value and type
        } else if(pvFieldName.find("value") != string::npos) {
            int dataSize = strlen(data->get().c_str());
            if(dataSize != kPtr->edata.dataSize) {
                free(kPtr->edata.dataB);
                kPtr->edata.dataB = (void*) malloc((size_t) dataSize);
                kPtr->edata.dataSize = dataSize;
            }
            strcpy( (char*) kPtr->edata.dataB, data->get().c_str());
            kPtr->edata.valueCount = 1;
            kPtr->edata.fieldtype = caSTRING;
            kPtr->edata.precision = 0;
            kPtr->edata.units[0] = '\0';
        }
    }
        break;

    default:
        std::cout << "DataMonitorRequesterImpl::ParseScalar (unknown ScalarType)" <<  std::endl;
    }
}

int DataMonitorRequesterImpl::scanFormat(const QString &fmt, int &precision)
{
    precision = 1;
    QString pattern = QString("%(\\d+\\$)?([-#+ 0,(\\<]*)?(\\d+)?(\\.\\d+)?([tT])?([a-zA-Z%])");
    QRegExp re(pattern);
    int pos = re.indexIn(fmt);
    //printf("looking for pattern \"%s\" in \"%s\" : pos %d\n", qasc(pattern), qasc(fmt), pos);
    if(pos > -1) {
        QStringList captures = re.capturedTexts();

        //qDebug() << captures.size();
        //qDebug() << captures;

        if(captures.size() == 7) {
            QString prec = captures.at(4);
            prec = prec.replace(".", "");
            precision = prec.toInt();
            if(prec.contains("d")) precision = 0;
        }
    } else {
        return false;
    }

    return true;
}
