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

#if defined(_MSC_VER)
   #define _MATH_DEFINES_DEFINED
   #define NOMINMAX
   #include <windows.h>
   #define strdup _strdup
#endif

#include "searchfile.h"

#include "fileopenwindow.h"
#include "fileFunctions.h"
#include "QDebug"
#include <QFileDialog>
#include <QLocale>
#include <signal.h>
#include <iostream>
#include <stdlib.h>
#include "pipereader.h"

#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
#include <QApplication>
#else
#include <QtGui/QApplication>
#endif
/*
class MyApplication: public QApplication
{

public:
    MyApplication (int &argc, char ** argv ): QApplication ( argc, argv ) {}
    ~MyApplication() {}

private:
    virtual bool notify(QObject* receiver, QEvent* e)  {

        try {
            //qDebug() << "## trying to notify application..";
            return QApplication::notify(receiver, e);
        } catch (std::exception& e) {
            qDebug() << "## !!FATAL!! Exception thrown: " << e.what();
        }

        return false;
    }
};
*/

static void unixSignalHandler(int signum) {

    Q_UNUSED(signum);

    //qDebug("DBG: main.cpp::unixSignalHandler(). signal = %s\n", strsignal(signum));

    /*
     * Make sure your Qt application gracefully quits.
     * NOTE - purpose for calling qApp->exit(0):
     *      1. Forces the Qt framework's "main event loop `qApp->exec()`" to quit looping.
     *      2. Also emits the QCoreApplication::aboutToQuit() signal. This signal is used for cleanup code.
     */
    QCoreApplication::exit(0);
}

extern bool HTTPCONFIGURATOR;

static void createMap(QMap<QString, QString> &map, const QString& option)
{
    //qDebug() << "treat option" << option;
    // option of type KEY1=VALUE1,KEY2=VALUE2,KEY3=VALUE3
    if(option != NULL) {
        QStringList vars = option.split(",", QString::SkipEmptyParts);
        for(int i=0; i< vars.count(); i++) {
            int pos = vars.at(i).indexOf("=");
            if(pos != -1) {
                QString key = vars.at(i).mid(0, pos);
                QString value = vars.at(i).mid(pos+1);
                map.insert(key.trimmed(), value);
            } else {
                qDebug() <<"option" <<  option << "could not be parsed";
            }
        }
    }
    //qDebug() << "inserted int map from option:" << option;
    //qDebug() << "resulting map=" << map;
}

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(caQtDM);
#ifdef MOBILE
    Q_INIT_RESOURCE(qtcontrols);
#endif

    //MyApplication app(argc, argv);

    QApplication app(argc, argv);
    QApplication::setOrganizationName("Paul Scherrer Institut");
    QApplication::setApplicationName("caQtDM");

#ifdef MOBILE_ANDROID
    app.setStyle(QStyleFactory::create("fusion"));
#endif

    // we do not want numbers with a group separators
    QLocale loc = QLocale::system();
    loc.setNumberOptions(QLocale::OmitGroupSeparator);
    loc.setDefault(loc);

    QString fileNameStylesheet = "";
    QString fileName = "";
    QString macroString = "";
    QString geometry = "";
    QString macroFile = "";
    QMap<QString, QString> options;
    options.clear();

#if defined(_MSC_VER)
    if (AttachConsole(ATTACH_PARENT_PROCESS)){
        SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE),ENABLE_QUICK_EDIT_MODE| ENABLE_EXTENDED_FLAGS);
        freopen("CON", "w", stdout);
        freopen("CON", "w", stderr);
        freopen("CON", "r", stdin);
    }
