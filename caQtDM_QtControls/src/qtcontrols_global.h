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

#ifndef QTCONGLOBAL_H
#define QTCONGLOBAL_H

#include "qtdefinitions.h"
#include <QDebug>
#include <QScrollArea>
#include <QVariant>
#include <QLoggingCategory>

#define NOMINMAX

#if defined(_MSC_VER)
		#if defined(QTCON_MAKEDLL)     // create a qtControls DLL library
			#define QTCON_EXPORT  __declspec(dllexport)
		#else                        // use a qtControls DLL library
			#define QTCON_EXPORT  __declspec(dllimport)
		#endif
#else
	#define QTCON_EXPORT
#endif

#if defined(__OSX__) || defined(__APPLE__)
  #include <cmath>
  #define isnan(x) std::isnan(x)
#endif

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#ifndef Q_NULLPTR
#if __cplusplus >= 201103L
    #define Q_NULLPTR nullptr
#else
    #define Q_NULLPTR 0
#endif
#endif
#endif

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            #define SKIP_EMPTY_PARTS QString::SkipEmptyParts
            #define QMETRIC_QT456_FONT_WIDTH(metric,text) metric.width(text)
            #define QMETRIC_QT456_FONT_HEIGHT(metric,text) metric.height()
            #define SETMARGIN_QT456(obj,value) obj->setMargin(value)
            #define SETSPACING_QT456(obj,value) obj->setSpacing(value)
#else
            #define SKIP_EMPTY_PARTS Qt::SkipEmptyParts
            #define QMETRIC_QT456_FONT_WIDTH(metric,text) metric.boundingRect(text).width()
            #define QMETRIC_QT456_FONT_HEIGHT(metric,text) metric.boundingRect(text).height()
            #define SETMARGIN_QT456(obj,value) obj->setContentsMargins(value,value,value,value)
            #define SETSPACING_QT456(obj,value) obj->setVerticalSpacing(value);\
                                                obj->setHorizontalSpacing(value)
            //#define QPalette::Background QPalette::Window
#endif

