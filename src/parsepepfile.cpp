#include <QtGui>
#include <math.h>
#include "parsepepfile.h"

ParsePepFile::ParsePepFile(QString filename)
{
    int grid = 0;
    int ll;
    int span =0;
    int nbFormats = 0;
    int actualLine = 0;
    int actualColumn = -1;
    QFile *file = new QFile;
    char charBuf[1024];

    char header[2000];
    char footer[100];

    QString formats[2];
    QString widgetType ="";
    QString widgetText ="";
    QString channel ="";
    QString comment="";

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
/*
           "<widget class=\"QWidget\" name=\"centralWidget\">\n"
           "<widget class=\"QWidget\" name=\"layoutWidget\">\n"
           "<layout class=\"QGridLayout\" name=\"gridLayout\">\n");
*/
            "<widget class=\"QWidget\" name=\"centralWidget\">\n"
           "<layout class=\"QGridLayout\" name=\"gridLayout_2\">\n"
            "<item row=\"0\" column=\"0\">\n"
             "<layout class=\"QGridLayout\" name=\"gridLayout\">\n");

    //strcpy(footer, "</layout></widget></widget></widget></ui>");

    strcpy(footer, "</layout></item></layout></widget></widget></ui>");

    QByteArray headerBytes(header);
    QByteArray footerBytes(footer);
    QByteArray *array= new QByteArray();
    array->append(headerBytes);

    strcpy(charBuf, header);

    buffer = new QBuffer();


    file->setFileName(filename);

    Initialize();

    if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) return ;

    // scan file
    QTextStream in(file);
    QString line = in.readLine();
    while (!line.isNull()) {

        // replace all blancs (except these between quotes by ;
        bool inside = false;
        for (int i = 0; i < line.size(); i++) {
            if((!inside) && ((line.at(i) == QChar('\'')) || (line.at(i) == QChar('\"')))) {
                inside = true;
            } else if((inside) && ((line.at(i) == QChar('\'')) || (line.at(i) == QChar('\"')))) {
                inside = false;
            }
            if (!inside) {
                if(line.at(i) == QChar(' ')) {
                    line.replace(i, 1, ";");
                }
            }
        }

        QStringList elements= line.split(";",  QString::SkipEmptyParts);

        // empty line ?
        if(line.size() == 0) {

        }
        // we have a grid definition at the beginning of the file
        else if(line.contains("#!grid")) {
            grid = elements.at(1).toInt();
            //printf("grid=%d\n", grid);
        } else if(!line.contains("#")){

            if(mapStringValues[elements.at(0)] == evSeparator) {
                actualColumn = -1;
                actualLine++;
            } else {

                actualColumn++;
                if(actualColumn > grid-1) {
                    actualColumn = 0;
                    actualLine++;
                }
            }

            if(elements.count() > 1) {
                //printf("%s %d %d\n", elements.at(0).toAscii().constData(), actualLine, actualColumn);

                channel = elements.at(0);
                if(elements.count() > 1) widgetType = elements.at(1);
                //if(elements.count() > 2) widgetText = elements.at(2); else  widgetText = "";
                //printf("channel <%s> widgetType=<%s>\n", channel.toAscii().constData(), widgetType.toAscii().constData());

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
                            printf("span detected, value=%s\n", elements.at(ll).toAscii().constData());
                            span = elements.at(ll).toInt();
                        }
                    } else if(elements.at(ll).contains("-confirm")) {
                        printf("confirm detected\n");
                        break;

                    } else if(elements.at(ll).contains("-hys")) {
                        printf("hys detected\n");
                        break;

                    } else if(elements.at(ll).contains("-comlab")) {
                        printf("comlab detected\n");
                        break;

                    } else if(elements.at(ll).contains("-commmand")) {
                        printf("commmand detected\n");
                        break;

                    } else if(elements.at(ll).contains("-text")) {
                        printf("text detected\n");
                        break;

                    } else if(ok && (widgetType.contains("setRdbk") ||
                                     widgetType.contains("wheelswitch") ||
                                     widgetType.contains("formread"))) {
                        printf("format detected %s\n", elements.at(ll).toAscii().constData());
                        formats[nbFormats++] =elements.at(ll);
                    } else {
                        printf("something else  detected %s\n", elements.at(ll).toAscii().constData());

                        widgetText.append(elements.at(ll));
                        widgetText.append(" ");
                    }
                    ll++;
                }

                widgetType = elements.at(1);

                if(widgetType.contains("comment")) {
                    widgetText.replace("\"","");
                    widgetText.replace("\'","");
                    widgetText.replace("&","&amp;");
                    displayItem(actualLine, actualColumn, widgetType, widgetText, channel, span, formats, array);

                } else if(widgetType.contains("separator")) {
                    displayItem(actualLine, 0, elements.at(1), "", "", span, formats, array);
                    actualLine++;

                } else {
                    displayItem(actualLine, actualColumn, widgetType, widgetText, channel, span, formats, array);
                }

                if(mapStringValues[elements.at(0)] != evSeparator) {
                    for(int i=1; i <span; i++) {
                        actualColumn++;
                        if(actualColumn > grid-1) {
                            actualColumn = 0;
                            actualLine++;
                        }
                    }
                }
            }
        }

        line = in.readLine();
    }
    file->close();

    strcat(charBuf, footer);
    array->append(footerBytes);
    buffer->open(QIODevice::ReadWrite);
    buffer->write(*array);
    buffer->close();
}

