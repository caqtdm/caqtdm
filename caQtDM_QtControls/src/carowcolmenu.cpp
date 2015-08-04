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

#include "carowcolmenu.h"
#include <QApplication>
#include <QGridLayout>
#include <QMenu>
#include <math.h>
#include <QPainter>

caRowColMenu::caRowColMenu(QWidget *parent) : QWidget(parent)
{
    numCells = 2;
    files << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9" << "10" << "11" << "12" << "13" << "14" << "15" << "16";
    args  << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9" << "10" << "11" << "12" << "13" << "14" << "15" << "16";
    labels<< "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9" << "10" << "11" << "12" << "13" << "14" << "15" << "16";
    texts << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9" << "10" << "11" << "12" << "13" << "14" << "15" << "16";
    signalMapper = new QSignalMapper(this);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    grid = new QGridLayout(this);
    grid->setMargin(0);
    grid->setSpacing(2);

    thisStacking = Row;
    thisForeColor = Qt::black;
    thisScaleMode = EPushButton::WidthAndHeight;
    setBackground(Qt::gray);

    alpha = 255;

    installEventFilter(this);
}

void caRowColMenu::populateCells()
{
    double dSqrt = ceil(sqrt((double) numCells)); // preferentially vertical orientation
    int nbLines = qMax(2, (int) dSqrt);
    int column = 0;
    int row = 0;

    numCells = qMin(files.size(), args.size());
    numCells = qMin(numCells, labels.size());

    if(numCells < 1) return;
    if(numCells > MAXITEMS) numCells=MAXITEMS;

    if(thisStacking == Hidden) numCells = 1;

    // signalmapper will map signals from all buttons to one signal
    delete signalMapper;
    signalMapper = new QSignalMapper(this);

    foreach(ImagePushButton *l, cellsI) {
        grid->removeWidget(l);
        l->hide();
        l->deleteLater();
    }

    foreach(EPushButton *l, cellsP) {
        grid->removeWidget(l);
        l->hide();
        l->deleteLater();
    }

    cellsP.clear();
    cellsI.clear();

    // we want a menu
    if(thisStacking == Menu || thisStacking == Hidden) {

        // more than one command, make menu
        if(numCells > 1) {

            // class for painting with a pixmap and text
            ImagePushButton* temp = new ImagePushButton("", thisImage);
            temp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            temp->setMinimumSize(2,2); //important for resizing as small as possible
            temp->setObjectName(texts[0]);
            grid->addWidget(temp, 0, 0);

            cellsP.append(temp);
            QMenu* menu = new QMenu();
            for (int i = 0; i < numCells; i++) {
                QAction *action = new QAction(labels[i], this);
                signalMapper->setMapping(action, i);
                connect(action, SIGNAL(triggered()), signalMapper, SLOT(map()));
                menu->addAction(action);
            }
            temp->setMenu(menu);
            temp->show();

            connect(signalMapper, SIGNAL(mapped(int)), this, SIGNAL(triggered(int)));

            // menu with one command --> pushbutton
        } else {

            // class for painting with a pixmap and text
            ImagePushButton* temp = new ImagePushButton("", thisImage);
            temp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            temp->setMinimumSize(2,2); //important for resizing as small as possible

            if(thisStacking == Hidden) {
                temp->setInVisible(thisBackColor, thisForeColor, thisBorderColor);
                borderSize = 0;
                alpha = 0;
            } else {
                alpha = 255;
                borderSize = 1;
            }

            temp->setObjectName(texts[0]);
            grid->addWidget(temp, 0, 0);

            temp->show();

            cellsI.append(temp);

            signalMapper->setMapping(temp, 0);
            connect(temp, SIGNAL(clicked()), signalMapper, SLOT(map()));
            connect(signalMapper, SIGNAL(mapped(int)), this, SIGNAL(clicked(int)));
        }
    } else {
        // we want only pusbuttons

        for (int i = 0; i < numCells; i++) {
            EPushButton* temp = new EPushButton(labels[i], this);
            temp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            temp->setMinimumSize(2,2); //important for resizing as small as possible

            temp->setObjectName(texts[i]);

            // take care of stacking
            if(thisStacking == Row) {
                grid->addWidget(temp, i, 0);
            } else if(thisStacking == Column) {
                grid->addWidget(temp, 0, i);
            } else {
                if(row >= nbLines) {
                    row=0;
                    column++;
                }
                grid->addWidget(temp, row, column);
                row++;
            }
            cellsP.append(temp);
            temp->show();

            signalMapper->setMapping(temp, i);
            connect(temp, SIGNAL(clicked()), signalMapper, SLOT(map()));
        }

        connect(signalMapper, SIGNAL(mapped(int)), this, SIGNAL(clicked(int)));
    }

    updateLabel();
    updateColors();
    updateFontScaleMode();
}

