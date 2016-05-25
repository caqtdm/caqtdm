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

#include <QtGui>
#include <math.h>
#include "parsepepfile.h"

ParsePepFile::ParsePepFile(QString filename, bool willprint)
{
    QString header;
    QString footer;

    QFile *file = new QFile;
    int nbRows, nbCols;
    QColor bgColor;

    willPrint = willprint;

    for(int i=0; i< MaxLines; i++) {
        for (int j=0; j<MaxGrid; j++) {
            gridLayout[i][j].widgetType = "";
            gridLayout[i][j].textPresent = false;
            gridLayout[i][j].widgetText = "";
            gridLayout[i][j].widgetChannel = "";
            gridLayout[i][j].comlab = "";
            gridLayout[i][j].command = "";
            gridLayout[i][j].nbElem = 0;
            gridLayout[i][j].span = 0;
            gridLayout[i][j].widgetHeight = "";
        }
    }
    for (int j=0; j<MaxGrid; j++) {
        firstCols[j] = 0;
        maxCols[j] = 0;
    }

    buffer = new QBuffer();

    // parese file
    file->setFileName(filename);
    bgColor = QColor(218, 218, 218, 255); // default bg color is gray, can be overwritten with !qtbg
    TreatFile(nbRows, nbCols, bgColor, file);

    PRINT(printf("nbRows=%d nbCols=%d\n", nbRows, nbCols));

    header = QString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
           "<ui version=\"4.0\">\n"
           "<class>MainWindow</class>\n"
           "<widget class=\"QMainWindow\" name=\"MainWindow\">\n"
           "<property name=\"styleSheet\">\n"
           "<string>\n"
           "QWidget#centralWidget {background: " + bgColor.name() + "; }\n"
           "caLineEdit {border-radius: 1px;background: white; color: black;}\n"
           "</string>\n"
           "</property>\n"
           "<widget class=\"QWidget\" name=\"centralWidget\">\n"
           "<layout class=\"QGridLayout\" name=\"gridLayout\">\n"
           "<item row=\"0\" column=\"0\">\n"
           "<layout class=\"QGridLayout\" name=\"gridLayout\">\n"
           "<property name=\"spacing\">\n"
           "<number>5</number>\n"
           "</property>\n");

    // copy first header to our byte array
    QByteArray *array= new QByteArray();
    array->append(header);

    // fill array with the scanned data
    DisplayFile(nbRows, nbCols, array);

    // and finish with the footer
    footer = QString("</layout></item></layout></widget></widget></ui>");
    array->append(footer);

    // fill buffer with the byte array data
    buffer->open(QIODevice::ReadWrite);
    buffer->write(*array);
    buffer->close();

    // delete allocated data
    delete file;
    delete array;
}

