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

#ifndef _CALED_H
#define _CALED_H

#include <QWidget>
#include <qtcontrols_global.h>
#include <eled.h>

class QTCON_EXPORT caLed : public ELed
{
Q_OBJECT

  Q_PROPERTY(QString channel READ getPV WRITE setPV)
  Q_PROPERTY(int bitNr READ getBitNr WRITE setBitNr)
  Q_PROPERTY(colMode colorMode READ getColorMode WRITE setColorMode)
  Q_PROPERTY(QColor falseColor READ getFalseColor WRITE setFalseColor)
  Q_PROPERTY(QColor trueColor READ getTrueColor WRITE setTrueColor)
  Q_ENUMS(colMode)

public:

    enum colMode {Static=0, Alarm};

	caLed(QWidget*);
        virtual ~caLed(){}

        QString getPV() const;
        void setPV(QString const &newPV);
        int getBitNr() const;
        void setBitNr(int const newBitNr);

        QColor getFalseColor() const {return thisFalseColor;}
        void setFalseColor(QColor c);

        QColor getTrueColor() const {return thisTrueColor;}
        void setTrueColor(QColor c);

        colMode getColorMode() const { return thisColorMode; }
        void setColorMode(colMode colormode);

        void setState(bool state);

        void setAlarmColors(short status);

private:
        QString thisPV;
        int thisBitNr;
        QColor thisFalseColor, thisTrueColor;
        colMode thisColorMode;
};

#endif
