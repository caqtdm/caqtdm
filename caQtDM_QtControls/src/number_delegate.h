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

#ifndef NUMBER_DELEGATE
#define NUMBER_DELEGATE

#include <QString>


class NumberDelegate
{
	public:
		NumberDelegate();
        virtual ~NumberDelegate() {}
		
		int integerDigits() { return d_intDigits; }
		int decimalDigits() { return d_decDigits; }
		
		/** \brief decodes on the basis of the format, correcting integer and decimal digits with minimum 
		 *         and maximum values
		 */
		void configureNumber(const double min, const double max, const QString& format);
		
		/** \brief decodes on the basis of the format only
		 */
		bool configureNumber(const QString& format);
		
	protected:
		
		int d_intDigits, d_decDigits;
};

#endif
