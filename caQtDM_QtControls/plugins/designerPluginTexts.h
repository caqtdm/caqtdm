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

// channel general
# define CHANNEL        "channel: Name of PV (processvariable) used for this object,\na PV can be prefixed with a plugin name (ex: epics3://)"
# define CHANNELLIST    "channels: List of PV-Names (processvariables) used for this object,\na PV can be prefixed with a plugin name (ex: epics3://)"
# define CHANNELLISTC   "channels: List of up to 2 PV-Names (processvariables) used for this object,\nfirst or second PV can be blanc (see manual)\na PV can be prefixed with a plugin name (ex: epics3://)"
# define STRINGFROMLIST "String representation of above list as used in caQtDM, seperated with ;"

// visibility
# define COLORMODE      "colorMode: Static: Show the object in its here defined colors.\nAlarm: Show the object in alarm colors based on the severity of the associated process variable."
# define VISIBILITY     "visibility: StaticV: The object is always displayed.\nIf not zero:The object is displayed if the process variable is not zero.\nIf zero:The object is displayed if the object is zero.\nCalc: Use calc expression to determine its visibility."
# define TEXTPVA        "channel: Name of PV (processvariable) A used in calculation,\na PV can be prefixed with a plugin name (ex: epics3://)"
# define TEXTPVB        "channelB: Name of PV (processvariable) B used in calculation,\na PV can be prefixed with a plugin name (ex: epics3://)"
# define TEXTPVC        "channelC: Name of PV (processvariable) C used in calculation,\na PV can be prefixed with a plugin name (ex: epics3://)"
# define TEXTPVD        "channelD: Name of PV (processvariable) D used in calculation,\na PV can be prefixed with a plugin name (ex: epics3://)"
# define VISIBILITYCALC "visibilityCalc: Calc expression for visible/hidden, \none may use an epics calc expression,\na regular expression with channel A (ex: %/^$|\\s+/)\nor a Python function (ex: %P/def PythonCalc(a, b, c, d):return (a*b)/)"

// vertical label
# define DISPLAYTEXT    "text: Text that will be displayed in this widget"
# define DIRECTIONTEXT  "direction: Display direction of text (rotated vertically, Up or Down)"

// frame
# define MACRO          "macro: Macro substitution for the objects inside this frame can de defined through this property\n (ex: A=QUAD)"
# define BCKMODE        "backgroundMode: Background of the frame can be completely filled with the background color or only outlined"
# define VISIBILITYMODE "visibilityMode: For mode=All, the visibility of the frame is given by the visibility calculation.\nFor mode=Background, only the background is visble/hidden (for visibility != staticV)"

// animated gif
# define IMAGEFILE      "filename: Filename of the animated gif image (or any other supported format) to be used\n(normally in CAQTDM_DISPLAY_PATH)"
# define IMAGECALC      "imageCalc: Epics calc expression giving the index of the frame of the animated gif image"
# define IMAGEFRAME     "frame: Index of the frame to be displayed\n(not used when frame is dynamically calculated)"
# define IMAGEDELAY     "delayMilliseconds: Repetition delay between frames\n(when frames are present and not dynamically calculated)"

// graphics
# define LINESTYLE      "linestyle: Lines can be drawn with solid, dash or bigdash style"
# define LINESIZE       "lineSize: Thickness of the drawn lines"
# define FILLSTYLE      "fillstyle: Object can be drawn with a filled or outlined style"
# define GRAPHICFORM    "form: Object can have several forms (rectangle, circle, arc, triangle, line or arrow)"
# define GRAPHICSPECIALS "This property depends on the form of the object"

// polyline
# define POLYSTYLE      "polystyle: This style defines if the polygon will be closed or open"
# define XYPAIRS        "xyPairs: Coordinates of the polygon with the following syntax:\n points separated by semicolumn, coordinated separated by comma (ex: x,y;x,y),\nhowever by double clicking on the object, you may draw the points in  dedicated editor"

// clock
# define TIMETYPE       "timeType: The clock can display the current (internalTime) time\nor the time (receiveTime) of the last update of the PV"

//led
# define TRUEVALUE      "trueValue: LED will get its true color when the trueValue fits the given value\nin case the value does not match either the true or false value\nthe led will get the undefined color"
# define FALSEVALUE     "falseValue: LED will get its true color when the falseValue fits the given value\nin case the value does not match either the true or false value\nthe led will get the undefined color" 

