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

enum Alarms {NO_ALARM=0, MINOR_ALARM, MAJOR_ALARM, INVALID_ALARM, NOTCONNECTED=99};

// colours by name
#define AL_GREEN QColor(0x00, 0xcd, 0x00)
#define AL_YELLOW QColor(0xff, 0xff, 0x00)
#define AL_RED QColor(0xff, 0x00, 0x00)
#define AL_WHITE QColor(0xff, 0xff, 0xff)
#define AL_BLACK QColor(0, 0, 0)
#define AL_GREY QColor(0x88, 0x88, 0x88)
#define AL_LIGHTGREY QColor(0xd3, 0xd3, 0xd3)
#define AL_BLUE QColor(0x00, 0x00, 0xff)
#define AL_LIGHTBLUE QColor(0xad, 0xd8, 0xe6)
#define AL_STEELBLUE QColor(0x46, 0x82, 0xb4)
#define AL_ROYALBLUE QColor(0x41, 0x69, 0xe1)
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

#endif
