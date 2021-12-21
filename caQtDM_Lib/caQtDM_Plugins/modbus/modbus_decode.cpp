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
 *  Copyright (c) 2010 - 2020
 *
 *  Author:
 *    Helge Brands
 *  Contact details:
 *    helge.brands@psi.ch
 */
#include <QtCore>
#include <QThread>
#include <QDebug>
#include <QModbusTcpClient>
#include <QMapIterator>
#include <QByteArray>
#include "controlsinterface.h"
#include "modbus_decode.h"
#include <postfix.h>

#define CYCLE_CONSTANT_TIME 500

modbus_decode::modbus_decode(QObject *parent) : QObject(parent)
{
    loop = new QEventLoop(this);

    mutexknobdataP = Q_NULLPTR;
    modbustargetP="";
    modbustimeout=200;
    modbusretries=10;
}

QString modbus_decode::removeHost(QString pv)
{
    QString chan_desc=pv;
    QString removable=modbustargetP.host()+":"+QString::number(modbustargetP.port());
    chan_desc=chan_desc.remove(removable,Qt::CaseInsensitive);
    return chan_desc;
}

QString modbus_decode::removeEPICSExtensions(QString pv)
{
   QString chan_desc=pv;
   chan_desc=chan_desc.remove(".FTVL",Qt::CaseInsensitive);
   chan_desc=chan_desc.remove(".EGU",Qt::CaseInsensitive);
   chan_desc=chan_desc.remove(".NELM",Qt::CaseInsensitive);
   chan_desc=chan_desc.remove(".NORD",Qt::CaseInsensitive);

   return chan_desc;
}

QString modbus_decode::getEPICSExtensions(QString pv)
{
    QString toCapture = "(?:({.+}.))\\w+";
    QRegExp rx_json = QRegExp(toCapture);
    rx_json.indexIn(pv);

    QStringList list = rx_json.capturedTexts();
    if (list.length()>0)
     return list.at(0);
     else return "";
}

QModbusDataUnit *modbus_decode::generateDataUnit(QString pv)
{
    QModbusDataUnit::RegisterType modbus_type=QModbusDataUnit::Invalid;

    QString chan_desc=removeHost(pv);
    QJsonDocument chan_doc = QJsonDocument::fromJson(chan_desc.toUtf8());
    QJsonObject chan_obj = chan_doc.object();

    QJsonValue value = chan_obj.value(QString("type"));
    if (value.isString()){
        QString teststr=value.toString();
        if (teststr.isEmpty())  return Q_NULLPTR;
        if (teststr.compare("DiscreteInputs",Qt::CaseInsensitive)==0) modbus_type=QModbusDataUnit::DiscreteInputs;
        if (teststr.compare("D",Qt::CaseSensitive)==0) modbus_type=QModbusDataUnit::DiscreteInputs;
        if (teststr.compare("Coils",Qt::CaseInsensitive)==0) modbus_type=QModbusDataUnit::Coils;
        if (teststr.compare("C",Qt::CaseSensitive)==0) modbus_type=QModbusDataUnit::Coils;
        if (teststr.compare("InputRegisters",Qt::CaseInsensitive)==0) modbus_type=QModbusDataUnit::InputRegisters;
        if (teststr.compare("I",Qt::CaseSensitive)==0) modbus_type=QModbusDataUnit::InputRegisters;
        if (teststr.compare("HoldingRegisters",Qt::CaseInsensitive)==0) modbus_type=QModbusDataUnit::HoldingRegisters;
        if (teststr.compare("H",Qt::CaseSensitive)==0) modbus_type=QModbusDataUnit::HoldingRegisters;

        QJsonValue modbus_addr = chan_obj.value(QString("addr"));
        if (modbus_addr.isDouble()){
            modbus_addr=int(modbus_addr.toDouble());
            quint16 modbus_count=1;
            value = chan_obj.value(QString("count"));
            if (value.isDouble()){
                modbus_count=quint16(value.toDouble());
            }
            return new QModbusDataUnit(modbus_type,int(modbus_addr.toDouble()),modbus_count);
        }
    }




    return Q_NULLPTR;
}

