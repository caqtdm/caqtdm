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

#include "cacalc.h"


caCalc::caCalc( QWidget *parent ) :  ESimpleLabel(parent)
{
    thisChannelA="";
    thisChannelB="";
    thisChannelC="";
    thisChannelD="";
    thisVariable="";
    thisValue = 0.0;
    keepText="";

    setForeAndBackground(Qt::black, Qt::lightGray);
}

void caCalc::setValue(double value)
{
    QString strng;
    strng.setNum(value);
    setText(strng);
}

void caCalc::setText(const QString &txt)
{
    if(keepText == txt) {  // accelerate things
        return;
    }
    //printf("set qstring=%s old=%s\n", txt.toAscii().constData(), keepText.toAscii().constData());
    QLabel::setText(txt);
    keepText = txt;
}

void caCalc::setForeAndBackground(QColor fg, QColor bg)
{
    QString thisStyle = "background-color: rgb(%1, %2, %3); color: rgb(%4, %5, %6);";
    thisStyle = thisStyle.arg(bg.red()).arg(bg.green()).arg(bg.blue()).
            arg(fg.red()).arg(fg.green()).arg(fg.blue());

    setStyleSheet(thisStyle);
}
