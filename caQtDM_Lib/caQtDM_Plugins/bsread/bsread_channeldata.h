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
#include <QAtomicInt>

enum bsread_types{
    bs_string,bs_float64,bs_float32,bs_int64,bs_int32,bs_uint64,bs_uint32,bs_int16,bs_uint16,bs_int8,bs_uint8,bs_bool
};

enum bsread_endian{
    bs_little,bs_big
};
typedef struct _bs_data{
   QString bs_string;
   double bs_float64;
   float bs_float32;
   qint64 bs_int64;
   qint32 bs_int32;
   quint64 bs_uint64;
   quint32 bs_uint32;
   qint16 bs_int16;
   quint16 bs_uint16;
   qint8 bs_int8;
   quint8 bs_uint8;
   bool bs_bool;
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
    bool valid;
    int offset;
    int modulo;
    int precision;
    QString units;
    bsread_endian endianess;
    double timestamp;
    bs_data bsdata;
signals:

public slots:
};

#endif // BSREAD_CHANNELDATA_H
