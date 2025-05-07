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
#include "parser.h"
#include <iostream>
#include <adlParserMain.h>
#include <QFileDialog>
#include "dmsearchfile.h"
#include <QDebug>
#include <QFileInfo>
#define zorder_size 20000

extern "C" TOKEN parseAndAppendDisplayList(DisplayInfo *displayInfo, FrameOffset *offset, char *firstToken, TOKEN firstTokenType);
extern "C" TOKEN getToken(DisplayInfo *displayInfo, char *word);
extern "C" void parseFile(DisplayInfo *displayInfo);
extern "C" void *parseDisplay(DisplayInfo *displayInfo);
extern "C" DlColormap *parseColormap(DisplayInfo *displayInfo, FILE *filePtr);
extern "C" void Qt_writeZorder();
extern "C" int parsingCompositeFile;
extern "C" int generateFlatFile;
extern "C" int generateDeviceOnMenus;
extern "C" int expandText;
extern "C" int legendsForStripplot;
extern "C" void initScanning();

typedef char string40[40];
typedef struct _zOrder {
    int indx;
    int vis;
    string40 z;
} zOrder;

extern "C" zOrder zorder[zorder_size];
extern "C" int zindex;
extern "C" char filePrefix[128];

// pointer used by external C
extern "C" {
	myParser* myParserPtr;
}
// constructor
myParser::myParser () {
    tmp_directory="";
}

void myParser::openFile(char *outFile)
{
    dmsearchFile *s = new dmsearchFile("stylesheet.qss");
    QString fileNameFound = s->findFile();
    if(fileNameFound.isNull()) {
        printf("adl2ui -- file <stylesheet.qss> could not be loaded, is 'CAQTDM_DISPLAY_PATH' <%s> defined?\n", s->displayPath().toLatin1().constData());
        printf("adl2ui -- could be a problem!\n");
    } else {
        QFile file(fileNameFound);
        file.open(QFile::ReadOnly);
        StyleSheet = QLatin1String(file.readAll());
        printf("adl2ui -- file <stylesheet.qss> found and will be integrated in the resulting ui file\n");
        printf("adl2ui -- if you do not want any styles, redefine CAQTDM_DISPLAY_PATH\n");
        file.close();
    }

    //printf("write to file %s\n", outFile);

    file = new QFile(outFile);
    if (!file->open(QIODevice::WriteOnly | QIODevice::Text)) exit(0);

    xw = new XmlWriter(file);

    xw->setAutoNewLine(true);

    xw->writeRaw( "<ui version=\"4.0\">" );
    xw->newLine();
    xw->writeTaggedString( "class", "MainWindow" );
    xw->writeOpenTag("widget", AttrMap("class", "QMainWindow"), AttrMap("name", "MainWindow"));
}

void myParser::writeStyleSheet(int r, int g, int b)
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

void myParser::closeFile()
{
    QVector<zOrder> myvector;
    QVector<zOrder>::iterator it;

    // create a vector with the order we had when parsing
    for(int i=0; i<zindex; i++) {
        myvector.append(zorder[i]);
    }
    // sort according to the static elements
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    qStableSort(myvector.begin(), myvector.end(), compareFunc);
#else
    std::stable_sort(myvector.begin(), myvector.end(), compareFunc);
#endif
    int index = 0;
    for (it=myvector.begin(); it!=myvector.end(); ++it) {
        //qDebug() << "sorted" << it->indx << it->vis << it->z;
        xw->writeTaggedString("zorder", it->z);
        index++;
    }

    xw->writeCloseTag( "widget");
    xw->writeCloseTag( "widget");
    xw->writeRaw( "</ui>");
    file->close();
}

void myParser::writeProperty(const QString& name, const QString& type, const QString& value )
{
    xw->writeOpenTag( "property", AttrMap("name", name) );
    xw->writeTaggedString( type, value );
    xw->writeCloseTag( "property" );
}

void myParser::writeOpenProperty(const QString& name)
{
    xw->writeOpenTag( "property", AttrMap("name", name) );
}

void myParser::writeTaggedString(const QString& type, const QString& value )
{
    xw->writeTaggedString( type, value );
}

void myParser::writeCloseProperty()
{
    xw->writeCloseTag( "property" );
}

void myParser::writeOpenTag(const QString& type, const QString& cls, const QString& name )
{
    if(cls.size() != 0 && name.size() != 0) {
        xw->writeOpenTag( type, AttrMap("class", cls), AttrMap("name", name));
    } else {
        xw->writeOpenTag( type) ;
    }
}

void myParser::writeCloseTag(const QString& type)
{
    xw->writeCloseTag(type);
}

void myParser::Init(myParser* adlParser)
{
    myParserPtr = adlParser;
}

void myParser::writeMessage(char *mess) {
    Q_UNUSED(mess);
    //qDebug() << mess;
}

