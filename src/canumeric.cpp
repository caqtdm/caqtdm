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

#include "canumeric.h"
#include <QResizeEvent>

caNumeric::caNumeric(QWidget *parent) : ENumeric(parent)
{
     setAccessW(true);
     setPrecisionMode(Channel);
     setLimitsMode(Channel);
     thisMaximum = 100000.0;
     thisMinimum = -100000.0;
     setDigitsFontScaleEnabled(true);
     setForeground(Qt::black);
     setBackground(QColor(230,230,230));
}

QString caNumeric::getPV() const
    {
       return thisPV;
    }

void caNumeric::setPV(QString const &newPV)
    {
        thisPV = newPV;
    }

void caNumeric::setAccessW(int access)
{
     thisAccessW = access;
     writeAccessW(thisAccessW);
}

void caNumeric::setBackground(QColor c)
{
    thisBackColor = c;
    setColors(thisBackColor, thisForeColor);
}

void caNumeric::setForeground(QColor c)
{
    thisForeColor = c;
    setColors(thisBackColor, thisForeColor);
}

void caNumeric::setColors(QColor bg, QColor fg)
{
    if((bg != oldBackColor) || (fg != oldForeColor)) {
        QString style = "background: rgb(%1, %2, %3, %4); color: rgb(%5, %6, %7, %8);";
        style = style.arg(bg.red()).arg(bg.green()).arg(bg.blue()).arg(bg.alpha()).
                arg(fg.red()).arg(fg.green()).arg(fg.blue()).arg(fg.alpha());
        setStyleSheet(style);
        oldForeColor = fg;
        oldBackColor = bg;
        // force resize for repainting
        QResizeEvent *re = new QResizeEvent(size(), size());
        resizeEvent(re);
        delete re;
    }
}

void caNumeric::setConnectedColors(bool connected)
{
    if(!connected) {
       setColors(QColor(Qt::white), QColor(Qt::white));
    } else {
       setColors(thisBackColor, thisForeColor);
    }
}

