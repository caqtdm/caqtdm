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

#ifndef MYPARSEREDM_H
#define MYPARSEREDM_H

#include <qfile.h>
#include "XmlWriter.h"

typedef char string40[40];
typedef struct _zOrder {
    int indx;
    int vis;
    string40 z;
} zOrder;


using namespace std;

class myParserEDM {

public:

    myParserEDM ();
    void openFile(char *outFile);
    void parseFile(char *infile);
    void closeFile();
    void writeProperty(const QString& name, const QString& type, const QString& value );
    void writeOpenProperty(const QString& name);
    void writeTaggedString(const QString& type, const QString& value );
    void writeCloseProperty();
    void writeStyleSheet(int r, int g, int b);

    void writeOpenTag(const QString& type, const QString& cls = "", const QString& name = "");
    void writeCloseTag(const QString& type);

    void Qt_writeOpenTag(char *tag, char *typ, char *value);
    void Qt_writeCloseTag(char *tag, char *value, int visibilityStatic);
    void Qt_handleString(char *prop, char *tag, char *value);
    void Qt_setColor(char *property, int r, int g, int b, int alpha);
    void Qt_setColorForeground(char *widget, int r, int g, int b, int alpha);
    void Qt_setColorBackground(char *widget, int r, int g, int b, int alpha);
    void Qt_setColorLine(char *widget, int r, int g, int b, int alpha);
    void Qt_setColorGrid(char *widget, int r, int g, int b, int alpha);
    void Qt_setColorScale(char *widget, int r, int g, int b, int alpha);
    void Qt_setColorTrace(char *widget, int r, int g, int b, int alpha, int trace);
    void Qt_setSymbolTrace(char *widget, char *symbol, int trace);
    void Qt_setStyleTrace(char *widget, char *style, int trace);
    void Qt_ChannelsXY(char *widget, char *channels, int trace);
    void Qt_extractString(char *str, char*retStr, int *status);

    void writeRectangleDimensions(int x, int y, int w, int h);
    // Zai added
    void writeFontProperties(int size, bool bold);

    void test();
    void Init(myParserEDM* edlParser);
    void writeMessage(char *mess);

    XmlWriter *xw;
    myParserEDM *edlParser;
    QFile *file;
    QString StyleSheet;
    zOrder zorder[10000];
    int zindex;
};

#endif