extern "C" myParser* C_writeOpenTag(myParser* p, char *type, char *cls, char *name )
{
    p->writeOpenTag(type, cls, name);
    return p;
}

extern "C" myParser* C_writeCloseTag(myParser* p, char *type)
{
    p->writeCloseTag(type);
    return p;
}

extern "C" myParser* C_writeProperty(myParser* p, char *name, char *type, char *value)
{
    p->writeProperty(name, type, value);
    return p;
}

extern "C" myParser* C_writeOpenProperty(myParser* p, char *name)
{
    p->writeOpenProperty(name);
    return p;
}

extern "C" myParser* C_writeTaggedString(myParser* p, char *type, char *value )
{
    p->writeTaggedString(type, value);
    return p;
}

extern "C" myParser* C_writeCloseProperty(myParser* p)
{
    p->writeCloseProperty();
    return p;
}

extern "C" myParser* C_adlParser(myParser* p, char* strng)
{
    p->writeMessage(strng);
    p->writeProperty( "name", "cstring", strng );
    return p;
}

extern "C" myParser* C_writeStyleSheet(myParser* p, int r, int g, int b)
{
    p->writeStyleSheet(r, g, b);
    return p;
}

/*
 *
 */
bool myParser::adl2ui(QString inputFile)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;

    if(inputFile.size() < 1) {
        qDebug() << "adl2ui -- sorry: no input file";
        return false;
    }

    QString openFile1, openFile2;
    QString outputFile;

    int found = inputFile.lastIndexOf(".adl");
    if (found != -1) {
        openFile1 = inputFile.mid(0, found);
    } else {
        openFile1 = inputFile;
    }

    openFile2 = openFile1;

    outputFile = openFile1.append(".ui");
    inputFile = openFile2.append(".adl");

    // when file exists open it
    QFileInfo fi(inputFile);
    if(!fi.exists()) {
        qDebug() << "adl2ui -- sorry, file" << inputFile << "does not exist";
        return false;
    }

    // get path for composite file parsing
    //qDebug() << fi.absolutePath();
    strcpy(filePrefix, fi.absolutePath().toLatin1().data());
    zindex = 0; // ZW
    // init adlParser
    Init(this);

    //get rid of path, we want to generate where we are
    if (!tmp_directory.isEmpty()){
        QFileInfo fileInfo(outputFile);
        outputFile=fileInfo.fileName();
        outputFile=tmp_directory+'/'+outputFile;
    }else{
      outputFile = outputFile.section('/',-1);
    }
    openFile(outputFile.toLatin1().data());

    // open input file
    FILE *filePtr = fopen(inputFile.toLatin1().data(), "r");
    FrameOffset offset;

    DisplayInfo *cdi = (DisplayInfo *) malloc(sizeof (DisplayInfo));
    offset.frameX = 0;
    offset.frameY = 0;
    cdi->filePtr = filePtr;

    // start parsing
    initScanning();
    tokenType = getToken(cdi, token);
    if (tokenType == T_WORD && !strcmp(token, "file")) {
        parseFile(cdi);
    } else {
        qDebug() << "dmDisplayListParse: Invalid .adl file (First block is not file block) file: " <<  inputFile;
        closeFile();
        return false;
    }
    // continue parsing
    tokenType = getToken(cdi, token);
    if (tokenType == T_WORD && !strcmp(token, "display")) {
        parseDisplay(cdi);
    } else {
        closeFile();
        qDebug() << "dmDisplayListParse: Invalid .adl file (Second block is not display block) " << inputFile;
        return false;
    }

    // Read the colormap if there.  Will also create cdi->dlColormap.
    tokenType = getToken(cdi, token);
    if (tokenType == T_WORD && (!strcmp(token, "color map") || !strcmp(token, "<<color map>>"))) {
        cdi->dlColormap = parseColormap(cdi, cdi->filePtr);
        if (cdi->dlColormap) {
            tokenType = getToken(cdi, token);
        } else {
            printf("Invalid .adl file (Cannot parse colormap file: %s\n", inputFile.toLatin1().data());
            closeFile();
            return false;
        }
    }
    // Proceed with parsing
    while (parseAndAppendDisplayList(cdi, &offset, token, tokenType) != T_EOF) {
        tokenType = getToken(cdi, token);
    }

    closeFile();
    return true;
}

