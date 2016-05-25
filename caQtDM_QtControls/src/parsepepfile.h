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

#ifndef PARSEPEPFILE_H
#define PARSEPEPFILE_H

#include <qtcontrols_global.h>
#include <QMap>
#include <QBuffer>
#include <QDataStream>
#include <QSharedMemory>
#include <QWidget>
#include <QFile>
#include <QtUiTools/QUiLoader>

#define PRINT(x)

class QTCON_EXPORT ParsePepFile
{

public:
    ParsePepFile(QString filename, bool willPrint = false);
    QWidget *load(QWidget *parent);

protected:

private slots:

private:

    bool willPrint;

     enum { MaxGrid = 20};
     enum { MaxLines = 50};

    typedef struct _gridInfo {
        QString widgetType, widgetText, widgetHeight, widgetChannel, command, comlab;
        QString formats[2];
        int nbElem,span;
        bool textPresent;
        QColor fg, bg;
    } gridInfo;

    void TreatFile(int &nbRows, int &nbCols, QColor &bgColor, QFile *file);
    void DisplayFile(int nbRows, int nbCols, QByteArray *array);

    void getColumnPositions(int nbItems, int actualGridColumn, int spanGrid, int pos[], int span[]);
    void displayItem(int gridRow, int gridCol, gridInfo gridinfo, int spanGrid, int spanCols, int nbCols, QByteArray *array);
    void writeOpenProperty(QString property, QByteArray *array);
    void writeCloseProperty(QByteArray *array);
    void writeTaggedString(QString tag, QString value, QByteArray *array);
    void setColor(QString property, int r, int g, int b, int alpha, QByteArray *array);
    void setGeometry(int x, int y, int width, int height, QByteArray *array);
    void writeItemRowCol(int &row, int &column,  int span, QByteArray *array);
    void writeOpenTag(QString tag,  QByteArray *array);
    void writeCloseTag(QString tag,  QByteArray *array);
    void writeSimpleProperty(QString prop, QString tag, QString value, QByteArray *array);

    void writeLineEdit(QString format, QString text, QString minwidth, QString minheight, QString maxwidth, QString maxheight, QString pv, QString pointsize,
                       QString alignment, QString colormode, QString calc, QString visibility, int rgba[4], QByteArray *array);

    void writeTextEntry(QString format, QString text, QString minwidth, QString minheight, QString maxwidth, QString maxheight, QString pv, QString pointsize,
                       QString alignment, QString colormode, QString calc, QString visibility, int rgba[4], QByteArray *array);

    void writeLabel(QString text,QString minwidth,  QString minheight, QString maxwidth, QString maxheight, QString pointsize, QString alignment,QString colormode,
                    QString calcpv, QString calc, QString visibility, bool transparent, QColor fg, QColor bg, QByteArray *array);

    void writeChoice(QString pv, QByteArray *array);
    void writeMenu(QString pv, QByteArray *array);
    void writeWheelswitch(QString format, QString pv, QByteArray *array);
    void writeTogglebutton(QString pv, QByteArray *array);
    void writeLineEdit(QString format, QString pv, QByteArray *array);
    void writeShellCommand(QString label, QString command, QByteArray *array);
    void replaceStrings(gridInfo &grid);

    QBuffer *buffer;

    gridInfo gridLayout[MaxLines][MaxGrid];
    int firstCols[MaxGrid];
    int maxCols[MaxGrid];

};

#endif
