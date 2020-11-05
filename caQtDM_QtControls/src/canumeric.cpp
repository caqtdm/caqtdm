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

#include "canumeric.h"
#include <QResizeEvent>
#include <QPainter>
#include <QPen>
#include "alarmdefs.h"

caNumeric::caNumeric(QWidget *parent) : ENumeric(parent)
{
     setStyleSheet("");

     setAccessW(true);
     setPrecisionMode(Channel);
     setLimitsMode(Channel);
     setMaxValue(100000.0);
     setMinValue(-100000.0);
     thisFixedFormat = false;
     thisColorMode = Static;
     setDigitsFontScaleEnabled(true);
     setForeground(Qt::black);

     renewStyleSheet = true;
     setBackground(QColor(230,230,230));
     setElevation(on_top);
}

QString caNumeric::getPV() const
    {
       return thisPV;
    }

void caNumeric::setPV(QString const &newPV)
    {
        thisPV = newPV;
    }

void caNumeric::setAccessW(bool access)
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

void caNumeric::setColors(QColor bg, QColor fg, bool init)
{
    if(thisColorMode == Default) {
        if(!styleSheet().isEmpty()) {
            setStyleSheet("");
            renewStyleSheet = true;
        }
        if(!init) {
            // force resize for repainting
            QResizeEvent *re = new QResizeEvent(size(), size());
            resizeEvent(re);
            delete re;
            return;
        }
    }

    if((bg != oldBackColor) || (fg != oldForeColor)  || renewStyleSheet || styleSheet().isEmpty()) {
        renewStyleSheet = false;
        QString style = "background: rgba(%1, %2, %3, %4); color: rgba(%5, %6, %7, %8);";
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
       setColors(QColor(Qt::white), QColor(Qt::white), true);
    } else {
       setColors(thisBackColor, thisForeColor);
    }
}

void caNumeric::setAlarmColors(short status)
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
    case INVALID_ALARM:
    case NOTCONNECTED:
        c=AL_WHITE;
        break;
    default:
        c=AL_DEFAULT;
        break;
    }
    setBackground(c);
}

void caNumeric::paintEvent(QPaintEvent *event) {
    QPen	pen;
    QPainter p(this);
    Q_UNUSED(event);
    if(hasFocus()) {
      pen.setColor(Qt::red);
      p.setPen(pen);
      //p.drawRect(rect());
      p.drawRect(0,0,rect().width()-1, rect().height()-1);
    }
}

