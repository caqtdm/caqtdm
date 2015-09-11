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

#ifndef CACALC_H
#define CACALC_H

#include <QWidget>
#include <QLabel>
#include "esimplelabel.h"
#include <qtcontrols_global.h>


class  QTCON_EXPORT caCalc : public  ESimpleLabel
{
    Q_OBJECT

    Q_PROPERTY(QString variable READ getVariable WRITE setVariable)
    Q_PROPERTY(QString calc READ getCalc WRITE setCalc)
    Q_PROPERTY(QString channel READ getChannelA WRITE setChannelA)
    Q_PROPERTY(QString channelB READ getChannelB WRITE setChannelB)
    Q_PROPERTY(QString channelC READ getChannelC WRITE setChannelC)
    Q_PROPERTY(QString channelD READ getChannelD WRITE setChannelD)
    Q_PROPERTY(double initialValue READ getInitialValue WRITE setInitialValue)
    Q_PROPERTY(int precision READ getPrecision WRITE setPrecision)

#include "addevent.h"

public:

    QString getVariable() const {return thisVariable;}
    void setVariable(QString const &var) {thisVariable = var;}

    QString getCalc() const {return thisCalc;}
    void setCalc(QString const &calc) {thisCalc = calc;}

    QString getChannelA() const {return thisChannelA;}
    void setChannelA(QString const &pv) {thisChannelA = pv;}

    QString getChannelB() const {return thisChannelB;}
    void setChannelB(QString const &pv) {thisChannelB = pv;}

    QString getChannelC() const {return thisChannelC;}
    void setChannelC(QString const &pv) {thisChannelC = pv;}

    QString getChannelD() const {return thisChannelD;}
    void setChannelD(QString const &pv) {thisChannelD = pv;}

    double getInitialValue() const {return thisValue;}
    void setInitialValue(double const &value) {thisValue = value;}

    caCalc( QWidget *parent = 0 );

    void setValue(double value);
    void setTextLine(const QString &txt);
    void setForeAndBackground(QColor fg, QColor bg);

    int getPrecision() const {return thisPrecision;}
    void setPrecision(int prec) {thisPrecision = prec;}

private:
    QString thisChannelA, thisChannelB, thisChannelC, thisChannelD, thisVariable;
    QString thisCalc;
    QString keepText;
    double thisValue;
    int thisPrecision;
};

#endif // CACALC_H
