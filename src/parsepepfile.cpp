#include <QtGui>
#include <math.h>
#include "parsepepfile.h"

ParsePepFile::ParsePepFile(QString filename)
{
    char header[2000];
    char footer[100];
    QFile *file = new QFile;
    int nbRows, nbCols;

    for(int i=0; i< 100; i++) {
        for (int j=0; j<20; j++) {
            gridLayout[i][j].widgetType = "";
            gridLayout[i][j].widgetText = "";
            gridLayout[i][j].widgetChannel = "";
            gridLayout[i][j].comlab = "";
            gridLayout[i][j].command = "";
            gridLayout[i][j].nbElem = 0;
            gridLayout[i][j].span = 0;
        }
    }
    for (int j=0; j<20; j++) {
        firstCols[j] = 0;
        maxCols[j] = 0;
    }

    // define header and footer for this display
    strcpy(header, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
           "<ui version=\"4.0\">\n"
           "<class>MainWindow</class>\n"
           "<widget class=\"QMainWindow\" name=\"MainWindow\">\n"
           "<property name=\"styleSheet\">\n"
           "<string>\n"
           "QWidget#centralWidget {background: rgba(200, 200, 200, 0); }\n"
           "caLineEdit {border-radius: 1px;background: lightyellow;color: black;}\n"
           //"caLabel { border:1px solid rgb(0, 0, 255); }\n"
           "</string>\n"
           "</property>\n"
           "<widget class=\"QWidget\" name=\"centralWidget\">\n"
           "<layout class=\"QGridLayout\" name=\"gridLayout\">\n"
           "<item row=\"0\" column=\"0\">\n"
           "<layout class=\"QGridLayout\" name=\"gridLayout\">\n"
           " <property name=\"spacing\">\n"
           "<number>5</number>\n"
           "</property>\n");


    strcpy(footer, "</layout></item></layout></widget></widget></ui>");

    QByteArray headerBytes(header);
    QByteArray footerBytes(footer);
    QByteArray *array= new QByteArray();
    array->append(headerBytes);

    buffer = new QBuffer();

    file->setFileName(filename);

    Initialize();

    TreatFile(nbRows, nbCols, file);

    printf("nbRows=%d nbCols=%d\n", nbRows, nbCols);

    DisplayFile(nbRows, nbCols, array);

    array->append(footerBytes);
    buffer->open(QIODevice::ReadWrite);
    buffer->write(*array);
    buffer->close();

    delete file;
}

void ParsePepFile::Initialize()
{
    mapStringValues["comment"] = evComment;
    mapStringValues["separator"] = evSeparator;
    mapStringValues["end"] = evEnd;
}