void ParsePepFile::TreatFile(int &nbRows, int &nbCols, QColor &bgColor, QFile *file)
{
    int grid = 1;
    int ll;
    int span =0;
    int nbFormats = 0;
    int actualLine = 0;
    int actualColumn = 0;
    QString formats[2];
    QString widgetType ="";
    QString widgetText ="";
    QString channel ="";
    QString separator((QChar)27);
    QString widgetHeight ="2";
    QColor fg;
    QColor bg;

    if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) return ;
    // scan file
    QTextStream in(file);
    QString line = in.readLine();
    while (!line.isNull()) {

        PRINT(printf("line : <%s>\n", qasc(line)));

        // replace all tabs by blancs
        line.replace("\t", " ");
        // replace all blancs  by \e, except the ones between quotes
        bool inside = false;
        for (int i = 0; i < line.size(); i++) {
            if((!inside) && ((line.at(i) == QChar('\'')) || (line.at(i) == QChar('\"')) || (line.at(i) == QChar('{')) )) {
                inside = true;
            } else if((inside) && ((line.at(i) == QChar('\'')) || (line.at(i) == QChar('\"')) || (line.at(i) == QChar('}')))) {
                inside = false;
            }
            if (!inside) {
                if(line.at(i) == QChar(' ')) {
                    line.replace(i, 1, separator);
                }
            }
        }

        // break the line down in their elements
        QStringList elements= line.split(separator,  QString::SkipEmptyParts);
/*
        printf("%d %s\n", elements.count(), qasc(line));
        for (int i=0; i< elements.count(); i++) {
            printf("<%s>\n",  qasc(elements.at(i)));
        }
*/
        // empty line ?
        if(line.size() == 0) {

        }
        else if (line.contains("#!grid")) {
            grid = elements.at(1).toInt();
            if(grid > MaxGrid) {
                printf("parse pep file -- grid value exceeds the maximum of %d\n", MaxGrid);
                grid = MaxGrid;
            }
            PRINT(printf("grid=%d\n", grid));
        } else if(line.contains("#!qtbg")) {
            bgColor = QColor(elements.at(1));
        } else if(line.contains("channel")) {
        } else if(line.at(0) != QChar('#')) {

            if(elements.count() > 1) {

                channel = elements.at(0);
                if(elements.count() > 1) {
                    widgetType = elements.at(1);
                    widgetType =  widgetType.toLower();
                }

                ll = 2;
                widgetText = "";
                widgetHeight ="2";
                formats[0] = formats[1] = "";
                span = 1;
                nbFormats = 0;
                fg = QColor(0, 0, 0);
                bg = QColor(200, 200, 200);


                while (ll < elements.count()) {
                    bool ok = true;
                    PRINT(printf("ll=%d elem=%s\n",ll, qasc(elements.at(ll))));
                    float number = elements.at(ll).toFloat(&ok);
                    Q_UNUSED(number);

                    if(elements.at(ll).contains("-span")) {
                        ll++;
                        if(ll < elements.count()) {
                            PRINT(printf("span detected, value=%s\n", qasc(elements.at(ll))));
                            span = elements.at(ll).toInt();
                        }
                    }
                    else if(elements.at(ll).contains("-confirm")) {
                        PRINT(printf("confirm detected\n"));
                    }

                    else if(elements.at(ll).contains("-hys")) {
                        PRINT(printf("hys detected\n"));
                    }

                    else if(elements.at(ll).contains("-comlab")) {
                        ll++;
                        PRINT(printf("comlab detected <%s>\n", qasc(elements.at(ll))));
                        gridLayout[actualLine][actualColumn].comlab = qasc(elements.at(ll));
                    }

                    else if(elements.at(ll).contains("-command")) {
                        ll++;
                        PRINT(printf("commmand detected <%s>\n", qasc(elements.at(ll))));
                        gridLayout[actualLine][actualColumn].command = qasc(elements.at(ll));
                        gridLayout[actualLine][actualColumn].command =  gridLayout[actualLine][actualColumn].command.replace("\"", "");
                    }

                    else if(elements.at(ll).contains("-text")) {
                        ll++;
                        PRINT(printf("text detected <%s>\n", qasc(elements.at(ll))));
                        widgetText = elements.at(ll);
                        gridLayout[actualLine][actualColumn].textPresent = true;
                    }


                    else if(elements.at(ll).contains("-notext")) {
                        widgetText = "";
                        gridLayout[actualLine][actualColumn].textPresent = true;
                    }

                    else if(elements.at(ll).contains("-ledstate")) {
                        ll++;
                    }

                    else if(elements.at(ll).contains("-height")) {
                        ll++;
                        PRINT(printf("height detected <%s>\n", qasc(elements.at(ll))));
                        widgetHeight = elements.at(ll);
                    }

                    else if(elements.at(ll).contains("-fg") || elements.at(ll).contains("-comfg")) {
                        QString fgs;
                        ll++;
                        PRINT(printf("fg or comfg detected <%s>\n", qasc(elements.at(ll))));
                        fgs = elements.at(ll);
                        fgs = fgs.replace("\"", "");
                        fg = QColor(fgs);
                    }

                    else if(elements.at(ll).contains("-bg")) {
                        QString bgs;
                        ll++;
                        PRINT(printf("bg detected <%s>\n", qasc(elements.at(ll))));
                        bgs = elements.at(ll);
                        bgs = bgs.replace("\"", "");
                        bg = QColor(bgs);

                    } else if(elements.at(ll-1).contains("comment")) {
                        PRINT(printf("comment  detected %s\n", qasc(elements.at(ll))));
                        widgetText = elements.at(ll);
                    }

                    else {
                        PRINT(printf("something else  detected <%s>\n", qasc(elements.at(ll))));
                        QString item = elements.at(ll);
                        // is this a format
                        if(item.at(0) == '%') ok = true;
                        if(item.right(1).contains("e") || item.right(1).contains("f") || item.right(1).contains("g") || item.right(1).contains("x") || item.right(1).contains("o"))   ok = true;
                        widgetText.append(" ");
                        widgetText.append(elements.at(ll));
                    }

                    if(ok && (widgetType.contains("setrdbk") ||
                              widgetType.contains("wheelswitch") ||
                              widgetType.contains("formread")) ) {
                        PRINT(printf("format detected %s\n", qasc(elements.at(ll))));
                        formats[nbFormats++] =elements.at(ll);
                    }
                    ll++;
                }

                widgetType = elements.at(1);
                widgetType = widgetType.toLower();
                widgetText = widgetText.replace("\"", "");

                // if only blancs then there should be no text
                QString aux = widgetText.trimmed();
                if(aux.size() < 1) widgetText = "";

                gridLayout[actualLine][actualColumn].span = span;
                gridLayout[actualLine][actualColumn].widgetType = widgetType;
                gridLayout[actualLine][actualColumn].widgetText = widgetText;
                gridLayout[actualLine][actualColumn].widgetChannel = channel;
                gridLayout[actualLine][actualColumn].widgetHeight = widgetHeight;
                gridLayout[actualLine][actualColumn].formats[0] = formats[0];
                gridLayout[actualLine][actualColumn].formats[1] = formats[1];
                gridLayout[actualLine][actualColumn].fg = fg;
                gridLayout[actualLine][actualColumn].bg = bg;

                PRINT(printf("%d %d %d <%s>\n", actualLine, actualColumn,  span, qasc(widgetType)));

                if(widgetType.contains("led") || widgetType.contains("formread") || widgetType.contains("wheelswitch") || widgetType.contains("choicebutton") ||
                  widgetType.contains("menubutton")) {
                    if(gridLayout[actualLine][actualColumn].textPresent && gridLayout[actualLine][actualColumn].widgetText.size() == 0) {
                        gridLayout[actualLine][actualColumn].nbElem = 1;
                    } else {
                        gridLayout[actualLine][actualColumn].nbElem = 2;
                    }

                    if(gridLayout[actualLine][actualColumn].command.size() > 0) {
                         gridLayout[actualLine][actualColumn].nbElem++;
                    }

                }

                else if(widgetType.contains("binary")) {
                    if(gridLayout[actualLine][actualColumn].textPresent && gridLayout[actualLine][actualColumn].widgetText.size() == 0) {
                        gridLayout[actualLine][actualColumn].nbElem = 2;
                    } else {
                        gridLayout[actualLine][actualColumn].nbElem = 3;
                    }
                }

                else if(widgetType.contains("text")) gridLayout[actualLine][actualColumn].nbElem = 4;
                else if(widgetType.contains("setrdbk")) gridLayout[actualLine][actualColumn].nbElem = 7;
                else if(widgetType.contains("comment")) {
                    gridLayout[actualLine][actualColumn].nbElem = 1;
                    if(gridLayout[actualLine][actualColumn].command.size() > 0) {
                        if(gridLayout[actualLine][actualColumn].widgetText.size() > 0)  gridLayout[actualLine][actualColumn].nbElem++;
                    }
                }
                else if(widgetType.contains("separator")) gridLayout[actualLine][actualColumn].nbElem = 1;
                else gridLayout[actualLine][actualColumn].nbElem = 1;

                actualColumn += span;
                if(actualColumn > grid-1) {
                    actualColumn = 0;
                    actualLine++;
                    if(actualLine > MaxLines -1) {
                        printf("parse pep file -- number of lines exceeds the maximum of %d\n", MaxLines);
                        actualLine--;
                        break;
                    }
                }
            }
        }

        line = in.readLine();
    }
    file->close();

    actualLine++;
/*
        for(int i=0; i< actualLine; i++) {
            for (int j=0; j<grid; j++) {
                printf("row=%d col=%d %20s text=%s nbelem=%d span=%d\n", i, j, qasc(gridLayout[i][j].widgetType),
                       qasc(gridLayout[i][j].widgetText), gridLayout[i][j].nbElem, gridLayout[i][j].span);
            }
        }
*/
    nbRows = actualLine;
    nbCols = grid;

}

