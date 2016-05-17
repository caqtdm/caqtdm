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

#include <QtGui>

#include "caframe.h"

caFrame::caFrame(QWidget *parent) : QFrame(parent)
{
    thisBackColor = Qt::gray;
    thisBackgroundMode = Outline;
    setVisibility(StaticV);
    setVisibilityMode(All);
    setBackground(thisBackColor);
}


void caFrame::setBackground(QColor c)
{
    thisBackColor = c;
    QColor thisBackgroundColor = c;
    QColor thisLightColor = c.lighter();
    QColor thisDarkColor = c.darker();

    QPalette thisPalette = palette();
    thisPalette.setColor(QPalette::WindowText, thisBackColor);
    thisPalette.setColor(QPalette::Light, thisLightColor);
    thisPalette.setColor(QPalette::Dark, thisDarkColor);
    thisPalette.setColor(QPalette::Window, thisBackgroundColor);
    setPalette(thisPalette);
    setAutoFillBackground(thisBackgroundMode == Filled ? true : false);
    update();
}


