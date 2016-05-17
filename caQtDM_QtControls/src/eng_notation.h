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

#ifndef ENG_NOTATION_H
#define ENG_NOTATION_H

#include <QString>
#include <QVariant>
#include <QStringList>
#include <qtcontrols_global.h>

#define DISPLAY_MICRO 	"µ"
#define MICRO 			"u"

/* the format to specify if we want the engineering notation */
#define ENGFMT 		"eng"

/** \brief a QString that recognizes the <em>eng</em> engineering format and formats a
 *         value according to the desired significant digits.
 *
 * <h3>Usage</h3>
 * <p>
 * EngString is a QString. Its constructor accepts three parameters:
 * <ul>
 * <li>the default string to initialize the string itself if no format is specified or if the specified
 *     format does not contain the <em>eng</em> conversion string;</li>
 * <li>the format, which should contain the <em>eng</em> specifier to benefit from this special QString;</li>
 * <li>a QVariant representing the value. The value is normally converted to a double value, unless the 
 * <em>format</em> string is empty. In the latter case, setNum() is called if the QVariant can be converted
 * to a <em>double</em> type or the string is initilized with the value of QVariant::toString().
 * Of course it is not a good idea to use EngString with an empty format. It is instead perfectly legal to
 * pass a standard <em>printf()</em> format, such as <em>%d, %.2f, %e</em>... and so on, because 
 * such formats are completely <em>ignored</em> by EngString. In this case, when a standard printf 
 * conversion specifier is passed to an EngString, you must be sure  to correctly initialize  the QString
 * passed as first parameter to the constructor.</li>
 * </ul>
 * EngString is used inside TVariant::format() method and in qtcontrols EGauge.
 * </p>
 *
 * \par Example
 * \code 
 * double d = 1e10;
 * QString defaultString = QString().sprintf("%.2d, d);
 * QString format = "%3eng"; // represent the double value with 3 significant digits
 * EngString engString (defaultString, format, v);
 * QLabel *label = new QLabel(this);
 * label->setText(engString); // will show 10G
 * \endcode
 *
 */
class QTCON_EXPORT EngString : public QString
{
  public:
	/** \brief constructs an EngString ready to be used.
	 *
	 * @param s the default string if the format does not contain the <em>eng</em> conversion specifier.
	 * @param format the format. If it contains the <em>eng</em> specifier, then the string is represented in
	 *               the engineering notation, with IS standard suffixes.
	 * @param value a generic QVariant whose value has to be represented by the eng string.
	 *
	 * <h3>Recognized formats</h3>
	 * <p>
	 * The recognized formats are those exemplified below:
	 * <ul><li><em>%3eng</em>: represent the number with 3 significant digits;</li>
	 * <li><em>%3.2eng</em>: accepted but only the first digit is considered: will
	 * represent the number with 3 significant digits;</li>
	 * <li><em>%.3eng</em>: <strong>this is wrong</strong> and will be <em>ignored</em>.</li>
	 * </ul>
	 * </p>
	 */
	EngString(QString s, const QString& format, const QVariant &value);
	
	QString format() { return d_format; }
	QStringList suffixes;
	
	/** \brief returns the significant digits recognized by the regular expression  
	 *         that evaluates the format. 
	 * 
	 * May return 0 if something went wrong in the format parsing (i.e. you passed something like 
	 * <em>%.2eng</em>.
	 */
	int significantDigits() { return d_significantDigits; }
	
  private:
	
	int extractSignificantDigits(const QString &fmt);
	
	int d_prefixStart;
	int d_prefixEnd;
	int d_significantDigits;
	QString d_format;
};


#endif