void ParsePepFile::DisplayFile(int nbRows, int nbCols, QByteArray *array)
{

    for(int i=0; i< nbRows; i++) {
        for (int j=0; j<nbCols; j++) {
            if(gridLayout[i][j].nbElem > maxCols[j]) maxCols[j] =  gridLayout[i][j].nbElem;
        }
    }
/*
    printf("maxcols\n");
    for(int j=0; j<nbCols; j++) printf("%d ", maxCols[j]);
    printf("\n");
*/
    for (int j=0; j<nbCols; j++) {
        firstCols[j] = 0;
        for(int k=0; k<j; k++) firstCols[j] += maxCols[k];
    }
/*
    printf("firstcols\n");
    for(int j=0; j<nbCols; j++) printf("%d ", firstCols[j]);
    printf("\n");
*/

    for(int i=0; i< nbRows; i++) {
        for (int j=0; j<nbCols; j++) {

            int spanColumns = 1;
/*
            printf("===== %d %d %20s text=%10s span=%d col=%d\n", i, j,  qasc(gridLayout[i][j].widgetType),
                   qasc(gridLayout[i][j].widgetText), gridLayout[i][j].span, col);
*/

            // compute how many columns have to be spanned for our grid span
            if(gridLayout[i][j].span > 1) {
                for(int k=0; k< gridLayout[i][j].span; k++) {
                    spanColumns = spanColumns + maxCols[k+j];
                }
                spanColumns -=1;
            }

            // and display it
            if(gridLayout[i][j].widgetType.size() > 0) {
                replaceStrings(gridLayout[i][j]);
                displayItem(i, j, gridLayout[i][j], gridLayout[i][j].span, spanColumns, nbCols, array);
            }
        }
    }

}

void ParsePepFile::getColumnPositions(int nbItems, int actualGridColumn, int spanGrid, int pos[], int span[])
{

    // to be completely reworked
    int sum[MaxGrid];

    for(int i=0; i<MaxGrid; i++) {
        span[i] = 0;
        sum[i] = 0;
    }

    // in case only one item has to put into a row with a span > 1 we have to calculate the internal colum position
    // and number of intercolumns to span
    if(nbItems == 1) {
        PRINT(printf("========= one item to put with span=%d\n", spanGrid));
        pos[0] = firstCols[actualGridColumn];
        span[0] = 0;
        if(spanGrid > 1) {
            for(int k=0; k< spanGrid; k++) {
                span[0] = span[0] + maxCols[k+actualGridColumn];
            }
        }
        for(int i=0; i< nbItems; i++) {
            if(span[i] < 1) span[i] = 1;
            PRINT(printf("item=%d at column=%d span=%d\n", i, pos[i], span[i]));
        }
        return;
    }

    // more than one item with span > 1, calculate what we can put in the different columns

    PRINT(printf("========= more than one item nbitems=%d with span=%d\n", nbItems, spanGrid));

    sum[0] = nbItems;
    for(int i=1; i< qMin(spanGrid, nbItems); i++) {
        sum[i] = maxCols[actualGridColumn+i];
        sum[0] -= sum[i];
        // nothing left in this column
        if(sum[0] <= 0) {
            sum[0] = 1;
            sum[i]= nbItems - sum[0];
        }
        if(sum[0] == 1) break;
    }

    for(int i=0; i< spanGrid; i++) {
        span[i] = span[i] + maxCols[actualGridColumn+i];
        PRINT(printf("items in this grid %d nb=%d\n", i, sum[i]));
    }

    int pos1 = 0;
    int pos2 = sum[0];
    for(int j=0; j < spanGrid; j++) {
        for(int i=pos1; i< pos2; i++) {
            pos[i] = firstCols[actualGridColumn+j] + i - pos1;
        }
        pos1 = pos1 + sum[j];
        pos2 = pos1 + sum[j] + sum[j+1];
    }

    for(int i=0; i< nbItems; i++) {
        if(span[i] < 1) span[i] = 1;
        PRINT(printf("item=%d at column=%d spancol=%d\n", i, pos[i], span[i]));
    }
}

void ParsePepFile::replaceStrings(gridInfo &grid)
{
    grid.widgetText.replace("\"","");
    grid.widgetText.replace("\'","");
    grid.widgetText.replace("&","&amp;");
    grid.widgetText.replace("<", "&lt;");
    grid.widgetText.replace(">", "&gt;");


    grid.command.replace("{","");
    grid.command.replace("}","");
    grid.command.replace("&","&amp;");
    grid.command.replace("<", "&lt;");
    grid.command.replace(">", "&gt;");

    grid.comlab.replace("{","");
    grid.comlab.replace("}","");
    grid.comlab.replace("\"","");
}

