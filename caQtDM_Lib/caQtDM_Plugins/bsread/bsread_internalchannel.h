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
#ifndef BSREAD_INTERNALCHANNEL_H
#define BSREAD_INTERNALCHANNEL_H

#include <QObject>
#include <QList>
#include <QAtomicInt>
#include "knobData.h"
#include <QMutex>


class bsread_internalchannel : public QObject
{
    Q_OBJECT
    Q_ENUMS(internal_types)
public:
    bsread_internalchannel(QObject *parent = 0,QString channelname="",QString option="");

    enum internal_types{in_string,in_enum,in_none};

    void addIndex(int setindex);
    int getIndex(int i) const;
    int getIndexCount() const;
    void deleteIndex(int i);

    QString getPv_name() const;
    QString getOption_name() const;

    void *getData() const;
    void setData(void *value,internal_types value_type);
    internal_types getType() const;

    QString getString();
    bool setString(QString value);

    void addEnumString(QString value);
    int getEnumCount();
    QString getEnumStrings();
    QString setEnumIndex(int value);
    int getEnumIndex();

    bool getProc();
    void resetProc();
private:
    QMutex mutex;
    bool proc;
    QString pv_name;
    QString option_name;
    QList<int> index;
    internal_types type;
    QString internal_string;
    QList<QString> internal_enum_strings;
    int internal_enum_index;
    void* data;






signals:

public slots:
};



#endif // BSREAD_INTERNALCHANNEL_H