void ParsePepFile::Initialize()
{
    mapStringValues["comment"] = evComment;
    mapStringValues["separator"] = evSeparator;
    mapStringValues["end"] = evEnd;
}

void ParsePepFile::displayItem(int row, int column, QString widgetType, QString text, QString pv, int span, QString *formats,
                               QByteArray *array)
{
    char widgetBytes[1024];
    QString newpv = "";
    QString partialpv = "";
    QString minsize[2];
    QString maxsize[2];
    int rgba[4];

    maxsize[0] = ""; maxsize[1] = "";  minsize[0] = ""; minsize[1] = "20";

    rgba[0] = rgba[1] = rgba[2] = 0; rgba[3] = 0;

    QStringList pvElements= pv.split(":",  QString::SkipEmptyParts);
    if(pvElements.count() > 0) partialpv = pvElements.at(0);

    if(widgetType.contains("LED")) {
        sprintf(widgetBytes,"<item row=\"%d\"  column=\"%d\">\n", row, column);
        strcat(widgetBytes, " <widget class=\"caGraphics\" name=\"cagraphics\">\n");
        array->append(widgetBytes);

        writeOpenProperty("maximumSize", array);
        writeOpenTag("size", array);
        writeTaggedString("width", "15", array);
        writeTaggedString("height", "15", array);
        writeCloseTag("size", array);
        writeCloseProperty(array);

        writeSimpleProperty("channel", "string", pv, array);
        writeSimpleProperty("colorMode", "enum", "caGraphics::Alarm", array);
        writeSimpleProperty("form", "enum", "caGraphics::Circle", array);
        writeSimpleProperty("fillstyle", "enum", "caGraphics::Filled", array);

        writeCloseTag("widget", array);
        writeCloseTag("item", array);

    } else if (widgetType.contains("separator")) {
        printf("create separator over %d columns\n",span);
        sprintf(widgetBytes,"<item row=\"%d\" column=\"%d\" colspan=\"%d\">\n", row, column, span);
        array->append(widgetBytes);

        writeOpenTag("widget class=\"Line\" name=\"line\"", array);
        writeSimpleProperty("frameShadow", "enum", "QFrame::Plain", array);
        writeSimpleProperty("lineWidth", "number", "2", array);
        writeSimpleProperty("orientation", "enum", "Qt::Horizontal", array);
        writeCloseTag("widget", array);
        writeCloseTag("item", array);

    } else if (widgetType.contains("comment")) {
        printf("create comment over %d columns\n",span);
        sprintf(widgetBytes,"<item row=\"%d\"  column=\"%d\" colspan=\"%d\">\n", row, column, span);
        array->append(widgetBytes);

        maxsize[0] = ""; maxsize[1] = "";  minsize[0] = ""; minsize[1] = "20";
        rgba[3] = 0;
        // write text as label
        if(text.size() < 1) {
            writeLabel("&lt;html&gt;&lt;head/&gt;&lt;body&gt;&amp;nbsp;&amp;nbsp;&lt;/body&gt;&lt;/html&gt;", minsize, maxsize, "10",
                       "Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter", "", "", "",  "", rgba, array);
        } else {
            writeLabel(text, minsize, maxsize, "10", "Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter", "", "", "", "", rgba, array);
        }

        writeCloseTag("item", array);

    } else  if (widgetType.contains("wheelswitch")){

        printf("create wheelswitch over %d columns\n",span);

        sprintf(widgetBytes,"<item row=\"%d\"  column=\"%d\">\n", row, column);
        array->append(widgetBytes);

        // write the layout header
        writeLayoutHeader(array);

        // write first the label
        writeOpenTag("item", array);

        maxsize[0] = ""; maxsize[1] = "";  minsize[0] = "150"; minsize[1] = "20";
        rgba[3] = 0;
        writeLabel(pv, minsize, maxsize, "10", "Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter", "", "", "", "", rgba, array);
        writeCloseTag("item", array);

        // write now the wheelswitch
        writeOpenTag("item", array);
        writeWheelswitch(formats[0], pv, array);
        writeCloseTag("item", array);

        // layout finished
        writeLayoutFooter(array);

    } else  if (widgetType.contains("setRdbk")){

        printf("create setRdbk over %d columns\n",span);

        sprintf(widgetBytes,"<item row=\"%d\"  column=\"%d\"  colspan=\"%d\">\n", row, column, span);
        array->append(widgetBytes);

        // write layout
        writeLayoutHeader(array);

        // write first the label
        writeOpenTag("item", array);
        maxsize[0] = ""; maxsize[1] = "";  minsize[0] = "150"; minsize[1] = "20";
        rgba[3] = 0;
        writeLabel(pv, minsize, maxsize, "10", "Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter", "", "", "", "", rgba, array);
        writeCloseTag("item", array);

        // write now the wheelswitch
        writeOpenTag("item", array);
        writeWheelswitch(formats[0], pv, array);
        writeCloseTag("item", array);

        // write now the two labels with equal and unequal sign, but first hard code the new pv
        newpv = partialpv;
        newpv.append(":I-COMP");
        writeOpenTag("item", array);

        maxsize[0] = "20"; maxsize[1] = "20";  minsize[0] = "20"; minsize[1] = "20";
        rgba[3] = 0;
        writeLabel("&lt;html&gt;&lt;head/&gt;&lt;body&gt;&amp;#61;&amp;nbsp;&lt;/body&gt;&lt;/html&gt;", minsize, maxsize,
                   "16", "Qt::AlignLeading|Qt::AlignCenter|Qt::AlignVCenter", "caLabel::Alarm", newpv, "A=0", "caLabel::Calc", rgba, array);
        writeCloseTag("item", array);
        writeOpenTag("item", array);
        writeLabel("&lt;html&gt;&lt;head/&gt;&lt;body&gt;&amp;ne;&amp;nbsp;&lt;/body&gt;&lt;/html&gt;", minsize, maxsize,
                   "16", "Qt::AlignLeading|Qt::AlignCenter|Qt::AlignVCenter", "caLabel::Alarm", newpv, "A>0", "caLabel::Calc", rgba, array);
        writeCloseTag("item", array);


        // write now the lineedit, but first hard code the new pv
        newpv = partialpv;
        newpv.append(":I-READ");
        writeOpenTag("item", array);
        maxsize[0] = ""; maxsize[1] = "";  minsize[0] = "100"; minsize[1] = "20";
        rgba[3] = 255;
        writeLineEdit(formats[0], newpv, minsize, maxsize, "", "", "", "", "", "", rgba, array);
        minsize[0] = "";
        writeCloseTag("item", array);

        // write now the label for cycle direction, but first hardcode the new pv
        newpv = partialpv;
        newpv.append(":HYS-CYDIR");

        maxsize[0] = "30"; maxsize[1] = "30";  minsize[0] = "30"; minsize[1] = "30";
        rgba[3] = 0;
        writeOpenTag("item", array);
        writeLineEdit(formats[0], newpv, minsize, maxsize, "", "", "", "", "", "", rgba, array);
        writeCloseTag("item", array);
        minsize[0] = maxsize[0] = maxsize[1] = "";

        // write now the led, but first hard code the new pv
        newpv = partialpv;
        newpv.append(":ONOFF");

        writeOpenTag("item", array);
        strcpy(widgetBytes, " <widget class=\"caLed\" name=\"caled\">\n");
        array->append(widgetBytes);
        writeSimpleProperty("channel", "string", newpv, array);
        setColor("trueColor", 255, 170, 0, 255, array);
        writeCloseTag("widget", array);
        writeCloseTag("item", array);

        // write now the mode, but first hard code the new pv
        newpv = partialpv;
        newpv.append(":PS-MODE");
        writeOpenTag("item", array);
        maxsize[0] = ""; maxsize[1] = "";  minsize[0] = "100"; minsize[1] = "20";
        rgba[3] = 255;
        writeLineEdit(formats[0], newpv, minsize, maxsize, "", "", "", "", "", "", rgba, array);
        writeCloseTag("item", array);

        writeLayoutFooter(array);


    } else  if (widgetType.contains("formread")){

        printf("create linedit over %d columns\n",span);

        // horizontal layout
        sprintf(widgetBytes,"<item row=\"%d\"  column=\"%d\">\n", row, column);
        array->append(widgetBytes);

        // write layout header
        writeLayoutHeader(array);

        // write first the label
        writeOpenTag("item", array);
        minsize[0] = "150";
        rgba[3] = 0;
        writeLabel(pv, minsize, maxsize, "10", "Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter", "", "", "", "", rgba, array);
        writeCloseTag("item", array);

        // write now the lineedit
        writeOpenTag("item", array);
        rgba[3] = 255;
        writeLineEdit(formats[0], pv, minsize, maxsize, "", "", "", "", "", "", rgba, array);
        writeCloseTag("item", array);

        // layout finished
        writeLayoutFooter(array);
    }
}

