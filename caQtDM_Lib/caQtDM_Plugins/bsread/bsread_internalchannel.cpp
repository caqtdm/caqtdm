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
void bsread_internalchannel::resetProc()
{
    proc=false;
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
    QMutexLocker locker(&mutex);
    if (index.count()>0){
        if (index.indexOf(i)!=-1){
          index.removeAt(index.indexOf(i));
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
    return internal_string;
}

bool bsread_internalchannel::setString(QString value)
{
    QMutexLocker locker(&mutex);

    if (this->getType()==bsread_internalchannel::in_enum){
     int i = internal_enum_strings.indexOf(value);
     if (i != -1){
      if (internal_enum_index!=i) proc=true;
         internal_enum_index=i;

     }else{
      return false;
     }
    }
    if (internal_string.compare(value)!=0) proc=true;
    internal_string=value;

    return true;
}

void bsread_internalchannel::addEnumString(QString value)
{
    internal_enum_strings.append(value);
}

int bsread_internalchannel::getEnumCount()
{
    return internal_enum_strings.count();
}

QString bsread_internalchannel::getEnumStrings()
{
    QString return_value;
    for(int x=0;x<internal_enum_strings.count();x++){
      return_value.append(internal_enum_strings.at(x));
      if (x<internal_enum_strings.count()-1) return_value.append((QChar)27);
    }

    return return_value;
}

QString bsread_internalchannel::setEnumIndex(int value)
{
    QString Old_internal_string=internal_string;

    if (value>(internal_enum_strings.count()-1)){
        internal_enum_index=internal_enum_strings.count()-1;
        internal_string=internal_enum_strings.at(internal_enum_index);
    }else{
        if (value>0){
            internal_string=internal_enum_strings.at(value);
            internal_enum_index=value;
        }else{
            internal_string=internal_enum_strings.at(0);
            internal_enum_index=0;
        }
    }
    if (internal_string.compare(Old_internal_string)!=0) proc=true;
    return internal_string;
}

int bsread_internalchannel::getEnumIndex()
{
    if (internal_enum_index>(internal_enum_strings.count()-1)){
        internal_enum_index=internal_enum_strings.count()-1;
    }else{
        if (internal_enum_index<0){
          internal_enum_index=0;
        }
    }
 return internal_enum_index;
}
