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


#if defined(_MSC_VER)
   #define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "QtProperties.h"

#define DEFAULT_FORMAT "% 6.2f"
#define DEFAULT_FORMAT_WIDTH 6
#define DEFAULT_FORMAT_PRECISION 2

zOrder zorder[zorder_size];
int zindex=0;

/*
static void uppercase(char *str)
{
    int i;
    for( i = 0; str[ i ]; i++)
        str[i] = toupper( str[ i ] );
    return;
}
*/

static void replace_char (char *s, char find, char replace) {
    while (*s != 0) {
        if (*s == find) *s = replace;
        s++;
    }
}

void niceChannel(char *value)
{
    replace_char (value, ' ', '\0');
    replace_char (value, ';', ':');
    //uppercase(value);
}

/* Checks format validity and calls compute_format_size */
static void wheelSwitchFormat(char *format, int *width, int *precision)
{
    char *percent = NULL,*fpart = NULL,*ad;
    char format_used[255];
    char flag_char[2] = " ";
    int nparsed = 0;
    *width = DEFAULT_FORMAT_WIDTH;
    *precision = DEFAULT_FORMAT_PRECISION;

    /* Check for a % with an f following somewhere after */
    percent = strchr(format, '%');
    if(percent != NULL) {
        fpart = strchr(percent, 'f');
    }
    if(percent == NULL || fpart == NULL) {
        /* Doesn't have % with an f following, use the default */
        strcpy(format_used, DEFAULT_FORMAT);
    } else {
        /* Copy prefix and % (i.e. everything through %) */
        format_used[0] = '\0';
        strncat(format_used, format, percent - format + 1);

        /* Check flag(s).  We want '+' or ' ', with '+' superceding ' ' */
        ad = percent + 1;
        while(1) {
            if(*ad == '+') {
                /* Use + as the flag */
                flag_char[0]='+';
                ad++;
            } else if(*ad == ' ' || *ad == '#' || *ad == '0' || *ad == '-') {
                /* A flag, but use what we have now (+ or space) */
                ad++;
            } else {
                /* Not a flag */
                break;
            }
        }
        /* Add the single flag we want to use */
        strcat(format_used, flag_char);

        /* Width, point, and precision: Must be of form n.m.  Ignore
       * anything between this and f. Copy f to end. */
        nparsed=sscanf(ad,"%d.%d", width, precision);
        if(nparsed == 2) {
            if(*width < 0) *width=DEFAULT_FORMAT_WIDTH;
            if(*precision < 0) *precision=DEFAULT_FORMAT_PRECISION;
            if(*precision > *width-1) *precision=*width-1;
            if(*precision < 0) *precision=0;
        } else if(nparsed == 1) {
            if(*width < 0) {
                *width=DEFAULT_FORMAT_WIDTH;
                *precision=DEFAULT_FORMAT_PRECISION;
            } else {
                *precision=0;
            }
        } else {
            *width=DEFAULT_FORMAT_WIDTH;
            *precision=DEFAULT_FORMAT_PRECISION;
        }
    }
}

void Qt_handleString(char *prop, char *tag, char *value)
{
    C_writeOpenProperty(myParserPtr, prop);

    // in case of a channel, we have to replace blancs, dots and ; used by AC

    if(!strcmp(prop, "channel")) {
        niceChannel(value);
    }

    C_writeTaggedString(myParserPtr, tag, value);
    C_writeCloseProperty(myParserPtr);
}

void Qt_taggedString(char *tag, char *value)
{
    C_writeTaggedString(myParserPtr, tag, value);
}

void Qt_writeOpenTag(char *tag, char *typ, char *value)
{
    C_writeOpenTag(myParserPtr, tag, typ, value);
}

void Qt_writeCloseTag(char *tag, char *value, int visibilityStatic)
{
    if(!strcmp(tag, "widget") && strstr(value, "ca")) {

        strcpy(zorder[zindex].z, value);
        zorder[zindex].vis = visibilityStatic;
        zorder[zindex].indx = zindex;
        if (zindex<zorder_size) zindex++;
                else printf("Error: to many Layers(%i)\n",zindex);
    }
    C_writeCloseTag(myParserPtr, tag);
}