// include widget
# define INCLUDEFILE    "filename: Filename of the ui file to be included, can be an ui file or a prc script \n(normally in CAQTDM_DISPLAY_PATH)"
# define INCLUDESTACKING "stacking: The included files can be displayed in a row or in a column as well as in rows with multiple columns\ndepending on the number of files included and the maximum number of rows specified."
# define INCLUDEMAXLINES "maximumLines; The number of rows that will be displayed in case of stacking=rowcolumn.\nWhen more rows that would fit, a new column will be created"
# define INCLUDENBITEMS "numberOfItems: Number of instances of the included file"
# define INCLUDEMACRO   "macro: Macro substitution for the include file can de defined through this property\nFor specifying for each instance of the specified file separate\nthe macro with ; (ex: A=QUAD; A=QUAD1; A=QUAD2)"

// gauge
# define ORIENTATION    "orientation: Gauge can be of horizontal or vertical orientation"
# define FILLMODE       "fillMode: Gauge has different display possibilities for its indicator:\nALL has an indicator sign; FROMMIN has an indicator bar from the bottom;\nFROMZERO from zero value; FROMREF from the reference value" 
# define VALUEDISPLAY   "valueDisplayed: Circular gauge can display the associated value"
# define GAUGELABEL     "label: Circular Gauge can display a label"

// meter
# define MAXVALUE       "maxValue: Maximum scale value in case of limitsMode=User"
# define MINVALUE       "minValue: Minimum scale value in case of limitsMode=User"
# define LIMITSMODE     "limitsMode: Scale limits can be given by the User or by the acquired Controlsystem limits"
# define BASECOLOR      "baseColor: Color that will be used to draw the meter in case of colorMode=static\nColors will be choosen from the alarm colors otherwise"
# define SCALEDEFAULTCOLOR "scaleDefaultColor: The default scale color will be turned too white when set" 
# define SCALECOLOR     "scaleColor: Color that will be used to draw the scale texts"
# define SCALEENABLED   "scaleEnabled: Scale label display can be enabled or disabled"
# define VALUEDISPLAYED "valueDisplayed: The value display at the bottom of the meter can be disabled or enabled"  
# define PRECISION      "precision: Displayed precision of the value and the scale in case of PRECISIONMODE=User"
# define PRECISIONMODE  "precisionMode: Displayed precision can be obtained from the Controlsystem or from user value" 
# define FORMATTYPE     "formatType: Special formats can be chosen for display of the value and scale"
# define UNITSENABLED   "unitsEnabled: Device Units will be appended to the value when enabled"

// lineedit
# define LFOREGROUND    "foreground: Color that will be used for the text in case of colorMode=static"
# define LBACKGROUND    "background: Color that will be used for the text in case of colorMode=static"
# define LCOLORMODE     "colorMode: Static - Show the object in its here defined colors.\n"\
                        "Alarm, Alarm_Default - Show the object in alarm colors based on the severity of the associated process variable.\n"\
                        "Alarm_Default - the stylesheet of the mainwindow will use the background specified in the stylesheet\n"\
                        "Default - Show the object in colors specified by the stylesheet of the mainwindow\n"
# define FRAMEPRESENT   "framePresent: When framePresent is specified, a border is drawn with the specified color and linewidth"
# define FRAMECOLOR     "frameColor: Color of the frame when framePresent is specified"
# define FRAMELINEWIDTH "frameLineWidth: Width of the frame when framePresetn is pecified"
# define ALARMHANDLING  "alarmHandling: Alarm colors (white, green, yellow, red) will be displayed on foreground or background\n"\
			"in case of alarmHandling=background and colorMode=Alarm_Default for the Major alarm, the default background\n will be taken from the stylesheet"
# define FONTSCALEMODE  "fontScaleMode: Texts can be specified not to resize or to resize by using height only or by using width and height"
# define LPRECISION      "precision: Displayed precision of the value in case of PRECISIONMODE=User"
# define LMAXVALUE       "maxValue: Maximum value used for alarmhandling in case of limitsMode=User"
# define LMINVALUE       "minValue: Minimum value used for alarmhandling in case of limitsMode=User"

#endif
