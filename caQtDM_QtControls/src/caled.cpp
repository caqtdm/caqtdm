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

#include "caled.h"
#include "alarmdefs.h"
#include "themecolors.h"

// colour set of caLed (level 2), values come from themecolors.h
namespace caLedColor {
    constexpr const ThemeColor &AlarmNoAlarm = ThemeColors::AlarmNoAlarm;
    constexpr const ThemeColor &AlarmMinor   = ThemeColors::AlarmMinor;
    constexpr const ThemeColor &AlarmMajor   = ThemeColors::AlarmMajor;
    constexpr const ThemeColor &AlarmInvalid = ThemeColors::AlarmInvalid;
    constexpr const ThemeColor &AlarmDefault = ThemeColors::AlarmDefault;
    constexpr const ThemeColor &NotConnected = ThemeColors::AlarmInvalid;
}

caLed::caLed(QWidget *parent) : ELed(parent)
{
    thisBitNr = 0;
    thisFalseColor = Qt::gray;
    thisTrueColor = Qt::red;
    thisUndefinedColor = Qt::black;
    thisBorderColor = Qt::black;              // ELed default, the getter returned an invalid colour
    thisColorMode = Static;
    setTrueValue("1");
    setFalseValue("0");
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

void caLed::setUndefinedColor(QColor c)
{
    thisUndefinedColor = c;
}

void caLed::setFalseColor(QColor c)
{
    thisFalseColor = c;
    ELed::setColor(c);
}

void caLed::setState(Qt::CheckState state)
{
    if(state == Qt::Checked) {
        ELed::setColor(thisTrueColor);
    } else if(state ==  Qt::Unchecked){
        ELed::setColor(thisFalseColor);
    } else {
        ELed::setColor(thisUndefinedColor);
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
        ELed::setColor(ThemeColors::color(caLedColor::AlarmNoAlarm));
        break;
    case MINOR_ALARM:
        ELed::setColor(ThemeColors::color(caLedColor::AlarmMinor));
        break;
    case MAJOR_ALARM:
        ELed::setColor(ThemeColors::color(caLedColor::AlarmMajor));
        break;
    case INVALID_ALARM:
    case NOTCONNECTED:
        ELed::setColor(ThemeColors::color(caLedColor::NotConnected));
        break;
    default:
        ELed::setColor(ThemeColors::color(caLedColor::AlarmDefault));
        break;
    }
}

