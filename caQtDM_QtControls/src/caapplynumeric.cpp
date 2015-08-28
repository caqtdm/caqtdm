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

#include "caapplynumeric.h"
#include <QApplication>
#include <QResizeEvent>

caApplyNumeric::caApplyNumeric(QWidget *parent) : EApplyNumeric(parent)
{
    setAccessW(true);
    setPrecisionMode(Channel);
    setLimitsMode(Channel);
    thisMaximum = 100000.0;
    thisMinimum = -100000.0;
    setDigitsFontScaleEnabled(true);
    setForeground(Qt::black);
    setBackground(QColor(230,230,230));
    thisFixedFormat = false;
    installEventFilter(this);

    setElevation(on_top);
}

QString caApplyNumeric::getPV() const
    {
       return thisPV;
    }

void caApplyNumeric::setPV(QString const &newPV)
    {
        thisPV = newPV;
    }

void caApplyNumeric::setAccessW(int access)
{
     thisAccessW = access;
}


bool caApplyNumeric::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Enter) {
        if(!thisAccessW) {
            QApplication::setOverrideCursor(QCursor(Qt::ForbiddenCursor));
            //setEnabled(false);
        } else {
            QApplication::restoreOverrideCursor();
        }
    } else if(event->type() == QEvent::Leave) {
        QApplication::restoreOverrideCursor();
        //setEnabled(true);
    }
    return QObject::eventFilter(obj, event);
}


void caApplyNumeric::setBackground(QColor c)
{
    thisBackColor = c;
    setColors(thisBackColor, thisForeColor);
}

void caApplyNumeric::setForeground(QColor c)
{
    thisForeColor = c;
    setColors(thisBackColor, thisForeColor);
}

void caApplyNumeric::setColors(QColor bg, QColor fg)
{
    if((oldBackColor != bg) || (oldForeColor != fg)) {
        QString style = "QFrame { background: rgb(%1, %2, %3, %4); color: rgb(%5, %6, %7, %8);}";
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

void caApplyNumeric::setConnectedColors(bool connected)
{
    if(!connected) {
       setColors(QColor(Qt::white), QColor(Qt::white));
    } else {
       setColors(thisBackColor, thisForeColor);
    }
}

