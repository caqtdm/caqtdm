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

#include <QDebug>
#include <QPaintEvent>
#include <QPainter>
#include <QApplication>
#include "cainclude.h"
#include "searchfile.h"
#include "fileFunctions.h"

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
   delete layout;
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
    QUiLoader loader;

    fileFunctions filefunction;

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
            //printf("we have to scan pep file %s\n", newFileName.toLatin1().constData());

            // this will check for file existence and when an url is defined, download the file from a http server
            filefunction.checkFileAndDownload(newFileName);

            searchFile *s = new searchFile(newFileName);
            QString fileNameFound = s->findFile();

            // file was not found, remove previous widget if any
            if(fileNameFound.isNull()) {
                //printf("file not found\n");
                removeIncludedWidget();
                return;
            } else {
                //printf("file %s has been found\n", fileNameFound.toLatin1().constData());
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

        // this will check for file existence and when an url is defined, download the file from a http server
        filefunction.checkFileAndDownload(fileName);
        searchFile *s = new searchFile(fileName);
        QString fileNameFound = s->findFile();

        // file was not found, remove previous widget if any
        if(fileNameFound.isNull()) {
            //printf("file not found\n");
            removeIncludedWidget();
            return;
        } else {
            //printf("file %s has been found\n", fileNameFound.toLatin1().constData());
        }
        delete s;

        // file was found, remove previous widget if any
        removeIncludedWidget();

        // load new file
        QFile *file = new QFile;
        file->setFileName(fileNameFound);
        file->open(QFile::ReadOnly);
        thisLoadedWidget = loader.load(file, thisParent);
        delete file;
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






