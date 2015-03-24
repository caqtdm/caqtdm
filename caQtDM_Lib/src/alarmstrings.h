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

#ifndef ALARMDEFS_H
#define ALARMDEFS_H

char AlarmStatusStrings[][30] = {"OK", "READ_ALARM", "WRITE_ALARM", "HIHI_ALARM", "HIGH_ALARM", "LOLO_ALARM", "LOW_ALARM", "STATE_ALARM", "COS_ALARM",
                          "COMM_ALARM", "TIMEOUT_ALARM", "HW_LIMIT_ALARM", "CALC_ALARM", "SCAN_ALARM", "LINK_ALARM", "SOFT_ALARM", "BAD_SUB_ALARM",
                          "UDF_ALARM", "DISABLE_ALARM", "SIMM_ALARM", "READ_ACCESS_ALARM", "WRITE_ACCESS_ALARM", "ALARM_NSTATUS"};

#endif