#endif

    searchFile *s = new searchFile("caQtDM_stylesheet.qss");
    QString fileNameFound = s->findFile();
    if(fileNameFound.isNull()) {
        printf("caQtDM -- file <caQtDM_stylesheet.qss> could not be loaded, is 'CAQTDM_DISPLAY_PATH' <%s> defined?\n", qasc(s->displayPath()));
    } else {
        QFile file(fileNameFound);
        file.open(QFile::ReadOnly);
        QString StyleSheet = QLatin1String(file.readAll());
        printf("caQtDM -- file <caQtDM_stylesheet.qss> loaded as the default application stylesheet\n");
        app.setStyleSheet(StyleSheet);
        file.close();
    }

    int	in, numargs;
    bool attach = false;
    bool minimize= false;
    bool printscreen = false;
    bool resizing = true;

    for (numargs = argc, in = 1; in < numargs; in++) {
        qDebug() << argv[in];
        if ( strcmp (argv[in], "-display" ) == 0 ) {
            in++;
            printf("caQtDM -- display <%s>\n", argv[in]);
        } else if ( strcmp (argv[in], "-macro" ) == 0 ) {
            in++;
            printf("caQtDM -- macro <%s>\n", argv[in]);
            macroString = QString(argv[in]);
        } else if ( strcmp (argv[in], "-attach" ) == 0 ) {
            printf("caQtDM -- will attach to another caQtDM if running\n");
            attach = true;
        } else if ( strcmp (argv[in], "-noMsg" ) == 0 ) {
            printf("caQtDM -- will minimize its main windows\n");
            minimize = true;
        } else if( strcmp (argv[in], "-macrodefs" ) == 0) {
            in++;
            printf("caQtDM -- will load macro string from file <%s>\n", argv[in]);
            macroFile = QString(argv[in]);
        } else if ( strcmp (argv[in], "-stylefile" ) == 0 ) {
            in++;
            printf("caQtDM -- will replace the default stylesheet with stylesheet <%s>\n", argv[in]);
            fileNameStylesheet = QString(argv[in]);
        } else if ( strcmp (argv[in], "-x" ) == 0 ) {

        } else if ( strcmp (argv[in], "-displayFont" ) == 0 ) {
             in++;
        } else if(!strcmp(argv[in],"-help") || !strcmp(argv[in],"-h") || !strcmp(argv[in],"-?")) {
             in++;
                 printf("Usage:\n"
                   "  caQtDM[X options]\n"
                   "  [-help | -h | -?]\n"
                   "  [-x]\n"
                   "  [-attach]\n"
                   "  [-noMsg]\n"
                   "  [-stylefile filename] will replace the default stylesheet with the specified file (works only when not attaching)\n"
                   "  [-macro \"xxx=aaa,yyy=bbb, ...\"]\n"
                   "  [-macrodefs filename] will load macro definitions from file\n"
                   "  [-dg [<width>x<height>][+<xoffset>-<yoffset>]\n"
                   "  [-httpconfig] will display a network configuration screen at startup\n"
                   "  [-print] will print file and exit\n"
                   "  [-noResize] will prevent resizing\n"
                   "  [-cs defaultcontrolsystempluginname]\n"
                   "  [-option \"xxx=aaa,yyy=bbb, ...\"] options for cs plugins\n"
                   "  [-url url] will look for files on the specified url and download them to a local directory\n"
                   "  [-emptycache] will empty the local cache used for downloading"
                   "  [file]\n"
                   "  [&]\n"
                   "\n"
                   "  -x -displayFont -display are ignored !\n\n"
                   "  on linux plattforms, ui data can be piped to caQtDM, but then -httpconfig & -attach will not work\n\n");
                 exit(1);
        } else if((!strcmp(argv[in],"-displayGeometry")) || (!strcmp(argv[in],"-dg"))) {
            // [-dg [xpos[xypos]][+xoffset[+yoffset]]
             in++;
             geometry = QString(argv[in]);
        } else if(!strcmp(argv[in], "-print")) {
             printscreen = true;
             minimize = true;
             resizing = false;
        } else if(!strcmp(argv[in], "-noResize")) {
            resizing = false;
        } else if(!strcmp(argv[in], "-httpconfig")) {
            HTTPCONFIGURATOR = true;
        } else if(!strcmp(argv[in], "-url")) {
            in++;
            setenv("CAQTDM_URL_DISPLAY_PATH", argv[in], 1);
        } else if(!strcmp(argv[in], "-emptycache")) {
            Specials specials;
            QString path =  specials.getStdPath();
            fileFunctions filefunction;
            path.append("/");
            filefunction.removeFilesInTree(path);
            printf("caQtDM -- cache @ %s with ui & graphic files has been emptied\n", qasc(path));
        } else if(!strcmp(argv[in], "-cs")) {
            in++;
            options.insert("defaultPlugin", QString(argv[in]));
        } else if ( strcmp (argv[in], "-option" ) == 0 ) {
            in++;
            printf("caQtDM -- option <%s>\n", argv[in]);
            createMap(options, QString(argv[in]));
        } else if (strncmp (argv[in], "-" , 1) == 0) {
            /* unknown application argument */
            printf("caQtDM -- Argument %d = [%s] is unknown!, possible -attach -macro -noMsg -stylefile -dg -x -print -httpconfig -noResize\n",in,argv[in]);
        } else {
            printf("caQtDM -- file = <%s>\n", argv[in]);
            fileName = QString(argv[in]);
            break;
        }
    }

    // get data from pipe if any (ui data can be piped to this application, for linux at this time)
    // only when no file is given, attaching is not allowed, in order to get rid of the temporary file when exit
