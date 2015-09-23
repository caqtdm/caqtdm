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
#ifndef DESIGNERPLUGINTEXTS_H
#define DESIGNERPLUGINTEXTS_H

# define COLORMODE "Static: Show the object in its normal colors.\nAlarm: Show the object in alarm colors based on the severity of the associated process variable."
# define VISIBILITY "StaticV: The object is always displayed.\nIf not zero:The object is displayed if the process variable is not zero.\nIf zero:The object is displayed if the object is zero.\nCalc: Use calc expression to determine its visibility."
# define TEXTPVA "Name of PV (processvariable) A used in calculation,\n a PV can be prefixed with a plugin name (ex: epics3://)"
# define TEXTPVB "Name of PV (processvariable) B used in calculation,\n a PV can be prefixed with a plugin name (ex: epics3://)"
# define TEXTPVC "Name of PV (processvariable) C used in calculation,\n a PV can be prefixed with a plugin name (ex: epics3://)"
# define TEXTPVD "Name of PV (processvariable) D used in calculation,\n a PV can be prefixed with a plugin name (ex: epics3://)"
# define VISIBILITYCALC "Calc expression for visible/hidden, \none may use an epics calc expression,\n a regular expression with channel A (ex: %/^$|\\s+/)\n or a Python function (ex: %P/def PythonCalc(a, b, c, d):return (a*b)/)"
# define DISPLAYTEXT "Text that will be displayed in this widget"
# define DIRECTIONTEXT "Display direction of text (rotated vertically, Up or Down)"

#endif