caType modbus_decode::generatecaDataType(QString pv)
{
    QString chan_desc=removeHost(pv);
    QString epics_pv=getEPICSExtensions(chan_desc);
    QJsonDocument chan_doc = QJsonDocument::fromJson(chan_desc.toUtf8());
    QJsonObject chan_obj = chan_doc.object();

    QJsonValue value = chan_obj.value(QString("rcalc"));
    if (value.isString()){
        value = chan_obj.value(QString("rcalc"));
        if (value.isString()) return caDOUBLE;
    }
    value = chan_obj.value(QString("dtyp"));
    if (value.isString()){
        if (value.toString().compare("float",Qt::CaseInsensitive)==0){
            return caFLOAT;
        }
    }

    if (epics_pv.compare("FTVL")==0){
         return caINT;
    }
    if ((epics_pv.compare("NORD")==0)||(epics_pv.compare("NELM")==0)){
        return caINT;
    }


    return caINT;
}

void modbus_decode::process()
{

    connect(this,  SIGNAL(TerminateIO()), this,SLOT(handle_TerminateIO()),Qt::QueuedConnection);
    connect(this,  SIGNAL(pvReconnect(knobData *)), this,SLOT(handle_pvReconnect(knobData *)),Qt::QueuedConnection);
    connect(this,  SIGNAL(create_Timer(int)), this,SLOT(handle_createTimer(int)),Qt::QueuedConnection);

    //init a standard readout timer
    handle_createTimer(500);
    modbus_terminate=false;
    modbus_disabled=false;
    device = new QModbusTcpClient(this);
    const QUrl url = modbustargetP;
    //qDebug()<<"URL:"<< url;
    //qDebug()<<"TST:"<< modbustargetP;

    device->setConnectionParameter(QModbusDevice::NetworkPortParameter,url.port());
    device->setConnectionParameter(QModbusDevice::NetworkAddressParameter,url.host());
    device->setTimeout(modbustimeout);
    device->setNumberOfRetries(modbusretries);

    connect(device, &QModbusClient::errorOccurred, [=](QModbusDevice::Error) {
           qDebug()<<"Error:"<<device->errorString()<<url;
        });

    connect(device,  SIGNAL(stateChanged(QModbusDevice::State)), this,SLOT(devicestate_changed(QModbusDevice::State)));



    if (!device->connectDevice()){
       qDebug()<< "Connection Error: " << device->errorString();
    }

    while (!modbus_terminate){
        QThread::msleep(100);
        //fflush(stdout);
        loop->processEvents();
        QModbusDataUnit* chdata=Q_NULLPTR;
        QString channel;
        {
            QMutexLocker locker(&writeData_mutex);
            if (writeData.size()){
                {
                  QPair<QString, QModbusDataUnit*> pair;
                  QPair<QString, QModbusDataUnit*> pair_next;
                  //with this loop we remove some mutiple write access
                  //specially form caSlider
                  do{
                    pair = writeData.first();
                    writeData.removeFirst();
                    if (writeData.isEmpty()) break;

                    pair_next = writeData.first();
                    if (pair.first==pair_next.first) delete(pair.second);
                  } while (pair.first==pair_next.first);
                  channel=pair.first;
                  chdata=pair.second;
                }
            }
        }
        if (device_state == QModbusDevice::ConnectedState){
            if (chdata){
                //qDebug()<<"write Data"<< chdata->value(0);
                //qDebug() << readUnit->startAddress() << int(readUnit->registerType() );
                if (chdata->isValid()){
                    //qDebug()<< "QModbusDataUnit: "<< readData;
                    int modbus_station=1;
                    modbus_channeldata* reply_channel = readData.value(channel,Q_NULLPTR);
                    if (reply_channel) modbus_station=reply_channel->getStation();
                    QModbusDataUnit cpy_chdata=*chdata;
                    delete chdata;
                    if (auto *reply = device->sendWriteRequest(cpy_chdata,modbus_station)){
                        if (!reply->isFinished()){
                            reply->setProperty("kData.channel",channel);
                            reply->setProperty("QModbusDataUnitDelete",true);
                            connect(reply, SIGNAL(finished()), this, SLOT(device_reply_data()));

                        }
                        else
                            //delete reply; // broadcast replies return immediately
                            reply->deleteLater();
                    } else{
                        qDebug()<< "Write error: " << device->errorString();
                    }

                }

            }
        }


    }





}

