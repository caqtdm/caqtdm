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

#include "dec_int_from_format.h"
#include <QRegExp>
#include <QStringList>
#include <stdio.h>

#define qstoc(x) ""

DecIntFromFormat::DecIntFromFormat(QString f)
{
	d_error = false;
	d_intDefaults =  d_decDefaults = true;
	d_intDigits = 3;
	d_decDigits = 2;
	d_errorMsg = "No error";
	d_format = f;
}

bool DecIntFromFormat::decode()
{
	int pos = - 1;
	QStringList captures;
// 	printf("\e[1;36mdecode() format %s\e[0m\n", qstoc(d_format));
	if(d_format == "%d")
	{
		d_decDefaults = false;
		d_decDigits = 0;
		return true;
	}
        /* add ' in [0-9] to recognize "%'d" */
        QRegExp intRe("%[0-9]*\\.*[0-9']*d\\b");
	pos = intRe.indexIn(d_format);
	if(pos >= 0) /* integer */
	{
		d_decDefaults = false;
                d_decDigits = 0;
                QRegExp capRe("%\\.[']?([0-9]*)d\\b");
                QRegExp capReV2("%[']?([0-9]*)d\\b");
		pos = capRe.indexIn(d_format);
		if(pos >= 0 && capRe.capturedTexts().size() > 1)
		{
			d_intDigits = capRe.capturedTexts().at(1).toInt();
			d_intDefaults = false;
		}
		else
		{
			pos = capReV2.indexIn(d_format);
			if(pos >= 0  && capReV2.capturedTexts().size() > 1)
			{
				d_intDigits = capReV2.capturedTexts().at(1).toInt();
				d_intDefaults = false;
                    printf("warning: format \"%s\" is not correct, anyway accepting it and setting %d decimals\n",
					qstoc(d_format),  d_intDigits);
                    printf("note the correct format is \"%%.%d\"\n", d_intDigits);
			}
		}
	}
	else 
	{
		if(d_format == "%f")
			return true;
		QRegExp floatRe("%[0-9]*\\.{1,1}[0-9]+f\\b");
		pos = floatRe.indexIn(d_format);
		if(pos >= 0)
		{
			/* type %.3f  decimal digits only */
			QRegExp flRe1("%\\.{1,1}([0-9]+)f\\b");
			/* type %2.3f integer and decimal digits */
			QRegExp flRe2("%([0-9]*)\\.{1,1}([0-9]+)f\\b");
			pos = flRe1.indexIn(d_format);
			if(pos >= 0 && flRe1.capturedTexts().size() > 1)
			{
				d_decDigits = flRe1.capturedTexts().at(1).toInt();
				d_decDefaults = false;
			}
			else 
			{
				pos = flRe2.indexIn(d_format);
				if(pos >= 0 && flRe2.capturedTexts().size() > 2)
				{
					d_intDigits = flRe2.capturedTexts().at(1).toInt();
					d_decDigits = flRe2.capturedTexts().at(2).toInt();
                    printf("captured %d int %d dec\n", d_intDigits, d_decDigits);
					d_decDefaults = false;
					d_intDefaults =  false;
				}
			}
		}
	}
	if(pos < 0)
	{
                printf("format error in \"%s\"", qstoc(d_format));
		return false;
	}
	else
		return true;
}