void ParsePepFile::displayItem(int actualgridRow,int actualgridColumn, gridInfo grid, int spanGrid, int spanColumns, int nbColumns, QByteArray *array)
{
    Q_UNUSED(nbColumns);
    QString fontSize = "12";
    QString lineHeight = "20";
    QString newpv = "";
    QString partialpv = "";
    int rgba[4];
    int pos[MaxGrid], span[MaxGrid], count;

    int effectiveColumn= firstCols[actualgridColumn];
    int effectiveSpan;
    QColor fg = grid.fg;
    QColor bg = grid.bg;
    QString widgetHeight = grid.widgetHeight;

    rgba[0] = 0; rgba[1] = 0; rgba[2] = 0; rgba[3] = 255;

    QStringList pvElements= grid.widgetChannel.split(":",  QString::SkipEmptyParts);
    if(pvElements.count() > 0) partialpv = pvElements.at(0);

    if(grid.widgetType.contains("menubutton")) {
        PRINT(printf("create menubutton row=%d column=%d spangrid=%d\n", actualgridRow, effectiveColumn, spanGrid));

        writeItemRowCol(actualgridRow, effectiveColumn, spanGrid, array);

        if(!grid.textPresent || grid.widgetText.size() != 0) {
            if(!grid.textPresent) {
               writeLabel(grid.widgetChannel, "", lineHeight, "", "", fontSize, "Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter", "", "", "", "", true, fg, bg, array);
            } else {
               writeLabel(grid.widgetText, "", lineHeight, "", "", fontSize, "Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter", "", "", "", "", true, fg, bg, array);
            }
            writeCloseTag("item", array);
            // write now the led
            writeItemRowCol(actualgridRow, effectiveColumn, 1, array);
        }

        writeMenu(grid.widgetChannel, array);
        writeCloseTag("item", array);

        //////////////////////////////////////////////////////////////////////////////////
    } else if(grid.widgetType.contains("led")) {
        PRINT(printf("create led row=%d column=%d\n", actualgridRow, effectiveColumn));

        writeItemRowCol(actualgridRow, effectiveColumn, spanGrid, array);

        if(!grid.textPresent || grid.widgetText.size() != 0) {
            if(!grid.textPresent) {
               writeLabel(grid.widgetChannel, "", lineHeight, "", "", fontSize, "Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter", "", "", "", "", true, fg, bg, array);
            } else {
               writeLabel(grid.widgetText, "", lineHeight, "", "", fontSize, "Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter", "", "", "", "", true, fg, bg, array);
            }
            writeCloseTag("item", array);
            // write now the led
            writeItemRowCol(actualgridRow, effectiveColumn, 1, array);
        }

        writeOpenTag("widget class=\"caFrame\" name=\"caframe\"", array);
        writeOpenProperty("maximumSize", array);
        writeOpenTag("size", array);
        writeTaggedString("width", "24", array);
        writeTaggedString("height", "24", array);
        writeCloseTag("size", array);
        writeCloseProperty(array);
        writeOpenProperty("minimumSize", array);
        writeOpenTag("size", array);
        writeTaggedString("width", "24", array);
        writeTaggedString("height", "24", array);
        writeCloseTag("size", array);
        writeCloseProperty(array);

        writeOpenTag("widget class=\"caLed\" name=\"caled\"", array);
        writeOpenProperty("maximumSize", array);
        writeOpenTag("size", array);
        writeTaggedString("width", "18", array);
        writeTaggedString("height", "18", array);
        writeCloseTag("size", array);
        writeCloseProperty(array);
        writeOpenProperty("minimumSize", array);
        writeOpenTag("size", array);
        writeTaggedString("width", "18", array);
        writeTaggedString("height", "18", array);
        writeCloseTag("size", array);
        writeCloseProperty(array);
        setGeometry(3, 3, 15, 15, array);
        writeSimpleProperty("channel", "string", grid.widgetChannel, array);
        writeSimpleProperty("colorMode", "enum", "caLed::Static", array);
        setColor("trueColor", 0, 205, 0, 255, array);
        setColor("falceColor", 160, 160, 164, 255, array);
        writeCloseTag("widget", array);

        writeOpenTag("widget class=\"caGraphics\" name=\"cagraphics\"", array);
        writeOpenProperty("maximumSize", array);
        writeOpenTag("size", array);
        writeTaggedString("width", "24", array);
        writeTaggedString("height", "24", array);
        writeCloseTag("size", array);
        writeCloseProperty(array);
        writeOpenProperty("minimumSize", array);
        writeOpenTag("size", array);
        writeTaggedString("width", "24", array);
        writeTaggedString("height", "24", array);
        writeCloseTag("size", array);
        writeCloseProperty(array);
        writeSimpleProperty("channel", "string", grid.widgetChannel, array);
        writeSimpleProperty("colorMode", "enum", "caGraphics::Alarm", array);
        writeSimpleProperty("form", "enum", "caGraphics::Circle", array);
        writeSimpleProperty("fillstyle", "enum", "caGraphics::Filled", array);
        writeCloseTag("widget", array);

        writeTaggedString("zorder", "cagraphics", array);
        writeTaggedString("zorder", "caled", array);
        writeCloseTag("widget", array);

        writeCloseTag("item", array);

        //////////////////////////////////////////////////////////////////////////////////
    } else if (grid.widgetType.contains("text")) {

        PRINT(printf("create text row=%d column=%d\n", actualgridRow, effectiveColumn));

        writeItemRowCol(actualgridRow, effectiveColumn, spanGrid, array);

        writeLabel(grid.widgetChannel, "", lineHeight, "", "", fontSize, "Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter", "", "", "", "", true, fg, bg, array);
        writeCloseTag("item", array);

        // write now the lineedit in next column
        writeItemRowCol(actualgridRow, effectiveColumn, 1, array);
        writeLineEdit(grid.formats[0], grid.widgetChannel, "100", lineHeight, "", "", fontSize, "", "", "", "", "", rgba, array);
        writeCloseTag("item", array);

        writeItemRowCol(actualgridRow, effectiveColumn, 1, array);
        writeLabel("new value", "", lineHeight, "", "", fontSize, "Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter", "", "", "", "", true, fg, bg, array);
        writeCloseTag("item", array);

        // write now the textentry in next column
        writeItemRowCol(actualgridRow, effectiveColumn, 1, array);
        writeTextEntry(grid.formats[0], grid.widgetChannel, "100", lineHeight, "", "", fontSize, "", "", "", "", "", rgba, array);
        writeCloseTag("item", array);

        //////////////////////////////////////////////////////////////////////////////////
    } else if (grid.widgetType.contains("choicebutton")) {

        PRINT(printf("create choicebutton row=%d column=%d <%s>\n", actualgridRow, effectiveColumn, qasc(grid.widgetText)));

        writeItemRowCol(actualgridRow, effectiveColumn, spanGrid, array);

        if(!grid.textPresent || grid.widgetText.size() != 0) {
            if(!grid.textPresent) {
               writeLabel(grid.widgetChannel, "", lineHeight, "", "", fontSize, "Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter", "", "", "", "", true, fg, bg, array);
            } else {
               writeLabel(grid.widgetText, "", lineHeight, "", "", fontSize, "Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter", "", "", "", "", true, fg, bg, array);
            }
            writeCloseTag("item", array);
            // write now the choicebutton
            writeItemRowCol(actualgridRow, effectiveColumn, 1, array);
        }

        writeChoice(grid.widgetChannel, array);
        writeCloseTag("item", array);

        //////////////////////////////////////////////////////////////////////////////////
    } else if (grid.widgetType.contains("separator")) {

        PRINT(printf("create separator over %d grid columns , %d columns\n", spanGrid, spanColumns));

        writeItemRowCol(actualgridRow, effectiveColumn, spanColumns, array);

        writeOpenTag("widget class=\"Line\" name=\"line\"", array);
        writeOpenProperty("minimumSize", array);
        writeOpenTag("size", array);
        writeTaggedString("height", widgetHeight, array);
        writeTaggedString("width", "0", array);
        writeCloseTag("size", array);
        writeCloseProperty(array);
        writeOpenProperty("maximumSize", array);
        writeOpenTag("size", array);
        writeTaggedString("width", "16777215", array);
        writeTaggedString("height", widgetHeight, array);
        writeCloseTag("size", array);
        writeCloseProperty(array);
        setGeometry(0, 0, 0, widgetHeight.toInt(), array);
        writeSimpleProperty("frameShadow", "enum", "QFrame::Plain", array);
        writeSimpleProperty("lineWidth", "number", widgetHeight, array);
        writeSimpleProperty("orientation", "enum", "Qt::Horizontal", array);
        writeSimpleProperty("styleSheet", "string", "color: " + fg.name(), array);
        writeCloseTag("widget", array);
        writeCloseTag("item", array);

        //////////////////////////////////////////////////////////////////////////////////
    } else if (grid.widgetType.contains("comment")) {

        PRINT(printf("create comment row=%d column=%d over %d grid columns\n", actualgridRow, effectiveColumn, spanGrid));

        // how many items have to be displayed
        int nbItems = 1;
        if(grid.command.size() > 0) {
            if(grid.widgetText.size() > 0) nbItems++;
        }

        // get effective column positions and span values
        count = 0;
        getColumnPositions(nbItems, actualgridColumn, spanGrid, pos, span);
        effectiveSpan = span[count];
        effectiveColumn = pos[count++];

        writeItemRowCol(actualgridRow, effectiveColumn, effectiveSpan, array);

        if(grid.command.size() > 0) {
            if(grid.widgetText.size() > 0) {
                writeLabel(grid.widgetText, "", "20",  "16777215", "20", fontSize, "Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter", "", "", "", "", false, fg, bg, array);
                writeCloseTag("item", array);
                effectiveSpan = span[count];
                effectiveColumn = pos[count++];
                writeItemRowCol(actualgridRow, effectiveColumn, effectiveSpan, array);
            }
            writeShellCommand(grid.comlab, grid.command, array);
            writeCloseTag("item", array);

        } else {
            // write text as label
            if(grid.widgetText.size() < 1) {
                writeLabel("&lt;html&gt;&lt;head/&gt;&lt;body&gt;&amp;nbsp;&amp;nbsp;&lt;/body&gt;&lt;/html&gt;", "10", "20",  "16777215", "20", fontSize,
                           "Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter", "", "", "",  "", false, fg, bg, array);
            } else {
                writeLabel(grid.widgetText, "", "20", "16777215", "20", fontSize, "Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter", "", "", "", "", false, fg, bg, array);
            }
            writeCloseTag("item", array);
        }

        //////////////////////////////////////////////////////////////////////////////////
    } else  if (grid.widgetType.contains("wheelswitch")){

        PRINT(printf("create wheelswitch row=%d column=%d textPresent=%d textsize=%d\n", actualgridRow, effectiveColumn, grid.textPresent,  grid.widgetText.size()));

        writeItemRowCol(actualgridRow, effectiveColumn, spanGrid, array);

        if(!grid.textPresent || grid.widgetText.size() != 0) {
            if(!grid.textPresent) {
               writeLabel(grid.widgetChannel, "", lineHeight, "", "", fontSize, "Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter", "", "", "", "", true, fg, bg, array);
            } else {
               writeLabel(grid.widgetText, "", lineHeight, "", "", fontSize, "Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter", "", "", "", "", true, fg, bg, array);
            }
            writeCloseTag("item", array);
            // write now the wheelswitch
            writeItemRowCol(actualgridRow, effectiveColumn, 1, array);
        }
        writeWheelswitch(grid.formats[0], grid.widgetChannel, array);
        writeCloseTag("item", array);
        //////////////////////////////////////////////////////////////////////////////////
    } else  if (grid.widgetType.contains("binary")){

        PRINT(printf("create binary row=%d column=%d textPresent=%d textsize=%d\n", actualgridRow, effectiveColumn, grid.textPresent,  grid.widgetText.size()));

        writeItemRowCol(actualgridRow, effectiveColumn, spanGrid, array);

        if(!grid.textPresent || grid.widgetText.size() != 0) {
            if(!grid.textPresent) {
               writeLabel(grid.widgetChannel, "", lineHeight, "", "", fontSize, "Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter", "", "", "", "", true, fg, bg, array);
            } else {
               writeLabel(grid.widgetText, "", lineHeight, "", "", fontSize, "Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter", "", "", "", "", true, fg, bg, array);
            }
            writeCloseTag("item", array);
            // write now the next stuff
            writeItemRowCol(actualgridRow, effectiveColumn, 1, array);
        }
        writeLineEdit(grid.formats[0], grid.widgetChannel, "100", lineHeight, "", "", fontSize, "", "", "", "", "", rgba, array);
        writeCloseTag("item", array);
        writeItemRowCol(actualgridRow, effectiveColumn, 1, array);
        writeTogglebutton(grid.widgetChannel, array);
        writeCloseTag("item", array);

        //////////////////////////////////////////////////////////////////////////////////
    } else  if (grid.widgetType.contains("setrdbk")) {

        PRINT(printf("create setrdbk gridrow=%d gridcolumn=%d actual column=%d span=%d nbGridCols=%d\n", actualgridRow, actualgridColumn, firstCols[actualgridColumn], spanGrid, nbColumns));

        count = 0;
        getColumnPositions(7, actualgridColumn, spanGrid, pos, span);

        effectiveColumn = pos[count++];
        writeItemRowCol(actualgridRow, effectiveColumn, 1, array);

        // 1. write first the label
        writeLabel(grid.widgetChannel, "", lineHeight, "16777215", lineHeight, fontSize, "Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter", "", "", "", "",  true, fg, bg, array);
        writeCloseTag("item", array);

        // 2. write now the wheelswitch
        effectiveColumn = pos[count++];
        writeItemRowCol(actualgridRow, effectiveColumn, 1, array);
        writeWheelswitch(grid.formats[0], grid.widgetChannel, array);
        writeCloseTag("item", array);

        // 3. write now the two labels with equal and unequal sign, but first hard code the new pv
        newpv = partialpv;
        newpv.append(":I-COMP");
        effectiveColumn = pos[count++];
        writeItemRowCol(actualgridRow, effectiveColumn, 1, array);

        writeOpenTag("layout class=\"QVBoxLayout\" name=\"verticalLayout\"><item", array);
        writeLabel("&lt;html&gt;&lt;head/&gt;&lt;body&gt;&amp;#61;&amp;nbsp;&lt;/body&gt;&lt;/html&gt;", "20", "20", "20", "20",
                   "16", "Qt::AlignLeading|Qt::AlignCenter|Qt::AlignVCenter", "caLabel::Alarm", newpv, "A=0", "caLabel::Calc", true, fg, bg, array);
        writeCloseTag("item", array);
        writeOpenTag("item", array);
        writeLabel("&lt;html&gt;&lt;head/&gt;&lt;body&gt;&amp;ne;&amp;nbsp;&lt;/body&gt;&lt;/html&gt;",  "20", "20", "20", "20",
                   "16", "Qt::AlignLeading|Qt::AlignCenter|Qt::AlignVCenter", "caLabel::Alarm", newpv, "A>0", "caLabel::Calc", true, fg, bg, array);
        writeCloseTag("item", array);
        writeCloseTag("layout", array);
        writeCloseTag("item", array);

        // 5. write now the lineedit, but first hard code the new pv
        newpv = partialpv;
        newpv.append(":I-READ");
        effectiveColumn = pos[count++];
        writeItemRowCol(actualgridRow, effectiveColumn, 1, array);
        rgba[3] = 255;
        writeLineEdit(grid.formats[0], newpv, "100", lineHeight, "16777215", lineHeight, fontSize, "", "", "", "", "", rgba, array);
        writeCloseTag("item", array);

        // 6. write now the label for cycle direction, but first hardcode the new pv
        newpv = partialpv;
        newpv.append(":HYS-CYDIR");
        rgba[3] = 0;
        effectiveColumn = pos[count++];
        writeItemRowCol(actualgridRow, effectiveColumn, 1, array);
        writeLineEdit(grid.formats[0], newpv, "30", "30","30", "30", fontSize, "", "", "", "", "", rgba, array);
        writeCloseTag("item", array);

        // 7. write now the checkbox, but first hard code the new pv
        newpv = partialpv;
        newpv.append(":ONOFF");
        effectiveColumn = pos[count++];
        writeItemRowCol(actualgridRow, effectiveColumn, 1, array);
        writeChoice(newpv, array);
        writeOpenTag("widget class=\"caToggleButton\" name=\"catogglebutton\"", array);
        writeSimpleProperty("channel", "string", newpv, array);
        writeOpenProperty("minimumSize", array);
        writeOpenTag("size", array);
        writeTaggedString("height", "24", array);
        writeTaggedString("width", "65", array);
        writeCloseTag("size", array);
        writeCloseProperty(array);
        writeOpenProperty("maximumSize", array);
        writeOpenTag("size", array);
        writeTaggedString("width", "16777215", array);
        writeTaggedString("height", "24", array);
        writeCloseTag("size", array);
        writeCloseProperty(array);
        writeCloseTag("widget", array);
        writeCloseTag("item", array);

        // 8. write now the mode, but first hard code the new pv
        newpv = partialpv;
        newpv.append(":PS-MODE");

        effectiveColumn = pos[count++];
        writeItemRowCol(actualgridRow, effectiveColumn, 1, array);
        rgba[3] = 255;
        writeLineEdit(grid.formats[0], newpv, "100", lineHeight, "", "", fontSize, "", "", "", "", "", rgba, array);
        writeCloseTag("item", array);

        //////////////////////////////////////////////////////////////////////////////////
    } else  if (grid.widgetType.contains("formread")){

        PRINT(printf("create formread row=%d column=%d\n", actualgridRow, effectiveColumn));

        writeItemRowCol(actualgridRow, effectiveColumn, spanGrid, array);

        if(!grid.textPresent || grid.widgetText.size() != 0) {
            if(!grid.textPresent) {
               writeLabel(grid.widgetChannel, "", lineHeight, "", "", fontSize, "Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter", "", "", "", "", true, fg, bg, array);
            } else {
               writeLabel(grid.widgetText, "", lineHeight, "", "", fontSize, "Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter", "", "", "", "", true, fg, bg, array);
            }
            writeCloseTag("item", array);
            // write now the lineedit
            writeItemRowCol(actualgridRow, effectiveColumn, 1, array);
        }

        // write now the lineedit
        rgba[3] = 255;
        writeLineEdit(grid.formats[0], grid.widgetChannel, "100", lineHeight, "", "", fontSize, "", "", "", "", "", rgba, array);
        writeCloseTag("item", array);

        if(grid.command.size() > 0) {
            writeItemRowCol(actualgridRow, effectiveColumn, 1, array);
            writeShellCommand(grid.comlab, grid.command, array);
            writeCloseTag("item", array);
        }

    } else {
        printf("%s not treated\n", qasc(grid.widgetType));
    }
}

