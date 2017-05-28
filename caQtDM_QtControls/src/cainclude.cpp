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
#include <QFrame>
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
    thisMaxColumns = 1;
    thisStacking = Row;
    thisAdjust = prvAdjust= true;
    thisSpacingVertical = thisSpacingHorizontal=0;
    thisFrameColor = Qt::gray;
    setVisibility(StaticV);

    thisPalette = palette();

    boxLayout = new QHBoxLayout();
    boxLayout->setMargin(0);
    boxLayout->setSpacing(0);
    frame = new QFrame();
    thisFrameShape = NoFrame;
    thisFrameShadow = QFrame::Plain;
    thisFrameLineWidth = 1;
    boxLayout->addWidget(frame);

    gridLayout = new QGridLayout();
    gridLayout->setMargin(0);
    gridLayout->setSpacing(0);
    effectiveSize = QSize(100,100);
    frame->setLayout(gridLayout);

    setPropertyVisible(maximumLines, false);

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
        setPropertyVisible(maximumColumns, false);
    } else if(thisStacking == ColumnRow) {
        setPropertyVisible(maximumColumns, true);
        setPropertyVisible(maximumLines, false);
    } else {
        setPropertyVisible(maximumLines, false);
        setPropertyVisible(maximumColumns, false);
    }
}

