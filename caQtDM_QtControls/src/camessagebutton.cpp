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
#include <QApplication>

caMessageButton::caMessageButton(QWidget *parent) : EPushButton(parent)
{
    setAccessW(true);
    thisColorMode = Static;
    thisForeColor = Qt::black;
    thisDisabledForeColor = Qt::gray;
    setBackground(QColor(0xe8, 0xe8, 0xe8));
    setFontScaleMode(EPushButton::WidthAndHeight);
    installEventFilter(this);

    setElevation(on_top);
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

void caMessageButton::setColors(QColor bg, QColor fg, QColor hover, QColor border, QColor disabledfg)
{
    //set colors and style filled
    if((bg != oldBackColor) || (fg != oldForeColor) || (hover != oldHoverColor) || disabledfg != oldDisabledForeColor) {
       QString style = "QPushButton{ background-color: rgba(%1, %2, %3, %4); color: rgba(%5, %6, %7, %8); border-color: rgba(%9, %10, %11, %12);";
       style = style.arg(bg.red()).arg(bg.green()).arg(bg.blue()).arg(bg.alpha()).
             arg(fg.red()).arg(fg.green()).arg(fg.blue()).arg(fg.alpha()).
             arg(border.red()).arg(border.green()).arg(border.blue()).arg(border.alpha());
       style.append("border-radius: 1px; padding: 0px; border-width: 3px;"
                  "border-style: outset; margin:0px;}");

       QString disabledColor = "QPushButton:disabled { color: rgba(%1, %2, %3, %4);}";
       disabledColor = disabledColor.arg(disabledfg.red()).arg(disabledfg.green()).arg(disabledfg.blue()).arg(disabledfg.alpha());
       style.append(disabledColor);

       QString hoverC = "QPushButton:hover {background-color: rgba(%1, %2, %3, %4);}  QPushButton:pressed {background-color: rgba(%5, %6, %7, %8)};";
       hoverC = hoverC.arg(hover.red()).arg(hover.green()).arg(hover.blue()).arg(hover.alpha()).
               arg(thisBorderColor.red()).arg(thisBorderColor.green()).arg(thisBorderColor.blue()).arg(thisBorderColor.alpha());
       style.append(hoverC);


       setStyleSheet(style);

       oldBackColor = bg;
       oldForeColor = fg;
       oldHoverColor = hover;
       oldDisabledForeColor = disabledfg;
     }
}

void caMessageButton::setBackground(QColor c)
{
    thisBackColor = c;
    thisHoverColor = c.lighter(120);
    thisBorderColor = c.darker(150);
    setColors(thisBackColor, thisForeColor, thisHoverColor, thisBorderColor, thisDisabledForeColor);
}

void caMessageButton::setForeground(QColor c)
{
    thisForeColor = c;
    setColors(thisBackColor, thisForeColor, thisHoverColor, thisBorderColor, thisDisabledForeColor);
}

void caMessageButton::setDisabledForeground(QColor c)
{
    thisDisabledForeColor = c;
    setColors(thisBackColor, thisForeColor, thisHoverColor, thisBorderColor, thisDisabledForeColor);
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
    case INVALID_ALARM:
    case NOTCONNECTED:
        bg = AL_WHITE;
        fg = bg;
        break;
    default:
        bg = AL_DEFAULT;
        fg = thisForeColor;
        break;
    }
    setColors(bg, fg, hover, thisBorderColor, thisDisabledForeColor);
}

void caMessageButton::setNormalColors()
{
    setColors(thisBackColor, thisForeColor, thisHoverColor, thisBorderColor, thisDisabledForeColor);
}

void caMessageButton::setAccessW(bool access)
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