void modbus_decode::devicestate_changed(QModbusDevice::State state)
{
    device_state=state;
    if (state==QModbusClient::ConnectingState){
        //qDebug()<<"QModbusClient::ConnectingState";
    }else if (state==QModbusClient::ConnectedState){
        //qDebug()<<"QModbusClient::ConnectedState";
    }else if (state == QModbusDevice::UnconnectedState){
        QMutexLocker locker(&mutex);
        //qDebug()<< "QModbusDevice::UnconnectedState";
        QList<modbus_channeldata*> usedkeys=readData.values();
        //qDebug()<< usedkeys;
        foreach (modbus_channeldata* index,usedkeys){
            if (index && (index->getIndexCount()>0)){
                knobData *kData=mutexknobdataP->GetMutexKnobDataPtr(index->getIndex());
                kData->edata.monitorCount++;
                //kData->edata.displayCount=0;
                kData->edata.connected=false;
                kData->edata.severity=INVALID_ALARM;
                //mutexknobdataP->SetMutexKnobDataConnected(index->getIndex(),0);
                mutexknobdataP->SetMutexKnobData(kData->index, *kData);
                mutexknobdataP->SetMutexKnobDataReceived(kData);
            }
        }
        if (!modbus_disabled)  device->connectDevice();
    }else if (state == QModbusDevice::ConnectedState){
        //qDebug()<< "QModbusDevice::ConnectedStat";
    }

}

void modbus_decode::trigger_modbusrequest()
{
    QMutexLocker locker(&mutex);
    QTimer *timer = (QTimer *)sender();
    int timer_cycle=CYCLE_CONSTANT_TIME;
    QVariant varcycle = timer->property("modbus_cycle");
    if (!varcycle.isNull())
        if (varcycle.canConvert<int>())
           timer_cycle=varcycle.toInt();

    //qDebug() << "ModbusCycle: "<< timer_cycle << device_state;
    QModbusDataUnit readUnit;
    if (device_state == QModbusDevice::ConnectedState){
        bool request_data=true;
        {
            QMutexLocker locker(&writeData_mutex);
            request_data=writeData.size()==0;
        }
        if (request_data){
            QList<QString> usedkeys=readData.keys();
            //qDebug() << "ModbusKeys: "<< usedkeys;
            foreach (QString Channel,usedkeys){

                QMap<QString,modbus_channeldata*>::iterator i = readData.find(Channel);
                while (i !=readData.end() && i.key() == Channel) {
                   //qDebug() << "ModbusCount: "<<i.value()->getIndexCount();
                    if (i.value()->getIndexCount()>0)
                    for (int x=0;x<i.value()->getReadUnit_count();x++){
                        readUnit=i.value()->getReadUnit(x);

                        if (timer_cycle==i.value()->getCycleTime())

                            //qDebug() << readUnit.startAddress() << int(readUnit.registerType() );
                            if (readUnit.isValid()){
                                //qDebug()<< "QModbusDataUnit: "<< readUnit.registerType() << readUnit.startAddress() << readUnit.valueCount() << Channel;
                                if (auto *reply = device->sendReadRequest(readUnit,i.value()->getStation())) {
                                    if (!reply->isFinished()){

                                        reply->setProperty("kData.channel",Channel);
                                        reply->setProperty("QModbusDataUnitDelete",false);
                                        connect(reply, SIGNAL(finished()), this, SLOT(device_reply_data()));
                                    }
                                    else
                                        delete reply; // broadcast replies return immediately
                                } else{
                                    qDebug()<< "Read error: " << device->errorString();
                                    qDebug()<< readUnit.registerType() << readUnit.startAddress() << readUnit.valueCount();

                                }

                            } else {printf(".");fflush(stdout);}


                    }





                    ++i;
                }

            }

        }
    }
}