void caRowColMenu::setLabel(QString const &label)
{
    thisLabel = label;
    updateLabel();
}

void caRowColMenu::setImage(QString const &image) {
    thisImage = image;
}

void caRowColMenu::setStacking(Stacking stacking)
{
    QSize newSize;
    thisStacking = stacking;
    newSize.setHeight( this->size().height()+1);
    newSize.setWidth(this->size().width()+1);
    resize(newSize);          // schedule a new resize event
    QApplication::processEvents(); // process pending events
    newSize.setHeight( this->size().height()-1);
    newSize.setWidth(this->size().width()-1);
    QApplication::processEvents(); // process pending events
    resize(newSize);          // schedule a new resize event
    populateCells();
}

void  caRowColMenu::setLabels(QString const &newL)
{
    labels= newL.split(";");
    populateCells();
}

void caRowColMenu::setArgs(QString const &newL)
{
    //printf("new arguments=<%s>\n", newL.toLatin1().constData());
    args = newL.split(";");
}

void caRowColMenu::setFiles(QString const &newL)
{
    files = newL.split(";");
}

void caRowColMenu::setBackground(QColor c)
{
    thisBackColor = c;
    thisBackColorHover = c.lighter(120);
    thisBorderColor = c.darker(150);
    updateColors();
}

void caRowColMenu::setForeground(QColor c)
{
    thisForeColor = c;
    updateColors();
}

void caRowColMenu::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e);
}

void caRowColMenu::setFontScaleModeL(EPushButton::ScaleMode m)
{
   thisScaleMode = m;
   updateFontScaleMode();
}

EPushButton::ScaleMode caRowColMenu::fontScaleMode()
{
    populateCells();
    return thisScaleMode;
}

void caRowColMenu::updateLabel() {

    if(thisStacking == Menu || thisStacking == Hidden) {

        // more than one command, make menu
        if(numCells > 1 ) {

            if(cellsP.isEmpty() || cellsP.count() < 1) return;

            ImagePushButton *temp = (ImagePushButton *) cellsP[0];

            QString newLabel= thisLabel;
            QString pixLabel = thisLabel;
            if(pixLabel[0] == '-') pixLabel.remove(0,1);
            else                   pixLabel.insert(0, "     ");        // not very nice, we prefix with some blancs to stay outdise the pixmap
            temp->setText(pixLabel);

            // in case of a - at first position, take it off and say no pixmap
            if(newLabel[0] == '-') {
                newLabel.remove(0,1);
                temp->setIconVisible(false);
                temp->setLabelText(newLabel);
                // otherwise a pixmap
            } else temp->setIconVisible(true);

            // menu with one command --> pushbutton
        } else {

            if(cellsI.isEmpty() || cellsI.count() < 1) return;

            ImagePushButton *temp = cellsI[0];
            QString newLabel= thisLabel;
            QString pixLabel = thisLabel;
            if(pixLabel[0] == '-') {
                pixLabel.remove(0,1);
            } else {
                pixLabel.insert(0, "     ");        // not very nice, we prefix with some blancs to stay outdise the pixmap
            }

            temp->setText(pixLabel);

            // in case of a - at first position, take it off and say no pixmap
            if(newLabel[0] == '-') {
                newLabel.remove(0,1);
                temp->setIconVisible(false);
                temp->setLabelText(newLabel);
                // otherwise a pixmap
            } else {
                temp->setIconVisible(true);
            }

        }
    }
}

