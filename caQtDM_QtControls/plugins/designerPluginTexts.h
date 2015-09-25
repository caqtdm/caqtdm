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

# define COLORMODE      "Static: Show the object in its normal colors.\nAlarm: Show the object in alarm colors based on the severity of the associated process variable."
# define VISIBILITY     "StaticV: The object is always displayed.\nIf not zero:The object is displayed if the process variable is not zero.\nIf zero:The object is displayed if the object is zero.\nCalc: Use calc expression to determine its visibility."
# define TEXTPVA        "Name of PV (processvariable) A used in calculation,\na PV can be prefixed with a plugin name (ex: epics3://)"
# define TEXTPVB        "Name of PV (processvariable) B used in calculation,\na PV can be prefixed with a plugin name (ex: epics3://)"
# define TEXTPVC        "Name of PV (processvariable) C used in calculation,\na PV can be prefixed with a plugin name (ex: epics3://)"
# define TEXTPVD        "Name of PV (processvariable) D used in calculation,\na PV can be prefixed with a plugin name (ex: epics3://)"
# define VISIBILITYCALC "Calc expression for visible/hidden, \none may use an epics calc expression,\na regular expression with channel A (ex: %/^$|\\s+/)\nor a Python function (ex: %P/def PythonCalc(a, b, c, d):return (a*b)/)"
# define DISPLAYTEXT    "Text that will be displayed in this widget"
# define DIRECTIONTEXT  "Display direction of text (rotated vertically, Up or Down)"

# define MACRO          "Macro substitution for the objects inside this frame can de defined through this property\n (ex: A=QUAD)"
# define BCKMODE        "Background of the frame can be completely filled with the background color or only outlined"
# define VISIBILITYMODE "For mode=All, the visibility of the frame is given by the visibility calculation.\nFor mode=Background, only the background is visble/hidden (for visibility != staticV)"

# define IMAGEFILE      "Filename of the animated gif image (or any other supported format) to be used\n(normally in CAQTDM_DISPLAY_PATH)"
# define IMAGECALC      "Epics calc expression giving the index of the frame of the animated gif image"
# define IMAGEFRAME     "Index of the frame to be displayed\n(not used when frame is dynamically calculated)"
# define IMAGEDELAY     "Repetition delay between frames\n(when frames are present and not dynamically calculated)"

# define LINESTYLE      "Lines can be drawn with solid, dash or bigdash style"
# define LINESIZE       "Thickness of the drawn lines"
# define FILLSTYLE      "Object can be drawn with a filled or outlined style"
# define GRAPHICFORM    "Object can have several forms (rectangle, circle, arc, triangle, line or arrow)"
# define GRAPHICSPECIALS "This property depends on the form of the object"

# define POLYSTYLE      "This style defines if the polygon will be closed or open"
# define XYPAIRS        "Coordinates of the polygon with the following syntax:\n points separated by semicolumn, coordinated separated by comma (ex: x,y;x,y),\nhowever by double clicking on the object, you may draw the points in  dedicated editor"

# define INCLUDEFILE    "Filename of the ui file to be included\n(normally in CAQTDM_DISPLAY_PATH)"

# define CHANNEL        "Name of PV (processvariable) used for this object,\na PV can be prefixed with a plugin name (ex: epics3://)"
# define TIMETYPE       "The clock can display the current (internalTime) time\nor the time (receiveTime) of the last update of the PV"

#define TRUEVALUE       "LED will get its true color when the trueValue fits the given value\nin case the value does not match either the true or false value\nthe led will get the undefined color"
#define FALSEVALUE      "LED will get its true color when the falseValue fits the given value\nin case the value does not match either the true or false value\nthe led will get the undefined color"
#endif