void Qt_writeOpenProperty(char *property)
{
    C_writeOpenProperty(myParserPtr, property);
}

void Qt_writeCloseProperty()
{
    C_writeCloseProperty(myParserPtr);
}

void Qt_writeStyleSheet(int r, int g, int b)
{
    C_writeStyleSheet(myParserPtr, r, g, b);
}

void Qt_setWheelSwitchForm(char *widget, char *token)
{
    int width, precision;
    char asc[10];

    UNUSED(widget);

    wheelSwitchFormat(token, &width, &precision);
    if(precision != 0) {
      sprintf(asc, "%d", width-precision-2);
    } else {
      sprintf(asc, "%d", width-precision-1);
    }
    Qt_handleString("integerDigits", "number", asc);
    sprintf(asc, "%d", precision);
    Qt_handleString("decimalDigits", "number", asc);
}

void Qt_setMinimumLimit(char *widget, int pen, char *token) {
    if(strstr(widget, "caStripPlot") == (char*) 0) {
        Qt_handleString("minValue", "double", token);
    } else {
        char asc[30];
        sprintf(asc, "YaxisLimitsMin_%d", pen+1);
        Qt_handleString(asc, "double", token);
    }
}

void Qt_setMaximumLimit(char *widget, int pen, char *token) {
    if(strstr(widget, "caStripPlot") == (char*) 0) {
        Qt_handleString("maxValue", "double", token);
    } else {
        char asc[30];
        sprintf(asc, "YaxisLimitsMax_%d", pen+1);
        Qt_handleString(asc, "double", token);
    }
}

void Qt_setPrecision(char *widget, int pen, char *token) {

    UNUSED(pen);

    if(!strcmp(widget, "caStripPlot")) return;
    if(!strcmp(widget, "caThermo")) return;
    //if(!strcmp(widget, "caSlider")) return;
    if(strstr(widget, "caNumeric") != (char*) 0) {
        int prec;
        char asc[10];
        sscanf(token, "%d", &prec);
        Qt_handleString("decimalDigits", "number", token);
        sprintf(asc, "%d", prec+1);
        Qt_handleString("integerDigits", "number", asc);
    } else {
        int prec;
        char asc[10];
        sscanf(token, "%d", &prec);
        sprintf(asc, "%d", prec);
        Qt_handleString("precision", "number", asc);
    }
}

void Qt_setXaxisLimitSource(char *widget, char *token)
{
    char asc[80];
    if(strstr(token, "auto") != (char*) 0) {
        sprintf(asc, "%s::Auto", widget);
    }else if(strstr(token, "default") != (char*) 0) {
        sprintf(asc, "%s::User", widget);
    }else if(strstr(token, "user") != (char*) 0) {
        sprintf(asc, "%s::User", widget);
    }else if(strstr(token, "channel") != (char*) 0) {
        sprintf(asc, "%s::Channel", widget);
    }
    Qt_handleString("XaxisScaling", "enum", asc);

}
void Qt_setYaxisLimitSource(char *widget, char *token)
{
    char asc[80];

    if(strstr(token, "auto") != (char*) 0) {
        sprintf(asc, "%s::Auto", widget);
    }else if(strstr(token, "default") != (char*) 0) {
        sprintf(asc, "%s::User", widget);
    }else if(strstr(token, "user") != (char*) 0) {
        sprintf(asc, "%s::User", widget);
    }else if(strstr(token, "channel") != (char*) 0) {
        sprintf(asc, "%s::Channel", widget);
    }
    Qt_handleString("YaxisScaling", "enum", asc);
}

void Qt_setMinimumLimitSource(char *widget, int pen, char *token)
{
    char strng[30];

    if(strstr(widget, "Gauge") != (char*) 0) return;

    if(strstr(token, "default") != (char*) 0) {
        sprintf(strng, "%s::User", widget);
    } else if(strstr(token, "channel") != (char*) 0) {
        sprintf(strng, "%s::Channel", widget);
    }
    if(!strcmp(widget, "caStripPlot") || !strcmp(widget, "caCartesianPlot")) {
        char asc[30];
        sprintf(asc, "YaxisScalingMin_%d", pen+1);
        Qt_handleString(asc, "enum", strng);
    } else if(!strcmp(widget, "caSlider")) {
        Qt_handleString("lowLimitMode", "enum", strng);
    } else {
        Qt_handleString("limitsMode", "enum", strng);
    }

}

