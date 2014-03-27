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
    if(type == 0) setChecked(true);
    else setChecked(false);
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
       QString hoverC = "QPushButton:hover {background-color: rgb(%1, %2, %3);}  QPushButton:pressed {background-color: rgb(%4, %5, %6)};";
       hoverC = hoverC.arg(hover.red()).arg(hover.green()).arg(hover.blue()).arg(thisBorderColor.red()).arg(thisBorderColor.green()).arg(thisBorderColor.blue());
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
         QMouseEvent *me = static_cast<QMouseEvent *>(event);
        if (me->button()==Qt::LeftButton) {
          if(_AccessW) buttonhandle(0);
        }
    } else if(event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        if (me->button()==Qt::LeftButton) {
           if(_AccessW) buttonhandle(1);
        }
    // intercept space key, so that no keyboard spacebar will trigger when button has focus
    }  else if(event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
      QKeyEvent *me = static_cast<QKeyEvent *>(event);
      if(me->key() == Qt::Key_Space) {
        return true;
      }
    }

    return QObject::eventFilter(obj, event);
}