void modbus_decode::device_reply_data()
{

    QModbusReply *reply = (QModbusReply *)sender();
    if (!reply) return;
    if (reply->isFinished()){

        if (reply->error() == QModbusDevice::NoError) {
            const QModbusDataUnit unit = reply->result();

            QVariant varindex = reply->property("kData.channel");
            if (!varindex.isNull())
                if (varindex.canConvert<QString>()){
                    modbus_channeldata* reply_channel = readData.value(varindex.toString(),Q_NULLPTR);
                    if (reply_channel){
                        foreach (int index,reply_channel->getIndexes()) {



                            knobData *kData=mutexknobdataP->GetMutexKnobDataPtr(index);
                            modbus_channeldata* chdata=(modbus_channeldata*)kData->edata.info;

                            if  (unit.valueCount()>1){
                                if ((kData->edata.fieldtype==caFLOAT)&&(unit.valueCount()*sizeof(quint16)==sizeof(float))){
                                    qint32 combined = (unit.value(1) << 16) | unit.value(0);
                                    //qDebug() << "ReadFloat:" << unit.value(0) << unit.value(1);
                                    float* num =(float*) &combined;
                                    kData->edata.rvalue=(double)*num;
                                    kData->edata.monitorCount++;
                                }else{
                                    if (kData->edata.fieldtype==caINT){
                                        if ((chdata->getModbus_count()*sizeof(qint16)+1024)!=kData->edata.dataSize){
                                            kData->edata.dataSize=(chdata->getModbus_count()*sizeof(qint16))+10;
                                            if (kData->edata.dataB){
                                                kData->edata.dataB=realloc(kData->edata.dataB,kData->edata.dataSize);
                                            }else{
                                                kData->edata.dataB=malloc(kData->edata.dataSize);
                                            }
                                        }
                                        int datashift=unit.startAddress()-reply_channel->getModbus_addr();
                                        for (uint i = 0; i < unit.valueCount(); i++) {
                                            ((qint16*) kData->edata.dataB)[i+datashift]=unit.value(i);
                                        }
                                        // Achtung sollte noch optimiert werden!!!!
                                        kData->edata.monitorCount++;
                                    }
                                    if (kData->edata.fieldtype==caDOUBLE){
                                        if ((chdata->getModbus_count()*sizeof(double)+1024)!=kData->edata.dataSize){
                                            kData->edata.dataSize=(chdata->getModbus_count()*sizeof(double))+1024;
                                            if (kData->edata.dataB){
                                                kData->edata.dataB=realloc(kData->edata.dataB,kData->edata.dataSize);
                                            }else{
                                                kData->edata.dataB=malloc(kData->edata.dataSize);
                                            }
                                        }
                                        QModbusDataUnit convert = unit;
                                        do_the_calculation(varindex.toString(),&convert,kData,modbus_READ);
                                        // Achtung sollte noch optimiert werden!!!!
                                        kData->edata.monitorCount++;
                                    }



                                }
                            }else{
                                kData->edata.fieldtype=caINT;
                                if (kData->edata.ivalue!=unit.value(0)){
                                    kData->edata.ivalue=unit.value(0);
                                    kData->edata.rvalue=unit.value(0);
                                    kData->edata.monitorCount++;
                                }

                                if (chdata->getValid_calc())  {
                                    QModbusDataUnit convert = unit;
                                    do_the_calculation(varindex.toString(),&convert,kData,modbus_READ);
                                }
                            }

                            if (kData->edata.monitorCount<2){
                                kData->edata.monitorCount++;
                            }
                            kData->edata.severity=NO_ALARM;
                            kData->edata.connected=true;


                            mutexknobdataP->SetMutexKnobData(kData->index, *kData);
                            mutexknobdataP->SetMutexKnobDataReceived(kData);
                        }}
                        //reply->setProperty("QModbusDataUnitDelete",true);
//                        QVariant vardelete = reply->property("QModbusDataUnitDelete");
//                        if (!vardelete.isNull())
//                            if (vardelete.canConvert<bool>())
//                                if (vardelete.toBool())


                }else{
                    qDebug()<< "NoIndex";
                }
        } else{
            //qDebug()<< "Error: ";
            QVariant varindex = reply->property("kData.channel");

            if (!varindex.isNull())
                if (varindex.canConvert<QString>()){
                    modbus_channeldata* reply_channel = readData.value(varindex.toString(),Q_NULLPTR);
                    if (reply_channel){
                        foreach (int index,reply_channel->getIndexes()) {

                            knobData *kData=mutexknobdataP->GetMutexKnobDataPtr(index);
                            kData->edata.severity=INVALID_ALARM;
                            kData->edata.monitorCount++;
                            mutexknobdataP->SetMutexKnobDataReceived(kData);
                        }
                    }
                }
        }
        reply->deleteLater();

    }
}

void modbus_decode::setModbus_translation_map(const QMap<QString, QString> &value)
{
    modbus_translation_map = value;
}

