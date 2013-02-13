//******************************************************************************
// Copyright (c) 2012 Paul Scherrer Institut PSI), Villigen, Switzerland
// Disclaimer: neither  PSI, nor any of their employees makes any warranty
// or assumes any legal liability or responsibility for the use of this software
//******************************************************************************
//******************************************************************************
//
//     Author : Anton Chr. Mezger
//
//******************************************************************************

#include "cacalc.h"


caCalc::caCalc( QWidget *parent ) :  QLabel(parent)
{
    thisChannelA="";
    thisChannelB="";
    thisChannelC="";
    thisChannelD="";
    thisVariable="";
    thisValue = 0.0;
    keepText="";

    setForeAndBackground(Qt::black, Qt::lightGray);
}

void caCalc::setValue(double value)
{
    QString strng;
    strng.setNum(value);
    setText(strng);
}

void caCalc::setText(const QString &txt)
{
    if(keepText == txt) {  // accelerate things
        return;
    }
    //printf("set qstring=%s old=%s\n", txt.toAscii().constData(), keepText.toAscii().constData());
    QLabel::setText(txt);
    keepText = txt;
}

void caCalc::setForeAndBackground(QColor fg, QColor bg)
{
    QString thisStyle = "background-color: rgb(%1, %2, %3); color: rgb(%4, %5, %6);";
    thisStyle = thisStyle.arg(bg.red()).arg(bg.green()).arg(bg.blue()).
            arg(fg.red()).arg(fg.green()).arg(fg.blue());

    setStyleSheet(thisStyle);
}
