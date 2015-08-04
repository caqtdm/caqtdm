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

#include "calabel.h"
#include "alarmdefs.h"
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
        thisStyle = "background-color: rgba(%1, %2, %3, %4); color: rgba(%5, %6, %7, %8);";
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
    case INVALID_ALARM:
    case NOTCONNECTED:
        c=AL_WHITE;
        break;
    default:
        c=AL_DEFAULT;
        break;
    }
    setColors(thisBackColor, c);
}