int modbus_decode::do_the_calculation(QString modbus_connection,QModbusDataUnit *unit, knobData *kData, modbus_calc_direction direction)
{

    double valueArray[CALCPERFORM_NARGS];
    char post[256];
    short errnum;
    double result;
    int status = -1;

    if (!kData && !unit) return MODBUS_ERROR;
    modbus_channeldata* chdata=(modbus_channeldata*)kData->edata.info;

    switch(direction){
    case modbus_READ:{
        valueArray[0]=unit->value(0);
        if (!chdata->getRcalc().isEmpty()){
            status = postfix(chdata->getRcalc().toLatin1().data(), post, &errnum);
        }else chdata->setInvalid_calc();
        break;
    }
    case modbus_WRITE:{

        valueArray[0]=kData->edata.rvalue;
        if (!chdata->getWcalc().isEmpty()){
            status = postfix(chdata->getWcalc().toLatin1().data(), post, &errnum);
        }else chdata->setInvalid_calc();
        break;
    }
    default: return MODBUS_ERROR;
    }
    if (!chdata->getValid_calc()) return MODBUS_ERROR;

    if (status){
        chdata->setInvalid_calc();
        QString msg="modbus: Invalid calc found in: ";
        msg.append(kData->pv);
        msg.append("/"+modbus_connection);
        if(messagewindowP != Q_NULLPTR) messagewindowP->postMsgEvent(QtDebugMsg,(char*) msg.toLatin1().constData());
        return MODBUS_ERROR;
    }

    switch(direction){
        case modbus_READ:{
            if (chdata->getModbus_count()>1){
                int datashift=unit->startAddress()-chdata->getModbus_addr();
                for (uint i = 0; i < unit->valueCount(); i++) {
                    valueArray[0]=(qint16)unit->value(i);
                    status = calcPerform(valueArray, &result, post);
                    if(status) break;
                    ((double*) kData->edata.dataB)[i+datashift]=result;
                }
                if (status){
                    chdata->setInvalid_calc();
                    QString msg="modbus: Invalid READ array calc found in: ";
                    msg.append(kData->pv);
                    msg.append("/"+modbus_connection);
                    if(messagewindowP != Q_NULLPTR) messagewindowP->postMsgEvent(QtDebugMsg,(char*) msg.toLatin1().constData());
                    return MODBUS_ERROR;
                }
            }else{
                status = calcPerform(valueArray, &result, post);
                if(!status) {
                    kData->edata.fieldtype=caDOUBLE;
                    kData->edata.rvalue=result;
                } else{
                    chdata->setInvalid_calc();
                    QString msg="modbus: Invalid READ calc found in: ";
                    msg.append(kData->pv);
                    msg.append("/"+modbus_connection);
                    if(messagewindowP != Q_NULLPTR) messagewindowP->postMsgEvent(QtDebugMsg,(char*) msg.toLatin1().constData());
                    return MODBUS_ERROR;
                }
            }


            break;
        }
        case modbus_WRITE:{
            status = calcPerform(valueArray, &result, post);
            if(!status) {
                unit->setValue(0,(quint16)qRound(result));
            } else{
                chdata->setInvalid_calc();
                QString msg="modbus: Invalid WRITE calc found in: ";
                msg.append(kData->pv);
                msg.append("/"+modbus_connection);
                if(messagewindowP != Q_NULLPTR) messagewindowP->postMsgEvent(QtDebugMsg,(char*) msg.toLatin1().constData());
                return MODBUS_ERROR;
            }
            break;
        }
        default: return MODBUS_ERROR;
    }

    return MODBUS_OK;
}

QUrl modbus_decode::getModbustarget() const
{
    return modbustargetP;
}

void modbus_decode::setModbustarget(const QUrl &value)
{
    modbustargetP = value;
}

int modbus_decode::getModbustimeout() const
{
    return modbustimeout;
}

void modbus_decode::setModbustimeout(int value)
{
    modbustimeout = value;
}

int modbus_decode::getModbusretries() const
{
    return modbusretries;
}

void modbus_decode::setModbusretries(int value)
{
    modbusretries = value;
}

