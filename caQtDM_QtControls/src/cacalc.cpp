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

#include "cacalc.h"


caCalc::caCalc( QWidget *parent ) :  ESimpleLabel(parent)
{
    // to start with, clear the stylesheet, so that playing around
    // is not possible.
    setStyleSheet("");

    thisChannelA="";
    thisChannelB="";
    thisChannelC="";
    thisChannelD="";
    thisVariable="";
    thisValue = 0.0;
    thisPV=QStringList();
    keepText="";

    setVariableType(scalar);

    setFontScaleMode(WidthAndHeight);
    setForeAndBackground(Qt::black, Qt::lightGray);
    setPrecision(2);

    thisEventSignal = Never;
    eventFired = false;
}

void caCalc::setValue(double value)
{
    char asc[1024], format[20];
    int precision;
    if((precision = getPrecision()) >= 0) {
       sprintf(format, "%s.%dlf", "%", precision);
    } else {
       sprintf(format, "%s.%dle", "%", -precision);
    }
    sprintf(asc, format, value);

    setTextLine(QString(asc));

    // emit signal when requested
    if(thisEventSignal == onFirstChange) {
        if(!eventFired) {
            emit emitSignal((int) value);
            emit emitSignal(value);
        }
        eventFired = true;
    } else if(thisEventSignal == onAnyChange) {
        emit emitSignal((int) value);
        emit emitSignal(value);
    } else if(thisEventSignal == TriggerZeroToOne) {
        if((qRound(thisValue) == 0) && (qRound(value) == 1)) {
            emit emitSignal((int) value);
            emit emitSignal(value);
        }
    } else if(thisEventSignal == TriggerOneToZero) {
        if((qRound(thisValue) == 1) && (qRound(value) == 0)) {
            emit emitSignal((int) value);
            emit emitSignal(value);
        }
    }
    thisValue =value;
}

void caCalc::setValue(QString value)
{
    setTextLine(value);
}

void caCalc::setValue(QRect value)
{
    setTextLine("QRect=ok");

    // emit signal when requested
    if(thisEventSignal == onFirstChange) {
        if(!eventFired) {
            emit emitSignal(value);
        }
        eventFired = true;
    } else if(thisEventSignal == onAnyChange) {
        emit emitSignal(value);
    }
    thisValue =0;
}

void caCalc::setTextLine(const QString &txt)
{
    if(keepText == txt) {  // accelerate things
        return;
    }
    //printf("set qstring=%s old=%s\n", qasc(txt), qasc(keepText));
    setText(txt);
    keepText = txt;
}

void caCalc::setForeAndBackground(QColor fg, QColor bg)
{
    QString thisStyle = "background-color: rgb(%1, %2, %3); color: rgb(%4, %5, %6);";
    thisStyle = thisStyle.arg(bg.red()).arg(bg.green()).arg(bg.blue()).
            arg(fg.red()).arg(fg.green()).arg(fg.blue());

    setStyleSheet(thisStyle);
}

void caCalc::setVariableType(varType vartype) {
    thisVarType = vartype;
    if(vartype == vector) {
        setPropertyVisible(calcabcd, false);
        setPropertyVisible(channela, false);
        setPropertyVisible(channelb, false);
        setPropertyVisible(channelc, false);
        setPropertyVisible(channeld, false);
        setPropertyVisible(initialvalue, false);
        setPropertyVisible(pvlist, true);
    } else {
        setPropertyVisible(calcabcd, true);
        setPropertyVisible(channela, true);
        setPropertyVisible(channelb, true);
        setPropertyVisible(channelc, true);
        setPropertyVisible(channeld, true);
        setPropertyVisible(initialvalue, true);
        setPropertyVisible(pvlist, false);
    }
}

bool caCalc::isPropertyVisible(Properties property)
{
    return designerVisible[property];
}

void caCalc::setPropertyVisible(Properties property, bool visible)
{
    designerVisible[property] = visible;
}
