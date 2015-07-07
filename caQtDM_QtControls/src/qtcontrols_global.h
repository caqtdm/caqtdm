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

#endif //QTCONGLOBAL_H