void ParsePepFile::writeWheelswitch(QString format, QString pv, QByteArray *array)
{
    bool ok;
    int totalDigits = 8, decimalDigits = 3;
    int integerDigits = totalDigits - decimalDigits -2;
    QStringList elements= format.split(".",  QString::SkipEmptyParts);
    if(elements.count() == 2) {
       totalDigits = elements[0].toInt(&ok);
       if(!ok) totalDigits = 8;
       decimalDigits = elements[1].toInt(&ok);
       if(!ok) decimalDigits = 3;
       integerDigits = totalDigits - decimalDigits -2;
    }
    QString Digits1= QString("%1").arg(integerDigits);
    QString Digits2= QString("%1").arg(decimalDigits);

    writeOpenTag("widget class=\"caSpinbox\" name=\"caspinbox\"", array);
    writeSimpleProperty("channel", "string", pv, array);
    setColor("background", 150, 245, 120, 255, array);
    writeOpenProperty("minimumSize", array);
    writeOpenTag("size", array);
    writeTaggedString("width", "100", array);
    writeTaggedString("height", "25", array);
    writeCloseTag("size", array);
    writeCloseProperty(array);
    writeOpenProperty("maximumSize", array);
    writeOpenTag("size", array);
    writeTaggedString("width", "150", array);
    writeTaggedString("height", "25", array);
    writeCloseTag("size", array);
    writeCloseProperty(array);
    writeSimpleProperty("integerDigits", "number", Digits1, array);
    writeSimpleProperty("decimalDigits", "number", Digits2, array);
    writeSimpleProperty("fixedFormat", "bool", "true", array);
    writeCloseTag("widget", array);
}

