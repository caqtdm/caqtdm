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


#include <cstdlib>

#include <iostream>
#include <qfile.h>
#include "XmlWriter.h"
#include <QFileDialog>
#include "dmsearchfile.h"
#include <QDebug>
#include <QFileInfo>
#include "myParserEDM.h"
#include "parserClass.h"

#define MAX_ASCII               80      /* max size of many asci strings */

//extern "C" void Qt_writeZorder();


extern "C" char filePrefix[128];

// pointer used by external C
extern "C" {
    myParserEDM* myParserPtr;
}

// constructor
myParserEDM::myParserEDM () {
    zindex = 0;
}


void myParserEDM::openFile(char *outFile)
{
    dmsearchFile *s = new dmsearchFile("stylesheet.qss");
    QString fileNameFound = s->findFile();
    if(fileNameFound.isNull()) {
        printf("edl2ui -- file <stylesheet.qss> could not be loaded, is 'CAQTDM_DISPLAY_PATH' <%s> defined?\n", s->displayPath().toLatin1().constData());
        printf("edl2ui -- could be a problem!\n");
    } else {
        QFile file(fileNameFound);
        file.open(QFile::ReadOnly);
        StyleSheet = QLatin1String(file.readAll());
        printf("edl2ui -- file <stylesheet.qss> found and will be integrated in the resulting ui file\n");
        printf("edl2ui -- if you do not want any styles, redefine CAQTDM_DISPLAY_PATH\n");
        file.close();
    }

    file = new QFile(outFile);
    if (!file->open(QIODevice::WriteOnly | QIODevice::Text)) exit(0);

    xw = new XmlWriter(file);

    xw->setAutoNewLine(true);

    xw->writeRaw( "<ui version=\"4.0\">" );
    xw->newLine();
    xw->writeTaggedString( "class", "MainWindow" );
    xw->writeOpenTag("widget", AttrMap("class", "QMainWindow"), AttrMap("name", "MainWindow"));
}

void myParserEDM::parseFile(char *inFile){
    parserClass *parser = new parserClass(inFile);
    parser->loadFile(this);
}

void myParserEDM::writeStyleSheet(int r, int g, int b)
{
    xw->writeOpenTag( "property", AttrMap("name", "styleSheet") );

    QString color = "\n\nQWidget#centralWidget {background: rgba(%1, %2, %3, 255);}\n\n";
    color = color.arg(r).arg(g).arg(b);

    QString styles = color;
    styles.append(StyleSheet);

    xw->writeTaggedString( "string",  styles);
    xw->writeCloseTag( "property");
}

static int compareFunc(const zOrder &a, const zOrder &b)
{
    //printf("%d %d\n", a->vis, b->vis);
    return (b.vis > a.vis);
}

void myParserEDM::closeFile()
{
    QVector<zOrder> myvector;
    QVector<zOrder>::iterator it;

    // create a vector with the order we had when parsing
    for(int i=0; i<zindex; i++) {
        myvector.append(zorder[i]);
    }
    // sort according to the static elements
    qStableSort(myvector.begin(), myvector.end(), compareFunc);
    for (it=myvector.begin(); it!=myvector.end(); ++it) {
        //qDebug() << "sorted" << it->indx << it->vis << it->z;
        xw->writeTaggedString("zorder", it->z);
    }

    xw->writeCloseTag( "widget");
    xw->writeCloseTag( "widget");
    xw->writeRaw( "</ui>");
    file->close();
}

void myParserEDM::writeProperty(const QString& name, const QString& type, const QString& value )
{
    xw->writeOpenTag( "property", AttrMap("name", name) );
    xw->writeTaggedString( type, value );
    xw->writeCloseTag( "property" );
}

void myParserEDM::writeOpenProperty(const QString& name)
{
    xw->writeOpenTag( "property", AttrMap("name", name) );
}

void myParserEDM::writeTaggedString(const QString& type, const QString& value )
{
    xw->writeTaggedString( type, value );
}

void myParserEDM::writeCloseProperty()
{
    xw->writeCloseTag( "property" );
}