void Qt_setMaximumLimitSource(char *widget, int pen, char *token)
{
    char strng[30] = {'\0'};

    if(strstr(widget, "Gauge") != (char*) 0) return;

    if(strstr(token, "default") != (char*) 0) {
        sprintf(strng, "%s::User", widget);
    } else if(strstr(token, "channel") != (char*) 0) {
        sprintf(strng, "%s::Channel", widget);
    }
    if(!strcmp(widget, "caStripPlot") || !strcmp(widget, "caCartesianPlot")) {
        char asc[30];
        sprintf(asc, "YaxisScalingMax_%d", pen+1);
        Qt_handleString(asc, "enum", strng);
    } else if(!strcmp(widget, "caSlider")) {
        Qt_handleString("highLimitMode", "enum", strng);
    } else {
        Qt_handleString("limitsMode", "enum", strng);
    }
}

void Qt_setPrecisionSource(char *widget, int pen, char *token)
{
    char asc[80], aux[80];

    UNUSED(pen);

    if(!strcmp(widget, "caStripPlot")) return;
    if(!strcmp(widget, "caThermo")) return;
    //if(!strcmp(widget, "caSlider")) return;
    if(strstr(widget, "Gauge") != (char*) 0) return;
    strcpy(aux, token);

    if(aux[0] >= 'a' && aux[0] <='z') aux[0] = aux[0] - 32;
    if(!strcmp(aux, "Default")) strcpy(aux, "User");
    sprintf(asc, "%s::%s", widget, aux);
    Qt_handleString("precisionMode", "enum", asc);
}

void Qt_setColorFill(char *widget) {

    UNUSED(widget);

    C_writeOpenProperty(myParserPtr, "fillstyle");
    C_writeTaggedString(myParserPtr, "enum", "Filled");
    C_writeCloseProperty(myParserPtr);
}

int Qt_setColorMode(char *widget, char *token)
{
    // returns 0 if alarm is static, 1 otherwise
    char asc[80];
    char aux[80];

    if(strstr(widget, "Gauge") != (char*) 0) return 0;

    if(!strcmp(token,"static")) {
        sprintf(asc, "%s::Static", widget);
        Qt_handleString("colorMode", "enum", asc);
        return 0;
    } else if(!strcmp(token,"alarm")) {
        strcpy(aux, "Alarm");
        if(!strcmp(widget, "caTextEntry")) strcpy(aux, "Alarm_Static");
        if(!strcmp(widget, "caLineEdit")) strcpy(aux, "Alarm_Static");
        if(!strcmp(widget, "caThermo")) strcpy(aux, "Alarm_Static");
        if(!strcmp(widget, "caSlider")) strcpy(aux, "Alarm_Static");
        sprintf(asc, "%s::%s", widget, aux);
        Qt_handleString("colorMode", "enum", asc);
        return 1;
    } else if(!strcmp(token,"discrete")) {
        sprintf(asc, "%s::Static", widget);
        Qt_handleString("colorMode", "enum", asc);
        return 0;
    }
    return 0;
}

int Qt_setVisibilityMode(char *widget, char *token)
{
    // returns 0 if visibility is static, 1 otherwise
    char asc[80];
    if(!strcmp(token,"static")) {
        sprintf(asc, "%s::StaticV", widget);
        Qt_handleString("visibility", "enum", asc);
        return 0;
    } else if(!strcmp(token,"if not zero")) {
        sprintf(asc, "%s::IfNotZero", widget);
        Qt_handleString("visibility", "enum", asc);
        return 1;
    } else if(!strcmp(token,"if zero")) {
        sprintf(asc, "%s::IfZero", widget);
        Qt_handleString("visibility", "enum", asc);
        return 1;
    } else if(!strcmp(token,"calc")) {
        sprintf(asc, "%s::Calc", widget);
        Qt_handleString("visibility", "enum", asc);
        return 1;
    }
    return 0;
}

