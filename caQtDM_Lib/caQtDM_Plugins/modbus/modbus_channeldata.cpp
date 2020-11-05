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

    this->index.clear();
    generation_time= QDateTime::currentDateTime();

    valid_calc=true;
}

modbus_channeldata::modbus_channeldata( int index, QModbusDataUnit *readUnit)
{
    this->index.clear();
    this->index.append(index);
    this->readUnits.append(*readUnit);
    generation_time= QDateTime::currentDateTime();

    valid_calc=true;
}

modbus_channeldata::modbus_channeldata( int index, QList<QModbusDataUnit> readUnits)
{
    this->index.clear();
    this->index.append(index);
    this->readUnits=readUnits;
    generation_time= QDateTime::currentDateTime();

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

void modbus_channeldata::setClearMonitor(int index)
{
    delIndex(index);
}

void modbus_channeldata::pvReconnect(int index)
{
    addIndex(index);
}

QModbusDataUnit modbus_channeldata::getReadUnit()
{
    return readUnits.first();
}

QModbusDataUnit modbus_channeldata::getReadUnit(int n)
{
    return readUnits.at(n);
}

int modbus_channeldata::getReadUnit_count()
{
    return readUnits.count();
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

int modbus_channeldata::getIndexCount() const
{
    return index.count();
}

int modbus_channeldata::getModbus_count() const
{
    int datasize=0;
    foreach (QModbusDataUnit i,readUnits) datasize=datasize+i.valueCount();

    return datasize;
}

void modbus_channeldata::setModbus_count(int value)
{
    readUnits.first().setValueCount(value);
}

int modbus_channeldata::getModbus_addr() const
{
    return readUnits.first().startAddress();
}

void modbus_channeldata::setModbus_addr(int value)
{
    readUnits.first().setStartAddress(value);
}

QModbusDataUnit::RegisterType modbus_channeldata::getModbus_type() const
{
    return readUnits.first().registerType();
}

void modbus_channeldata::setModbus_type(const QModbusDataUnit::RegisterType &value)
{
    readUnits.first().setRegisterType(value);
}
