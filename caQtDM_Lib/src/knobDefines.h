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

#ifndef __KNOBDEFINES_INC__
#define __KNOBDEFINES_INC__

#define MAX_CALC_INPUTS 12

#define PRINT(x)

#if _MSC_VER !=1800
 #define boolean int
 #define true 1==1
 #define false !(true)
#endif


#define MAXFILELEN 60
#define MAXPVLEN 120
#define MAXDISPLEN 20
#define NBSPECS 5

#define UNUSED(x) (void)(x)

typedef char pv_string[MAXPVLEN];  // temporary definition, I should allocate what we need

/* we wanted to really separate epics from the display part, but we still need this information */

#ifndef EPICS
enum caType {caSTRING	= 0, caINT = 1, caSHORT = 1, caFLOAT = 2, caENUM = 3, caCHAR = 4, caLONG = 5, caDOUBLE = 6};
#else
#include "cadef.h"
enum caType {caSTRING = DBF_STRING, caINT = DBF_INT, caSHORT = DBF_INT, caFLOAT = DBF_FLOAT, caENUM = DBF_ENUM, caCHAR = DBF_CHAR, caLONG = DBF_LONG, caDOUBLE = DBF_DOUBLE};
#endif


// not really used, now just inserted as property in the widgets, not used elsewhere
enum objectType {caCalc_Widget = 0,
                 caLabel_Widget,
                 caInclude_Widget,
                 caFrame_Widget,
                 caMenu_Widget,
                 caChoice_Widget,
                 caThermo_Widget,
                 caSlider_Widget,
                 caLinearGauge_Widget,
                 caCircularGauge_Widget,
                 caByte_Widget,
                 caLineEdit_Widget,
                 caGraphics_Widget,
                 caPolyLine_Widget,
                 caLed_Widget,
                 caApplyNumeric_Widget,
                 caNumeric_Widget,
                 caSpinbox_Widget,
                 caToggleButton_Widget,
                 caCartesianPlot_Widget,
                 caWaterfallPlot_Widget,
                 caStripPlot_Widget,
                 caImage_Widget,
                 caTable_Widget,
                 caWaveTable_Widget,
                 caBitnames_Widget,
                 caCamera_Widget,
                 caMessageButton_Widget,
                 caRelatedDisplay_Widget,
                 caShellCommand_Widget,
                 caTextEntry_Widget,
                 caScriptButton_Widget,
                 caClock_Widget,
                 caMeter_Widget
                };

typedef struct _connectInfoShort {
    int cs;          // 0= epics, 1 =acs
    int connected;
} connectInfoShort;

#endif