void ParsePepFile::writeLayoutHeader(QByteArray *array)
{
    char widgetBytes[1024];
    sprintf(widgetBytes,"<widget class=\"caFrame\" name=\"caframe\">\n"
            "<property name=\"background\">\n"
            "<color alpha=\"255\">\n"
            "<red>195</red>\n"
            "<green>195</green>\n"
            "<blue>195</blue>\n"
            "</color>\n"
            "</property>\n"
            "<property name=\"backgroundMode\">\n"
            "<enum>caFrame::Filled</enum>\n"
            "</property>\n"
            "<layout class=\"QHBoxLayout\" name=\"horizontalLayout\">\n"
            "<property name=\"margin\">\n"
            "<number>0</number>\n"
            "</property>\n"
            "<item>\n"
            "<layout class=\"QHBoxLayout\" name=\"horizontalLayout\">\n");
    array->append(widgetBytes);
}

void ParsePepFile::writeLayoutFooter(QByteArray *array)
{
    char widgetBytes[1024];
    sprintf(widgetBytes,"</layout></item></layout></widget></item>\n");
    array->append(widgetBytes);
}

void ParsePepFile::writeWheelswitch(QString format, QString pv, QByteArray *array)
{
    bool ok;
    char widgetBytes[1024];

    QStringList elements= format.split(".",  QString::SkipEmptyParts);

    int totalDigits = elements[0].toInt(&ok);
    int decimalDigits = elements[1].toInt(&ok);
    int integerDigits = totalDigits - decimalDigits -2;
    QString Digits1= QString("%1").arg(integerDigits);
    QString Digits2= QString("%1").arg(decimalDigits);

    sprintf(widgetBytes, "<widget class=\"caNumeric\" name=\"canumeric\">\n");
    array->append(widgetBytes);

    writeSimpleProperty("channel", "string", pv, array);
    setColor("background", 157, 236, 0, 255, array);

    writeOpenProperty("minimumSize", array);
    writeOpenTag("size", array);
    writeTaggedString("height", "45", array);
    writeCloseTag("size", array);
    writeCloseProperty(array);

    writeSimpleProperty("integerDigits", "number", Digits1, array);
    writeSimpleProperty("decimalDigits", "number", Digits2, array);

    writeSimpleProperty("fixedFormat", "bool", "true", array);

    sprintf(widgetBytes, "</widget>\n");
    array->append(widgetBytes);
}

