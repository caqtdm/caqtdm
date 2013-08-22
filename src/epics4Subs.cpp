#include <QDebug>

#include <QString>
#include <QStringList>

#include "epics4Subs.h"
#include "pvAccessImpl.h"

#include <epicsStdlib.h>
#include <epicsGetopt.h>
#include <epicsThread.h>
#include <epicsExit.h>


string request(DEFAULT_REQUEST);
double timeOut = DEFAULT_TIMEOUT;

epics4Subs::epics4Subs(MutexKnobData* mutexKnobData)
{
    /*
    In order to connect to a channel a client must:

    Call ChannelAccessFactory.getChannelAccess() to get the ChannelAccess interface.
    Call ChannelAccess.getProvider(String providerName) to get a ChannelProvider.
    Call ChannelProvider.createChannel(String channelName, ...) to create a Channel.
    A client must know the channel name and the name of the channel provider.
    */

    Requester::shared_pointer requesterG(new RequesterImpl("pvget"));
    Requester::shared_pointer requesterP(new RequesterImpl("pvput"));

    pvRequestG = getCreateRequest()->createRequest(request, requesterG);
    if(pvRequestG.get()==NULL) printf("failed to parse request string\n");

    pvRequestP = getCreateRequest()->createRequest(request, requesterP);
    if(pvRequestP.get()==NULL) printf("failed to parse request string\n");


    qDebug() << "client factory start and get provider";
    ClientFactory::start();
    provider = getChannelAccess()->getProvider("pvAccess");

    m_mutexKnobData = mutexKnobData;
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
    qDebug() << "CreateAndConnect4" << pv;
    shared_ptr<ChannelRequesterImpl> channelRequesterImpl(new ChannelRequesterImpl());
    Channel::shared_pointer channel = provider->createChannel(pv.toAscii().data(), channelRequesterImpl);
    channelArray.append(channel);

    channelRequesterImpl = dynamic_pointer_cast<ChannelRequesterImpl>(channel->getChannelRequester());
    channelRequesterImpl->setMyData(num, m_mutexKnobData);

    if (channelRequesterImpl->waitUntilConnected(timeOut)) {
        shared_ptr<GetFieldRequesterImpl> getFieldRequesterImpl;

        getFieldRequesterImpl.reset(new GetFieldRequesterImpl(channel));
        channel->getField(getFieldRequesterImpl, "");

        if (getFieldRequesterImpl.get() == 0 || getFieldRequesterImpl->waitUntilFieldGet(timeOut)) {
            qDebug() << "monitorrequest";

            shared_ptr<MonitorRequesterImpl> monitorRequesterImpl(new MonitorRequesterImpl(channel->getChannelName()));
            monitorRequesterImpl->setMyData(num, m_mutexKnobData);
            Monitor::shared_pointer monitorGet = channel->createMonitor(monitorRequesterImpl, pvRequestG);
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

void epics4Subs::Epics4SetValue(int num, QString const &pv, QString const & value)
{
    bool allOK = true;
    // create channel
    qDebug() << "Epics4SetValue " << pv;
    shared_ptr<ChannelRequesterImpl> channelRequesterImpl(new ChannelRequesterImpl());
    Channel::shared_pointer channel = provider->createChannel(pv.toAscii().data(), channelRequesterImpl);

    channelRequesterImpl = dynamic_pointer_cast<ChannelRequesterImpl>(channel->getChannelRequester());
    channelRequesterImpl->setMyData(num, m_mutexKnobData);

    if (channelRequesterImpl->waitUntilConnected(timeOut)) {
        shared_ptr<ChannelPutRequesterImpl> putRequesterImpl(new ChannelPutRequesterImpl(channel->getChannelName()));

        ChannelPut::shared_pointer channelPut = channel->createChannelPut(putRequesterImpl, pvRequestP);
        allOK &= putRequesterImpl->waitUntilDone(timeOut);
        if (allOK) {
            ParsePVStructure(putRequesterImpl->getStructure(), value);
            channelPut->put(false);
            allOK &= putRequesterImpl->waitUntilDone(timeOut);
        }
    }
}

void epics4Subs::ParsePVStructure(PVStructurePtr const & pvStructure, QString const & value)
{
    PVFieldPtrArray const & fieldsData = pvStructure->getPVFields();
    if (fieldsData.size() != 0) {
        size_t length = pvStructure->getStructure()->getNumberFields();
        for(size_t i = 0; i < length; i++) {
            PVFieldPtr fieldField = fieldsData[i];
            Type type = fieldField->getField()->getType();
            if(type==scalar) {
                PVScalarPtr pv = static_pointer_cast<PVScalar>(fieldField);
                QString thisFieldName  = QString::fromStdString(pv->getFieldName());
                if(thisFieldName.contains("value")) {
                    fromString(pv, value);
                    break;
                }
            }
        }
    }
}

void epics4Subs::fromString(PVScalarPtr const & pvScalar, QString const & from)
{
    ScalarConstPtr scalar = pvScalar->getScalar();
    ScalarType scalarType = scalar->getScalarType();
    switch(scalarType) {
    case pvBoolean: {
        PVBooleanPtr pv = static_pointer_cast<PVBoolean>(pvScalar);
        bool isTrue  = (from.compare("true")==0  || from.compare("1")==0);
        bool isFalse = (from.compare("false")==0 || from.compare("0")==0);
        if (!(isTrue || isFalse))
            throw runtime_error("failed to parse field " + pvScalar->getFieldName() + " (boolean) from string value '" + from.toAscii().data() + "'");
        if(isTrue) pv->put(true); else pv->put(false);
        return;
    }
    case pvByte : {
        PVBytePtr pv = static_pointer_cast<PVByte>(pvScalar);
        int ival;
        int result = sscanf(from.toAscii().data(),"%d",&ival);
        if (result != 1)
            throw runtime_error("failed to parse field " + pvScalar->getFieldName() + " (byte) from string value '" + from.toAscii().data() + "'");
        int8 value = ival;
        pv->put(value);
        return;
    }
    case pvShort : {
        PVShortPtr pv = static_pointer_cast<PVShort>(pvScalar);
        int ival;
        int result = sscanf(from.toAscii().data(),"%d",&ival);
        if (result != 1)
            throw runtime_error("failed to parse field " + pvScalar->getFieldName() + " (short) from string value '" + from.toAscii().data() + "'");
        int16 value = ival;
        pv->put(value);
        return;
    }
    case pvInt : {
        PVIntPtr pv = static_pointer_cast<PVInt>(pvScalar);
        int ival;
        int result = sscanf(from.toAscii().data(),"%d",&ival);
        if (result != 1)
            throw runtime_error("failed to parse field " + pvScalar->getFieldName() + " (int) from string value '" + from.toAscii().data() + "'");
        int32 value = ival;
        pv->put(value);
        return;
    }
    case pvLong : {
        PVLongPtr pv = static_pointer_cast<PVLong>(pvScalar);
        int64 ival;
        int result = sscanf(from.toAscii().data(),"%lld",(long long *)&ival);
        if (result != 1)
            throw runtime_error("failed to parse field " + pvScalar->getFieldName() + " (long) from string value '" + from.toAscii().data() + "'");
        int64 value = ival;
        pv->put(value);
        return;
    }
    case pvUByte : {
        PVUBytePtr pv = static_pointer_cast<PVUByte>(pvScalar);
        unsigned int ival;
        int result = sscanf(from.toAscii().data(),"%u",&ival);
        if (result != 1)
            throw runtime_error("failed to parse field " + pvScalar->getFieldName() + " (ubyte) from string value '" + from.toAscii().data() + "'");
        uint8 value = ival;
        pv->put(value);
        return;
    }
    case pvUShort : {
        PVUShortPtr pv = static_pointer_cast<PVUShort>(pvScalar);
        unsigned int ival;
        int result = sscanf(from.toAscii().data(),"%u",&ival);
        if (result != 1)
            throw runtime_error("failed to parse field " + pvScalar->getFieldName() + " (ushort) from string value '" + from.toAscii().data() + "'");
        uint16 value = ival;
        pv->put(value);
        return;
    }
    case pvUInt : {
        PVUIntPtr pv = static_pointer_cast<PVUInt>(pvScalar);
        unsigned int ival;
        int result = sscanf(from.toAscii().data(),"%u",&ival);
        if (result != 1)
            throw runtime_error("failed to parse field " + pvScalar->getFieldName() + " (uint) from string value '" + from.toAscii().data() + "'");
        uint32 value = ival;
        pv->put(value);
        return;
    }
    case pvULong : {
        PVULongPtr pv = static_pointer_cast<PVULong>(pvScalar);
        unsigned long long ival;
        int result = sscanf(from.toAscii().data(),"%llu",(long long unsigned int *)&ival);
        if (result != 1)
            throw runtime_error("failed to parse field " + pvScalar->getFieldName() + " (ulong) from string value '" + from.toAscii().data() + "'");
        uint64 value = ival;
        pv->put(value);
        return;
    }
    case pvFloat : {
        PVFloatPtr pv = static_pointer_cast<PVFloat>(pvScalar);
        float value;
        int result = sscanf(from.toAscii().data(),"%f",&value);
        if (result != 1)
            throw runtime_error("failed to parse field " + pvScalar->getFieldName() + " (float) from string value '" + from.toAscii().data() + "'");
        pv->put(value);
        return;
    }
    case pvDouble : {
        PVDoublePtr pv = static_pointer_cast<PVDouble>(pvScalar);
        double value;
        int result = sscanf(from.toAscii().data(),"%lf",&value);
        if (result != 1)
            throw runtime_error("failed to parse field " + pvScalar->getFieldName() + " (double) from string value '" + from.toAscii().data() + "'");
        pv->put(value);
        return;
    }
    case pvString: {
        PVStringPtr value = static_pointer_cast<PVString>(pvScalar);
        value->put(from.toAscii().data());
        return;
    }
    }
    String message("fromString unknown scalarType ");
    ScalarTypeFunc::toString(&message,scalarType);
    throw std::logic_error(message);
}
