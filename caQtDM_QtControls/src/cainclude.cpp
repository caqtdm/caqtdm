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

#include <QPaintEvent>
#include <QPainter>
#include <QApplication>
#include <math.h>
#include "cainclude.h"
#include "searchfile.h"
#include "fileFunctions.h"

caInclude::caInclude(QWidget *parent) : QWidget(parent)
{
    thisLoadedWidgets.clear();
    thisParent = parent;
    newFileName="";
    thisLineSize = 1;
    thisItemCount = 1;
    thisMaxLines = 1;
    thisStacking = Row;
    setBackground(Qt::black);
    setVisibility(StaticV);
    gridLayout = new QGridLayout;
    gridLayout->setMargin(0);
    gridLayout->setSpacing(0);

    setPropertyVisible(maximumLines, false);

    // when called from designer, we would like to see the included widgets
    loadIncludes = false;
    QVariant source = qApp->property("APP_SOURCE").value<QVariant>();

    // in the designer application through the command line we could have set this property to true or false
    // in order to do that add following code to qdesigner.cpp
/*
    if (argument ==  QLatin1String("-includesOutlined")) {
        if (++it == acend) {
            qWarning("** WARNING The option -includesOutlined requires a value true or false");
            return false;
        } else {
            QString boolValue = QString::fromLatin1(it->toLocal8Bit());
            if(boolValue.toLower() == "true")
               qApp->setProperty("includesOutlined", QVariant(QString("TRUE")));
            else
               qApp->setProperty("includesOutlined", QVariant(QString("FALSE")));
        }
        break;
    }
*/


    QVariant includesOutlined = qApp->property("includesOutlined").value<QVariant>();

    // next code will only be executed when in designer
    // in other apps we load everything there while taking care of the macros
    if(source.isValid()) {
        if(!source.isNull()) {
            QString test = source.toString();
            if(test.contains("DESIGNER")) {
                if(includesOutlined.toString() == "TRUE") loadIncludes = false;
                else loadIncludes = true;
            }
        }
    }
}

caInclude::~ caInclude()
{
    delete gridLayout;
}

bool caInclude::isPropertyVisible(Properties property)
{
    return designerVisible[property];
}

void caInclude::setPropertyVisible(Properties property, bool visible)
{
    designerVisible[property] = visible;
}

void caInclude::setBackground(QColor c)
{
    thisBackColor = c;
    update();
}

void caInclude::removeIncludedWidgets()
{
    if(thisLoadedWidgets.count() > 0) {
        foreach(QWidget *l, thisLoadedWidgets) {
            gridLayout->removeWidget(l);
            l->hide();
            l->deleteLater();
        }
        thisLoadedWidgets.clear();
    }
    prvFileName = "";
}

void caInclude::setStacking(Stacking stacking) {
    thisStacking = stacking;
    setFileName(newFileName);
    prvStacking = thisStacking;
    if(thisStacking == RowColumn) {
        setPropertyVisible(maximumLines, true);
    } else {
        setPropertyVisible(maximumLines, false);
    }
}

void caInclude::setFileName(QString const &filename)
{
    QUiLoader loader;
    QString fileName;
    QStringList openFile;
    fileFunctions filefunction;

    int nbLines = thisMaxLines;
    int column = 0;
    int row = 0;

    newFileName = filename.trimmed();

    // load widgets from includes
    if(loadIncludes) {

        //printf("cainclude -- setfilename %s for %s\n", qasc(filename), qasc(this->objectName()));

        if(newFileName.size() < 1) {
            removeIncludedWidgets();
            return;
        }

        if(!newFileName.contains(".")) {
            removeIncludedWidgets();
            return;
        }

        // modify stacking
        if(thisStacking != prvStacking || thisMaxLines != prvMaxLines) {
            if(thisLoadedWidgets.count() > 0) {
                //printf("modify stacking with %d items\n", thisLoadedWidgets.count());
                int j = 0;
                foreach(QWidget *l, thisLoadedWidgets) {
                    l->hide();
                    layout()->removeWidget(l);
                }
                delete layout();
                gridLayout = new QGridLayout;
                setLayout(gridLayout);
                gridLayout->setMargin(0);
                gridLayout->setSpacing(0);
                foreach(QWidget *l, thisLoadedWidgets) {
                    // find the row, column to add this widget
                    if(thisStacking == Row) {
                        gridLayout->addWidget(l, j, 0);
                    } else if(thisStacking == Column) {
                        gridLayout->addWidget(l, 0, j);
                    } else {
                        if(row >= nbLines) {
                            row=0;
                            column++;
                        }
                        gridLayout->addWidget(l, row, column);
                        row++;
                    }
                    j++;
                    l->show();
                }
                prvStacking = thisStacking;
                prvMaxLines = thisMaxLines;
                return;
            }
        }

        if(thisItemCount != prvItemCount) {
            //printf("count modified\n");

        } else if(!prvFileName.isNull() && !newFileName.isNull()) {
            //printf("filename did not change->return\n");
            int indx1 = prvFileName.indexOf(".");
            int indx2 = newFileName.indexOf(".");
            if(indx1 != -1 && indx2 != -1) {
                QString file1 = prvFileName.left(indx1);
                QString file2 = newFileName.left(indx2);
                if(file1 == file2) return;
            }
        }

        setLayout(gridLayout);

        if(newFileName.contains(".prc")) {
            fileName = newFileName;
        } else {
            openFile = newFileName.split(".", QString::SkipEmptyParts);
            fileName = openFile[0].append(".ui");
        }

        // this will check for file existence and when an url is defined, download the file from a http server
        filefunction.checkFileAndDownload(fileName);
        searchFile *s = new searchFile(fileName);
        QString fileNameFound = s->findFile();

        // file was not found, remove previous widget if any
        if(fileNameFound.isNull()) {
            removeIncludedWidgets();
            delete s;
            return;
        }
        delete s;

        // file was found, remove previous widget if any
        removeIncludedWidgets();

        // load file
        for(int j=0; j<thisItemCount; j++) {
            QWidget * loadedWidget = (QWidget *) 0;
            if(!fileName.contains(".prc")) {
                // load new file
                QFile *file = new QFile;
                file->setFileName(fileNameFound);
                file->open(QFile::ReadOnly);

                printf("effective load of file %s for widget %s\n", qasc(fileNameFound), qasc(this->objectName()));
                QWidget *tmp = loader.load(file, thisParent);

                file->close();
                delete file;
                if(tmp == (QWidget*) 0) return;
                thisLoadedWidgets.append(tmp);
                loadedWidget = tmp;
                // pep file
            } else {
                ParsePepFile *parsefile = new ParsePepFile(fileNameFound);
                printf("effective load of file %s for widget %s\n", qasc(fileNameFound), qasc(this->objectName()));
                QWidget *tmp= parsefile->load(thisParent);
                if(tmp == (QWidget*) 0) return;
                thisLoadedWidgets.append(tmp);
                loadedWidget = tmp;
            }

            // find the row, column to add this widget
            if(thisStacking == Row) {
                gridLayout->addWidget(loadedWidget, j, 0);
            } else if(thisStacking == Column) {
                gridLayout->addWidget(loadedWidget, 0, j);
            } else {
                if(row >= nbLines) {
                    row=0;
                    column++;
                }
                gridLayout->addWidget(loadedWidget, row, column);
                row++;
            }

            // show it
            loadedWidget->show();
        }

        prvFileName = newFileName;
        prvStacking = thisStacking;
        prvItemCount = thisItemCount;
        prvMaxLines = thisMaxLines;
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






