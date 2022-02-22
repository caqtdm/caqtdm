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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    #include <QRegExp>
#else
    #include <QRegularExpression>
#endif
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
    int posV2 = - 1;
    int posV3 = - 1;
	QStringList captures;
// 	printf("\e[1;36mdecode() format %s\e[0m\n", qstoc(d_format));
	if(d_format == "%d")
	{
		d_decDefaults = false;
		d_decDigits = 0;
		return true;
	}
        /* add ' in [0-9] to recognize "%'d" */

        QString pattern = QString("%[0-9]*\\.*[0-9']*d\\b");
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QRegExp intRe(pattern);
        pos = intRe.indexIn(d_format);
#else
        QRegularExpression intRe(pattern);
        QRegularExpressionMatch match = intRe.match(d_format);
        pos=match.capturedStart();

#endif


	if(pos >= 0) /* integer */
	{
		d_decDefaults = false;
        d_decDigits = 0;

        QString pattern = QString("%\\.[']?([0-9]*)d\\b");
        QString patternV2 = QString("%[']?([0-9]*)d\\b");

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QRegExp capRe(pattern);
        QRegExp capReV2(patternV2);
        pos = capRe.indexIn(d_format);
        posV2 = capReV2.indexIn(d_format);
        QStringList capRe_Text=capRe.capturedTexts();
        QStringList capReV2_Text=capReV2.capturedTexts();

#else
        QRegularExpression capRe(pattern);
        QRegularExpression capReV2(patternV2);

        QRegularExpressionMatch match = intRe.match(d_format);
        QRegularExpressionMatch matchV2 = intRe.match(d_format);
        pos=match.capturedStart();
        posV2=matchV2.capturedStart();
        QStringList capRe_Text=match.capturedTexts();
        QStringList capReV2_Text=matchV2.capturedTexts();
#endif


        if(pos >= 0 && capRe_Text.size() > 1)
		{
            d_intDigits = capRe_Text.at(1).toInt();
			d_intDefaults = false;
		}
		else
		{

            if(posV2 >= 0  && capReV2_Text.size() > 1)
			{
                d_intDigits = capReV2_Text.at(1).toInt();
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

        QString pattern = QString("%[0-9]*\\.{1,1}[0-9]+f\\b");

        QString patternV2 = QString("%\\.{1,1}([0-9]+)f\\b");

        QString patternV3 = QString("%([0-9]*)\\.{1,1}([0-9]+)f\\b");
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QRegExp floatRe(pattern);
        QRegExp flRe1(patternV2);
        QRegExp flRe2(patternV3);
        pos = floatRe.indexIn(d_format);
        posV2 = flRe1.indexIn(d_format);
        posV3 = flRe2.indexIn(d_format);
        QStringList floatRe_Text=floatRe.capturedTexts();
        QStringList flRe1_Text=flRe1.capturedTexts();
        QStringList flRe2_Text=flRe2.capturedTexts();
#else
        QRegularExpression floatRe(pattern);
        QRegularExpression flRe1(patternV2);
        QRegularExpression flRe2(patternV3);

        QRegularExpressionMatch match = floatRe.match(d_format);
        QRegularExpressionMatch matchV2 = flRe1.match(d_format);
        QRegularExpressionMatch matchV3 = flRe2.match(d_format);
        pos=match.capturedStart();
        posV2=matchV2.capturedStart();
        posV3=matchV3.capturedStart();
        QStringList floatRe_Text=match.capturedTexts();
        QStringList flRe1_Text=matchV2.capturedTexts();
        QStringList flRe2_Text=matchV3.capturedTexts();
#endif

		if(pos >= 0)
		{
            pos = posV2;
            if(pos >= 0 && flRe1_Text.size() > 1)
			{
                d_decDigits = flRe1_Text.at(1).toInt();
				d_decDefaults = false;
			}
			else 
			{
                pos = posV3;
                if(pos >= 0 && flRe2_Text.size() > 2)
				{
                    d_intDigits = flRe2_Text.at(1).toInt();
                    d_decDigits = flRe2_Text.at(2).toInt();
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


