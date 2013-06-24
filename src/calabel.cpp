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

#include "calabel.h"
#include "alarmdefs.h"
#include <QtDebug>
#include <QEvent>

caLabel::caLabel(QWidget *parent) : ESimpleLabel(parent)
{
    setFontScaleMode(WidthAndHeight);
    thisBackColor = Qt::gray;
    thisForeColor = Qt::black;
    thisBackColorOld = Qt::black;
    thisForeColorOld = Qt::black;
    thisColorMode=Static;
    oldColorMode =Static;
    setColorMode(Static);
    thisVisibility = StaticV;
}

void caLabel::setColors(QColor bg, QColor fg)
{
    if((bg != thisBackColorOld) || (fg != thisForeColorOld)) {
        thisStyle = "background-color: rgb(%1, %2, %3, %4); color: rgb(%5, %6, %7, %8);";
        thisStyle = thisStyle.arg(bg.red()).arg(thisBackColor.green()).arg(bg.blue()).arg(bg.alpha()).
                arg(fg.red()).arg(fg.green()).arg(fg.blue()).arg(fg.alpha());
        setStyleSheet(thisStyle);
        // oups, was forgotten
        thisBackColorOld = bg;
        thisForeColorOld = fg;
    }

    if(thisStyle != oldStyle || thisColorMode != oldColorMode) {
        setStyleSheet(thisStyle);
        oldStyle = thisStyle;
        update();
    }
}

void caLabel::setBackground(QColor c)
{
    thisBackColor = c;
    setColors(thisBackColor, thisForeColor);
}

void caLabel::setForeground(QColor c)
{
    thisForeColor = c;
    setColors(thisBackColor, thisForeColor);
}

void caLabel::setAlarmColors(short status)
{
    QColor c;
    switch (status) {

    case NO_ALARM:
        c=AL_GREEN;
        break;
    case MINOR_ALARM:
        c=AL_YELLOW;
        break;
    case MAJOR_ALARM:
        c=AL_RED;
        break;
    case ALARM_INVALID:
    case NOTCONNECTED:
        c=AL_WHITE;
        break;
    default:
        c=AL_DEFAULT;
        break;
    }
    setColors(thisBackColor, c);
}


