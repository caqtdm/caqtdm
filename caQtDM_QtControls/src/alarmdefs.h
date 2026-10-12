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

#ifndef ALARMS_H
#define ALARMS_H

#include "themecolors.h"

enum Alarms {NO_ALARM=0, MINOR_ALARM, MAJOR_ALARM, INVALID_ALARM, NOTCONNECTED=99};

// colours by name; values live in themecolors.h (level 1), these are aliases
#define AL_GREEN ThemeColors::color(ThemeColors::AlarmNoAlarm)
#define AL_YELLOW ThemeColors::color(ThemeColors::AlarmMinor)
#define AL_RED ThemeColors::color(ThemeColors::AlarmMajor)
#define AL_WHITE ThemeColors::color(ThemeColors::White)
#define AL_BLACK ThemeColors::color(ThemeColors::Black)
#define AL_GREY ThemeColors::color(ThemeColors::AlarmDefault)
#define AL_LIGHTGREY ThemeColors::color(ThemeColors::LightGrey)
#define AL_BLUE ThemeColors::color(ThemeColors::Blue)
#define AL_LIGHTBLUE ThemeColors::color(ThemeColors::LightBlue)
#define AL_STEELBLUE ThemeColors::color(ThemeColors::SteelBlue)
#define AL_ROYALBLUE ThemeColors::color(ThemeColors::RoyalBlue)
#define AL_DEFAULT AL_GREY

// alarm handler colours by purpose, _LIGHT/_DARK = palette mode
#define ALH_CHANNEL_BUTTON_LIGHT AL_LIGHTBLUE
#define ALH_CHANNEL_BUTTON_DARK AL_STEELBLUE
#define ALH_CHANNEL_TEXT_LIGHT AL_BLACK
#define ALH_CHANNEL_TEXT_DARK AL_WHITE
#define ALH_CHANNEL_INACTIVE_TEXT_LIGHT AL_GREY
#define ALH_CHANNEL_INACTIVE_TEXT_DARK AL_LIGHTGREY
#define ALH_MASK_HIGHLIGHT_LIGHT AL_BLUE
#define ALH_MASK_HIGHLIGHT_DARK AL_ROYALBLUE
#define ALH_SILENCE_AREA_LIGHT AL_BLUE
#define ALH_SILENCE_AREA_DARK AL_ROYALBLUE
#define ALH_HIGHLIGHT_TEXT AL_WHITE          // on mask highlight / silence area
#define ALH_SEVR_TEXT AL_BLACK               // on the severity colours
#define ALH_INACTIVE_TEXT AL_GREY

// Panel theme legacy-light palette: see ThemeColors::palette(ThemeColors::Light)

#endif
