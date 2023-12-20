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


#endif //QTCONGLOBAL_H
