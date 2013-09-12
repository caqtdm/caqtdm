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

#include "carowcolmenu.h"
#include <QApplication>
#include <QGridLayout>
#include <QMouseEvent>
#include <QtDebug>
#include <QMenu>
#include <math.h>
#include <QPainter>

caRowColMenu::caRowColMenu(QWidget *parent) : QWidget(parent)
{
    numCells = 2;
    files << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9" << "10";
    args  << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9" << "10";
    labels << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9" << "10";
    texts << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9" << "10";
    signalMapper = new QSignalMapper(this);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    grid = new QGridLayout(this);
    grid->setMargin(0);
    grid->setSpacing(2);

    thisStacking = Row;
    thisForeColor = Qt::black;
    thisScaleMode = EPushButton::WidthAndHeight;
    setBackground(Qt::gray);

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
    if(numCells > 10) numCells=10;

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

            QString newLabel= thisLabel;
            QString pixLabel = thisLabel;
            if(pixLabel[0] == '-') {
                pixLabel.remove(0,1);
            } else {
               pixLabel.insert(0, "     ");        // not very nice, we prefix with some blancs to stay outdise the pixmap
            }

            // class for painting with a pixmap and text
            ImagePushButton* temp = new ImagePushButton(pixLabel, thisImage);
            temp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            temp->setMinimumSize(2,2); //important for resizing as small as possible
            if(thisScaleMode == EPushButton::None) {  // in this case we may use font
                temp->setFont(this->font());
            } else {
                temp->setFontScaleMode(thisScaleMode);
            }

            // in case of a - at first position, take it off and say no pixmap
            if(newLabel[0] == '-') {
                newLabel.remove(0,1);
                temp->setIconVisible(false);
                temp->setLabelText(newLabel);
            // otherwise a pixmap
            } else {
                temp->setIconVisible(true);
            }
            //temp->setLabelText(newLabel);

            //set colors and style

            QString style = "QPushButton{ background-color: rgb(%1, %2, %3); color: rgb(%4, %5, %6); border-color: rgb(%7, %8, %9);";
            style = style.arg(thisBackColor.red()).arg(thisBackColor.green()).arg(thisBackColor.blue()).
                    arg(thisForeColor.red()).arg(thisForeColor.green()).arg(thisForeColor.blue()).
                    arg(thisBorderColor.red()).arg(thisBorderColor.green()).arg(thisBorderColor.blue());
            style.append("border-radius: 3px; padding: 1px; border-width: 1px;"
                         "border-style: outset;}");
            QString hover = "QPushButton:hover {background-color: rgb(%1, %2, %3);}";
            hover = hover.arg(thisBackColorHover.red()).arg(thisBackColorHover.green()).arg(thisBackColorHover.blue());
            style.append(hover);
            temp->setStyleSheet(style);

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
            int alpha = 255;
            QString newLabel= thisLabel;
            QString pixLabel = thisLabel;
            if(pixLabel[0] == '-') {
                pixLabel.remove(0,1);
            } else {
                pixLabel.insert(0, "     ");        // not very nice, we prefix with some blancs to stay outdise the pixmap
            }
            // class for painting with a pixmap and text
            ImagePushButton* temp = new ImagePushButton(pixLabel, thisImage);
            temp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            temp->setMinimumSize(2,2); //important for resizing as small as possible

            if(thisScaleMode == EPushButton::None) {  // in this case we may use font
                temp->setFont(this->font());
            } else {
                temp->setFontScaleMode(thisScaleMode);
            }

            // in case of a - at first position, take it off and say no pixmap
            if(newLabel[0] == '-') {
                newLabel.remove(0,1);
                temp->setIconVisible(false);
                temp->setLabelText(newLabel);
            // otherwise a pixmap
            } else {
                temp->setIconVisible(true);
            }
            //temp->setLabelText(newLabel);

            if(thisStacking == Hidden) {
                temp->setInVisible(thisBackColor, thisForeColor, thisBorderColor);
                borderSize = 0;
                alpha = 0;
            } else {
                borderSize = 1;
            }

           //set colors and style filled
            QString style = "QPushButton{ background-color: rgba(%1, %2, %3, %4); color: rgb(%5, %6, %7); border-color: rgb(%8, %9, %10);";
            style = style.arg(thisBackColor.red()).arg(thisBackColor.green()).arg(thisBackColor.blue()).arg(alpha).
                    arg(thisForeColor.red()).arg(thisForeColor.green()).arg(thisForeColor.blue()).
                    arg(thisBorderColor.red()).arg(thisBorderColor.green()).arg(thisBorderColor.blue());
            QString border ="border-radius: 3px; padding: 1px; border-style: outset; border-width: %1px;}";
            border = border.arg(borderSize);
            style.append(border);
            QString hover = "QPushButton:hover {background-color: rgb(%1, %2, %3, %4);}";
            hover = hover.arg(thisBackColorHover.red()).arg(thisBackColorHover.green()).arg(thisBackColorHover.blue()).arg(alpha);
            style.append(hover);
            temp->setStyleSheet(style);

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
            if(thisScaleMode == EPushButton::None) {  // in this case we may use font
                temp->setFont(this->font());
            } else {
                temp->setFontScaleMode(thisScaleMode);
            }

            //set colors and style

            QString style = "QPushButton{ background-color: rgb(%1, %2, %3); color: rgb(%4, %5, %6); border-color: rgb(%7, %8, %9);";
            style = style.arg(thisBackColor.red()).arg(thisBackColor.green()).arg(thisBackColor.blue()).
                    arg(thisForeColor.red()).arg(thisForeColor.green()).arg(thisForeColor.blue()).
                    arg(thisBorderColor.red()).arg(thisBorderColor.green()).arg(thisBorderColor.blue());
            style.append("border-radius: 3px; padding: 1px; border-width: 1px;"
                         "border-style: outset;}");
            QString hover = "QPushButton:hover {background-color: rgb(%1, %2, %3);}";
            hover = hover.arg(thisBackColorHover.red()).arg(thisBackColorHover.green()).arg(thisBackColorHover.blue());
            style.append(hover);
            temp->setStyleSheet(style);

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

}

void caRowColMenu::setLabel(QString const &label)
{
    thisLabel = label;
    populateCells();
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
    //printf("new arguments=<%s>\n", newL.toAscii().constData());
    args = newL.split(";");
}

void caRowColMenu::setFiles(QString const &newL)
{
    files = newL.split(";");
}

void caRowColMenu::setBackground(QColor c)
{
    thisBackColor = c;
    thisBackColorHover = c.light(120);
    thisBorderColor = c.dark(150);
    populateCells();
}

void caRowColMenu::setForeground(QColor c)
{
    thisForeColor = c;
    populateCells();
}

void caRowColMenu::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e);
}

void caRowColMenu::setFontScaleMode(EPushButton::ScaleMode m)
{
   thisScaleMode = m;
   populateCells();
}

EPushButton::ScaleMode caRowColMenu::fontScaleMode()
{
    populateCells();
    return thisScaleMode;
}


