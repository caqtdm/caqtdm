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
 *  Copyright (c) 2010 - 2015
 *
 *  Author:
 *    Helge Brands
 *  Contact details:
 *    helge.brands@psi.ch
 */
#include "bsread_internalchannel.h"


void *bsread_internalchannel::getData() const
{
    return data;
}

void bsread_internalchannel::setData(void *value, bsread_internalchannel::internal_types value_type)
{
    data = value;
    type = value_type;
}

bsread_internalchannel::internal_types bsread_internalchannel::getType() const
{
    return type;
}
bool bsread_internalchannel::getProc()
{
    bool returnvalue=proc;
    proc=false;
    return returnvalue;
}


bsread_internalchannel::bsread_internalchannel(QObject *parent,QString channelname,QString option) : QObject(parent)
{
    pv_name=channelname;
    option_name=option;
    index.clear();
}
QString bsread_internalchannel::getOption_name() const
{
    return option_name;
}

QString bsread_internalchannel::getPv_name() const
{
    return pv_name;
}

void bsread_internalchannel::addIndex(int setindex)
{
    index.append(setindex);
}

void bsread_internalchannel::deleteIndex(int i)
{
    int d=0;
    if (index.count()>0){
        while ((d<index.count())||(index.at(d)!=i) ){
            d++;
        }
        if (index.at(d)==i){
          index.removeAt(d);
        }
    }
}


int bsread_internalchannel::getIndex(int i) const
{
    return index.value(i);
}

int bsread_internalchannel::getIndexCount() const
{
    return index.count();
}


QString bsread_internalchannel::getString()
{
    QMutexLocker locker(&mutex);
    //proc=true;
    return internal_string;
}

void bsread_internalchannel::setString(QString value)
{
    QMutexLocker locker(&mutex);
    proc=true;
    internal_string=value;
}