int myParser::myMain(int argc, char *argv[])
{
    int	in, numargs;
    char inFile[80] = "";
    generateFlatFile = false;
    parsingCompositeFile = false;
    generateDeviceOnMenus = false;
    expandText = false;
    legendsForStripplot = true;

    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;

    for (numargs = argc, in = 1; in < numargs; in++) {
        if ( strcmp (argv[in], "-flat" ) == 0 ) {
            in++;
            generateFlatFile = true;
        }
        if ( strcmp (argv[in], "-deviceonmenu" ) == 0 ) {
            in++;
            generateDeviceOnMenus = true;
        }
        if ( strcmp (argv[in], "-nolegends" ) == 0 ) {
            in++;
            legendsForStripplot = false;
        }
        if ( strcmp (argv[in], "-expandtext" ) == 0 ) {
            in++;
            expandText= true;
        }
        if ( strcmp (argv[in], "-v" ) == 0 ) {
            printf("adl2ui version %s for %s\n", BUILDVERSION, BUILDARCH);
            exit(0);
        }
        if(!strcmp(argv[in],"-help") || !strcmp(argv[in],"-h") || !strcmp(argv[in],"-?")) {
            in++;
            printf("Usage:\n adl2ui [options] file\n");
            printf("[-flat] :        flat file will be generated, includes are integrated\n");
            printf("[-nolegends] :   no legends will be generated for the stripplots\n");
            printf("[-deviceonmenu] : part of pv will be used for the label of menu\n");
            printf("[-expandtext] : when textlabels do not fit, try this option\n");
            exit(1);
        }
        if (strncmp (argv[in], "-" , 1) == 0) {
            /* unknown application argument */
            printf("adl2ui -- Argument %d = [%s] is unknown! ",in,argv[in]);
            printf("possible are: '-flat and '-deviceonmenu' and '-nolegends' and '-v'\n");
            exit(-1);
        } else {
            printf("adl2ui -- file = <%s>\n", argv[in]);
            strcpy(inFile, argv[in]);
        }
    }

    if(generateFlatFile) printf("adl2ui -- a flat file will be generated\n");
    if(generateDeviceOnMenus)printf("adl2ui -- device name will be put on menus\n");
    if(!legendsForStripplot)printf("adl2ui -- legends will not be set for stripplot\n");
    if(expandText)printf("adl2ui -- try to adjust lenght of labels thta do not fit\n");

    // input and out files
    QString inputFile = inFile;
    if(inputFile.size() < 1) {
        qDebug() << "adl2ui -- sorry: no input file";
        exit(-1);
    }

    QString openFile1, openFile2;
    QString outputFile;

    int found = inputFile.lastIndexOf(".adl");
    if (found != -1) {
        openFile1 = inputFile.mid(0, found);
    } else {
        openFile1 = inputFile;
    }

    openFile2 = openFile1;

    outputFile = openFile1.append(".ui");
    inputFile = openFile2.append(".adl");

    // when file exists open it
    QFileInfo fi(inputFile);
    if(!fi.exists()) {
        qDebug() << "adl2ui -- sorry, file" << inFile << "does not exist";
        exit(-1);
    }

    // get path for composite file parsing
    //qDebug() << fi.absolutePath();
    strcpy(filePrefix, fi.absolutePath().toLatin1().data());

    // init adlParser
    myParser *adlParser = new myParser;
    adlParser->Init(adlParser);

    //get rid of path, we want to generate where we are
    outputFile = outputFile.section('/',-1);
    adlParser->openFile(outputFile.toLatin1().data());

    // open input file
    FILE *filePtr = fopen(inputFile.toLatin1().data(), "r");
    FrameOffset offset;

    DisplayInfo *cdi = (DisplayInfo *) malloc(sizeof (DisplayInfo));
    offset.frameX = 0;
    offset.frameY = 0;
    cdi->filePtr = filePtr;

    // start parsing
    tokenType = getToken(cdi, token);
    if (tokenType == T_WORD && !strcmp(token, "file")) {
        parseFile(cdi);
    } else {
        qDebug() << "dmDisplayListParse: Invalid .adl file (First block is not file block) file: " <<  inputFile;
        return 0;
    }
    // continue parsing
    tokenType = getToken(cdi, token);
    if (tokenType == T_WORD && !strcmp(token, "display")) {
        parseDisplay(cdi);
    } else {
        qDebug() << "dmDisplayListParse: Invalid .adl file (Second block is not display block) " << inputFile;
        return 0;
    }

    // Read the colormap if there.  Will also create cdi->dlColormap.
    tokenType = getToken(cdi, token);
    if (tokenType == T_WORD && (!strcmp(token, "color map") || !strcmp(token, "<<color map>>"))) {
        cdi->dlColormap = parseColormap(cdi, cdi->filePtr);
        if (cdi->dlColormap) {
            tokenType = getToken(cdi, token);
        } else {
            printf("Invalid .adl file (Cannot parse colormap file: %s\n", inFile);
            return 0;
        }
    }

    // Proceed with parsing
    while (parseAndAppendDisplayList(cdi, &offset, token, tokenType) != T_EOF) {
        tokenType = getToken(cdi, token);
    }

    // close output file
    adlParser->closeFile();

    return 0;
}

QString myParser::getTmp_directory() const
{
    return tmp_directory;
}

void myParser::setTmp_directory(const QString &value)
{
    tmp_directory = value;
}

