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

#ifndef MYPARSERADL_H
#define MYPARSERADL_H

#include <qfile.h>
#include "XmlWriter.h"

class myParser {

public:

    myParser ();
    void openFile(char *outFile);
    void closeFile();
    void writeProperty(const QString& name, const QString& type, const QString& value );
    void writeOpenProperty(const QString& name);
    void writeTaggedString(const QString& type, const QString& value );
    void writeCloseProperty();
    void writeStyleSheet(int r, int g, int b);

    void writeOpenTag(const QString& type, const QString& cls = "", const QString& name = "");
    void writeCloseTag(const QString& type);
    XmlWriter *xw;
    QFile *file;
    QString StyleSheet;
    void test();
    void Init(myParser* adlParser);
    void writeMessage(char *mess);
    bool adl2ui(QString inputFile);
    int myMain(int argc, char *argv[]);

    QString getTmp_directory() const;
    void setTmp_directory(const QString &value);

private:
    QString tmp_directory;
};
#endif