void caInclude::setFileName(QString const &filename)
{
    newFileName = filename.trimmed();

    // load widgets from includes
    if(loadIncludes) {

        QUiLoader loader;
        QString fileName;
        QStringList openFile;
        fileFunctions filefunction;

        int nbLines = thisMaxLines;
        int nbColumns = thisMaxColumns;
        int column = 0;
        int row = 0;
        int maxRows = 0;
        int maxColumns=0;
        int adjustMargin = 0;

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
        if(thisStacking != prvStacking || thisMaxLines != prvMaxLines || thisMaxColumns != prvMaxColumns || thisAdjust != prvAdjust ||
           thisSpacingHorizontal != prvSpacingHorizontal || thisSpacingVertical != prvSpacingVertical || thisFrameUpdate) {
            thisFrameUpdate = false;

            setLayout(boxLayout);
            gridLayout->setMargin(0);
            switch(thisFrameShape) {
                case NoFrame:
                      frame->setFrameShape(QFrame::NoFrame);
                      break;
                case Box:
                      frame->setFrameShape(QFrame::Box);    
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
                      gridLayout->setMargin(thisFrameLineWidth);
#endif
                      break;
                case Panel:
                      frame->setFrameShape(QFrame::Panel);
                      break;
                default:
                     frame->setFrameShape(QFrame::Panel);
            }

            frame->setFrameShadow(thisFrameShadow);
            frame->setLineWidth(thisFrameLineWidth);

            QColor thisLightColor = thisFrameColor.lighter();
            QColor thisDarkColor = thisFrameColor.darker();

            thisPalette.setColor(QPalette::WindowText, thisFrameColor);
            thisPalette.setColor(QPalette::Light, thisLightColor);
            thisPalette.setColor(QPalette::Dark, thisDarkColor);
            thisPalette.setColor(QPalette::Window, thisFrameColor);
            frame->setPalette(thisPalette);

            gridLayout->setVerticalSpacing(thisSpacingVertical);
            gridLayout->setHorizontalSpacing(thisSpacingHorizontal);

            if(thisLoadedWidgets.count() > 0) {
                //printf("modify stacking with %d items\n", thisLoadedWidgets.count());
                int j = 0;
                foreach(QWidget *l, thisLoadedWidgets) {
                    l->hide();
                    gridLayout->removeWidget(l);
                }

                foreach(QWidget *l, thisLoadedWidgets) {
                    // find the row, column to add this widget
                    if(thisStacking == Row) {
                        gridLayout->addWidget(l, j, 0);
                        row++;
                        maxRows = row;
                        maxColumns = 1;
                    } else if(thisStacking == Column) {
                        gridLayout->addWidget(l, 0, j);
                        column++;
                        maxColumns = column;
                        maxRows = 1;
                    } else if(thisStacking == RowColumn){
                        if(row >= nbLines) {
                            row=0;
                            column++;
                        }
                        gridLayout->addWidget(l, row, column);
                        row++;
                        if(row > maxRows) maxRows = row;
                        maxColumns = column + 1;
                    } else if(thisStacking == ColumnRow){
                        if(column >= nbColumns) {
                            row++;
                            column=0;
                        }
                        gridLayout->addWidget(l, row, column);
                        column++;
                        if(column > maxColumns) maxColumns = column;
                        maxRows = row + 1;
                    }

                    j++;
                    l->show();
                }
                //printf("resize 1 for row=%d column=%d\n", maxRows, maxColumns);
                if(thisFrameShape == Box) adjustMargin = 4*thisFrameLineWidth;
                else if(thisFrameShape == NoFrame) adjustMargin = 0;
                else adjustMargin = 2*thisFrameLineWidth;
                if(thisAdjust) resize(maxColumns * effectiveSize.width() + (maxColumns-1) * thisSpacingHorizontal + adjustMargin,
                                      maxRows * effectiveSize.height() + (maxRows-1) * thisSpacingVertical + adjustMargin);
                prvAdjust = thisAdjust;
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

        frame->setLayout(gridLayout);

        if(newFileName.contains(".prc")) {
            fileName = newFileName;
            thisAdjust = false;
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
                effectiveSize= tmp->size();
                // pep file
            } else {
                ParsePepFile *parsefile = new ParsePepFile(fileNameFound);
                printf("effective load of file %s for widget %s\n", qasc(fileNameFound), qasc(this->objectName()));
                QWidget *tmp= parsefile->load(thisParent);
                if(tmp == (QWidget*) 0) return;
                thisLoadedWidgets.append(tmp);
                loadedWidget = tmp;
                effectiveSize= tmp->size();
            }

            // find the row, column to add this widget
            if(thisStacking == Row) {
                gridLayout->addWidget(loadedWidget, j, 0);
                row++;
                maxRows = row;
                maxColumns = 1;
            } else if(thisStacking == Column) {
                gridLayout->addWidget(loadedWidget, 0, j);
                column++;
                maxColumns = column;
                maxRows = 1;
            } else if(thisStacking == RowColumn){
                if(row >= nbLines) {
                    row=0;
                    column++;
                }
                gridLayout->addWidget(loadedWidget, row, column);
                row++;
                if(row > maxRows) maxRows = row;
                maxColumns = column + 1;

            } else if(thisStacking == ColumnRow){
                if(column >= nbColumns) {
                    row++;
                    column=0;
                }
                gridLayout->addWidget(loadedWidget, row, column);
                column++;
                if(column > maxColumns) maxColumns = column;
                maxRows = row + 1;
            }

            // show it
            loadedWidget->show();
        }

        //printf("resize 2 for row=%d column=%d\n", maxRows, maxColumns);
        if(thisFrameShape == Box) adjustMargin = 4*thisFrameLineWidth;
        else if(thisFrameShape == NoFrame) adjustMargin = 0;
        else adjustMargin = 2*thisFrameLineWidth;
        if(thisAdjust) resize(maxColumns * effectiveSize.width() + (maxColumns-1) * thisSpacingHorizontal + adjustMargin,
                              maxRows * effectiveSize.height() + (maxRows-1) * thisSpacingVertical + adjustMargin);
        prvFileName = newFileName;
        prvStacking = thisStacking;
        prvItemCount = thisItemCount;
        prvMaxLines = thisMaxLines;
        prvMaxColumns = thisMaxColumns;
        prvSpacingHorizontal = thisSpacingHorizontal;
        prvSpacingVertical = thisSpacingVertical;
        prvAdjust = thisAdjust;
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
        painter.setPen( QPen(thisFrameColor, thisLineSize, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ) );
        painter.drawRect(0, 0, width() - 1, height() -1);
    }
}