Q_DECLARE_LOGGING_CATEGORY(caApplyNumericLog)
Q_DECLARE_LOGGING_CATEGORY(caBitnamesLog)
Q_DECLARE_LOGGING_CATEGORY(caByteLog)
Q_DECLARE_LOGGING_CATEGORY(caByteControllerLog)
Q_DECLARE_LOGGING_CATEGORY(caCalcLog)
Q_DECLARE_LOGGING_CATEGORY(caCameraLog)
Q_DECLARE_LOGGING_CATEGORY(caCartesianPlotLog)
Q_DECLARE_LOGGING_CATEGORY(caChoiceLog)
Q_DECLARE_LOGGING_CATEGORY(caCircularGaugeLog)
Q_DECLARE_LOGGING_CATEGORY(caClockLog)
Q_DECLARE_LOGGING_CATEGORY(caDoubleTabWidgetLog)
Q_DECLARE_LOGGING_CATEGORY(caFrameLog)
Q_DECLARE_LOGGING_CATEGORY(caGaugeLog)
Q_DECLARE_LOGGING_CATEGORY(caGraphicsLog)
Q_DECLARE_LOGGING_CATEGORY(caImageLog)
Q_DECLARE_LOGGING_CATEGORY(caIncludeLog)
Q_DECLARE_LOGGING_CATEGORY(caLabelLog)
Q_DECLARE_LOGGING_CATEGORY(caLabelVerticalLog)
Q_DECLARE_LOGGING_CATEGORY(caLedLog)
Q_DECLARE_LOGGING_CATEGORY(caLinearGaugeLog)
Q_DECLARE_LOGGING_CATEGORY(caLineDrawLog)
Q_DECLARE_LOGGING_CATEGORY(caAlarmTreeLog)
Q_DECLARE_LOGGING_CATEGORY(caAlarmLogLog)
Q_DECLARE_LOGGING_CATEGORY(caAlarmEventLog)
Q_DECLARE_LOGGING_CATEGORY(caLineEditLog)
Q_DECLARE_LOGGING_CATEGORY(caMenuLog)
Q_DECLARE_LOGGING_CATEGORY(caMessageButtonLog)
Q_DECLARE_LOGGING_CATEGORY(caMeterLog)
Q_DECLARE_LOGGING_CATEGORY(caMimeDisplayLog)
Q_DECLARE_LOGGING_CATEGORY(caMultiLineStringLog)
Q_DECLARE_LOGGING_CATEGORY(caNumericLog)
Q_DECLARE_LOGGING_CATEGORY(caPolyLineLog)
Q_DECLARE_LOGGING_CATEGORY(caRelatedDisplayLog)
Q_DECLARE_LOGGING_CATEGORY(caScan2DLog)
Q_DECLARE_LOGGING_CATEGORY(caScriptButtonLog)
Q_DECLARE_LOGGING_CATEGORY(caShellCommandLog)
Q_DECLARE_LOGGING_CATEGORY(caSliderLog)
Q_DECLARE_LOGGING_CATEGORY(caSpinboxLog)
Q_DECLARE_LOGGING_CATEGORY(caStripPlotLog)
Q_DECLARE_LOGGING_CATEGORY(caTableLog)
Q_DECLARE_LOGGING_CATEGORY(caTextEntryLog)
Q_DECLARE_LOGGING_CATEGORY(caThermoLog)
Q_DECLARE_LOGGING_CATEGORY(caToggleButtonLog)
Q_DECLARE_LOGGING_CATEGORY(caWaterfallPlotLog)
Q_DECLARE_LOGGING_CATEGORY(caWaveTableLog)
Q_DECLARE_LOGGING_CATEGORY(decIntFromFormatLog)
Q_DECLARE_LOGGING_CATEGORY(eApplyNumericLog)
Q_DECLARE_LOGGING_CATEGORY(eGaugeLog)
Q_DECLARE_LOGGING_CATEGORY(eLabelLog)
Q_DECLARE_LOGGING_CATEGORY(eLedLog)
Q_DECLARE_LOGGING_CATEGORY(engNotationLog)
Q_DECLARE_LOGGING_CATEGORY(eNumericLog)
Q_DECLARE_LOGGING_CATEGORY(ePushButtonLog)
Q_DECLARE_LOGGING_CATEGORY(eSimpleLabelLog)
Q_DECLARE_LOGGING_CATEGORY(fileFunctionsLog)
Q_DECLARE_LOGGING_CATEGORY(fontScalingWidgetLog)
Q_DECLARE_LOGGING_CATEGORY(imageWidgetLog)
Q_DECLARE_LOGGING_CATEGORY(mdaReaderLog)
Q_DECLARE_LOGGING_CATEGORY(networkAccessLog)
Q_DECLARE_LOGGING_CATEGORY(numberDelegateLog)
Q_DECLARE_LOGGING_CATEGORY(parseOtherFileLog)
Q_DECLARE_LOGGING_CATEGORY(parsePepFileLog)
Q_DECLARE_LOGGING_CATEGORY(qwtThermoMarkerLog)
Q_DECLARE_LOGGING_CATEGORY(qwtPlotIntervalCurveNaNLog)
Q_DECLARE_LOGGING_CATEGORY(replaceMacroLog)
Q_DECLARE_LOGGING_CATEGORY(searchFileLog)
Q_DECLARE_LOGGING_CATEGORY(sNumericLog)
Q_DECLARE_LOGGING_CATEGORY(specialFunctionsLog)
Q_DECLARE_LOGGING_CATEGORY(wmSignalPropagatorLog)
Q_DECLARE_LOGGING_CATEGORY(wmSignalRescaleLog)

#endif //QTCONGLOBAL_H
