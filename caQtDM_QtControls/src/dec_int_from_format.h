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

#ifndef DEC_INT_CONVERTER_H
#define DEC_INT_CONVERTER_H

#include <QString>

/** \brief Given a format expressed by a string, this class decodes it and extracts the number of integer and decimal digits
 *
 * Given a format expressed as QString in the constructor, the decode() method decodes the format and tells 
 * how many integer and decimal digits are needed.
 * In this first simple implementation, numbers of the form ddd.ddd are decoded, exponentials are not.
 */
class DecIntFromFormat
{
	public:
		/** \brief the constructor receiving the format to be converted.
		 *
		 * @param f the format.
		 * <strong>Examples</strong>
		 * <ul>
		 * <li><em>%d</em>, an integer, default values for the number of digits: 3 integers 0 decimals;</li>
		 * <li><em>%f</em>, a float, default values for the number of digits: 3 integers, 2 decimals;</li>
		 * <li><em>%.2f</em> or <em>%'.2f</em>, a float with 2 decimals, default number of digits</li>
		 * <li><em>%3.2f</em>, a float with three  integers and 2 decimals</li>
		 * <li><em>%4d</em> or <em>%.4d</em>, integer with four digits;</li>
		 * </ul>
		 * <strong>Note</strong> <p>Formats containing <em>`</em> are understood but haven't any
		 * effect on the output number of integers and decimals.</p>
		 */
		DecIntFromFormat(QString f);
		
		int numIntDigits() { return d_intDigits;}
		int numDecDigits() { return d_decDigits; }
		
		bool error() { return d_error; }
		QString errorMsg() { return d_errorMsg; }
		
		bool integerDefaults() { return d_intDefaults; }
		bool decimalDefaults() { return d_decDefaults; }
		
		/** \brief decoding method
		 *
		 * After construction, call decode on your object to retrieve later numIntDigits() and numDecDigits()
		 * @return true if the decodification was successful, false otherwise.
		 */
		bool decode();
		
		private:
			bool d_error, d_intDefaults, d_decDefaults;
			int d_intDigits, d_decDigits;
			QString d_errorMsg;
			QString d_format;
};

#endif