#if defined linux || defined TARGET_OS_MAC
    if(fileName.length() <= 0) {

        QEventLoop *loop = new QEventLoop();
        PipeReader *reader = new PipeReader(loop);
        loop->exec();

        QString newFilename = reader->getTemporaryFilename();
        if(newFilename.size() > 0) {
            fileName = newFilename;
            attach = false;
            HTTPCONFIGURATOR = false;
        }
        delete reader;
        delete loop;
        //qDebug() << "use now file" << fileName;
    }
#endif

    // must be always true for mobile plattforms
#ifdef MOBILE
     HTTPCONFIGURATOR = true;
#endif

     if(fileNameStylesheet.length() > 0) {
        s = new searchFile(fileNameStylesheet);
        fileNameFound = s->findFile();
        if(fileNameFound.isNull()) {
            printf("caQtDM -- file <stylesheet.qss> could not be loaded, is 'CAQTDM_DISPLAY_PATH' <%s> defined?\n", qasc(s->displayPath()));
        } else {
            QFile file(fileNameFound);
            file.open(QFile::ReadOnly);
            QString StyleSheet = QLatin1String(file.readAll());
            printf("caQtDM -- file <%s> replaced the default stylesheet\n", qasc(fileNameStylesheet));
            app.setStyleSheet(StyleSheet);
            file.close();
        }
    }

    // load macro definitions from file (located in this directory or in the caQTDM_DISPLAY_PATH)
    if(macroFile.length() > 0) {
        s = new searchFile(macroFile);
        fileNameFound = s->findFile();
        if(fileNameFound.isNull()) {
            printf("caQtDM -- file <stylesheet.qss> could not be loaded, is 'CAQTDM_DISPLAY_PATH' <%s> defined?\n", qasc(s->displayPath()));
        } else {
            QFile file(fileNameFound);
            file.open(QFile::ReadOnly);
            macroString = QLatin1String(file.readAll());
            file.close();
        }
    }

#ifdef IO_OPTIMIZED_FOR_TABWIDGETS
    printf("caQtDM -- viewer will disable monitors for hidden pages of QTabWidgets, in case of problems\n");
    printf("          you may disable this by not defining IO_OPTIMIZED_FOR_TABWIDGETS in qtdefs.pri\n");
#else
    printf("caQtDM -- viewer will not disable monitors for hidden pages of QTabWidgets\n");
    printf("          you may enable this by defining IO_OPTIMIZED_FOR_TABWIDGETS in qtdefs.pri\n");
#endif

#ifndef CONFIGURATOR
    QString displayPath = (QString)  qgetenv("CAQTDM_URL_DISPLAY_PATH");
    if(displayPath.length() > 0) {
         printf("caQtDM -- files will be downloaded from <%s> when not locally found\n", qasc(displayPath));
    } else {
        printf("caQtDM -- files will not be downloaded from an url when not locally found, while CAQTDM_URL_DISPLAY_PATH is not defined\n");
    }
#endif

    FileOpenWindow window (0, fileName, macroString, attach, minimize, geometry, printscreen, resizing, options);
    window.setWindowIcon (QIcon(":/caQtDM.ico"));
    window.show();
    window.move(0,0);

    if (signal(SIGINT, unixSignalHandler) == SIG_ERR) {
        qFatal("ERR - %s(%d): An error occurred while setting a signal handler.\n", __FILE__,__LINE__);
    }
    if (signal(SIGTERM, unixSignalHandler) == SIG_ERR) {
        qFatal("ERR - %s(%d): An error occurred while setting a signal handler.\n", __FILE__,__LINE__);
    }

    QObject::connect(&app, SIGNAL(aboutToQuit()), &window, SLOT(doSomething()));

    return app.exec();
}
