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

#include "cabitnames.h"

caBitnames::caBitnames(QWidget *parent) : EFlag(parent)
{
    setNumColumns(1);
    thisStartBit=0;
    thisEndBit = 15;
    setStartBit(0);
    setEndBit(15);
    thisString = "";
    setTrueColor(Qt::blue);
    setFalseColor(Qt::gray);
    numRows = 16;
    thisAlignment = EFlag::left;
    setFontScaleModeL(ESimpleLabel::WidthAndHeight);
    setEnumStrings("1;2;3;4;5;6;7;8;9;10;11;12;13;14;15;16");
    setValue(0);
}

QString caBitnames::getEnumPV() const
{
    return thisEnumPV;
}

void caBitnames::setEnumPV(QString const &newPV)
{
    thisEnumPV = newPV;
}

QString caBitnames::getValuePV() const
{
    return thisValuePV;
}

void caBitnames::setValuePV(QString const &newPV)
{
    thisValuePV = newPV;
}

bool caBitnames::bitState(long value, int bitNr)
{
    return ((((int) value >> bitNr) & 1) == 1);
}

void caBitnames::setEnumStrings(QString string)
{
    thisString = string;
    QStringList stringlist = thisString.split((QChar)27);
    for(int i=0; i<= thisEndBit - thisStartBit; i++) {
        if(i+thisStartBit >= stringlist.count()) return;
        EFlag::setEnumDisplay(i, 1, stringlist[i+thisStartBit], thisTrueColor);
        EFlag::setEnumDisplay(i, 0, stringlist[i+thisStartBit], thisFalseColor);
    }
}

void caBitnames::setValue(int valueP)
{
    tf.clear();
    for (int i = thisStartBit; i <= thisEndBit; i++) {
        if(bitState(valueP, i)) {
            tf << 1;
        } else {
            tf << 0;
        }
    }
    EFlag::setValue(tf);
}

void caBitnames::setTrueColor(QColor c)
{
    thisTrueColor = c;
}

void caBitnames::setFalseColor(QColor c)
{
    thisFalseColor = c;
}

void caBitnames::setStartBit(int const &bit) {
    thisStartBit = bit;
    numRows = thisEndBit - thisStartBit + 1;
    if(numRows < 1) {
        thisStartBit=0;
        thisEndBit = 0;
        numRows = 1;
    } else if(numRows > 16) {
        thisStartBit=0;
        thisEndBit = 15;
        numRows = 16;
    }
    EFlag::setNumRows(numRows);
    setEnumStrings(thisString);
}

void caBitnames::setEndBit(int const &bit) {
    thisEndBit = bit;
    numRows = thisEndBit - thisStartBit + 1;
    if(numRows < 1) {
        thisStartBit=0;
        thisEndBit = 0;
        numRows = 1;
    } else if(numRows > 16) {
        thisStartBit=0;
        thisEndBit = 15;
        numRows = 16;
    }
    EFlag::setNumRows(numRows);
    setEnumStrings(thisString);
}

void caBitnames::setAlignment(alignmentHor alignment)
{
    thisAlignment = alignment;
    EFlag::setTextAlignment(alignment);
}

void caBitnames::setFontScaleModeL(ESimpleLabel::ScaleMode m)
{
   EFlag::setFontScaleMode(m);
}

ESimpleLabel::ScaleMode caBitnames::fontScaleModeL()
{
   return EFlag::fontScaleMode();
}

void caBitnames::fontChange(const QFont & oldFont) {
     Q_UNUSED(oldFont);
    if(fontScaleMode() == ESimpleLabel::None) EFlag::setFont(this->font());
}


