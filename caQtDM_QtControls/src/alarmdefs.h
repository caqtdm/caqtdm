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

#define AL_GREEN QColor(0x00, 0xcd, 0x00)
#define AL_YELLOW QColor(0xff, 0xff, 0x00)
#define AL_RED QColor(0xff, 0x00, 0x00)
#define AL_WHITE QColor(0xff, 0xff, 0xff)
#define AL_DEFAULT QColor(0x88, 0x88, 0x88)

#endif