static void Qt_setColor(char *property, int r, int g, int b, int alpha)
{
    char asc[80];
    C_writeOpenProperty(myParserPtr, property);
    sprintf(asc,"color alpha=\"%d\"", alpha);
    C_writeOpenTag(myParserPtr, asc, "", "");
    sprintf(asc,"%d", r);
    C_writeTaggedString(myParserPtr, "red", asc);
    sprintf(asc,"%d", g);
    C_writeTaggedString(myParserPtr, "green", asc);
    sprintf(asc,"%d", b);
    C_writeTaggedString(myParserPtr, "blue", asc);
    C_writeCloseTag(myParserPtr,"color");
    C_writeCloseProperty(myParserPtr);
}

void Qt_setColorForeground(char *widget, int r, int g, int b, int alpha) {

    UNUSED(widget);

    Qt_setColor("foreground", r, g, b, alpha);
}

void Qt_setColorBackground(char *widget, int r, int g, int b, int alpha) {

    UNUSED(widget);

    Qt_setColor("background", r, g, b, alpha);
}

void Qt_setColorScale(char *widget, int r, int g, int b, int alpha) {

    UNUSED(widget);

    Qt_setColor("scaleColor", r, g, b, alpha);
}

void Qt_setColorLine(char *widget, int r, int g, int b, int alpha) {

    UNUSED(widget);

    Qt_setColor("lineColor", r, g, b, alpha);
}

void Qt_setColorTrace(char *widget, int r, int g, int b, int alpha, int trace)
{
    char asc[80];

    UNUSED(widget);

    sprintf(asc,"color_%d", trace);
    Qt_setColor(asc, r, g, b, alpha);
}


void Qt_setColormain(char *widget, int r, int g, int b, int alpha)
{
    char asc[80];

    UNUSED(widget);

    C_writeOpenProperty(myParserPtr, "palette");

    C_writeOpenTag(myParserPtr, "palette", "", "");

    C_writeOpenTag(myParserPtr, "active", "", "");
    C_writeOpenTag(myParserPtr, "colorrole role=\"Window\"", "","");
    C_writeOpenTag(myParserPtr, "brush brushstyle=\"SolidPattern\"",  "","");
    sprintf(asc,"color alpha=\"%d\"", alpha);
    C_writeOpenTag(myParserPtr, asc, "", "");
    sprintf(asc,"%d", r);
    C_writeTaggedString(myParserPtr, "red", asc);
    sprintf(asc,"%d", g);
    C_writeTaggedString(myParserPtr, "green", asc);
    sprintf(asc,"%d", b);
    C_writeTaggedString(myParserPtr, "blue", asc);
    C_writeCloseTag(myParserPtr,"color");
    C_writeCloseTag(myParserPtr,"brush");
    C_writeCloseTag(myParserPtr,"colorrole");
    C_writeCloseTag(myParserPtr,"active");

    C_writeOpenTag(myParserPtr, "inactive", "", "");
    C_writeOpenTag(myParserPtr, "colorrole role=\"Window\"", "","");
    C_writeOpenTag(myParserPtr, "brush brushstyle=\"SolidPattern\"",  "","");
    sprintf(asc,"color alpha=\"%d\"", alpha);
    C_writeOpenTag(myParserPtr, asc, "", "");
    sprintf(asc,"%d", r);
    C_writeTaggedString(myParserPtr, "red", asc);
    sprintf(asc,"%d", g);
    C_writeTaggedString(myParserPtr, "green", asc);
    sprintf(asc,"%d", b);
    C_writeTaggedString(myParserPtr, "blue", asc);
    C_writeCloseTag(myParserPtr,"color");
    C_writeCloseTag(myParserPtr,"brush");
    C_writeCloseTag(myParserPtr,"colorrole");
    C_writeCloseTag(myParserPtr,"inactive");

    C_writeCloseTag(myParserPtr,"palette");

    C_writeCloseProperty(myParserPtr);

}