void ParsePepFile::writeMenu(QString pv, QByteArray *array)
{
    writeOpenTag("widget class=\"caMenu\" name=\"caMenu\"", array);
    writeSimpleProperty("channel", "string", pv, array);
    writeOpenProperty("minimumSize", array);
    writeOpenTag("size", array);
    writeTaggedString("height", "24", array);
    writeTaggedString("width", "120", array);
    writeCloseTag("size", array);
    writeCloseProperty(array);
    writeOpenProperty("maximumSize", array);
    writeOpenTag("size", array);
    writeTaggedString("width", "16777215", array);
    writeTaggedString("height", "24", array);
    writeCloseTag("size", array);
    writeCloseProperty(array);
    writeCloseTag("widget", array);
}

void ParsePepFile::writeChoice(QString pv, QByteArray *array)
{
    writeOpenTag("widget class=\"caChoice\" name=\"cachoice\"", array);
    writeSimpleProperty("channel", "string", pv, array);
    writeOpenProperty("minimumSize", array);
    writeOpenTag("size", array);
    writeTaggedString("height", "24", array);
    writeTaggedString("width", "120", array);
    writeCloseTag("size", array);
    writeCloseProperty(array);
    writeOpenProperty("maximumSize", array);
    writeOpenTag("size", array);
    writeTaggedString("width", "16777215", array);
    writeTaggedString("height", "24", array);
    writeCloseTag("size", array);
    writeCloseProperty(array);
    writeSimpleProperty("stackingMode", "enum", "caChoice::Column", array);
    writeSimpleProperty("colorMode", "enum", "caChoice::Alarm", array);
    setColor("bordercolor", 0, 0, 0, 255, array);
    writeCloseTag("widget", array);
}

void ParsePepFile::writeTogglebutton(QString pv, QByteArray *array)
{
    writeOpenTag("widget class=\"caToggleButton\" name=\"catogglebutton\"", array);
    writeSimpleProperty("channel", "string", pv, array);
    writeOpenProperty("minimumSize", array);
    writeOpenTag("size", array);
    writeTaggedString("height", "24", array);
    writeTaggedString("width", "65", array);
    writeCloseTag("size", array);
    writeCloseProperty(array);
    writeOpenProperty("maximumSize", array);
    writeOpenTag("size", array);
    writeTaggedString("width", "16777215", array);
    writeTaggedString("height", "24", array);
    writeCloseTag("size", array);
    writeCloseProperty(array);

    writeSimpleProperty("text", "string", "toggle", array);
    writeSimpleProperty("colorMode", "enum", "caToggleButton::Alarm", array);

    writeCloseTag("widget", array);
}