void ParsePepFile::TreatFile(int &nbRows, int &nbCols, QFile *file)
{
    int grid = 1;
    bool textPresent = false;
    int ll;
    int span =0;
    int nbFormats = 0;
    int actualLine = 0;
    int actualColumn = 0;
    int lastColumn = 0;
    QString formats[2];
    QString widgetType ="";
    QString widgetText ="";
    QString channel ="";
    QString comment="";

    if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) return ;
    // scan file
    QTextStream in(file);
    QString line = in.readLine();
    while (!line.isNull()) {

        // replace all blancs (except these between quotes by ;
        bool inside = false;
        for (int i = 0; i < line.size(); i++) {
            if((!inside) && ((line.at(i) == QChar('\'')) || (line.at(i) == QChar('\"')) || (line.at(i) == QChar('{')) )) {
                inside = true;
            } else if((inside) && ((line.at(i) == QChar('\'')) || (line.at(i) == QChar('\"')) || (line.at(i) == QChar('}')))) {
                inside = false;
            }
            if (!inside) {
                if(line.at(i) == QChar(' ')) {
                    line.replace(i, 1, ";");
                }
            }
        }

        QStringList elements= line.split(";",  QString::SkipEmptyParts);
/*
        printf("%d %s\n", elements.count(), line.toAscii().constData());
        for (int i=0; i< elements.count(); i++) {
            printf("<%s>\n",  elements.at(i).toAscii().constData());
        }
*/
        // empty line ?
        if(line.size() == 0) {

        }
        // we have a grid definition at the beginning of the file
        else if(line.contains("#!grid")) {
            grid = elements.at(1).toInt();
            //printf("grid=%d\n", grid);
        } else if(line.at(0) != QChar('#')){

            if(elements.count() > 1) {

                channel = elements.at(0);
                if(elements.count() > 1) {
                    widgetType = elements.at(1);
                    widgetType =  widgetType.toLower();
                }

                ll = 2;
                widgetText = "";
                formats[0] = formats[1] = "";
                span = 1;
                nbFormats = 0;


                while (ll < elements.count()) {
                    bool ok;
                    float number = elements.at(ll).toFloat(&ok);

                    if(elements.at(ll).contains("-span")) {
                        ll++;
                        if(ll < elements.count()) {
                            //printf("span detected, value=%s\n", elements.at(ll).toAscii().constData());
                            span = elements.at(ll).toInt();
                        }
                    }
                    else if(elements.at(ll).contains("-confirm")) {
                        //printf("confirm detected\n");
                    }

                    else if(elements.at(ll).contains("-hys")) {
                        //printf("hys detected\n");
                    }

                    else if(elements.at(ll).contains("-comlab")) {
                        ll++;
                        //printf("comlab detected <%s>\n", elements.at(ll).toAscii().constData() );
                        gridLayout[actualLine][actualColumn].comlab = elements.at(ll).toAscii().constData();
                    }

                    else if(elements.at(ll).contains("-command")) {
                        ll++;
                        //printf("commmand detected <%s>\n", elements.at(ll).toAscii().constData() );
                        gridLayout[actualLine][actualColumn].command = elements.at(ll).toAscii().constData();
                    }

                    else if(elements.at(ll).contains("-text")) {
                        ll++;
                        //printf("text detected <%s>\n", elements.at(ll).toAscii().constData());
                        widgetText = elements.at(ll);
                        gridLayout[actualLine][actualColumn].textPresent = true;
                     }

                    else if(elements.at(ll).contains("-fg")) {
                        ll++;
                        //printf("fg detected <%s>\n", elements.at(ll).toAscii().constData());

                    } else if(elements.at(ll-1).contains("comment")) {
                        //printf("comment  detected %s\n", elements.at(ll).toAscii().constData());
                        widgetText = elements.at(ll);
                    }

                    else {
                        printf("something else  detected %s\n", elements.at(ll).toAscii().constData());
                        widgetText.append(elements.at(ll));
                        widgetText.append(" ");
                    }

                    if(ok && (widgetType.contains("setrdbk") ||
                                     widgetType.contains("wheelswitch") ||
                                     widgetType.contains("formread")) ) {
                        //printf("format detected %s\n", elements.at(ll).toAscii().constData());
                        formats[nbFormats++] =elements.at(ll);
                    }
                    ll++;
                }

                widgetType = elements.at(1);
                widgetType = widgetType.toLower();


                gridLayout[actualLine][actualColumn].span = span;
                gridLayout[actualLine][actualColumn].widgetType = widgetType;
                gridLayout[actualLine][actualColumn].widgetText = widgetText;
                gridLayout[actualLine][actualColumn].widgetChannel = channel;
                gridLayout[actualLine][actualColumn].formats[0] = formats[0];
                gridLayout[actualLine][actualColumn].formats[1] = formats[1];

                //printf("%d %d %d <%s>\n", actualLine, actualColumn,  span, widgetType.toAscii().constData());


                if(widgetType.contains("led")) gridLayout[actualLine][actualColumn].nbElem = 1;
                else if(widgetType.contains("formread")) gridLayout[actualLine][actualColumn].nbElem = 2;
                else if(widgetType.contains("wheelswitch")) {
                    if(gridLayout[actualLine][actualColumn].textPresent && gridLayout[actualLine][actualColumn].widgetText.size() == 0) {
                       gridLayout[actualLine][actualColumn].nbElem = 1;
                    } else {
                       gridLayout[actualLine][actualColumn].nbElem = 2;
                    }
                }
                else if(widgetType.contains("choicebutton")) gridLayout[actualLine][actualColumn].nbElem = 2;
                else if(widgetType.contains("text")) gridLayout[actualLine][actualColumn].nbElem = 4;
                else if(widgetType.contains("setrdbk")) gridLayout[actualLine][actualColumn].nbElem = 7;
                else if(widgetType.contains("comment")) {
                    gridLayout[actualLine][actualColumn].nbElem = 1;
                    if(gridLayout[actualLine][actualColumn].command.size() > 0) {
                        if(gridLayout[actualLine][actualColumn].widgetText.size() > 0)  gridLayout[actualLine][actualColumn].nbElem++;
                    }
                }
                else if(widgetType.contains("separator")) gridLayout[actualLine][actualColumn].nbElem = 1;
                else if(widgetType.contains("menubutton")) gridLayout[actualLine][actualColumn].nbElem = 2;
                else gridLayout[actualLine][actualColumn].nbElem = 1;

                actualColumn += span;
                if(actualColumn > grid-1) {
                    actualColumn = 0;
                    lastColumn = 0;
                    actualLine++;
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
                printf("row=%d col=%d %20s text=%s nbelem=%d span=%d\n", i, j, gridLayout[i][j].widgetType.toAscii().constData(),
                       gridLayout[i][j].widgetText.toAscii().constData(), gridLayout[i][j].nbElem, gridLayout[i][j].span);
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

    for(int j=0; j<nbCols; j++) printf("%d ", maxCols[j]);
    printf("\n");


     for (int j=0; j<nbCols; j++) {
         firstCols[j] = 0;
         for(int k=0; k<j; k++) firstCols[j] += maxCols[k];
     }


    for(int i=0; i< nbRows; i++) {
        for (int j=0; j<nbCols; j++) {

            int spanColumns = 1;
/*
            printf("===== %d %d %20s text=%10s span=%d col=%d\n", i, j,  gridLayout[i][j].widgetType.toAscii().constData(),
                   gridLayout[i][j].widgetText.toAscii().constData(),
                   gridLayout[i][j].span, col);
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
    int sum[20];

    for(int i=0; i<20; i++) {
        span[i] = 0;
    }

    // in case only one item has to put into a row with a span > 1 we have to calculate the internal colum position
    // and number of intercolumns to span
    if(nbItems == 1) {
        printf("========= one item to put with span=%d\n", spanGrid);
        pos[0] = firstCols[actualGridColumn];
        span[0] = 0;
        if(spanGrid > 1) {
            for(int k=0; k< spanGrid; k++) {
                span[0] = span[0] + maxCols[k+actualGridColumn];
            }
        }
        for(int i=0; i< nbItems; i++) {
            if(span[i] < 1) span[i] = 1;
            printf("item=%d at column=%d span=%d\n", i, pos[i], span[i]);
        }
        return;
    }

    // more than one item with span > 1, calculate what we can put in the different columns

    printf("========= more than one item nbitems=%d with span=%d\n", nbItems, spanGrid);

    sum[0] = nbItems;
    for(int i=1; i< spanGrid; i++) {
        sum[i] = maxCols[actualGridColumn+i];
        sum[0] -= sum[i];
        // nothing left in this column
        if(sum[0] <= 0) {
            sum[0] = 1;
            sum[i]=nbItems - sum[0];
        }
        if(sum[0] == 1) break;
    }

    for(int i=0; i< spanGrid; i++) {
        span[i] = span[i] + maxCols[actualGridColumn+i];
        printf("items in this grid %d nb=%d\n", i, sum[i]);
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
        printf("item=%d at column=%d spancol=%d\n", i, pos[i], span[i]);
    }
}

void ParsePepFile::replaceStrings(gridInfo &grid)
{
    grid.widgetText.replace("\"","");
    grid.widgetText.replace("\'","");
    grid.widgetText.replace("&","&amp;");

    grid.command.replace("{","");
    grid.command.replace("}","");
    grid.command.replace("&","&amp;");

    grid.comlab.replace("{","");
    grid.comlab.replace("}","");
    grid.comlab.replace("\"","");
}

void ParsePepFile::displayItem(int actualgridRow,int actualgridColumn, gridInfo grid, int spanGrid, int spanColumns, int nbColumns, QByteArray *array)
{
    QString newpv = "";
    QString partialpv = "";
    int rgba[4];
    int pos[20], span[20], count;

    int effectiveColumn= firstCols[actualgridColumn];
    int effectiveSpan;

    rgba[0] = 150; rgba[1] = 245; rgba[2] = 120; rgba[3] = 255;

    QStringList pvElements= grid.widgetChannel.split(":",  QString::SkipEmptyParts);
    if(pvElements.count() > 0) partialpv = pvElements.at(0);

    if(grid.widgetType.contains("menubutton")) {
        printf("create menubutton row=%d column=%d spangrid=%d\n", actualgridRow, effectiveColumn, spanGrid);

        writeItemRowCol(actualgridRow, effectiveColumn, spanGrid, array);

        // write first the label
        if(grid.widgetText.size() > 0) {
            writeLabel(grid.widgetText, "", "18", "", "", "10", "Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter", "", "", "", "", true, array);
        } else {
            writeLabel(grid.widgetChannel, "", "18", "", "", "10", "Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter", "", "", "", "", true, array);
        }
        writeCloseTag("item", array);

        writeItemRowCol(actualgridRow, effectiveColumn, 1, array);
        writeChoice(grid.widgetChannel, array);
        writeCloseTag("item", array);


        //////////////////////////////////////////////////////////////////////////////////
    } else if(grid.widgetType.contains("led")) {
        printf("create led row=%d column=%d\n", actualgridRow, effectiveColumn);

        writeItemRowCol(actualgridRow, effectiveColumn, spanGrid, array);

        writeOpenTag("widget class=\"caGraphics\" name=\"cagraphics\"", array);

        writeOpenProperty("maximumSize", array);
        writeOpenTag("size", array);
        writeTaggedString("width", "15", array);
        writeTaggedString("height", "15", array);
        writeCloseTag("size", array);
        writeCloseProperty(array);
        writeOpenProperty("minimumSize", array);
        writeOpenTag("size", array);
        writeTaggedString("width", "15", array);
        writeTaggedString("height", "15", array);
        writeCloseTag("size", array);
        writeCloseProperty(array);

        writeSimpleProperty("channel", "string", grid.widgetChannel, array);
        writeSimpleProperty("colorMode", "enum", "caGraphics::Alarm", array);
        writeSimpleProperty("form", "enum", "caGraphics::Circle", array);
        writeSimpleProperty("fillstyle", "enum", "caGraphics::Filled", array);

        writeCloseTag("widget", array);

        writeCloseTag("item", array);

        //////////////////////////////////////////////////////////////////////////////////
    } else if (grid.widgetType.contains("text")) {

        printf("create text row=%d column=%d\n", actualgridRow, effectiveColumn);

        writeItemRowCol(actualgridRow, effectiveColumn, spanGrid, array);

        writeLabel(grid.widgetChannel, "", "18", "", "", "10", "Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter", "", "", "", "", true, array);
        writeCloseTag("item", array);

        // write now the lineedit in next column
        writeItemRowCol(actualgridRow, effectiveColumn, 1, array);
        writeLineEdit(grid.formats[0], grid.widgetChannel, "", "18", "", "", "10", "", "", "", "", "", rgba, array);
        writeCloseTag("item", array);

        writeItemRowCol(actualgridRow, effectiveColumn, 1, array);
        writeLabel("new value", "", "18", "", "", "10", "Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter", "", "", "", "", true, array);
        writeCloseTag("item", array);

        // write now the textentry in next column
        writeItemRowCol(actualgridRow, effectiveColumn, 1, array);
        writeTextEntry(grid.formats[0], grid.widgetChannel, "", "22", "", "", "10", "", "", "", "", "", rgba, array);
        writeCloseTag("item", array);

        //////////////////////////////////////////////////////////////////////////////////
    } else if (grid.widgetType.contains("choicebutton")) {

        printf("create choicebutton row=%d column=%d\n", actualgridRow, effectiveColumn);

        writeItemRowCol(actualgridRow, effectiveColumn, spanGrid, array);

        // write first the label
        writeLabel(grid.widgetChannel, "", "18", "", "", "10", "Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter", "", "", "", "", true, array);
        writeCloseTag("item", array);

        writeItemRowCol(actualgridRow, effectiveColumn, 1, array);
        writeChoice(grid.widgetChannel, array);
        writeCloseTag("item", array);

        //////////////////////////////////////////////////////////////////////////////////
    } else if (grid.widgetType.contains("separator")) {

        printf("create separator over %d grid columns , %d columns\n", spanGrid, spanColumns);

        writeItemRowCol(actualgridRow, effectiveColumn, spanColumns, array);

        writeOpenTag("widget class=\"Line\" name=\"line\"", array);
        writeSimpleProperty("frameShadow", "enum", "QFrame::Plain", array);
        writeSimpleProperty("lineWidth", "number", "2", array);
        writeSimpleProperty("orientation", "enum", "Qt::Horizontal", array);
        writeCloseTag("widget", array);
        writeCloseTag("item", array);

        //////////////////////////////////////////////////////////////////////////////////
    } else if (grid.widgetType.contains("comment")) {

        printf("create comment row=%d column=%d over %d grid columns\n", actualgridRow, effectiveColumn, spanGrid);

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
        //printf("comlab=%s comamnd=%s\n",grid.comlab.toAscii().constData(), grid.command.toAscii().constData());

        if(grid.command.size() > 0) {
            if(grid.widgetText.size() > 0) {
               writeLabel(grid.widgetText, "", "18", "", "", "10", "Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter", "", "", "", "", false, array);
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
                writeLabel("&lt;html&gt;&lt;head/&gt;&lt;body&gt;&amp;nbsp;&amp;nbsp;&lt;/body&gt;&lt;/html&gt;", "", "18", "", "", "10",
                           "Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter", "", "", "",  "", false, array);
            } else {
                writeLabel(grid.widgetText, "", "18", "", "", "10", "Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter", "", "", "", "", false, array);
            }
             writeCloseTag("item", array);
        }

        //////////////////////////////////////////////////////////////////////////////////
    } else  if (grid.widgetType.contains("wheelswitch")){

        printf("create wheelswitch row=%d column=%d\n", actualgridRow, effectiveColumn);

        writeItemRowCol(actualgridRow, effectiveColumn, spanGrid, array);

        if(!grid.textPresent || grid.widgetText.size() != 0) {
           writeLabel(grid.widgetChannel, "150", "18", "", "", "10", "Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter", "", "", "", "", true, array);
           writeCloseTag("item", array);
           // write now the wheelswitch
           writeItemRowCol(actualgridRow, effectiveColumn, 1, array);
        }
        writeWheelswitch(grid.formats[0], grid.widgetChannel, array);
        writeCloseTag("item", array);

        //////////////////////////////////////////////////////////////////////////////////
    } else  if (grid.widgetType.contains("setrdbk")) {

        printf("create setrdbk gridrow=%d gridcolumn=%d actual column=%d span=%d nbGridCols=%d\n", actualgridRow, actualgridColumn, firstCols[actualgridColumn], spanGrid, nbColumns);
        printf("columns width = %d %d %d %d\n", maxCols[actualgridColumn], maxCols[actualgridColumn+1],  maxCols[actualgridColumn+2], maxCols[actualgridColumn+3]);

        count = 0;
        getColumnPositions(7, actualgridColumn, spanGrid, pos, span);

        effectiveColumn = pos[count++];
        writeItemRowCol(actualgridRow, effectiveColumn, 1, array);

        // 1. write first the label
        writeLabel(grid.widgetChannel, "0", "18", "16777215", "18", "10", "Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter", "", "", "", "",  true, array);
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
                   "16", "Qt::AlignLeading|Qt::AlignCenter|Qt::AlignVCenter", "caLabel::Alarm", newpv, "A=0", "caLabel::Calc", true, array);
        writeCloseTag("item", array);
        writeOpenTag("item", array);
        writeLabel("&lt;html&gt;&lt;head/&gt;&lt;body&gt;&amp;ne;&amp;nbsp;&lt;/body&gt;&lt;/html&gt;",  "20", "20", "20", "20",
                   "16", "Qt::AlignLeading|Qt::AlignCenter|Qt::AlignVCenter", "caLabel::Alarm", newpv, "A>0", "caLabel::Calc", true, array);
        writeCloseTag("item", array);
        writeCloseTag("layout", array);
        writeCloseTag("item", array);

        // 5. write now the lineedit, but first hard code the new pv
        newpv = partialpv;
        newpv.append(":I-READ");
        effectiveColumn = pos[count++];
        writeItemRowCol(actualgridRow, effectiveColumn, 1, array);
        rgba[3] = 255;
        writeLineEdit(grid.formats[0], newpv, "100", "18", "16777215", "18", "10", "", "", "", "", "", rgba, array);
        writeCloseTag("item", array);

        // 6. write now the label for cycle direction, but first hardcode the new pv
        newpv = partialpv;
        newpv.append(":HYS-CYDIR");
        rgba[3] = 0;
        effectiveColumn = pos[count++];
        writeItemRowCol(actualgridRow, effectiveColumn, 1, array);
        writeLineEdit(grid.formats[0], newpv, "30", "30","30", "30", "10", "", "", "", "", "", rgba, array);
        writeCloseTag("item", array);

        // 7. write now the led, but first hard code the new pv
        newpv = partialpv;
        newpv.append(":ONOFF");
        effectiveColumn = pos[count++];
        writeItemRowCol(actualgridRow, effectiveColumn, 1, array);
        writeChoice(newpv, array);
        writeCloseTag("item", array);

        // 8. write now the mode, but first hard code the new pv
        newpv = partialpv;
        newpv.append(":PS-MODE");

        effectiveColumn = pos[count++];
        writeItemRowCol(actualgridRow, effectiveColumn, 1, array);
        rgba[3] = 255;
        writeLineEdit(grid.formats[0], newpv, "100", "18", "", "", "10", "", "", "", "", "", rgba, array);
        writeCloseTag("item", array);

        //////////////////////////////////////////////////////////////////////////////////
    } else  if (grid.widgetType.contains("formread")){

        printf("create formread row=%d column=%d\n", actualgridRow, effectiveColumn);

        writeItemRowCol(actualgridRow, effectiveColumn, 1, array);

        // write first the label
        writeLabel(grid.widgetChannel, "", "18", "", "", "10", "Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter", "", "", "", "", true, array);
        writeCloseTag("item", array);

        // write now the lineedit
        writeItemRowCol(actualgridRow, effectiveColumn, 1, array);
        rgba[3] = 255;
        writeLineEdit(grid.formats[0], grid.widgetChannel, "", "18", "", "", "10", "", "", "", "", "", rgba, array);
        writeCloseTag("item", array);

    } else {
        printf("%s not treated\n", grid.widgetType.toAscii().constData());
    }
}

void ParsePepFile::writeWheelswitch(QString format, QString pv, QByteArray *array)
{
    bool ok;

    QStringList elements= format.split(".",  QString::SkipEmptyParts);

    int totalDigits = elements[0].toInt(&ok);
    int decimalDigits = elements[1].toInt(&ok);
    int integerDigits = totalDigits - decimalDigits -2;
    QString Digits1= QString("%1").arg(integerDigits);
    QString Digits2= QString("%1").arg(decimalDigits);

    writeOpenTag("widget class=\"caNumeric\" name=\"canumeric\"", array);
    writeSimpleProperty("channel", "string", pv, array);
    setColor("background", 150, 245, 120, 255, array);
    writeOpenProperty("minimumSize", array);
    writeOpenTag("size", array);
    writeTaggedString("width", "100", array);
    writeTaggedString("height", "45", array);
    writeCloseTag("size", array);
    writeCloseProperty(array);
    writeOpenProperty("maximumSize", array);
    writeOpenTag("size", array);
    writeTaggedString("width", "150", array);
    writeTaggedString("height", "45", array);
    writeCloseTag("size", array);
    writeCloseProperty(array);
    writeSimpleProperty("integerDigits", "number", Digits1, array);
    writeSimpleProperty("decimalDigits", "number", Digits2, array);
    writeSimpleProperty("fixedFormat", "bool", "true", array);
    writeCloseTag("widget", array);
}

void ParsePepFile::writeChoice(QString pv, QByteArray *array)
{
    writeOpenTag("widget class=\"caMenu\" name=\"caMenu\"", array);
    writeSimpleProperty("channel", "string", pv, array);
    writeOpenProperty("minimumSize", array);
    writeOpenTag("size", array);
    writeTaggedString("height", "24", array);
     writeTaggedString("width", "80", array);
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

void ParsePepFile::writeShellCommand(QString label, QString command, QByteArray *array)
{
    writeOpenTag("widget class=\"caShellCommand\" name=\"cashellcommand\"", array);

    writeSimpleProperty("label", "string", label, array);
    writeSimpleProperty("labels", "string", "1", array);
    writeSimpleProperty("files", "string", command, array);
    writeSimpleProperty("args", "string", "", array);
    writeOpenProperty("minimumSize", array);
    writeOpenTag("size", array);
    writeTaggedString("height", "24", array);
    writeTaggedString("width", "120", array);
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
    Q_UNUSED(format);

    // a lineedit
    writeOpenTag("widget class=\"caLineEdit\" name=\"calinedit\"", array);

    if(minwidth.size() > 0 || minheight.size() > 0) {
        writeOpenProperty("minimumSize", array);
        writeOpenTag("size", array);
        if(minwidth.size() > 0) writeTaggedString("width", minwidth, array);
        if(minheight.size() > 0) writeTaggedString("height", minheight, array);
        writeCloseTag("size", array);
        writeCloseProperty(array);
    }
    if(maxwidth.size() > 0 || maxwidth.size() > 0) {
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
    writeSimpleProperty("unitsEnabled", "bool", "true", array);

    setColor("background", rgba[0], rgba[1], rgba[2], rgba[3], array);
    setColor("foreground", 0, 0, 0, 255, array);

    writeOpenProperty("font", array);
    writeOpenTag("font", array);
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
    if(maxwidth.size() > 0 || maxwidth.size() > 0) {
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
    setColor("foreground", 0, 0, 0, 255, array);

    writeOpenProperty("font", array);
    writeOpenTag("font", array);
    writeTaggedString("pointsize", pointsize, array);
    writeCloseTag("font", array);
    writeCloseProperty(array);


    writeCloseTag("widget", array);
}

void ParsePepFile::writeLabel(QString text, QString minwidth, QString minheight, QString maxwidth, QString maxheight, QString pointsize,
                              QString alignment, QString colormode, QString calcpv, QString calc, QString visibility, bool transparent, QByteArray *array)
{
    writeOpenTag("widget class=\"caLabel\" name=\"calabel\"", array);

    if(minwidth.size() > 0 || minheight.size() > 0) {
        writeOpenProperty("minimumSize", array);
        writeOpenTag("size", array);
        if(minwidth.size() > 0) writeTaggedString("width", minwidth, array);
        if(minheight.size() > 0) writeTaggedString("height", minheight, array);
        writeCloseTag("size", array);
        writeCloseProperty(array);
    }
    if(maxwidth.size() > 0 || maxwidth.size() > 0) {
        writeOpenProperty("maximumSize", array);
        writeOpenTag("size", array);
        if(maxwidth.size() > 0) writeTaggedString("width", maxwidth, array);
        if(maxheight.size() > 0) writeTaggedString("height", maxheight, array);
        writeCloseTag("size", array);
        writeCloseProperty(array);
    }

    writeSimpleProperty("text", "string",  text.toAscii().constData(), array);
    writeSimpleProperty("alignment", "set", alignment, array);
    if(transparent) setColor("background", 200, 200, 200, 0, array);
    else setColor("background", 200, 200, 200, 255, array);

    writeOpenProperty("font", array);
    writeOpenTag("font", array);
    writeTaggedString("pointsize", pointsize, array);
    writeCloseTag("font", array);
    writeCloseProperty(array);

    if(colormode.size() > 0) {
        writeSimpleProperty("colorMode", "enum", colormode, array);
    }

    if(calc.size() > 0 && visibility.size() > 0 && calcpv.size() > 0) {
        writeSimpleProperty("visibility", "enum", visibility, array);
        writeSimpleProperty("visibilityCalc", "string", calc, array);
        writeSimpleProperty("channel", "string", calcpv, array);
    }

    writeCloseTag("widget", array);
}

void ParsePepFile::writeOpenProperty(QString property, QByteArray *array)
{
    char widgetBytes[100];
    sprintf(widgetBytes, "<property name=\"%s\">\n", property.toAscii().constData());
    array->append(widgetBytes);
}

void ParsePepFile::writeCloseProperty(QByteArray *array)
{
    char widgetBytes[100];
    sprintf(widgetBytes, "</property>\n");
    array->append(widgetBytes);
}

void ParsePepFile::writeTaggedString(QString tag, QString value, QByteArray *array)
{
    char widgetBytes[100];
    sprintf(widgetBytes, "<%s>%s</%s>\n", tag.toAscii().constData(), value.toAscii().constData(), tag.toAscii().constData());
    array->append(widgetBytes);
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

void ParsePepFile::writeItemRowCol(int &row, int &column,  int span, QByteArray *array)
{
    char widgetBytes[1000];
    sprintf(widgetBytes,"<item row=\"%d\" column=\"%d\" colspan=\"%d\">\n", row, column, span);
    array->append(widgetBytes);
    column++;
}

void ParsePepFile::writeOpenTag(QString tag,  QByteArray *array)
{
    char widgetBytes[1000];
    sprintf(widgetBytes, "<%s>\n", tag.toAscii().constData());
    array->append(widgetBytes);
}

void ParsePepFile::writeCloseTag(QString tag,  QByteArray *array)
{
    char widgetBytes[1000];
    sprintf(widgetBytes, "</%s>\n", tag.toAscii().constData());
    array->append(widgetBytes);
}

void ParsePepFile::writeSimpleProperty(QString prop, QString tag, QString value, QByteArray *array)
{
    writeOpenProperty(prop, array);
    writeTaggedString(tag, value, array);
    writeCloseProperty(array);
}

QWidget* ParsePepFile::load(QWidget *parent)
{
    QWidget *widget = new QWidget;
    QUiLoader loader;
    QFile file("out.ui");
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);

    buffer->open(QIODevice::ReadOnly);

    out << buffer->data();
    file.close();

    buffer->seek(0);
    widget=loader.load(buffer, parent);
    buffer->close();
    return widget;
}