void caRowColMenu::updateColors()
{
    // we want a menu
    if(thisStacking == Menu || thisStacking == Hidden) {

        // more than one command, make menu
        if(numCells > 1) {

            if(cellsP.isEmpty() || cellsP.count() < 1) return;

            ImagePushButton *temp = (ImagePushButton *) cellsP[0];

            //set colors and style

            QString style = "QPushButton{ background-color: rgba(%1, %2, %3, %4); color: rgba(%5, %6, %7, %8); border-color: rgba(%9, %10, %11, %12);";
            style = style.arg(thisBackColor.red()).arg(thisBackColor.green()).arg(thisBackColor.blue()).arg(thisBackColor.alpha()).
                    arg(thisForeColor.red()).arg(thisForeColor.green()).arg(thisForeColor.blue()).arg(thisForeColor.alpha()).
                    arg(thisBorderColor.red()).arg(thisBorderColor.green()).arg(thisBorderColor.blue()).arg(thisBorderColor.alpha());
            style.append("border-radius: 3px; padding: 1px; border-width: 1px;"
                         "border-style: outset;}");
            QString hover = "QPushButton:hover {background-color: rgba(%1, %2, %3, %4);}  QPushButton:pressed {background-color: rgba(%5, %6, %7, %8)};";
            hover = hover.arg(thisBackColorHover.red()).arg(thisBackColorHover.green()).arg(thisBackColorHover.blue()).arg(thisBackColorHover.alpha()).
                    arg(thisBorderColor.red()).arg(thisBorderColor.green()).arg(thisBorderColor.blue()).arg(thisBorderColor.alpha());

            style.append(hover);
            temp->setStyleSheet(style);

            // menu with one command --> pushbutton
        } else {
            if(cellsI.isEmpty() || cellsI.count() < 1) return;

            ImagePushButton *temp = cellsI[0];

           //set colors and style filled
            QString style = "QPushButton{ background-color: rgba(%1, %2, %3, %4); color: rgba(%5, %6, %7, %8); border-color: rgba(%9, %10, %11, %12);";
            style = style.arg(thisBackColor.red()).arg(thisBackColor.green()).arg(thisBackColor.blue()).arg(thisBackColor.alpha()).
                    arg(thisForeColor.red()).arg(thisForeColor.green()).arg(thisForeColor.blue()).arg(thisForeColor.alpha()).
                    arg(thisBorderColor.red()).arg(thisBorderColor.green()).arg(thisBorderColor.blue()).arg(thisBorderColor.alpha());
            QString border ="border-radius: 3px; padding: 1px; border-style: outset; border-width: %1px;}";
            border = border.arg(borderSize);
            style.append(border);
            QString hover = "QPushButton:hover {background-color: rgba(%1, %2, %3, %4);}  QPushButton:pressed {background-color: rgba(%5, %6, %7, %8)};";
            hover = hover.arg(thisBackColorHover.red()).arg(thisBackColorHover.green()).arg(thisBackColorHover.blue()).arg(thisBackColorHover.alpha()).
                          arg(thisBorderColor.red()).arg(thisBorderColor.green()).arg(thisBorderColor.blue()).arg(thisBorderColor.alpha());
            style.append(hover);
            temp->setStyleSheet(style);

        }
    } else {
        // we want only pusbuttons

        if(cellsP.isEmpty() || cellsP.count() < numCells) return;
        for (int i = 0; i < numCells; i++) {
            EPushButton * temp = (EPushButton *) cellsP[i];

            //set colors and style
            QString style = "QPushButton{ background-color: rgba(%1, %2, %3, %4); color: rgba(%5, %6, %7, %8); border-color: rgba(%9, %10, %11, %12);";
            style = style.arg(thisBackColor.red()).arg(thisBackColor.green()).arg(thisBackColor.blue()).arg(thisBackColor.alpha()).
                    arg(thisForeColor.red()).arg(thisForeColor.green()).arg(thisForeColor.blue()).arg(thisForeColor.alpha()).
                    arg(thisBorderColor.red()).arg(thisBorderColor.green()).arg(thisBorderColor.blue()).arg(thisBorderColor.alpha());
            style.append("border-radius: 3px; padding: 1px; border-width: 1px;"
                         "border-style: outset;}");
            QString hover = "QPushButton:hover {background-color: rgba(%1, %2, %3, %4);}  QPushButton:pressed {background-color: rgba(%5, %6, %7, %8)};";
            hover= hover.arg(thisBackColorHover.red()).arg(thisBackColorHover.green()).arg(thisBackColorHover.blue()).arg(thisBackColorHover.alpha()).
                    arg(thisBorderColor.red()).arg(thisBorderColor.green()).arg(thisBorderColor.blue()).arg(thisBorderColor.alpha());
            style.append(hover);
            temp->setStyleSheet(style);
        }

    }

}

void caRowColMenu::updateFontScaleMode()
{
    // we want a menu
    if(thisStacking == Menu || thisStacking == Hidden) {

        // more than one command, make menu
        if(numCells > 1) {

            if(cellsP.isEmpty() || cellsP.count() < 1) return;

            ImagePushButton *temp = (ImagePushButton *) cellsP[0];

            if(thisScaleMode == EPushButton::None) {  // in this case we may use font
                temp->setFont(this->font());
            } else {
                temp->setFontScaleMode(thisScaleMode);
            }

            // menu with one command --> pushbutton
        } else {
            if(cellsI.isEmpty() || cellsI.count() < 1) return;

            ImagePushButton *temp = cellsI[0];

            if(thisScaleMode == EPushButton::None) {  // in this case we may use font
                temp->setFont(this->font());
            } else {
                temp->setFontScaleMode(thisScaleMode);
            }

        }
    } else {
        // we want only pusbuttons

        if(cellsP.isEmpty() || cellsP.count() < numCells) return;
        for (int i = 0; i < numCells; i++) {
            EPushButton * temp = (EPushButton *) cellsP[i];
            if(thisScaleMode == EPushButton::None) {  // in this case we may use font
                temp->setFont(this->font());
            } else {
                temp->setFontScaleMode(thisScaleMode);
            }
        }

    }
}
