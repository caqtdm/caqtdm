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

#include "QDebug"
#include <QFileDialog>

#include "fileopenwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QString fileName = "";
    QString macroString = "";

    dmsearchFile *s = new dmsearchFile("stylesheet.qss");
    if(s->findFile().isNull()) {
        printf("caQtDM --file <stylesheet.qss> could not be loaded, is 'CAQTDM_DISPLAY_PATH' <%s> defined?\n", s->displayPath().toAscii().constData());
    } else {
        QFile file(s->findFile());
        file.open(QFile::ReadOnly);
        QString StyleSheet = QLatin1String(file.readAll());
        app.setStyleSheet(StyleSheet);
        file.close();
    }

    int	in, numargs;
    bool attach;
    attach = false;
    for (numargs = argc, in = 1; in < numargs; in++) {
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
        } else if (strncmp (argv[in], "-" , 1) == 0) {
            /* unknown application argument */
            printf("caQtDM -- Argument %d = [%s] is unknown!\n",in,argv[in]);
        } else {
            printf("caQtDM -- file = <%s>\n", argv[in]);
            fileName = QString(argv[in]);
            break;
        }
    }

    FileOpenWindow window (0, fileName, macroString, attach);
    window.show();


    return app.exec();
}




