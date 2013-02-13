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

#include "camessagebutton.h"
#include "alarmdefs.h"
#include <QtDebug>
#include <QApplication>

caMessageButton::caMessageButton(QWidget *parent) : EPushButton(parent)
{
    setAccessW(true);
    thisColorMode = Static;
    thisForeColor = Qt::black;
    setBackground(Qt::gray);
    installEventFilter(this);
}

void caMessageButton::buttonhandle(int type)
{
    emit messageButtonSignal(type);
}

void caMessageButton::setLabel(QString const &label)
{
    thisLabel = label;
    setText(thisLabel);
}

void caMessageButton::setColors(QColor bg, QColor fg, QColor hover, QColor border)
{
    //set colors and style filled
    if((bg != oldBackColor) || (fg != oldForeColor) || (hover != oldHoverColor)) {
       QString style = "QPushButton{ background-color: rgba(%1, %2, %3, %4); color: rgb(%5, %6, %7); border-color: rgb(%8, %9, %10);";
       style = style.arg(bg.red()).arg(bg.green()).arg(bg.blue()).arg(255).
             arg(fg.red()).arg(fg.green()).arg(fg.blue()).
             arg(border.red()).arg(border.green()).arg(border.blue());
       style.append("border-radius: 1px; padding: 0px; border-width: 3px;"
                  "border-style: outset; margin:0px;}");
       QString hoverC = "QPushButton:hover {background-color: rgb(%1, %2, %3);}";
       hoverC = hoverC.arg(hover.red()).arg(hover.green()).arg(hover.blue());
       style.append(hoverC);
       setStyleSheet(style);

       oldBackColor = bg;
       oldForeColor = fg;
       oldHoverColor = hover;
     }
}

void caMessageButton::setBackground(QColor c)
{
    thisBackColor = c;
    thisHoverColor = c.light(120);
    thisBorderColor = c.dark(150);
    setColors(thisBackColor, thisForeColor, thisHoverColor, thisBorderColor);
}

void caMessageButton::setForeground(QColor c)
{
    thisForeColor = c;
    setColors(thisBackColor, thisForeColor, thisHoverColor, thisBorderColor);
}

void caMessageButton::setAlarmColors(short status)
{
    QColor bg, fg, hover;
    fg = thisForeColor;
    hover = thisHoverColor;
    switch (status) {

    case NO_ALARM:
        bg = AL_GREEN;
        break;
    case MINOR_ALARM:
        bg = AL_YELLOW;
        break;
    case MAJOR_ALARM:
        bg = AL_RED;
        break;
    case ALARM_INVALID:
    case NOTCONNECTED:
        bg = AL_WHITE;
        fg = bg;
        break;
    default:
        bg = AL_DEFAULT;
        fg = thisForeColor;
        break;
    }
    setColors(bg, fg, hover, thisBorderColor);
}

void caMessageButton::setNormalColors()
{
    setColors(thisBackColor, thisForeColor, thisHoverColor, thisBorderColor);
}

void caMessageButton::setAccessW(int access)
{
     _AccessW = access;
}

bool caMessageButton::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Enter) {
        if(!_AccessW) {
            QApplication::setOverrideCursor(QCursor(Qt::ForbiddenCursor));
        } else {
            QApplication::restoreOverrideCursor();
        }
    } else if(event->type() == QEvent::Leave) {
        QApplication::restoreOverrideCursor();
    } else if(event->type() == QEvent::MouseButtonPress) {
        if(_AccessW) buttonhandle(0);
    } else if(event->type() == QEvent::MouseButtonRelease) {
        if(_AccessW) buttonhandle(1);
    }
    return QObject::eventFilter(obj, event);
}



