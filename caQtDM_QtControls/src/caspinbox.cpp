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

#include "caspinbox.h"
#include <QResizeEvent>

caSpinbox::caSpinbox(QWidget *parent) : SNumeric(parent)
{
     setAccessW(true);
     setPrecisionMode(Channel);
     setLimitsMode(Channel);
     thisMaximum = 100000.0;
     thisMinimum = -100000.0;
     setDigitsFontScaleEnabled(true);
     setForeground(Qt::black);
     setBackground(QColor(230,230,230));

     setElevation(on_top);
}

QString caSpinbox::getPV() const
    {
       return thisPV;
    }

void caSpinbox::setPV(QString const &newPV)
    {
        thisPV = newPV;
    }

void caSpinbox::setAccessW(bool access)
{
     thisAccessW = access;
     writeAccessW(thisAccessW);
}

void caSpinbox::setBackground(QColor c)
{
    thisBackColor = c;
    setColors(thisBackColor, thisForeColor);
}

void caSpinbox::setForeground(QColor c)
{
    thisForeColor = c;
    setColors(thisBackColor, thisForeColor);
}

void caSpinbox::setColors(QColor bg, QColor fg)
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

void caSpinbox::setConnectedColors(bool connected)
{
    if(!connected) {
       setColors(QColor(Qt::white), QColor(Qt::white));
    } else {
       setColors(thisBackColor, thisForeColor);
    }
}