int modbus_decode::pvAddMonitor(int index, knobData *kData)
{
    Q_UNUSED(index)
    QMutexLocker locker(&mutex);
    // Example
    // modbus://129.129.130.73:502{type:"coils",addr:0,count:1,datatype:"float",rcalc:"A*10",wcalc:"A/10",egu:"Bla",cycle:10}

    //Rate = QTimer
        //DatenTypen = 4
            //Adresse,count
    QString target=kData->pv;

    QString replace=modbus_translation_map.value(removeEPICSExtensions(target),"");
    if (!replace.isEmpty()){
        target=target.replace(removeEPICSExtensions(target),modbus_translation_map.value(removeEPICSExtensions(target)));
    }



    QString chan_desc=removeHost(target);

    QString epics_pv=getEPICSExtensions(chan_desc);

    modbus_channeldata* chdata=readData.value(removeEPICSExtensions(chan_desc),Q_NULLPTR);
    if (chdata==Q_NULLPTR){
        QModbusDataUnit::RegisterType modbus_type;
        int modbus_addr;
        int modbus_count;
        int modbus_cycle;
        int modbus_station;
        short modbus_prec=0;
        QString modbus_rcalc;
        QString modbus_wcalc;

        QJsonDocument chan_doc = QJsonDocument::fromJson(removeEPICSExtensions(chan_desc).toUtf8());
        QJsonObject chan_obj = chan_doc.object();
        QJsonValue value;

//        foreach(const QString& key, chan_obj.keys()) {
//            QJsonValue value = chan_obj.value(key);
//            qDebug() << "Key = " << key << ", Value = " << value.toString();
//        }
        //qDebug() << "***********************************************************************";


        //DiscreteInputs,D
        //Coils,C
        //InputRegisters,I
        //HoldingRegisters,H

        value = chan_obj.value(QString("type"));
        if (value.isString()){
            QString teststr=value.toString();
            if (teststr.isEmpty())  return MODBUS_ERROR;
            if (teststr.compare("DiscreteInputs",Qt::CaseInsensitive)==0) modbus_type=QModbusDataUnit::DiscreteInputs;
            if (teststr.compare("D",Qt::CaseSensitive)==0) modbus_type=QModbusDataUnit::DiscreteInputs;
            if (teststr.compare("Coils",Qt::CaseInsensitive)==0) modbus_type=QModbusDataUnit::Coils;
            if (teststr.compare("C",Qt::CaseSensitive)==0) modbus_type=QModbusDataUnit::Coils;
            if (teststr.compare("InputRegisters",Qt::CaseInsensitive)==0) modbus_type=QModbusDataUnit::InputRegisters;
            if (teststr.compare("I",Qt::CaseSensitive)==0) modbus_type=QModbusDataUnit::InputRegisters;
            if (teststr.compare("HoldingRegisters",Qt::CaseInsensitive)==0) modbus_type=QModbusDataUnit::HoldingRegisters;
            if (teststr.compare("H",Qt::CaseSensitive)==0) modbus_type=QModbusDataUnit::HoldingRegisters;
        }

        value = chan_obj.value(QString("addr"));
        if (value.isDouble()){
            modbus_addr=int(value.toDouble());
        }
        value = chan_obj.value(QString("count"));
        if (value.isDouble()){
            modbus_count=int(value.toDouble());
        }else{
            modbus_count=1;
        }

        value = chan_obj.value(QString("dtyp"));
        if (value.isString()){
            if (value.toString().compare("float",Qt::CaseInsensitive)==0){

                modbus_count=2;
            }else if (value.toString().compare("char",Qt::CaseInsensitive)==0){
                //kData->edata.fieldtype=caCHAR; // not tested
            }
        }




        value = chan_obj.value(QString("cycle"));
        if (value.isDouble()){
            modbus_cycle=int(value.toDouble());
            if (modbus_cycle<10) modbus_cycle=10;
        }else{
            modbus_cycle=CYCLE_CONSTANT_TIME;
        }
        value = chan_obj.value(QString("stat"));
        if (value.isDouble()){
            modbus_station=int(value.toDouble());
        }else{
            modbus_station=1;
        }


        value = chan_obj.value(QString("egu"));
        if (value.isString()){
            qstrncpy(kData->edata.units,value.toString().toLatin1().data(),39);
        }

        value = chan_obj.value(QString("rcalc"));
        if (value.isString()){

            modbus_rcalc=value.toString();
        }
        value = chan_obj.value(QString("wcalc"));
        if (value.isString()){

            modbus_wcalc=value.toString();
        }

        value = chan_obj.value(QString("prec"));
        if (value.isDouble()){
            modbus_prec=short(value.toDouble());
        }

        //qDebug()<< "QModbusDataUnit : " << int(modbus_type)<< modbus_addr << modbus_count ;
        QList<QModbusDataUnit> readUnit;
        if (modbus_count<MODBUS_MAX_SEGMENT_SIZE){
            readUnit.append(QModbusDataUnit(modbus_type,modbus_addr,modbus_count));
        }else{
            int segmentcount=modbus_count/MODBUS_MAX_SEGMENT_SIZE;
            int segmentlast=modbus_count%MODBUS_MAX_SEGMENT_SIZE;
            for (int x=0;x<segmentcount;x++){
                readUnit.append(QModbusDataUnit(modbus_type,modbus_addr+(x*MODBUS_MAX_SEGMENT_SIZE),MODBUS_MAX_SEGMENT_SIZE));
            }
            readUnit.append(QModbusDataUnit(modbus_type,modbus_addr+(segmentcount*MODBUS_MAX_SEGMENT_SIZE),segmentlast));
        }

        //qDebug()<< "readData.insert : "<<kData->index<<readUnit->valueCount()<<removeEPICSExtensions(chan_desc);
        chdata=new modbus_channeldata(kData->index,readUnit);
        chdata->setCycleTime(modbus_cycle);
        chdata->setStation(modbus_station);
        chdata->setRcalc(modbus_rcalc);
        chdata->setWcalc(modbus_wcalc);
        chdata->setPrecision(modbus_prec);
        readData.insert(removeEPICSExtensions(chan_desc),chdata);
        //qDebug()<< "emit create_Timer ";
        emit this->create_Timer(modbus_cycle);

    }else{
       chdata->addIndex(kData->index);
    }

    if (chdata!=Q_NULLPTR){
        if (epics_pv.isEmpty()){
            switch (chdata->getModbus_type()){
            case QModbusDataUnit::Coils:{
                kData->edata.accessR=true;
                kData->edata.accessW=true;
                break;
            }
            case QModbusDataUnit::InputRegisters:{
                kData->edata.accessR=true;
                kData->edata.accessW=false;
                break;
            }
            case QModbusDataUnit::DiscreteInputs:{
                kData->edata.accessR=true;
                kData->edata.accessW=false;
                break;
            }
            case QModbusDataUnit::HoldingRegisters:{
                kData->edata.accessR=true;
                kData->edata.accessW=true;
                break;
            }
            case QModbusDataUnit::Invalid:{
                kData->edata.accessR=false;
                kData->edata.accessW=false;
                break;
            }
            }
            kData->edata.fieldtype=generatecaDataType(target);
            kData->edata.status=0;
            kData->edata.info=chdata;
            kData->edata.valueCount=chdata->getModbus_count();
            kData->edata.precision=chdata->getPrecision();
            if (kData->edata.fieldtype==caFLOAT){
               kData->edata.valueCount=1;
            }else
            if (chdata->getModbus_count()>1){
                kData->edata.dataSize=chdata->getModbus_count()*sizeof(qint16)+1024;
                kData->edata.dataB=malloc(kData->edata.dataSize);
            }else{
                kData->edata.dataB=Q_NULLPTR;
            }

        }else{
           if (epics_pv.compare("FTVL")==0){
                kData->edata.fieldtype=generatecaDataType(target);
                kData->edata.ivalue=4;//menuFtypeUSHORT,"USHORT"
           }
           if ((epics_pv.compare("NORD")==0)||(epics_pv.compare("NELM")==0)){
                kData->edata.fieldtype=generatecaDataType(target);
                QMap<QString,modbus_channeldata*>::iterator i = readData.find(chan_desc);
                while (i !=readData.end() && i.key() == chan_desc) {
                    kData->edata.ivalue=chdata->getModbus_count();
                }
           }



        }

        QString ioc_string=modbustargetP.host()+":"+QString::number(modbustargetP.port());
        qstrncpy(kData->edata.fec,ioc_string.toLatin1().constData(),39);
        mutexknobdataP->SetMutexKnobData(kData->index, *kData);
        //qDebug()<< "kdata params set ";
    }
    return MODBUS_OK;
}

