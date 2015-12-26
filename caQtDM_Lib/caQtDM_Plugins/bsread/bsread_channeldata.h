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
#ifndef BSREAD_CHANNELDATA_H
#define BSREAD_CHANNELDATA_H

#include <QObject>
#include <QList>


enum bsread_types{
    bs_double,bs_string,bs_integer,bs_long,bs_short
};

enum bsread_endian{
    bs_little,bs_big
};
typedef struct _bs_data{
   double bs_double;
   QString bs_string;
   int bs_integer;
   long bs_long;
   short bs_short;
   ulong wf_data_size;
   void* wf_data;
}bs_data;

class bsread_channeldata : public QObject
{
    Q_OBJECT
public:
    explicit bsread_channeldata(QObject *parent = 0);
    QString name;
    bsread_types type;
    QList<int> shape;
    int offset;
    int modulo;
    bsread_endian endianess;
    double timestamp;
    bs_data bsdata;
signals:

public slots:
};

#endif // BSREAD_CHANNELDATA_H