void ParsePepFile::writeShellCommand(QString label, QString command, QByteArray *array)
{
    QString newCommand = "";
    newCommand.append(command);
    newCommand.append("");
    writeOpenTag("widget class=\"caShellCommand\" name=\"cashellcommand\"", array);

    writeSimpleProperty("label", "string", label, array);
    writeSimpleProperty("labels", "string", "", array);
    writeSimpleProperty("files", "string", "", array);

    writeSimpleProperty("args", "string", newCommand, array);
    writeOpenProperty("minimumSize", array);
    writeOpenTag("size", array);
    writeTaggedString("height", "24", array);
    int width = 10 * (label.size() + 1);
    writeTaggedString("width",QString::number(width) , array);
    writeCloseTag("size", array);
    writeCloseProperty(array);
    writeOpenProperty("maximumSize", array);
    writeOpenTag("size", array);
    writeTaggedString("height", "24", array);
    writeTaggedString("width", "16777215", array);
    writeCloseTag("size", array);
    writeCloseProperty(array);

    writeCloseTag("widget", array);
}

void ParsePepFile::writeLineEdit(QString format, QString pv, QString minwidth, QString minheight, QString maxwidth, QString maxheight,
                                 QString pointsize, QString alignment, QString colormode, QString calcpv, QString calc, QString visibility,
                                 int rgba[], QByteArray *array)
{
    Q_UNUSED(alignment);
    Q_UNUSED(colormode);
    Q_UNUSED(calcpv);
    Q_UNUSED(calc);
    Q_UNUSED(visibility);
    Q_UNUSED(rgba);

    bool ok = true;
    int decimalDigits = 3;
    QString newFormat = format;

    // a lineedit
    writeOpenTag("widget class=\"caLineEdit\" name=\"calinedit\"", array);

    if(newFormat.size() != 0) {
        // take care of format
        if(newFormat.at(0) == '%') newFormat.remove(0,1);

        if(newFormat.contains("g") || newFormat.contains("e") || newFormat.contains("f")) {

            if(newFormat.contains("f")) {
                writeSimpleProperty("formatType", "enum", "caLineEdit::decimal", array);
            } else {
                writeSimpleProperty("formatType", "enum", "caLineEdit::exponential", array);
            }
            newFormat.replace("g", "");
            newFormat.replace("e", "");
            newFormat.replace("f", "");
            QStringList elements= newFormat.split(".",  QString::SkipEmptyParts);
            if(elements.count() == 2) {
                decimalDigits = elements[1].toInt(&ok);
                if(ok) {
                    QString Digits= QString("%1").arg(decimalDigits);
                    writeSimpleProperty("precisionMode", "enum", "caLineEdit::User", array);
                    writeSimpleProperty("precision", "number", Digits, array);
                }
            }

        } else if(newFormat.contains("x")) {
            writeSimpleProperty("formatType", "enum", "caLineEdit::hexadecimal", array);
        } else if(newFormat.contains("o")) {
            writeSimpleProperty("formatType", "enum", "caLineEdit::octal", array);
        } else {
            QStringList elements= newFormat.split(".",  QString::SkipEmptyParts);
            if(elements.count() == 2) {
                decimalDigits = elements[1].toInt(&ok);
                if(ok) {
                    QString Digits= QString("%1").arg(decimalDigits);
                    writeSimpleProperty("precisionMode", "enum", "caLineEdit::User", array);
                    writeSimpleProperty("precision", "number", Digits, array);
                }
            }
        }
    }

    //size
    if(minwidth.size() > 0 || minheight.size() > 0) {
        writeOpenProperty("minimumSize", array);
        writeOpenTag("size", array);
        if(minwidth.size() > 0) writeTaggedString("width", minwidth, array);
        if(minheight.size() > 0) writeTaggedString("height", minheight, array);
        writeCloseTag("size", array);
        writeCloseProperty(array);
    }
    if(maxwidth.size() > 0 || maxheight.size() > 0) {
        writeOpenProperty("maximumSize", array);
        writeOpenTag("size", array);
        if(maxwidth.size() > 0) writeTaggedString("width", maxwidth, array);
        if(maxheight.size() > 0) writeTaggedString("height", maxheight, array);
        writeCloseTag("size", array);
        writeCloseProperty(array);
    }

    // aligbment and colors
    writeSimpleProperty("alignment", "set", "Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter", array);
    writeSimpleProperty("channel", "string", pv, array);
    if(!willPrint) {
       writeSimpleProperty("colorMode", "enum", "caLineEdit::Alarm_Default", array);
       writeSimpleProperty("alarmHandling", "enum", "caLineEdit::onBackground", array);
    } else {
       setColor("background", 255, 255, 255, 0, array);
    }

    writeSimpleProperty("unitsEnabled", "bool", "true", array);

    // font and font size
    writeOpenProperty("font", array);
    writeOpenTag("font", array);
    writeTaggedString("family", "Lucida Sans Typewriter", array);
    writeTaggedString("pointsize", pointsize, array);
    writeCloseTag("font", array);
    writeCloseProperty(array);

    writeCloseTag("widget", array);
}

void ParsePepFile::writeTextEntry(QString format, QString pv, QString minwidth, QString minheight, QString maxwidth, QString maxheight,
                                  QString pointsize, QString alignment, QString colormode, QString calcpv, QString calc, QString visibility,
                                  int rgba[], QByteArray *array)
{
    Q_UNUSED(format);
    Q_UNUSED(alignment);
    Q_UNUSED(colormode);
    Q_UNUSED(calcpv);
    Q_UNUSED(calc);
    Q_UNUSED(visibility);

    // a lineedit
    writeOpenTag("widget class=\"caTextEntry\" name=\"catextentry\"", array);

    if(minwidth.size() > 0 || minheight.size() > 0) {
        writeOpenProperty("minimumSize", array);
        writeOpenTag("size", array);
        if(minwidth.size() > 0) writeTaggedString("width", minwidth, array);
        if(minheight.size() > 0) writeTaggedString("height", minheight, array);
        writeCloseTag("size", array);
        writeCloseProperty(array);
    }
    if(maxwidth.size() > 0 || maxheight.size() > 0) {
        writeOpenProperty("maximumSize", array);
        writeOpenTag("size", array);
        if(maxwidth.size() > 0) writeTaggedString("width", maxwidth, array);
        if(maxheight.size() > 0) writeTaggedString("height", maxheight, array);
        writeCloseTag("size", array);
        writeCloseProperty(array);
    }

    writeSimpleProperty("alignment", "set", "Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter", array);
    writeSimpleProperty("channel", "string", pv, array);
    writeSimpleProperty("colorMode", "enum", "caLineEdit::Static", array);

    setColor("background", rgba[0], rgba[1], rgba[2], rgba[3], array);
    setColor("foreground", 255, 255, 255, 255, array);

    writeOpenProperty("font", array);
    writeOpenTag("font", array);
    writeTaggedString("family", "Lucida Sans Typewriter", array);
    writeTaggedString("pointsize", pointsize, array);
    writeCloseTag("font", array);
    writeCloseProperty(array);

    writeCloseTag("widget", array);
}