void myParserEDM::writeOpenTag(const QString& type, const QString& cls, const QString& name )
{
    if(cls.size() != 0 && name.size() != 0) {
        xw->writeOpenTag( type, AttrMap("class", cls), AttrMap("name", name));
    } else {
        xw->writeOpenTag( type) ;
    }
}

void myParserEDM::writeCloseTag(const QString& type)
{
    xw->writeCloseTag(type);
}

void myParserEDM::Init(myParserEDM* edlParser)
{
    myParserPtr = edlParser;
}

void myParserEDM::writeMessage(char *mess) {
    Q_UNUSED(mess);
    //qDebug() << mess;
}

void myParserEDM::Qt_writeOpenTag(char *tag, char *typ, char *value)
{
    writeOpenTag(tag, typ, value);
}

void myParserEDM::Qt_writeCloseTag(char *tag, char *value, int visibilityStatic)
{
    if(!strcmp(tag, "widget") && strstr(value, "ca")) {
        strcpy(zorder[zindex].z, value);
        zorder[zindex].vis = visibilityStatic;
        zorder[zindex].indx = zindex;
        zindex++;
    }
    writeCloseTag(tag);
}

void myParserEDM::Qt_handleString(char *prop, char *tag, char *value)
{
    writeOpenProperty(prop);
    writeTaggedString(tag, value);
    writeCloseProperty();
}

void myParserEDM::writeRectangleDimensions(int x, int y, int w, int h)
{
    char asc[MAX_ASCII];

    writeOpenProperty("geometry");
    writeOpenTag("rect", "", "");

    sprintf(asc, "%d", x);
    writeTaggedString("x", asc);

    sprintf(asc, "%d", y);
    writeTaggedString("y", asc);

    sprintf(asc, "%d", w);
    writeTaggedString("width", asc);

    sprintf(asc, "%d", h);
    writeTaggedString("height", asc);

    writeCloseTag("rect");
    writeCloseProperty();
}
// Zai added
void myParserEDM::writeFontProperties(int size, bool bold)
{
    char asc[MAX_ASCII];

    writeOpenProperty("font");
    writeOpenTag("font", "", "");

    sprintf(asc, "%d", size - 4);
    writeTaggedString("pointsize", asc);

    if (bold && size > 14){
//        std::string trueString;
//        trueString.append("true");
        sprintf(asc, "%s", "true");
        writeTaggedString("bold", asc);
    }

    writeCloseTag("font");
    writeCloseProperty();
}

void myParserEDM::Qt_setColor(char *property, int r, int g, int b, int alpha)
{
    char asc[80];
    writeOpenProperty(property);
    sprintf(asc,"color alpha=\"%d\"", alpha);
    writeOpenTag(asc, "", "");
    sprintf(asc,"%d", r);
    writeTaggedString("red", asc);
    sprintf(asc,"%d", g);
    writeTaggedString("green", asc);
    sprintf(asc,"%d", b);
    writeTaggedString("blue", asc);
    writeCloseTag("color");
    writeCloseProperty();
}

void myParserEDM::Qt_setColorForeground(char *widget, int r, int g, int b, int alpha)
{
    Q_UNUSED(widget);
    Qt_setColor("foreground", r, g, b, alpha);
}

void myParserEDM::Qt_setColorBackground(char *widget, int r, int g, int b, int alpha)
{
    Q_UNUSED(widget);
    Qt_setColor("background", r, g, b, alpha);
}

void myParserEDM::Qt_setColorLine(char *widget, int r, int g, int b, int alpha) {
    Q_UNUSED(widget);
    Qt_setColor("lineColor", r, g, b, alpha);
}

void myParserEDM::Qt_setColorGrid(char *widget, int r, int g, int b, int alpha) {
    Q_UNUSED(widget);
    Qt_setColor("gridColor", r, g, b, alpha);
}

void myParserEDM::Qt_setColorScale(char *widget, int r, int g, int b, int alpha) {
    Q_UNUSED(widget);
    Qt_setColor("scaleColor", r, g, b, alpha);
}

void myParserEDM::Qt_setColorTrace(char *widget, int r, int g, int b, int alpha, int trace) {
    Q_UNUSED(widget);
    QString prop = "color_%1";
    prop = prop.arg(trace+1);
    Qt_setColor((char*) prop.toLatin1().constData(), r, g, b, alpha);
}

