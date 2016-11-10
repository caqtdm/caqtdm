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
#include <db_access.h>


#include <pv/nt.h>


#include "epics4_plugin.h"


using namespace std;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::pvaClient;
using namespace epics::nt;


namespace epics { namespace caqtdm { namespace epics4 {

class epicsShareClass PvaInterface :
    public PvaClientChannelStateChangeRequester,
    public CallbackRequester,
    public PvaClientMonitorRequester,
    public GetFieldRequester,
    public ChannelGetRequester,
    public std::tr1::enable_shared_from_this<PvaInterface>
{
public:
    POINTER_DEFINITIONS(PvaInterface);
    PvaInterface(
        PvaClientPtr const & pvaClient,
        MutexKnobData *mutexKnobData,
        int index,
        Epics4RequesterPtr const & requester,
        CallbackThreadPtr const & callbackThread,
        std::string const & channelName);
    virtual ~PvaInterface() {}

    virtual void channelStateChange(
        PvaClientChannelPtr const & channel, bool isConnected);
    virtual void event(PvaClientMonitorPtr const & monitor);
    virtual void unlisten();

    virtual std::string getRequesterName();
    virtual void message(std::string const & message,MessageType messageType);
    virtual void getDone(
        const Status& status,
        FieldConstPtr const & field);
    virtual void channelGetConnect(
        const Status& status,
        ChannelGet::shared_pointer const & channelGet,
        Structure::const_shared_pointer const & structure);
    virtual void getDone(
        const Status& status,
        ChannelGet::shared_pointer const & channelGet,
        PVStructurePtr const & pvStructure,
        BitSet::shared_pointer const & bitSet);

    virtual void callback();

    void connect();
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
    bool setValue(double rdata, int32_t idata, char *sdata, char *object, char *errmess, int forceType);
    bool getTimeStamp(char *timestamp);
    template <typename pureData> void fillData(pureData const &array, size_t size, knobData* kPtr);

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

    PvaClientPtr pvaClient;
    bool gotFirstConnection;
    bool unlistenCalled;
    CreateRequest::shared_pointer createRequest;
    MutexKnobData *mutexKnobData;
    int index;
    Epics4RequesterPtr requester;
    CallbackThreadPtr callbackThread;
    bool gotFirstConnect;
    NormativeType normativeType;
    CallbackType callbackType;
    PvaClientChannelPtr pvaClientChannel;
    ChannelGet::shared_pointer channelGet;
    ConvertPtr convert;

    knobData kData;
    PvaClientMonitorPtr pvaMonitor;
    StructureConstPtr structure;
    shared_vector<const string> choices;
    TimeStamp timeStamp;

    Mutex mutex;
};




class epicsShareClass PvaInterfaceGlue {
public:
    PvaInterfaceGlue() {}
    PvaInterfacePtr pvaInterface;
};


PvaInterface::PvaInterface(
        PvaClientPtr const & pvaClient,
        MutexKnobData *mutexKnobData,
        int index,
        Epics4RequesterPtr const & requester,
        CallbackThreadPtr const & callbackThread,
        string const & channelName)
: pvaClient(pvaClient),
  gotFirstConnection(false),
  unlistenCalled(false),
  createRequest(CreateRequest::create()),
  mutexKnobData(mutexKnobData),
  index(index),
  requester(requester),
  callbackThread(callbackThread),
  gotFirstConnect(false),
  normativeType(ntunknown_t),
  callbackType(unknown_t),
  pvaClientChannel(pvaClient->createChannel(channelName)),
  convert(getConvert())
{
}

void PvaInterface::channelStateChange(
       PvaClientChannelPtr const & channel, bool isConnected)
{
    if(gotFirstConnect) mutexKnobData->SetMutexKnobDataConnected(index,isConnected);
    if(!isConnected || gotFirstConnect) return;
    callbackType = interface_t;
    callbackThread->queueRequest(shared_from_this());
}

void PvaInterface::event(PvaClientMonitorPtr const & monitor)
{
    while(monitor->poll()) {
        PvaClientMonitorDataPtr monitorData = monitor->getData();
        PVStructurePtr pvStructure = monitorData->getPVStructure();
        kData = mutexKnobData->GetMutexKnobData(index);
        if(kData.index == -1) return;
        mutexKnobData->DataLock(&kData);
        bool gotAlarm = false;
        if(structure->getField("alarm")) gotAlarm = true;
        if(gotAlarm) {
            PVFieldPtr pvField = pvStructure->getSubField<PVStructure>("alarm");
            Alarm alarm;
            PVAlarm pvAlarm;
            pvAlarm.attach(pvField);
            pvAlarm.get(alarm);
            kData.edata.severity = alarm.getSeverity();
        }
        bool gotTimeStamp = false;
        if(structure->getField("alarm")) gotTimeStamp = true;
        if(gotTimeStamp) {
            PVFieldPtr pvField = pvStructure->getSubField<PVStructure>("timeStamp");
            PVTimeStamp pvTimeStamp;
            pvTimeStamp.attach(pvField);
            pvTimeStamp.get(timeStamp);
        }
        switch (normativeType) {
            case ntscalar_t : getScalarData(pvStructure); break;
            case ntenum_t : getEnumData(pvStructure); break;
            case ntscalararray_t : getScalarArrayData(pvStructure); break;
            default: throw std::runtime_error("PvaInterface::event logic error");
        }
        mutexKnobData->SetMutexKnobData(kData.index, kData);
        mutexKnobData->DataUnlock(&kData);
        monitor->releaseEvent();
    }
}



void PvaInterface::unlisten()
{
         std::cout << "ClientMonitorRequester::unlisten\n";
         unlistenCalled = true;
}

string PvaInterface::getRequesterName()
{
    return pvaClient->getRequesterName();
}

void PvaInterface::message(std::string const & message,MessageType messageType)
{
    pvaClient->message(pvaClientChannel->getChannelName() + " " + message,messageType);
}

void PvaInterface::getDone(
        const Status& status,
        FieldConstPtr const & field)
{
     structure =  std::tr1::dynamic_pointer_cast<const Structure>(field);
     if(!status.isOK()) {
          message(" getField failed",errorMessage);
          return;
     } else {
          if(NTScalar::is_a(structure)) {
               normativeType = ntscalar_t;
          } else if(NTScalarArray::is_a(structure)) {
               normativeType = ntscalararray_t;
          } else if(NTEnum::is_a(structure)) {
               normativeType = ntenum_t;
          } else {
                message(" value is not a valid nttype",errorMessage);
                return;
          }
     }
     gotInterface();
}



void PvaInterface::channelGetConnect(
    const Status& status,
    ChannelGet::shared_pointer const & channelGet,
    Structure::const_shared_pointer const & structure)
{
    if(!status.isOK()) {
          message("channelGetConnect failed",errorMessage);
          return;
     }
     channelGet->get();
}

void PvaInterface::getDone(
    const Status& status,
    ChannelGet::shared_pointer const & channelGet,
    PVStructurePtr const & pvStructure,
    BitSet::shared_pointer const & bitSet)
{
    if(!status.isOK()) {
          message("channelGetDone failed",errorMessage);
          return;
     }
     switch(normativeType) {
       case ntscalar_t : gotDisplayControl(pvStructure); break;
       case ntscalararray_t : gotDisplayControl(pvStructure); break;
       case ntenum_t : gotEnum(pvStructure); break;
       default: throw std::runtime_error("PvaInterface::getDone logic error");
     }
}

void PvaInterface::callback()
{
    try {
       switch(callbackType) {
       case interface_t : getInterface(); break;
       case displayControl_t : getDisplayControl(); break;
       case enum_t : getEnum(); break;
       case createMonitor_t : createMonitor(); break;
       default: throw std::runtime_error("PvaInterface::callback logic error");
       }
       callbackType = unknown_t;
    } catch (std::runtime_error e) {
        cerr << "exception " << e.what() << endl;
        return;
    }
}

void PvaInterface::connect()
{
     pvaClientChannel->setStateChangeRequester(shared_from_this());
     pvaClientChannel->issueConnect();
}

void PvaInterface::getInterface()
{
      pvaClientChannel->getChannel()->getField(shared_from_this(),"");
}

void PvaInterface::gotInterface()
{
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
           mutexKnobData->SetMutexKnobData(kData.index, kData);
           mutexKnobData->DataUnlock(&kData);
           callbackType = createMonitor_t;
           callbackThread->queueRequest(shared_from_this());
           return;
    }
    callbackType = displayControl_t;
    callbackThread->queueRequest(shared_from_this());
}


void PvaInterface::getDisplayControl()
{

    PVStructurePtr pvRequest = createRequest->createRequest("display,control");
    channelGet = pvaClientChannel->getChannel()->createChannelGet(shared_from_this(),pvRequest);
}

void PvaInterface::gotDisplayControl(PVStructurePtr const & pvStructure)
{
    double controlLow = 0;
    double controlHigh = 0;
    double displayLow = 0;
    double displayHigh = 0;
    short precision = 0;
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
    pvField = pvStructure->getSubField<PVStructure>(string("display"));
    if(pvField) {
        epics::pvData::Display display;
        PVDisplay pvDisplay;
        if(pvDisplay.attach(pvField)) {
            pvDisplay.get(display);
            displayLow = display.getLow();
            displayHigh = display.getHigh();
            string format = display.getFormat();
            string::size_type ind = format.find("%.");
            if(ind!=std::string::npos) {
                string rest(format.substr(ind +2));
                ind = rest.find('f');
                if(ind!=std::string::npos) {
                   rest = rest.substr(0,ind);
                   if(rest.length()!=std::string::npos) {
                        //precision = std::stoi(rest);
                        precision = std::atoi(rest.c_str());
                   }
                }
            }
        }
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
    kData.edata.accessR = 1;
    if(controlHigh>controlLow) kData.edata.accessW = 1;
    mutexKnobData->SetMutexKnobData(kData.index, kData);
    mutexKnobData->DataUnlock(&kData);
    callbackType = createMonitor_t;
    callbackThread->queueRequest(shared_from_this());
}

void PvaInterface::getEnum()
{
    PVStructurePtr pvRequest = createRequest->createRequest("value.choices");
    channelGet = pvaClientChannel->getChannel()->createChannelGet(shared_from_this(),pvRequest);
}

void PvaInterface::gotEnum(PVStructurePtr const & pvStructure)
{
    PVStringArrayPtr pvChoices = pvStructure->getSubField<PVStringArray>(string("value.choices"));
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
        if(kData.edata.dataSize>0 && (kData.edata.dataB != (void*) 0)) free(kData.edata.dataB);
        kData.edata.dataB = (void*) malloc((size_t) dataSize);
        kData.edata.dataSize = dataSize;
    }
    char * ptr = static_cast<char*>(kData.edata.dataB);
    for(size_t ind = 0; ind < enumCount; ++ind) {
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
    mutexKnobData->SetMutexKnobData(kData.index, kData);
    mutexKnobData->DataUnlock(&kData);
    callbackType = createMonitor_t;
    callbackThread->queueRequest(shared_from_this());
}

void PvaInterface::createMonitor()
{
    try {
       if(normativeType==ntunknown_t) return;
       string request("value,alarm,timeStamp");
       if(normativeType==ntenum_t) request = "alarm,timeStamp,value.index";
       pvaMonitor = pvaClientChannel->monitor(request,shared_from_this());
       gotFirstConnect = true;
       mutexKnobData->SetMutexKnobDataConnected(index,true);
    } catch (std::runtime_error e) {
         cerr << "exception " << e.what() << endl;
         return;
    }
}

void PvaInterface::gotMonitor()
{
}


void PvaInterface::getScalarData(PVStructurePtr const & pvStructure)
{

    PVScalarPtr pvScalar = pvStructure->getSubField<PVScalar>("value");
    ScalarType scalarType = pvScalar->getScalar()->getScalarType();

    switch (scalarType) {
    case pvBoolean:
    {
         PVBooleanPtr pvBoolean = std::tr1::dynamic_pointer_cast<PVBoolean>(pvScalar);
         bool value  = pvBoolean->get();
         kData.edata.ivalue = (value ? 1 : 0);
         kData.edata.fieldtype  = DBF_LONG;
         kData.edata.valueCount = 1;
         kData.edata.monitorCount++;
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
         PVStringPtr pvString = std::tr1::dynamic_pointer_cast<PVString>(pvScalar);
         string value = pvString->get();
         int len = value.length();
         const char * data = value.data();
         int length = len + 1;
         if(length>kData.edata.dataSize)
         {
               if(kData.edata.dataSize!=0 && (kData.edata.dataB != (void*) 0)) free(kData.edata.dataB);
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

void PvaInterface::getEnumData(PVStructurePtr const & pvStructure)
{
     int32 index = pvStructure->getSubField<PVInt>("value.index")->get();
     kData.edata.ivalue = index;
     kData.edata.valueCount = 1;
     kData.edata.monitorCount++;
}

template <typename pureData>
void PvaInterface::fillData(pureData const &array, size_t length, knobData* kPtr) {
    int size = sizeof(array[0]);
    if(length < 1) return;
    if((length * size) != (size_t) kPtr->edata.dataSize) {
        if(kData.edata.dataB != (void*) 0) free(kPtr->edata.dataB);
        kPtr->edata.dataB = (void*) malloc(length * size);
        kPtr->edata.dataSize = length * size;
    }
    memcpy(kPtr->edata.dataB, &array[0],  length * size);
    kData.edata.valueCount = length;

    kData.edata.monitorCount++;
}


void PvaInterface::getScalarArrayData(PVStructurePtr const & pvStructure)
{
    PVScalarArrayPtr pvs = pvStructure->getSubField<PVScalarArray>("value");
    ScalarArrayConstPtr scalar = pvs->getScalarArray();
    ScalarType scalarType = scalar->getElementType();
    size_t length = pvs->getLength();

    switch(scalarType) {

    case pvBoolean:
        pvaClient->message("array of pvString not yet supported", errorMessage);
        break;

    case pvString:
        pvaClient->message("array of pvString not yet supported", errorMessage);
        break;

    case pvByte: {
        PVByteArrayPtr ArrayData = std::tr1::static_pointer_cast<PVByteArray> (pvs);
        shared_vector<const int8> xxx(ArrayData->view());
        kData.edata.fieldtype = DBF_CHAR;
        fillData(xxx, length, &kData);
        break;
    }

    case pvUByte: {
        PVUByteArrayPtr ArrayData = std::tr1::static_pointer_cast<PVUByteArray> (pvs);
        shared_vector<const uint8> xxx(ArrayData->view());
        kData.edata.fieldtype = DBF_CHAR;
        fillData(xxx, length, &kData);
        break;
    }

    case pvShort: {
        PVShortArrayPtr ArrayData = std::tr1::static_pointer_cast<PVShortArray> (pvs);
        shared_vector<const int16> xxx(ArrayData->view());
        kData.edata.fieldtype = DBF_INT;
        fillData(xxx, length, &kData);
        break;
    }

    case pvUShort: {
        PVUShortArrayPtr ArrayData = std::tr1::static_pointer_cast<PVUShortArray> (pvs);
        shared_vector<const uint16> xxx(ArrayData->view());
        kData.edata.fieldtype = DBF_INT;
        fillData(xxx, length, &kData);
        break;
    }

    case pvInt: {
        PVIntArrayPtr ArrayData = std::tr1::static_pointer_cast<PVIntArray> (pvs);
        shared_vector<const int32> xxx(ArrayData->view());
        kData.edata.fieldtype = DBF_LONG;
        fillData(xxx, length, &kData);
    }

    case pvUInt: {
        PVUIntArrayPtr ArrayData = std::tr1::static_pointer_cast<PVUIntArray> (pvs);
        shared_vector<const uint32> xxx(ArrayData->view());
        kData.edata.fieldtype = DBF_LONG;
        fillData(xxx, length, &kData);
        break;
    }

    case pvLong:
    case pvULong:
        pvaClient->message("array of pvLong not yet supported", errorMessage);
        break;

    case pvFloat: {
        PVFloatArrayPtr ArrayData = std::tr1::static_pointer_cast<PVFloatArray> (pvs);
        shared_vector<const float> xxx(ArrayData->view());
        kData.edata.fieldtype = DBF_FLOAT;
        fillData(xxx, length, &kData);
        break;
    }


    case pvDouble: {
        PVDoubleArrayPtr ArrayData = std::tr1::static_pointer_cast<PVDoubleArray> (pvs);
        shared_vector<const double> xxx(ArrayData->view());
        kData.edata.fieldtype = DBF_DOUBLE;
        fillData(xxx, length, &kData);
    }
    }
}

bool PvaInterface::setValue(double rdata, int32_t idata, char *sdata, char *object, char *errmess, int forceType)
{
    if(!pvaClientChannel->getChannel()) {
         pvaClient->message("setValue called for unconnected channel",errorMessage);
         return false;
    }
    if(normativeType==ntenum_t){
       int enumCount = choices.size();
       for(int32 ind = 0; ind < enumCount; ++ind) {
           const string val = choices[ind];
           if(val.compare(sdata)==0) {
               PvaClientPutPtr put = pvaClientChannel->put("value.index");
               PvaClientPutDataPtr putData = put->getData();
               PVIntPtr pvInt = putData->getPVStructure()->getSubField<PVInt>("value.index");
               pvInt->put(ind);
               put->put();
               return true;
           }
       }
       return false;
    }
    PvaClientPutPtr put = pvaClientChannel->put();
    PvaClientPutDataPtr putData = put->getData();
    PVStructurePtr pvStructure = putData->getPVStructure();
    PVScalarPtr pvScalar = pvStructure->getSubField<PVScalar>("value");
    ScalarType scalarType = pvScalar->getScalar()->getScalarType();

    if(forceType == 1) scalarType = pvDouble;
    else if(forceType == 2) scalarType = pvInt;

    switch (scalarType) {
    case pvBoolean:
    {
         PVBooleanPtr pvBoolean = std::tr1::dynamic_pointer_cast<PVBoolean>(pvScalar);
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
         PVStringPtr pvString = std::tr1::dynamic_pointer_cast<PVString>(pvScalar);
         pvString->put(sdata);
    }
         break;
    }
    put->put();
    return true;
}

bool PvaInterface::getTimeStamp(char *buf)
{
    if(!structure->getField("timeStamp")) return false;
    time_t tt;
    timeStamp.toTime_t(tt);
    struct tm ctm;
    memcpy(&ctm,localtime(&tt),sizeof(struct tm));
    strftime(buf,40,"%G.%m.%d %H.%M.%S%n",&ctm);
    int len = strlen(buf);
    sprintf(buf + len,"TimeStamp .%09d tag %d\n",timeStamp.getNanoseconds(),timeStamp.getUserTag());
    return true;
}


}}}

using namespace epics::caqtdm::epics4;

QString Epics4Plugin::pluginName()
{
    return "epics4";
}

Epics4Plugin::Epics4Plugin()
: pvaClient(PvaClient::get("pva")),
  callbackThread(CallbackThread::create())
{
//PvaClient::setDebug(true);
    qDebug() << "Epics4Plugin::Epics4Plugin";

}

Epics4Plugin::~Epics4Plugin()
{
cout << "Epics4Plugin::~Epics4Plugin()\n";
    callbackThread->stop();
}

int Epics4Plugin::initCommunicationLayer(MutexKnobData *mutexKnobDataP, MessageWindow *messageWindow, QMap<QString, QString> options)
{
    qDebug() << "Epics4Plugin: InitCommunicationLayer with options" << options;
    mutexKnobData = mutexKnobDataP;
    requester = Epics4RequesterPtr(new Epics4Requester(messageWindow));
    pvaClient->setRequester(requester);
    return true;
}


int Epics4Plugin::pvAddMonitor(int index, knobData *kData, int rate, int skip) {
    Q_UNUSED(rate);
    Q_UNUSED(skip);
    qDebug() << "Epics4Plugin:pvAddMonitor" << kData->pv << kData->index;
    PvaInterfaceGlue *pvaInterfaceGlue = static_cast<PvaInterfaceGlue *>(kData->edata.info);
    if(pvaInterfaceGlue) return true;
    string name(kData->pv);
    PvaInterfacePtr pvaInterface(
         new PvaInterface(pvaClient, mutexKnobData,index,requester,callbackThread,name));
    map<string,PvaInterfacePtr>::iterator iter = pvaInterfaceMap.find(name);
    if(iter==pvaInterfaceMap.end()) {
        pvaInterfaceMap.insert(std::pair<string,PvaInterfacePtr>(
         name,pvaInterface));
    }
    pvaInterfaceGlue = new PvaInterfaceGlue();
    pvaInterfaceGlue->pvaInterface = pvaInterface;
    kData->edata.info = pvaInterfaceGlue;
    C_SetMutexKnobData(mutexKnobData, index, *kData);
    pvaInterface->connect();
    return true;
}


int Epics4Plugin::pvClearMonitor(knobData *kData) {
    qDebug() << "Epics4Plugin:pvClearMonitor";
    kData->index = -1;
    kData->pv[0] = '\0';
    return true;
}

int Epics4Plugin::pvFreeAllocatedData(knobData *kData)
{
    qDebug() << "Epics4Plugin:pvFreeAllocatedData";
    if (kData->edata.info != (void *) 0) {
      PvaInterfaceGlue *pvaInterfaceGlue  = static_cast<PvaInterfaceGlue *>(kData->edata.info);
        kData->edata.info = NULL;
        delete pvaInterfaceGlue;
    }
    if(kData->edata.dataB != (void*) 0) {
        if(kData->edata.dataB != (void*) 0) free(kData->edata.dataB);
        kData->edata.dataB = (void*) 0;
    }
    return true;
}

int Epics4Plugin::pvSetValue(char *pv, double rdata, int32_t idata, char *sdata, char *object, char *errmess, int forceType)
{
    qDebug() << "Epics4Plugin:pvSetValue";
    map<string,PvaInterfacePtr>::iterator iter = pvaInterfaceMap.find(pv);
    if(iter!=pvaInterfaceMap.end()){
         PvaInterfacePtr pvaInterface(iter->second);
         return pvaInterface->setValue(rdata,idata,sdata,object,errmess,forceType);
    }
cout << "did not find pvaInterface\n";
    return false;
}

int Epics4Plugin::pvSetWave(char *pv, float *fdata, double *ddata, int16_t *data16, int32_t *data32, char *sdata, int nelm, char *object, char *errmess) {
    Q_UNUSED(pv);
    Q_UNUSED(fdata);
    Q_UNUSED(ddata);
    Q_UNUSED(data16);
    Q_UNUSED(data32);
    Q_UNUSED(sdata);
    Q_UNUSED(nelm);
    Q_UNUSED(object);
    Q_UNUSED(errmess);
    qDebug() << "Epics4Plugin:pvSetWave";
    return false;
}

int Epics4Plugin::pvGetTimeStamp(char *pv, char *timestamp) {
    Q_UNUSED(pv);
    Q_UNUSED(timestamp);
    qDebug() << "Epics4Plugin:pvgetTimeStamp";
    map<string,PvaInterfacePtr>::iterator iter = pvaInterfaceMap.find(pv);
    if(iter!=pvaInterfaceMap.end()){
         PvaInterfacePtr pvaInterface(iter->second);
         return pvaInterface->getTimeStamp(timestamp);
    }
    return false;
}

int Epics4Plugin::pvGetDescription(char *pv, char *description) {
    Q_UNUSED(pv);
    Q_UNUSED(description);
    qDebug() << "Epics4Plugin:pvGetDescription";
    return false;
}

int Epics4Plugin::pvClearEvent(void * ptr) {
    Q_UNUSED(ptr);
    qDebug() << "Epics4Plugin:pvClearEvent";
    return false;
}

int Epics4Plugin::pvAddEvent(void * ptr) {
    Q_UNUSED(ptr);
    qDebug() << "Epics4Plugin:pvAddEvent";
    return false;
}

int Epics4Plugin::pvReconnect(knobData *kData) {
    Q_UNUSED(kData);
    qDebug() << "Epics4Plugin:pvReconnect";
    return false;
}

int Epics4Plugin::pvDisconnect(knobData *kData) {
    Q_UNUSED(kData);
    qDebug() << "Epics4Plugin:pvDisconnect";
    return false;
}

int Epics4Plugin::FlushIO() {

    qDebug() << "Epics4Plugin:FlushIO";
    return true;
}



int Epics4Plugin::TerminateIO() {
    qDebug() << "Epics4Plugin:TerminateIO";
    return false;
}


#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#else
    Q_EXPORT_PLUGIN2(Epics4Plugin, Epics4Plugin)
#endif

