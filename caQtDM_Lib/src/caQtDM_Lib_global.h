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

#ifndef CAQTDM_LIB_GLOBAL_H
#define CAQTDM_LIB_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(_MSC_VER)

   #if defined(CAQTDM_LIB_LIBRARY)
      #define CAQTDM_LIBSHARED_EXPORT __declspec(dllexport)
   #else
      #define CAQTDM_LIBSHARED_EXPORT __declspec(dllimport)
   #endif
   # if _MSC_VER < 1900
    #define snprintf _snprintf
   #endif
#else
 #if defined(__MINGW32__)
    #if defined(CAQTDM_LIB_LIBRARY)
      #define CAQTDM_LIBSHARED_EXPORT Q_DECL_EXPORT
    #else
      #define CAQTDM_LIBSHARED_EXPORT Q_DECL_IMPORT
    #endif
#else
  #if defined(CAQTDM_LIB_LIBRARY)
     #define CAQTDM_LIBSHARED_EXPORT
   #else
     #define CAQTDM_LIBSHARED_EXPORT
   #endif
 #endif
#endif

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
// Because QtInfoMsg doesnt exist there.
#define QtInfoMsg QtWarningMsg

#ifndef Q_NULLPTR
#if __cplusplus >= 201103L
    #define Q_NULLPTR nullptr
#else
    #define Q_NULLPTR 0
#endif
#endif
#endif
// in order to correctly define for c and c++ (no mismatched tags)
#ifdef __cplusplus
typedef class MessageWindow MessageWindow;
typedef class MutexKnobData MutexKnobData;
#else
typedef struct MessageWindow MessageWindow;
typedef struct MutexKnobData MutexKnobData;
#endif

#endif // CAQTDM_LIB_GLOBAL_H
