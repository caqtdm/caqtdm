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
    setFontScaleMode(ESimpleLabel::WidthAndHeight);
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
    stringlist = thisString.split( ";" );
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
        if(bitState((int) valueP, i)) {
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

void caBitnames::setFontScaleMode(ESimpleLabel::ScaleMode m)
{
   EFlag::setFontScaleMode(m);
}

ESimpleLabel::ScaleMode caBitnames::fontScaleMode()
{
   return EFlag::fontScaleMode();
}

void caBitnames::fontChange(const QFont & oldFont) {
     Q_UNUSED(oldFont);
    if(fontScaleMode() == ESimpleLabel::None) EFlag::setFont(this->font());
}