void ParsePepFile::writeLabel(QString text, QString minwidth, QString minheight, QString maxwidth, QString maxheight, QString pointsize,
                              QString alignment, QString colormode, QString calcpv, QString calc, QString visibility, bool transparent,
                              QColor fg, QColor bg, QByteArray *array)
{
    writeOpenTag("widget class=\"caLabel\" name=\"calabel\"", array);

    QFont font( "Lucida Sans Typewriter", pointsize.toInt());
    QFontMetrics metrics(font);
    int width = metrics.width(text);
    if( (minwidth.size() == 0) && (minheight.size() > 0) ) {
        writeOpenProperty("minimumSize", array);
        writeOpenTag("size", array);
        writeTaggedString("width", QString::number(width), array);
        writeTaggedString("height", minheight, array);
        writeCloseTag("size", array);
        writeCloseProperty(array);
    } else
        if(minwidth.size() > 0 || minheight.size() > 0) {
        writeOpenProperty("minimumSize", array);
        writeOpenTag("size", array);
        if(minwidth.size() > 0) writeTaggedString("width", minwidth, array);
        if(minheight.size() > 0) writeTaggedString("height", minheight, array);
        writeCloseTag("size", array);
        writeCloseProperty(array);
    }
    if(maxwidth.size() > 0 || maxheight.size() > 0) {
        writeOpenProperty("maximumSize", array);
        writeOpenTag("size", array);
        if(maxwidth.size() > 0) writeTaggedString("width", maxwidth, array);
        if(maxheight.size() > 0) writeTaggedString("height", maxheight, array);
        writeCloseTag("size", array);
        writeCloseProperty(array);
    }

    writeSimpleProperty("text", "string",  qasc(text), array);
    writeSimpleProperty("alignment", "set", alignment, array);

    if(transparent) setColor("background", 200, 200, 200, 0, array);
    else setColor("background", bg.red(), bg.green(), bg.blue(), 255, array);

    setColor("foreground", fg.red(), fg.green(), fg.blue(), 255, array);

    writeOpenProperty("font", array);
    writeOpenTag("font", array);
    writeTaggedString("family", "Lucida Sans Typewriter", array);
    writeTaggedString("pointsize", pointsize, array);
    writeCloseTag("font", array);
    writeCloseProperty(array);

    if(colormode.size() > 0) {
        writeSimpleProperty("colorMode", "enum", colormode, array);
    }

     writeSimpleProperty("fontScaleMode", "enum", "None", array);


    if(calc.size() > 0 && visibility.size() > 0 && calcpv.size() > 0) {
        writeSimpleProperty("visibility", "enum", visibility, array);
        writeSimpleProperty("visibilityCalc", "string", calc, array);
        writeSimpleProperty("channel", "string", calcpv, array);
    }

    writeCloseTag("widget", array);
}

void ParsePepFile::writeOpenProperty(QString property, QByteArray *array)
{
    QString aux = QString("<property name=\"%1\">\n").arg( property);
    array->append(aux);
}

void ParsePepFile::writeCloseProperty(QByteArray *array)
{
    QString aux = QString("</property>\n");
    array->append(aux);
}

void ParsePepFile::writeTaggedString(QString tag, QString value, QByteArray *array)
{
    QString aux = QString("<%1>%2</%3>\n").arg(tag).arg(value).arg(tag);
    array->append(aux);
}

void ParsePepFile::setColor(QString property, int r, int g, int b, int alpha, QByteArray *array)
{
    writeOpenProperty(property, array);

    QString alphaC = QString("color alpha=\"%1\"").arg(alpha);
    QString red= QString("%1").arg(r);
    QString green= QString("%1").arg(g);
    QString blue= QString("%1").arg(b);

    writeOpenTag(alphaC, array);
    writeTaggedString("red", red, array);
    writeTaggedString("green", green, array);
    writeTaggedString("blue", blue, array);

    writeCloseTag("color", array);
    writeCloseProperty(array);
}

void ParsePepFile::setGeometry(int x, int y, int width, int height, QByteArray *array)
{
    writeOpenProperty("geometry", array);

    QString axisX= QString("%1").arg(x);
    QString axisY= QString("%1").arg(y);
    QString w= QString("%1").arg(width);
    QString h= QString("%1").arg(height);

    writeOpenTag("rect", array);
    writeTaggedString("x", axisX, array);
    writeTaggedString("y", axisY, array);
    writeTaggedString("width", w, array);
    writeTaggedString("height", h, array);
    writeCloseTag("rect", array);
    writeCloseProperty(array);
}

void ParsePepFile::writeItemRowCol(int &row, int &column,  int span, QByteArray *array)
{
    QString Qrow, Qcolumn, Qspan;
    QString aux = QString("<item row=\"%1\" column=\"%2\" colspan=\"%3\">\n").arg(Qrow.setNum(row)).arg(Qcolumn.setNum(column)).arg(Qspan.setNum(span));
    array->append(aux);
    column++;
}

void ParsePepFile::writeOpenTag(QString tag,  QByteArray *array)
{
    QString aux = QString("<%1>\n").arg(tag);
    array->append(aux);
}

void ParsePepFile::writeCloseTag(QString tag,  QByteArray *array)
{
    QString aux = QString("</%1>\n").arg(tag);
    array->append(aux);
}

void ParsePepFile::writeSimpleProperty(QString prop, QString tag, QString value, QByteArray *array)
{
    writeOpenProperty(prop, array);
    writeTaggedString(tag, value, array);
    writeCloseProperty(array);
}

QWidget* ParsePepFile::load(QWidget *parent)
{
    QWidget *widget; // = new QWidget;
    QUiLoader loader;

/* used to output the data to an ui file for verification*/
    PRINT(
    QFile file("out.ui");
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    )

    buffer->open(QIODevice::ReadOnly);

/* output the data to the verification file */

    PRINT(
    out << buffer->data();
    file.close();
    )

    buffer->seek(0);
    widget=loader.load(buffer, parent);
    buffer->close();
    delete buffer;
    return widget;
}




