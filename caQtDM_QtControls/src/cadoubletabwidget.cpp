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

#include <stdio.h>
#include <QApplication>
#include <QWidget>
#include "cadoubletabwidget.h"

caDoubleTabWidget::caDoubleTabWidget(QWidget *parent) : QWidget(parent)

{
    row = 0;
    col = 0;
    vCount = 0;
    tableIndex = new QLineEdit("empty");

    // create horizontal bar, vertical buttongroup and a stacked widget
    hTabBar = new QTabBar;
    viewPort = new QStackedWidget;
    viewPort->setFrameShape(QFrame::Panel);

    vTabBar = new QButtonGroup;
    buttonLayout = new QVBoxLayout();
    buttonLayout->setSpacing(0);

    QVBoxLayout* buttonStretchLayout = new QVBoxLayout();
    buttonStretchLayout->setSpacing(0);
    buttonStretchLayout->addLayout(buttonLayout);
    buttonStretchLayout->addStretch();

    // make layout and add the widgets
    QGridLayout *gridLayout = new QGridLayout(this);
    gridLayout->addWidget(hTabBar, 0, 1, 1, 1);
    gridLayout->addLayout(buttonStretchLayout, 1, 0, 1, 1);
    gridLayout->addWidget(viewPort, 1, 1, 1, 1);
    gridLayout->addWidget(tableIndex, 2, 1, 1, 1);

    hTabBar->setShape(QTabBar::TriangularNorth);
    hTabBar->setExpanding(true);

    // size policy
    viewPort->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    hTabBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // signal /slots
    connect(hTabBar, SIGNAL(currentChanged(int)), SLOT(setCol(int)));
    connect(vTabBar,  SIGNAL(buttonClicked(int)), this, SLOT(setRow(int)));

    // add items to our bar and list
    addSampleWidget(0);
    addSampleWidget(1);
    addPages = false;

    // colorize horizontal bar
    QPalette pal = hTabBar->palette();
    pal.setColor(QPalette::Base, QColor(255, 0, 255));
    hTabBar->setPalette(pal);

    setRow(0);
    setCol(0);
}

void caDoubleTabWidget::addPage(QWidget *page)
{
    //printf("add a new page %s\n", page->objectName().toAscii().constData());
    QStringList stringlist = page->objectName().split( "_");
    if(stringlist.count() > 1 ) {
        row= stringlist[1].toInt();
        col = stringlist[2].toInt();
    } else {
        row = 0;
        col = 0;
    }
    addPages = true;
    insertPage(count(), page);
    addPages = false;
    setRow(0);
    setCol(0);

}

void caDoubleTabWidget::removePage(int index)
{
    QWidget *widget = viewPort->widget(index);
    viewPort->removeWidget(widget);
}

int caDoubleTabWidget::count() const
{
    return viewPort->count();
}

int caDoubleTabWidget::currentIndex() const
{
    return viewPort->currentIndex();
}

void caDoubleTabWidget::insertPage(int index, QWidget *page)
{
    page->setParent(viewPort);

    //printf("insert page at %d for actual row=%d column=%d\n", index, row, col);
    if(lookupPage(row, col) == -1 || addPages) {
        viewPort->insertWidget(index, page);
        storePage(index, row, col);
    } else {
        //printf("already done, return\n");
        return;
    }
    QString title = tr("Page_%1_%2").arg(row).arg(col);
    //printf("set page title to %s\n", title.toAscii().constData());
    page->setObjectName(title);
    page->setAutoFillBackground(true);
}

void caDoubleTabWidget::removeTabs(int dir)
{
    int i;

    // horizontal
    if(dir == 0) {
        int count =  hTabBar->count();
        for(i = count-1; i >= 0; i--) {
            hTabBar->removeTab(i);
        }

        // vertical
    } else {
        int count =  vTabBar->buttons().count();
        for(i = count-1; i >= 0; i--) {
           QPushButton* button = (QPushButton*) vTabBar->button(i);
           buttonLayout->removeWidget(button);
           vTabBar->removeButton(button);
           delete button;
        }
    }
}

void caDoubleTabWidget::addSampleWidget(int dir)
{
    int i;
    //printf("addsamplewidget %d\n", dir);
    // horizontal
    if(dir == 0) {
        if(thisHorItems.count() == 0) {
            for(i = 0; i < 5; i++) hTabBar->addTab(QString("Widget %1").arg(i));
        } else {
            for(i = 0; i < thisHorItems.count(); i++) hTabBar->addTab(thisHorItems.at(i));
        }

        // vertical
    } else {
        int count;
        if(thisVerItems.count() == 0) count = 5; else count = thisVerItems.count();
        vCount = count;
        for(i = 0; i < count; i++) {
           QPushButton* button;
           if(thisVerItems.count() == 0) {
               button = new QPushButton(QString("Widget %1").arg(i));
           } else {
               button = new QPushButton(thisVerItems.at(i));
           }
           button->setObjectName("__qt__passive_pushButton");
           button->setCheckable(true);
           vTabBar->addButton(button, i);
           buttonLayout->addWidget(button);
        }
        setFont(1);
    }
    setCurrentIndex(0);
    if(hTabBar->count() > 0) hTabBar->setCurrentIndex(0);
    if(vCount > 0) {
        vTabBar->button(0)->setChecked(true);

    }

}