void myParserEDM::Qt_setSymbolTrace(char *widget, char *symbol, int trace) {
    Q_UNUSED(widget);
    QString prop = "symbol_%1";
    prop = prop.arg(trace+1);
    Qt_handleString((char*) prop.toLatin1().constData(), "enum", symbol);
}

void myParserEDM::Qt_setStyleTrace(char *widget, char *style, int trace) {
    Q_UNUSED(widget);
    QString prop = "Style_%1";
    prop = prop.arg(trace+1);
    Qt_handleString((char*) prop.toLatin1().constData(), "enum", style);
}


void myParserEDM::Qt_ChannelsXY(char *widget, char *channels, int trace) {
    Q_UNUSED(widget);
    QString prop = "channels_%1";
    prop = prop.arg(trace+1);
    Qt_handleString((char *)prop.toLatin1().constData(), "string", channels);
}

void myParserEDM::Qt_extractString(char *str, char *retStr, int *status) {

    int startPos = 0;
    retStr[0] = '\0';
    QString newStr(str);
    startPos =newStr.indexOf("}(");
    if(startPos > -1) {
        newStr.remove(0,startPos+2);
    } else {
        *status = false;
        return;
    }
    if(newStr.endsWith(")")) newStr.remove(newStr.size()-1,1);
    strcpy(retStr, (char*) newStr.toLatin1().constData());
    *status = true;
}

/*
 *
 */
class myParserEDM;

int main(int argc, char *argv[])
{
    int	in, numargs;
    char inFile[80] = "";

    for (numargs = argc, in = 1; in < numargs; in++) {

        if ( strcmp (argv[in], "-v" ) == 0 ) {
            printf("edl2ui version %s for %s\n", BUILDVERSION, BUILDARCH);
            exit(0);
        }
        if(!strcmp(argv[in],"-help") || !strcmp(argv[in],"-h") || !strcmp(argv[in],"-?")) {
            in++;
            printf("Usage:\n edl2ui [options] file\n");
            printf("[-flat] :        flat file will be generated, includes are integrated\n");
            printf("[-nolegends] :   no legends will be generated for the stripplots\n");
            printf("[-deviceonmenu] : part of pv will be used for the label of menu\n");
            printf("[-expandtext] : when textlabels do not fit, try this option\n");
            exit(1);
        }
        if (strncmp (argv[in], "-" , 1) == 0) {
            /* unknown application argument */
            printf("edl2ui -- Argument %d = [%s] is unknown! ",in,argv[in]);
            printf("possible are: '-flat and '-deviceonmenu' and '-nolegends' and '-v'\n");
            exit(-1);
        } else {
            printf("edl2ui -- file = <%s>\n", argv[in]);
            strcpy(inFile, argv[in]);
        }
    }

    // input and out files
    QString inputFile = inFile;
    if(inputFile.size() < 1) {
        qDebug() << "edl2ui -- sorry: no input file";
        exit(-1);
    }

    QString openFile1, openFile2;
    QString outputFile;

    int found = inputFile.lastIndexOf(".edl");
    if (found != -1) {
        openFile1 = inputFile.mid(0, found);
    } else {
        openFile1 = inputFile;
    }

    openFile2 = openFile1;

    outputFile = openFile1.append(".ui");
    inputFile = openFile2.append(".edl");

    // when file exists open it
    QFileInfo fi(inputFile);
    if(!fi.exists()) {
        qDebug() << "edl2ui -- sorry, file" << inFile << "does not exist";
        exit(-1);
    }

    // get path for composite file parsing
    //qDebug() << fi.absolutePath();
    //strcpy(filePrefix, fi.absolutePath().toLatin1().data());

    // init edlParser
    myParserEDM *edlParser = new myParserEDM;
    edlParser->Init(edlParser);

    //get rid of path, we want to generate where we are
    outputFile = outputFile.section('/',-1);
    edlParser->openFile(outputFile.toLatin1().data());

    // open input file
    //FILE *filePtr = fopen(inputFile.toLatin1().data(), "r");
    parserClass *parser = new parserClass(inputFile.toLatin1().data());
    parser->loadFile(edlParser);


    // close output file
    edlParser->closeFile();

    return 0;
}

