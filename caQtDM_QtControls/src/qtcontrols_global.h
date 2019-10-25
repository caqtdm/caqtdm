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

// nullptr problem and "fix" warnings
// see http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2431.pdf
#if defined(_MSC_VER)
    #if (_MSC_VER >1800)
     #define nullptr 0
    #endif
#else
    #if __cplusplus <= 199711L
    const // this is a const object...
    class {
    public:
        template<class T> // convertible to any type
        operator T*() const // of null non-member
        { return 0; } // pointer...
        template<class C, class T> // or any type of null
        operator T C::*() const // member pointer...
        { return 0; }
    private:
        void operator&() const; // whose address can't be taken
    } nullptr = {};

    #endif
#endif


#endif //QTCONGLOBAL_H