void caDoubleTabWidget::setCurrentIndex(int index)
{
    //printf("setcurrent index %d %d %d\n", index, row, col);
    QString title = tr("Page_%1_%2").arg(row).arg(col);
    tableIndex->setText(title);
    viewPort->setCurrentIndex(index);
    emit currentIndexChanged(index);
}

QWidget* caDoubleTabWidget::widget(int index)
{
    return viewPort->widget(index);
}

void caDoubleTabWidget::setRow(int r) {
    int index;
    row = r;
    if((index = lookupPage(row, col)) != -1) {
        setCurrentIndex(index);
    } else {
        tableIndex->setText("empty");
    }
    vTabBar->button(r)->setChecked(true);
}

void caDoubleTabWidget::setCol(int c) {
    int index;
    col = c;
    if((index = lookupPage(row, col)) != -1) {
        setCurrentIndex(index);
    }  else {
        tableIndex->setText("empty");
    }
}

int caDoubleTabWidget::lookupPage(int row, int col)
{
    QMapIterator<int, twoInts> i(lookup);
    while (i.hasNext()) {
        i.next();
        if(row == i.value().r && col == i.value().c) {
            return i.key();
        }
    }
    return -1;
}

void  caDoubleTabWidget::storePage(int index, int row, int col)
{
    //printf("store page %d %d\n", row, col);
    twoInts item;
    item.r = row;
    item.c = col;
    lookup.insert(index, item);
}

void caDoubleTabWidget::fontChange(const QFont & oldFont) {
    Q_UNUSED(oldFont);

    // style for horizontal bar
    QString style=tr("QTabBar {font-size: %1pt; font-family: %2; ").arg(this->fontInfo().pointSize()).arg(this->fontInfo().family());
    if(this->fontInfo().underline())  style.append("text-decoration:underline; ");

#if QT_VERSION < 0x040700
    if(this->fontInfo().bold()) style.append("font-weight: bold; ");
    if(this->fontInfo().italic())  style.append("font-style: italic; ");
#else
    if(this->fontInfo().styleName().contains("Bold")) style.append("font-weight: bold; ");
    if(this->fontInfo().styleName().contains("Italic"))  style.append("font-style: italic; ");
#endif
    style.append("} ");
    hTabBar->setStyleSheet(style);

    setFont(1);

}

void caDoubleTabWidget::setItemsPadding(QString const &padding) {
    thisVerPadding= padding.split(";");
    setFont(1);
}

void caDoubleTabWidget::setFont(int dir)
{
    Q_UNUSED(dir);
    // style for vertical buttons
    QString style;
#ifdef _MSC_VER
	int *padding=new int[vTabBar->buttons().count()];
#else
    int padding[vTabBar->buttons().count()];
#endif
    for(int j=0; j<vTabBar->buttons().count(); j++) padding[j] = 0;
    for(int j=0; j < thisVerPadding.count(); j++) {
        padding[j] = thisVerPadding.at(j).toInt();
    }

    int count =  vTabBar->buttons().count();
    for(int i = count-1; i >= 0; i--) {
        QPushButton* button = (QPushButton*) vTabBar->button(i);
        style = "QPushButton {border: 2px solid #8f8f91; border-radius: 6px ; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #f6f7fa, stop: 1 #dadbde);  ";

        // style for vertical list
        style.append(tr("font-size: %1pt; font-family: %2; ").arg(this->fontInfo().pointSize()).arg(this->fontInfo().family()));
        if(this->fontInfo().underline())  style.append("text-decoration:underline; ");

#if QT_VERSION < 0x040700
        if(this->fontInfo().bold()) style.append("font-weight: bold; ");
        if(this->fontInfo().italic())  style.append("font-style: italic; ");
#else
        if(this->fontInfo().styleName().contains("Bold")) style.append("font-weight: bold; ");
        if(this->fontInfo().styleName().contains("Italic"))  style.append("font-style: italic; ");
#endif

        style.append(tr("text-align: left; padding-left: %1px;").arg(padding[i]));


        style.append("} ");

        style.append("QPushButton:checked {background-color: magenta;}");
        style.append("QPushButton:default {border-color: navy; }");
        button->setStyleSheet(style);



    }
    #ifdef _MSC_VER
     delete[] padding;
    #endif
}

