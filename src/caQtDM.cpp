#include <QtGui/QApplication>
#include "dmsearchfile.h"
//******************************************************************************
// Copyright (c) 2012 Paul Scherrer Institut PSI), Villigen, Switzerland
// Disclaimer: neither  PSI, nor any of their employees makes any warranty
// or assumes any legal liability or responsibility for the use of this software
//******************************************************************************
//******************************************************************************
//
//     Author : Anton Chr. Mezger
//
//******************************************************************************

#if defined(_MSC_VER)
   #include <windows.h>
#endif

#include "dmsearchfile.h"
#include <QtGui/QApplication>
#include "fileopenwindow.h"
#include "QDebug"
#include <QFileDialog>
#include <signal.h>

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

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(caQtDM);

    QApplication app(argc, argv);

    QString fileName = "";
    QString macroString = "";
    QString geometry = "";

    dmsearchFile *s = new dmsearchFile("caQtDM_stylesheet.qss");
    QString fileNameFound = s->findFile();
    if(fileNameFound.isNull()) {
        printf("caQtDM -- file <caQtDM_stylesheet.qss> could not be loaded, is 'CAQTDM_DISPLAY_PATH' <%s> defined?\n", s->displayPath().toAscii().constData());
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
    bool nostyles = false;
    bool printscreen = false;
    bool resizing = true;

    for (numargs = argc, in = 1; in < numargs; in++) {
        //qDebug() << argv[in];
        if ( strcmp (argv[in], "-display" ) == 0 ) {
            in++;
            printf("caQtDM -- display <%s>\n", strdup(argv[in]));
        } else if ( strcmp (argv[in], "-macro" ) == 0 ) {
            in++;
            printf("caQtDM -- macro <%s>\n", strdup(argv[in]));
            macroString = QString(argv[in]);
        } else if ( strcmp (argv[in], "-attach" ) == 0 ) {
            printf("caQtDM -- will attach to another caQtDM if running\n");
            attach = true;
        } else if ( strcmp (argv[in], "-noMsg" ) == 0 ) {
            printf("caQtDM -- will minimize its main windows\n");
            minimize = true;
        } else if ( strcmp (argv[in], "-noStyles" ) == 0 ) {
            printf("caQtDM -- will not replace the default application stylesheet caQtDM_stylesheet.qss\n");
            nostyles = true;
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
                   "  [-noStyles]      works only when not attaching\n"
                   "  [-macro \"xxx=aaa,yyy=bbb, ...\"]\n"
                   "  [-dg [xpos[xypos]][+xoffset[+yoffset]]\n"
                   "  [-print] will print file and exit\n"
                   "  [-noResize] will prevent resizing, works only when not attaching\n"
                   "  [file]\n"
                   "  [&]\n"
                   "\n"
                   "  -x -displayFont -display are ignored !\n\n");
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
        } else if (strncmp (argv[in], "-" , 1) == 0) {
            /* unknown application argument */
            printf("caQtDM -- Argument %d = [%s] is unknown!, possible -attach -macro -noMsg -noStyles -dg -x -print\n",in,argv[in]);
        } else {
            printf("caQtDM -- file = <%s>\n", argv[in]);
            fileName = QString(argv[in]);
            break;
        }
    }

    if(!nostyles) {
        s = new dmsearchFile("stylesheet.qss");
        fileNameFound = s->findFile();
        if(fileNameFound.isNull()) {
            printf("caQtDM -- file <stylesheet.qss> could not be loaded, is 'CAQTDM_DISPLAY_PATH' <%s> defined?\n", s->displayPath().toAscii().constData());
        } else {
            QFile file(fileNameFound);
            file.open(QFile::ReadOnly);
            QString StyleSheet = QLatin1String(file.readAll());
            printf("caQtDM -- file <stylesheet.qss> replaced the default stylesheet\n");
            app.setStyleSheet(StyleSheet);
            file.close();
        }
    }

    FileOpenWindow window (0, fileName, macroString, attach, minimize, geometry, printscreen, resizing);
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




