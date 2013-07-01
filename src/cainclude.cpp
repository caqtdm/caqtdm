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

#include <QDebug>
#include <QPaintEvent>
#include <QPainter>
#include <QApplication>
#include "cainclude.h"
#include "searchfile.h"

caInclude::caInclude(QWidget *parent) : QWidget(parent)
{
    thisLoadedWidget = (QWidget*) 0;
    thisParent = parent;
    newFileName="";
    thisLineSize = 1;
    setBackground(Qt::black);
    setVisibility(StaticV);
    layout = new QVBoxLayout;
    layout->setMargin(0);

    // when called from designer, we would like to see the included widgets
    loadIncludes = false;
    QVariant source = qApp->property("APP_SOURCE").value<QVariant>();

    // next code will only be executed when in designer
    // in other apps we load everything there while taking care of the macros
    if(source.isValid()) {
        if(!source.isNull()) {
            QString test = source.toString();
            if(test.contains("DESIGNER")) {
                loadIncludes = true;
            }
        }
    }
}

caInclude::~ caInclude()
{

}

void caInclude::setBackground(QColor c)
{
    thisBackColor = c;
    update();
}

void caInclude::removeIncludedWidget()
{
    if(thisLoadedWidget != (QWidget*) 0) {
        printf("and destroy the loaded widget\n");
        layout->removeWidget(thisLoadedWidget);
        delete thisLoadedWidget;
        thisLoadedWidget = (QWidget*) 0;
        prvFileName = "";
    }
}

void caInclude::setFileName(QString const &filename)
{
    QFile *file = new QFile;
    QUiLoader loader;
    newFileName = filename.trimmed();

    // load widgets from includes
    if(loadIncludes) {
        if(newFileName.size() < 1) {
            removeIncludedWidget();
            return;
        }

        if(!newFileName.contains(".")) {
            removeIncludedWidget();
            return;
        }

        if(!prvFileName.isNull() && !newFileName.isNull()) {
            int indx1 = prvFileName.indexOf(".");
            int indx2 = newFileName.indexOf(".");
            if(indx1 != -1 && indx2 != -1) {
                QString file1 = prvFileName.left(indx1);
                QString file2 = newFileName.left(indx2);
                if(file1 == file2) return;
            }
        }

#ifdef PRC
        if(newFileName.contains(".prc")) {
            // we have a pep file to scan
            //printf("we have to scan pep file %s\n", newFileName.toAscii().constData());
            searchFile *s = new searchFile(newFileName);
            QString fileNameFound = s->findFile();

            // file was not found, remove previous widget if any
            if(fileNameFound.isNull()) {
                //printf("file not found\n");
                removeIncludedWidget();
                return;
            } else {
                //printf("file %s has been found\n", fileNameFound.toAscii().constData());
            }
            delete s;

            // file was found, remove previous widget if any
            removeIncludedWidget();

            // load new file

            ParsePepFile *parsefile = new ParsePepFile(fileNameFound);
            thisLoadedWidget = parsefile->load(thisParent);
            if(thisLoadedWidget == (QWidget*) 0) return;
            layout->addWidget(thisLoadedWidget);
            setLayout(layout);
            thisLoadedWidget->show();
            return;
        }
#endif

        QStringList openFile = newFileName.split(".", QString::SkipEmptyParts);
        QString fileName = openFile[0].append(".ui");
        searchFile *s = new searchFile(fileName);
        QString fileNameFound = s->findFile();

        // file was not found, remove previous widget if any
        if(fileNameFound.isNull()) {
            //printf("file not found\n");
            removeIncludedWidget();
            return;
        } else {
            //printf("file %s has been found\n", fileNameFound.toAscii().constData());
        }
        delete s;

        // file was found, remove previous widget if any
        removeIncludedWidget();

        // load new file
        file->setFileName(fileNameFound);
        file->open(QFile::ReadOnly);
        thisLoadedWidget = loader.load(file, thisParent);
        if(thisLoadedWidget == (QWidget*) 0) return;
        layout->addWidget(thisLoadedWidget);
        setLayout(layout);
        thisLoadedWidget->show();
        prvFileName = newFileName;
    }
}

void caInclude::setLineSize(int size )
{
    thisLineSize = size;
    update();
}

void caInclude::paintEvent( QPaintEvent *event )
{
    Q_UNUSED(event);
    if(thisLineSize > 0) {
        QPainter painter( this );
        painter.setPen( QPen( getBackground(), thisLineSize, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ) );
        painter.drawRect(0, 0, width() - 1, height() -1);
    }
}






