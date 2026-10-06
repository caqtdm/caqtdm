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
 *  Copyright (c) 2010 - 2026
 *
 *  Author:
 *    Helge Brands
 *  Contact details:
 *    helge.brands@psi.ch
 */

#ifndef ALHPARSERDEFS_H
#define ALHPARSERDEFS_H

#include <QtGlobal>
#include <QLoggingCategory>

// self-contained like prcParserSrc: no dependency on qtcontrols_global.h
#if defined(_MSC_VER)
    #if defined(ALHPARSER_MAKEDLL)
        #define ALHPARSER_EXPORT __declspec(dllexport)
    #elif defined(ALHPARSER_DLL)
        #define ALHPARSER_EXPORT __declspec(dllimport)
    #else
        #define ALHPARSER_EXPORT
    #endif
#else
    #define ALHPARSER_EXPORT
#endif

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    #define ALH_SKIP_EMPTY QString::SkipEmptyParts
#else
    #define ALH_SKIP_EMPTY Qt::SkipEmptyParts
#endif

Q_DECLARE_LOGGING_CATEGORY(alhParserLog)

#endif // ALHPARSERDEFS_H