int modbus_decode::pvDisconnect(knobData *kData)
{
//    QMutexLocker locker(&mutex);
//    QString pv=removeHost(kData->pv);
//    QMap<QString,modbus_channeldata*>::iterator i = readData.find(pv);
//    while (i !=readData.end() && i.key() == pv) {
//        if (i.value()) i.value()->delIndex(kData->index);
//        ++i;
//    }
    pvClearMonitor(kData);
    return MODBUS_OK;
}

bool modbus_decode::pvClearMonitor(knobData *kData)
{
    QMutexLocker locker(&mutex);
    QString target=kData->pv;
    QString replace=modbus_translation_map.value(target,"");
    if (!replace.isEmpty()){
        target=modbus_translation_map.value(target);
    }

    QString pv=removeHost(target);
    QMap<QString,modbus_channeldata*>::iterator i = readData.find(pv);
    while (i !=readData.end() && i.key() == pv) {
        i.value()->setClearMonitor(kData->index);
        ++i;
    }
    kData->edata.connected=false;
    kData->edata.monitorCount++;
    mutexknobdataP->SetMutexKnobData(kData->index, *kData);
    mutexknobdataP->SetMutexKnobDataReceived(kData);
    return true;
}

void modbus_decode::handle_pvReconnect(knobData *kData)
{
    qDebug() << "modbus_decode:pvReconnect";
    QMutexLocker locker(&mutex);
    if (modbus_disabled){
        device->connectDevice();
        modbus_disabled=false;
    }

    QString target=kData->pv;
    QString replace=modbus_translation_map.value(target,"");
    if (!replace.isEmpty()){
        target=modbus_translation_map.value(target);
    }

    QString pv=removeHost(target);
    QMap<QString,modbus_channeldata*>::iterator i = readData.find(pv);
    while (i !=readData.end() && i.key() == pv) {
        i.value()->pvReconnect(kData->index);
        ++i;
    }
    kData->edata.connected=true;
    kData->edata.monitorCount++;
    mutexknobdataP->SetMutexKnobData(kData->index, *kData);
    mutexknobdataP->SetMutexKnobDataReceived(kData);

}

