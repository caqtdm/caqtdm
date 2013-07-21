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
#include "alarmdefs.h"

caLed::caLed(QWidget *parent) : ELed(parent)
{
    thisBitNr = 0;
    thisFalseColor = Qt::gray;
    thisTrueColor = Qt::red;
    thisColorMode = Static;
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


void caLed::setColorMode(colMode colormode)
{
    thisColorMode = colormode;
}

void caLed::setAlarmColors(short status)
{
    switch (status) {

    case NO_ALARM:
        ELed::setColor(AL_GREEN);
        break;
    case MINOR_ALARM:
        ELed::setColor(AL_YELLOW);
        break;
    case MAJOR_ALARM:
        ELed::setColor(AL_RED);
        break;
    case ALARM_INVALID:
    case NOTCONNECTED:
        ELed::setColor(AL_WHITE);
        break;
    default:
        ELed::setColor(AL_DEFAULT);
        break;
    }
}

