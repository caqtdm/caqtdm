/* 
 * File:   myParser.cpp
 * Author: mezger
 *
 * Created on January 4, 2012, 4:58 PM
 */

#include <cstdlib>
#include "parser.h"
#include <iostream>
#include <qfile.h>
#include "XmlWriter.h"
#include <QFileDialog>
#include <QDebug>


extern "C" TOKEN parseAndAppendDisplayList(DisplayInfo *displayInfo, FrameOffset *offset, char *firstToken, TOKEN firstTokenType);
extern "C" TOKEN getToken(DisplayInfo *displayInfo, char *word);
extern "C" void parseFile(DisplayInfo *displayInfo);
extern "C" void *parseDisplay(DisplayInfo *displayInfo);
extern "C" DlColormap *parseColormap(DisplayInfo *displayInfo, FILE *filePtr);
extern "C" void Qt_writeZorder();
extern "C" int parsingCompositeFile;
extern "C" int generateFlatFile;
extern "C" int generateDeviceOnMenus;

typedef char string40[40];
typedef struct _zOrder {
    int indx;
    int vis;
    string40 z;
} zOrder;

extern "C" zOrder zorder[1000];
extern "C" int zindex;
extern "C" char filePrefix[128];

class myParser {

public:

    myParser ();
    void openFile(char *outFile);
    void closeFile();
    void writeProperty(const QString& name, const QString& type, const QString& value );
    void writeOpenProperty(const QString& name);
    void writeTaggedString(const QString& type, const QString& value );
    void writeCloseProperty();
    void writeStyleSheet();

    void writeOpenTag(const QString& type, const QString& cls = "", const QString& name = "");
    void writeCloseTag(const QString& type);
    XmlWriter *xw;
    myParser *adlParser;
    QFile *file;
    QString StyleSheet;
    void test();
    void Init(myParser* adlParser);
    void writeMessage(char *mess);

private:

};

// pointer used by external C
myParser* myParserPtr;

// constructor
myParser::myParser () {
}


void myParser::openFile(char *outFile)
{
    // open stylesheet
    QFile stylefile("stylesheet.qss");
    stylefile.open(QFile::ReadOnly);
    StyleSheet = QLatin1String(stylefile.readAll());
    stylefile.close();

    file = new QFile(outFile);
    if (!file->open(QIODevice::WriteOnly | QIODevice::Text)) exit(0);

    xw = new XmlWriter(file);

    xw->setAutoNewLine(true);

    xw->writeRaw( "<ui version=\"4.0\">" );
    xw->newLine();
    xw->writeTaggedString( "class", "MainWindow" );
    xw->writeOpenTag("widget", AttrMap("class", "QMainWindow"), AttrMap("name", "MainWindow"));
}

void myParser::writeStyleSheet()
{
    xw->writeOpenTag( "property", AttrMap("name", "styleSheet") );
    xw->writeTaggedString( "string",  StyleSheet);
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
    qStableSort(myvector.begin(), myvector.end(), compareFunc);
    for (it=myvector.begin(); it!=myvector.end(); ++it) {
        qDebug() << "sorted" << it->indx << it->vis << it->z;
        xw->writeTaggedString("zorder", it->z);
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

extern "C" myParser* C_writeStyleSheet(myParser* p)
{
    p->writeStyleSheet();
    return p;
}

/*
 * 
 */
class myParser;

int main(int argc, char *argv[])
{
    int	in, numargs;
    char inFile[80] = "";
    generateFlatFile = false;
    parsingCompositeFile = false;
    generateDeviceOnMenus = false;
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;

    for (numargs = argc, in = 1; in < numargs; in++) {
        if ( strcmp (argv[in], "-flat" ) == 0 ) {
            in++;
            generateFlatFile = True;
        }
        if ( strcmp (argv[in], "-deviceonmenu" ) == 0 ) {
            in++;
            generateDeviceOnMenus = True;
        }
        if (strncmp (argv[in], "-" , 1) == 0) {
            /* unknown application argument */
            printf("adlParser: Argument %d = [%s] is unknown! ",in,argv[in]);
            printf("possible are: '-flat and '-deviceonmenu'\n");
            exit(-1);
        } else {
            printf("adlParser: file = <%s>\n", argv[in]);
            strcpy(inFile, argv[in]);
        }
    }

    if(generateFlatFile) printf("adlParser: a flat file will be generated\n");
    if(generateDeviceOnMenus)printf("adlParser: device name will be put on menus\n");

    // input and out files
    QString inputFile = inFile;
    if(inputFile.size() < 1) {
        qDebug() << "adlParser -- sorry: no input file";
        exit(-1);
    }

    QStringList openFile = inputFile.split(".", QString::SkipEmptyParts);
    inputFile = openFile[0].append(".adl");
    openFile = inputFile.split(".", QString::SkipEmptyParts);
    QString outputFile = openFile[0].append(".ui");

    // when file exists open it
    QFileInfo fi(inputFile);
    if(!fi.exists()) {
        qDebug() << "adlParser: sorry, file" << inFile << "does not exist";
        exit(-1);
    }

    // get path for composite file parsing
    //qDebug() << fi.absolutePath();
    strcpy(filePrefix, fi.absolutePath().toAscii().data());

    // init adlParser
    myParser *adlParser = new myParser;
    adlParser->Init(adlParser);

    //get rid of path, we want to generate where we are
    outputFile = outputFile.section('/',-1);
    adlParser->openFile(outputFile.toAscii().data());

    // open input file
    FILE *filePtr = fopen(inputFile.toAscii().data(), "r");
    FrameOffset offset;

    DisplayInfo *cdi = (DisplayInfo *) malloc(sizeof (DisplayInfo));
    offset.frameX = offset.frameXprv = 0;
    offset.frameY = offset.frameYprv = 0;
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