void modbus_decode::handle_createTimer(int modbus_cycle)
{

    QTimer* selected_timer=running_Timer.value(modbus_cycle,Q_NULLPTR);
    if (!selected_timer){
        //qDebug() << "handle_createTimer" << modbus_cycle;
        selected_timer=new QTimer(this);
        selected_timer->setInterval(modbus_cycle);
        selected_timer->setProperty("modbus_cycle",modbus_cycle);
        connect(selected_timer, SIGNAL(timeout()), this, SLOT(trigger_modbusrequest()));
        selected_timer->start();
        running_Timer.insert(modbus_cycle,selected_timer);
        QString msg=QString("modbus timer started cycle time %1 ms").arg(modbus_cycle);
        if(messagewindowP != Q_NULLPTR) messagewindowP->postMsgEvent(QtDebugMsg,(char*) msg.toLatin1().constData());

    }

}

void modbus_decode::handle_TerminateIO()
{
    qDebug() << "modbus_decode:TerminateIO";
    modbus_disabled=true;
    device->disconnectDevice();
}

void modbus_decode::setTerminate()
{
    messagewindowP=Q_NULLPTR;
    modbus_terminate = true;

}

int modbus_decode::pvSetValue(char *pv, double rdata, int32_t idata, char *sdata, char *object, char *errmess, int forceType)
{
    Q_UNUSED(sdata)
    Q_UNUSED(object)
    Q_UNUSED(errmess)
    Q_UNUSED(forceType)
    QPair<QString, QModbusDataUnit*> pair;

    QString target=pv;
    QString replace=modbus_translation_map.value(target,"");
    if (!replace.isEmpty()){
        target=modbus_translation_map.value(target);
    }


    QModbusDataUnit* data=generateDataUnit(target);
    if (data){
        switch (generatecaDataType(target)){
        case caINT:{
            data->setValue(0,quint16(idata));
            break;
        }
        case caFLOAT:{

            float floatdata=(float)rdata;
            quint32* num=(quint32*) &floatdata;
            quint32 combined = *num;
            QVector<quint16> rawdata(sizeof(float)/sizeof(quint16));
            rawdata[1]=(quint16)(((combined) & 0xFFFF0000)>>16);
            rawdata[0]=(quint16)((combined) & 0xFFFF);
            data->setValues(rawdata);
            //qDebug() << "Float:" << rdata << "/"<< rawdata[0] << rawdata[1] << rawdata;


            break;
        }
        case caDOUBLE:{
            data->setValue(0,quint16(rdata));
            modbus_channeldata* chdata = readData.value(removeHost(target),Q_NULLPTR);

            if (chdata && (chdata->getValid_calc()))  {
                knobData *kData=mutexknobdataP->GetMutexKnobDataPtr(chdata->getIndex());
                //qDebug()<< "rdata: "<< rdata;
                kData->edata.rvalue=rdata;
                do_the_calculation(removeHost(target),data,kData,modbus_WRITE);
            }

            break;
        }
        default:data->setValue(0,quint16(idata));

        }


        {
            pair.first=removeHost(target);
            pair.second=data;
            QMutexLocker locker(&writeData_mutex);
            writeData.append(pair);
        }
    }
    return MODBUS_OK;

}

int modbus_decode::pvGetTimeStamp(char *pv, char *timestamp)
{


    QString target=pv;
    QString replace=modbus_translation_map.value(target,"");
    if (!replace.isEmpty()){
        target=modbus_translation_map.value(target);
    }

    QString strippedpv=removeEPICSExtensions(removeHost(target));

    //qDebug() << "pvGetTimeStamp:"<< strippedpv;

    QMap<QString,modbus_channeldata*>::iterator i = readData.find(strippedpv);
    while (i !=readData.end() && i.key() == strippedpv) {
        if (i.value()->getIndexCount()>0){
            knobData *kData=mutexknobdataP->GetMutexKnobDataPtr(i.value()->getIndex());
            modbus_channeldata* chdata=(modbus_channeldata*)kData->edata.info;
            //qDebug() << "chdata" << chdata;
            if (chdata){
                chdata->process_timestamp(timestamp);

                return MODBUS_OK;

            }
        }
        i++;
    }
    return MODBUS_ERROR;
}
