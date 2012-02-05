/* 
 * File:   myParser.cpp
 * Author: mezger
 *
 * Created on January 4, 2012, 4:58 PM
 */

#include <cstdlib>
#include "parser.h"

#include <qfile.h>
#include "XmlWriter.h"

#include <QDebug>

extern "C" TOKEN parseAndAppendDisplayList(DisplayInfo *displayInfo, FrameOffset *offset, char *firstToken, TOKEN firstTokenType);
extern "C" TOKEN getToken(DisplayInfo *displayInfo, char *word);
extern "C" void parseFile(DisplayInfo *displayInfo);
extern "C" void *parseDisplay(DisplayInfo *displayInfo);
extern "C" DlColormap *parseColormap(DisplayInfo *displayInfo, FILE *filePtr);
//extern "C" DlList *createDlList();

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
    myParser *parser;
    QFile *file;
    QString StyleSheet;
    void test();
    void Init(myParser* parser);
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

    qDebug() << xw;
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

void myParser::closeFile()
{
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

void myParser::Init(myParser* parser)
{
    myParserPtr = parser;
}

void myParser::writeMessage(char *mess) {
    qDebug() << mess;
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

extern "C" myParser* C_Parser(myParser* p, char* strng)
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
    char inFile[80];
    char parsedFile[80];
    for (numargs = argc, in = 1; in < numargs; in++) {
    if (strncmp (argv[in], "-" , 1) == 0) {
                /* unknown application argument */
                printf("parser -- Argument %d = [%s] is unknown!\n",in,argv[in]);
                in++;
        } else {
            printf("parser -- file = <%s>\n", argv[in]);
            strcpy(inFile, argv[in]);
            strcpy(parsedFile, inFile);
        }
    }

    parsingCompositeFile = false;

    strcat(inFile, ".adl");
    strcat(parsedFile, ".ui");

    myParser *parser = new myParser;
    parser->Init(parser);
    parser->openFile(parsedFile);

    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;


    FILE *filePtr = fopen(inFile, "r");
    FrameOffset offset;

    DisplayInfo *cdi = (DisplayInfo *) malloc(sizeof (DisplayInfo));

    //cdi->dlElementList = createDlList();
    //cdi->selectedDlElementList = createDlList();
    offset.frameX = offset.frameXprv = 0;
    offset.frameY = offset.frameYprv = 0;

    cdi->filePtr = filePtr;

    tokenType = getToken(cdi, token);
    if (tokenType == T_WORD && !strcmp(token, "file")) {
        //cdi->dlFile =
                parseFile(cdi);
        //if (cdi->dlFile) {
            //cdi->versionNumber = cdi->dlFile->versionNumber;
            //strcpy(cdi->dlFile->name, inFile);
        //} else {
        //    printf("dmDisplayListParse: Out of memory\n"
        //           "  file: %s\n", inFile);
        //    return 0;
        //}
    } else {
        printf("dmDisplayListParse: Invalid .adl file "
               "(First block is not file block)\n"
               "  file: %s\n", inFile);
        return 0;
    }

    printf("  File: %s\n", inFile);

    tokenType = getToken(cdi, token);
    if (tokenType == T_WORD && !strcmp(token, "display")) {
        parseDisplay(cdi);
    } else {
        printf("dmDisplayListParse: Invalid .adl file (Second block is not display block) file: %s\n", inFile);
        return 0;
    }

    /* Read the colormap if there.  Will also create cdi->dlColormap. */
    tokenType = getToken(cdi, token);

    if (tokenType == T_WORD && (!strcmp(token, "color map") || !strcmp(token, "<<color map>>"))) {
        printf("parse color map\n");
        cdi->dlColormap = parseColormap(cdi, cdi->filePtr);

        if (cdi->dlColormap) {
            tokenType = getToken(cdi, token);
        } else {
            printf("Invalid .adl file (Cannot parse colormap file: %s\n", inFile);
            return 0;
        }
    }

    /* Proceed with parsing */

    while (parseAndAppendDisplayList(cdi, &offset, token, tokenType) != T_EOF) {
        tokenType = getToken(cdi, token);
    }

    parser->closeFile();

    qDebug() << "finished";

    return 0;
}