void ParsePepFile::writeLineEdit(QString format, QString pv, QString minsize[2], QString maxsize[2], QString pointsize, QString alignment,
QString colormode, QString calcpv,QString calc, QString visibility, int rgba[], QByteArray *array)
{
    char widgetBytes[1024];
    Q_UNUSED(format);
    // a lineedit
    sprintf(widgetBytes, "<widget class=\"caLineEdit\" name=\"calinedit\">\n");
    array->append(widgetBytes);

    if(minsize[0].size() > 0 || minsize[1].size() > 0) {
        writeOpenProperty("minimumSize", array);
        writeOpenTag("size", array);
        if(minsize[0].size() > 0) writeTaggedString("width", minsize[0], array);
        if(minsize[1].size() > 0) writeTaggedString("height", minsize[1], array);
        writeCloseTag("size", array);
        writeCloseProperty(array);
    }
    if(maxsize[0].size() > 0 || maxsize[1].size() > 0) {
        writeOpenProperty("maximumSize", array);
        writeOpenTag("size", array);
        if(maxsize[0].size() > 0) writeTaggedString("width", maxsize[0], array);
        if(maxsize[1].size() > 0) writeTaggedString("height", maxsize[1], array);
        writeCloseTag("size", array);
        writeCloseProperty(array);
    }

    writeSimpleProperty("alignment", "set", "Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter", array);
    writeSimpleProperty("channel", "string", pv, array);
    writeSimpleProperty("colorMode", "enum", "caLineEdit::Alarm_Static", array);
    writeSimpleProperty("unitsEnabled", "bool", "true", array);
    setColor("background", rgba[0], rgba[1], rgba[2], rgba[3], array);
    setColor("foreground", 255, 255, 255, 255, array);

    sprintf(widgetBytes, "</widget>\n");
    array->append(widgetBytes);
}

