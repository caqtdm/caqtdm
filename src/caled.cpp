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

#include "caled.h"

caLed::caLed(QWidget *parent) : ELed(parent)
{
        thisBitNr = 0;
        thisFalseColor = Qt::gray;
        thisTrueColor = Qt::red;
	return;
}

QString caLed::getPV() const
    {
       return thisPV;
    }

void caLed::setPV(QString const &newPV)
    {
        thisPV = newPV;
    }

int caLed::getBitNr() const
    {
       return thisBitNr;
    }

void caLed::setBitNr(int const newBitNr)
    {
        thisBitNr = newBitNr;
    }

void caLed::setTrueColor(QColor c)
{
    thisTrueColor = c;
}

void caLed::setFalseColor(QColor c)
{
    thisFalseColor = c;
    ELed::setColor(c);
}

void caLed::setState(bool state)
{
    if(state) {
        ELed::setColor(thisTrueColor);
    } else {
        ELed::setColor(thisFalseColor);
    }
}


