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
#include "modbus_channeldata.h"

modbus_channeldata::modbus_channeldata()
{
    readUnit=new QModbusDataUnit();
    this->index.clear();
    generation_time= QDateTime::currentDateTime();
    will_be_written=false;
    valid_calc=true;
}

modbus_channeldata::modbus_channeldata( int index, QModbusDataUnit *readUnit)
{
    this->index.clear();
    this->index.append(index);
    this->readUnit=readUnit;
    generation_time= QDateTime::currentDateTime();
    will_be_written=false;
    valid_calc=true;
}

int modbus_channeldata::getIndex() const
{
    return index.first();
}

void modbus_channeldata::trigger_request()
{
    request_time= QDateTime::currentDateTime();
}

void modbus_channeldata::trigger_process()
{
    process_time= QDateTime::currentDateTime();
}

void modbus_channeldata::process_timestamp(char *timestamp)
{
    QString time=process_time.toString("hh:mm:ss.zzz");
    time="TimeStamp: "+time;
    qstrncpy(timestamp,time.toLatin1().data(),50);// see defenition of timestamp char(50)
}

bool modbus_channeldata::getWill_be_written()
{
    QMutexLocker locker(&mutex);
    return will_be_written;
}

void modbus_channeldata::setWill_be_written()
{
    QMutexLocker locker(&mutex);
    will_be_written = true;
}

void modbus_channeldata::resetWill_be_written()
{
    QMutexLocker locker(&mutex);
    will_be_written=false;
}

int modbus_channeldata::getCycleTime() const
{
    return CycleTime;
}

void modbus_channeldata::setCycleTime(int value)
{
    CycleTime = value;
}

int modbus_channeldata::getStation() const
{
    return Station;
}

void modbus_channeldata::setStation(int value)
{
    Station = value;
}

QString modbus_channeldata::getRcalc() const
{
    return rcalc;
}

void modbus_channeldata::setRcalc(const QString &value)
{
    rcalc = value;
}

QString modbus_channeldata::getWcalc() const
{
    return wcalc;
}

void modbus_channeldata::setWcalc(const QString &value)
{
    wcalc = value;
}

bool modbus_channeldata::getValid_calc() const
{
    return valid_calc;
}

void modbus_channeldata::setInvalid_calc()
{
    valid_calc=false;
}

short modbus_channeldata::getPrecision() const
{
    return Precision;
}

void modbus_channeldata::setPrecision(short value)
{
    Precision = value;
}

QModbusDataUnit *modbus_channeldata::getReadUnit() const
{
    return readUnit;
}

void modbus_channeldata::addIndex(int pvindex)
{
    index.append(pvindex);
}

void modbus_channeldata::delIndex(int pvindex)
{
    int pos = index.indexOf(pvindex);
    while (pos>=0){
        index.takeAt(pos);
        pos=index.indexOf(pvindex);
    }
}

int modbus_channeldata::getModbus_count() const
{
    return readUnit->valueCount();
}

void modbus_channeldata::setModbus_count(int value)
{
    readUnit->setValueCount(value);
}

int modbus_channeldata::getModbus_addr() const
{
    return readUnit->startAddress();
}

void modbus_channeldata::setModbus_addr(int value)
{
    readUnit->setStartAddress(value);
}

QModbusDataUnit::RegisterType modbus_channeldata::getModbus_type() const
{
    return readUnit->registerType();
}

void modbus_channeldata::setModbus_type(const QModbusDataUnit::RegisterType &value)
{
    readUnit->setRegisterType(value);
}