void ParsePepFile::writeLabel(QString text, QString minsize[2], QString maxsize[2], QString pointsize, QString alignment,
                              QString colormode, QString calcpv, QString calc, QString visibility, int rgba[], QByteArray *array)
{
    char widgetBytes[1024];

    strcpy(widgetBytes, " <widget class=\"caLabel\" name=\"calabel\">\n");
    array->append(widgetBytes);

    if(minsize[0].size() > 0 || minsize[1].size() > 0) {
        writeOpenProperty("minimumSize", array);
        writeOpenTag("size", array);
        if(minsize[0].size() > 0) writeTaggedString("width", minsize[0], array);
        if(minsize[1].size() > 0) writeTaggedString("height", minsize[1], array);
        writeCloseTag("size", array);
        writeCloseProperty(array);
    }
    if(maxsize[0].size() > 0 || maxsize[1].size() > 0) {
        writeOpenProperty("maximumSize", array);
        writeOpenTag("size", array);
        if(maxsize[0].size() > 0) writeTaggedString("width", maxsize[0], array);
        if(maxsize[1].size() > 0) writeTaggedString("height", maxsize[1], array);
        writeCloseTag("size", array);
        writeCloseProperty(array);
    }


    writeSimpleProperty("text", "string",  text.toAscii().constData(), array);
    writeSimpleProperty("alignment", "set", alignment, array);
    setColor("background", rgba[0], rgba[1], rgba[2], rgba[3], array);

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
    /*
    printf("buffer size=%d\n", (int) buffer->size());
    char ch;
    for(int i=0; i< buffer->size(); i++) {
        buffer->getChar(&ch);
        printf("%c", ch);
    }
    printf("\n");
*/
    buffer->seek(0);
    widget=loader.load(buffer, parent);
    buffer->close();
    return widget;
}




