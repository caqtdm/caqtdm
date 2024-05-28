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
#if defined(_MSC_VER)
  #define snprintf _snprintf
#endif



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
    checkSignal_value_bool=false;
    checkSignal_value_int=0;
    checkSignal_value_double=0.0;
    changeValue_value_double=0.0;
    checkSignal_value_QRect=QRect();
    for (int i=0;i<4;i++){
        value_QRectF_const[i]=0;
        value_QRect_is_const[i]=false;
    }
    is_a_pure_constant=false;
}

void caCalc::setValue(double value)
{
    char asc[MAX_STRING_LENGTH], format[20];
    int precision;
    if((precision = getPrecision()) >= 0) {
       sprintf(format, "%s.%dlf", "%", precision);
    } else {
       sprintf(format, "%s.%dle", "%", -precision);
    }
    snprintf(asc, MAX_STRING_LENGTH, format, value);

    setTextLine(QString(asc));

    // emit signal when requested
    if(thisEventSignal == onFirstChange) {
        if(!eventFired) {
            emit emitSignal((int) value);
            emit emitSignal(value);
            emit emitSignal((bool) value);
        }
        eventFired = true;
    } else if(thisEventSignal == onAnyChange) {
        emit emitSignal(value);

        if (fabs(checkSignal_value_double-value)>std::numeric_limits<double>::epsilon()*10){
            //qDebug()<<"reduced_emitSignal"<<checkSignal_value_double<<value<< this ;
            checkSignal_value_double=value;
            emit reduced_emitSignal(value);

        }

        if ((!eventFired)||(checkSignal_value_int!=(int) value)){
            checkSignal_value_int = (int) value;
            emit emitSignal((int) value);
        }
        if ((!eventFired)||(checkSignal_value_bool!=(bool) value)){
            checkSignal_value_bool = (bool) value;
            emit emitSignal((bool) value);
        }
        eventFired = true;
    } else if(thisEventSignal == TriggerZeroToOne) {
        if((qRound(thisValue) == 0) && (qRound(value) == 1)) {
            emit emitSignal((int) value);
            emit emitSignal(value);
            emit emitSignal((bool) value);
        }
    } else if(thisEventSignal == TriggerOneToZero) {
        if((qRound(thisValue) == 1) && (qRound(value) == 0)) {
            emit emitSignal((int) value);
            emit emitSignal(value);
            emit emitSignal((bool) value);
        }
    }
    thisValue =value;
    //printf("%s emit change value %f\n", qasc(objectName()), value);
    // only in case of a change where no calculation takes places, will we update
    // the data container with the actual value
    if(thisCalc.trimmed().size() == 0)
        if (fabs(changeValue_value_double-value)>std::numeric_limits<double>::epsilon()*10){
            changeValue_value_double=value;
            emit changeValue(value);
        }
}

void caCalc::setValue(QString value)
{
    setTextLine(value);
}

void caCalc::setValue(QRect value)
{
    for (int i=0;i<4;i++){
      if (value_QRect_is_const[i]){
        switch(i){
            case 0:{
            value.setX(value_QRectF_const[i]);
               break;
            }
            case 1:{
               value.setY(value_QRectF_const[i]);
               break;
            }
            case 2:{
               value.setWidth(value_QRectF_const[i]);
               break;
            }
            case 3:{
               value.setHeight(value_QRectF_const[i]);
               break;
            }
        }
      }
    }
    if (is_a_pure_constant)
        setTextLine("QRect=const");
    else
        setTextLine("QRect=ok");

    // emit signal when requested
    if(thisEventSignal == onFirstChange) {
        if(!eventFired) {
            emit emitSignal(value);
        }
        eventFired = true;
    } else if(thisEventSignal == onAnyChange) {
        if (is_a_pure_constant||(!eventFired)||(checkSignal_value_QRect!= value)){
            emit emitSignal(value);
            checkSignal_value_QRect=value;
        }
        eventFired = true;
        thisValue =0;
    }
}

void caCalc::setValue(QRectF value)
{
    qDebug() << "setValue with:" << value;
    for (int i=0;i<4;i++){
        if (value_QRect_is_const[i]){
            switch(i){
            case 0:{
               value.setX(value_QRectF_const[i]);
               break;
            }
            case 1:{
               value.setY(value_QRectF_const[i]);
               break;
            }
            case 2:{
               value.setWidth(value_QRectF_const[i]);
               break;
            }
            case 3:{
               value.setHeight(value_QRectF_const[i]);
               break;
            }
            }
        }
    }
    if (is_a_pure_constant)
        setTextLine("QRectF=const");
    else
        setTextLine("QRectF=ok");

    // emit signal when requested
    if(thisEventSignal == onFirstChange) {
        if(!eventFired) {
            qDebug() << "emmitting: " << value;
            emit emitSignal(value);
        }
        eventFired = true;
    } else if(thisEventSignal == onAnyChange) {
        if (is_a_pure_constant||(!eventFired)||(checkSignal_value_QRect!= value)){
            qDebug() << "emmitting: " << value;
            emit emitSignal(value);
            checkSignal_value_QRectF=value;
        }
        eventFired = true;
        thisValue =0;
    }
}

void caCalc::setValue(int value)
{
    Q_UNUSED(value)
    double data=(int) value;
    QRect empty;
    setValue(empty);
    setValue(data);
}

void caCalc::setValue(bool value)
{
    QRect empty;
    setValue(empty);
    int data=(int) value;
    setValue((double) data);
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


void caCalc::setBackground(QColor c)
{
    m_BackColor = c;
    setForeAndBackground(m_ForeColor, m_BackColor);
}

void caCalc::setQRectParam(int x, double param)
{
    if (x<MAX_QRECT_PARAMS){
        value_QRectF_const[x]=param;
        switch(x){
            case 0:{
               value_QRect_is_const[x]=thisChannelA.isEmpty();
               break;
            }
            case 1:{
               value_QRect_is_const[x]=thisChannelB.isEmpty();
               break;
            }
            case 2:{
               value_QRect_is_const[x]=thisChannelC.isEmpty();
               break;
            }
            case 3:{
               value_QRect_is_const[x]=thisChannelD.isEmpty();
               break;
            }
        }
        is_a_pure_constant=true;
        for (int i=0;i<4;i++){
            is_a_pure_constant&=value_QRect_is_const[i];
        }
    }

}

void caCalc::setForeground(QColor c)
{
    m_ForeColor = c;
    setForeAndBackground(m_ForeColor, m_BackColor);
}

void caCalc::setForeAndBackground(QColor fg, QColor bg)
{
    m_ForeColor = fg;
    m_BackColor = bg;
    QString thisStyle = "background-color: rgba(%1, %2, %3, %4); color: rgba(%5, %6, %7, %8);";
    thisStyle = thisStyle.arg(bg.red()).arg(bg.green()).arg(bg.blue()).arg(bg.alpha()).
            arg(fg.red()).arg(fg.green()).arg(fg.blue()).arg(fg.alpha());

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
