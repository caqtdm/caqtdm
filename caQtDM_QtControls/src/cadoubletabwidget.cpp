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
#include <QToolTip>
#include "cadoubletabwidget.h"

#define PRINT(x)

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
    thisVerItems.clear();
    thisHorItems.clear();
    addSampleWidget(0);
    addSampleWidget(1);
    addPages = false;
    firstExecution = true;

    // colorize horizontal bar
    setFont(0);

    setRow(0);
    setCol(0);

    installEventFilter(this);
}

bool caDoubleTabWidget::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);
    QVariant source = qApp->property("APP_SOURCE").value<QVariant>();
    if(source.isValid()) {
        if(!source.isNull()) {
            QString test = source.toString();
            if(test.contains("DESIGNER")) {
                if (event->type() == QEvent::ToolTip) {
                    QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
                    QToolTip::showText(helpEvent->globalPos(), "to create a page (when empty) for these indexes, please use insert page in the context menu\n(after or before have no meaning here)");
                    return true;
                }
            }
        }
    }

    if(event->type() == QEvent::Polish){
        editStyleSheet(this->styleSheet());
    }
    return QWidget::event(event);
}

int caDoubleTabWidget::lookupArrayIndex(int row, int col)
{
    QMapIterator<int, twoInts> j(lookup);
    while (j.hasNext()) {
        j.next();
        PRINT(printf("viewportIndex %d in array row=%d col=%d\n", j.key(), j.value().r, j.value().c));
    }
    PRINT(printf("number of viewport pages = %d\n", count()));


    QMapIterator<int, twoInts> i(lookup);
    while (i.hasNext()) {
        i.next();
        if(row == i.value().r && col == i.value().c) {
            return i.key();
        }
    }
    return -1;
}

void  caDoubleTabWidget::storeArrayIndex(int pageIndex, int row, int col)
{
    PRINT(printf("store in array row=%d col=%d index of viewport=%d\n", row, col, pageIndex));
    twoInts item;
    item.r = row;
    item.c = col;
    lookup.insert(pageIndex, item);
}

void  caDoubleTabWidget::deleteArrayIndex(int pageIndex)
{
    PRINT(printf("delete in array row=%d col=%d index of viewport=%d\n", row, col, pageIndex));
    lookup.remove(pageIndex);
}


// add page
void caDoubleTabWidget::addPage(QWidget *page)
{
    PRINT(printf("add a new page %s\n", qasc(page->objectName())));
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

// remove a viewport
void caDoubleTabWidget::removePage(int index)
{
    int pageIndex = 0;
    PRINT(printf("we have to remove the page from array for row=%d col=%d\n", row, col));
    if((pageIndex =lookupArrayIndex(row, col)) != -1) {

        // delete this stackwidget page from the array
        deleteArrayIndex(pageIndex);
        QWidget *widget = viewPort->widget(index);
        PRINT(printf("remove widget at stacked widget index=%d with name=%s\n", index, qasc(widget->objectName())));
        viewPort->removeWidget(widget);
        setRow(row);
        setCol(col);

        // now that we deleted a page of the stackwidget we will have to change are lookup array
        // after the deleted page, change key to key -1
        QMap<int, twoInts> lookupNew;
        QMapIterator<int, twoInts> j(lookup);
        while (j.hasNext()) {
            j.next();
            twoInts item;
            item.r = j.value().r;
            item.c = j.value().c;
            if(j.key() > pageIndex) {
                lookupNew.insert(j.key()-1, item);
            } else {
                lookupNew.insert(j.key(), item);
            }
        }
        // and copy it back
        lookup = lookupNew;
        lookupNew.detach();

    } else {
        PRINT(printf("page not found, return\n"));
        return;
    }
}

// insert a new viewport
void caDoubleTabWidget::insertPage(int index, QWidget *page)
{
    Q_UNUSED(index)
    page->setParent(viewPort);
    page->activateWindow();

    PRINT(printf("insert page for actual row=%d column=%d\n", row, col));
    if(lookupArrayIndex(row, col) == -1 || addPages) {
        int viewPortIndex = viewPort->insertWidget((count()), page);
        PRINT(printf("stored with viewIndex=%d\n", viewPortIndex));
        storeArrayIndex(viewPortIndex, row, col);
    } else {
        PRINT(printf("already done, return\n"));
        return;
    }
    QString title = tr("Page_%1_%2").arg(row).arg(col);
    PRINT(printf("set page title to %s\n", qasc(title)));
    page->setObjectName(title);
    page->setAutoFillBackground(true);
    QString style = "QWidget#%1 { background-color : rgb(255,255,200); }";
    if(page->styleSheet().size() < 1) page->setStyleSheet(style.arg(title));
}

int caDoubleTabWidget::count() const
{
    return viewPort->count();
}

int caDoubleTabWidget::currentIndex() const
{
    return viewPort->currentIndex();
}

// remove tabs of widget
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

// construction of the widget
void caDoubleTabWidget::addSampleWidget(int dir)
{
    int i;
    // horizontal
    if(dir == 0) {
        if(thisHorItems.count() == 0) {
            for(i = 0; i < 5; i++) {
                hTabBar->addTab(QString("Widget %1").arg(i));
                thisHorItems << (QString("Widget %1").arg(i));
            }
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
        if(thisVerItems.count() == 0) for(i = 0; i < count; i++) thisVerItems << (QString("Widget %1").arg(i));
    }
    setCurrentIndex(0);
    if(hTabBar->count() > 0) hTabBar->setCurrentIndex(0);
    if(vCount > 0) {
        vTabBar->button(0)->setChecked(true);

    }
}

void caDoubleTabWidget::setCurrentIndex(int pageIndex)
{
    int Index;
    Q_UNUSED(pageIndex)
    QString title = tr("Page_%1_%2").arg(row).arg(col);
    tableIndex->setText(title);

    if((Index = lookupArrayIndex(row, col)) != -1) {
        PRINT(printf("found %d\n", Index));
        viewPort->setCurrentIndex(Index);
        emit currentIndexChanged(Index);
    } else {
        PRINT(printf("not found\n"));
    }
}

QWidget* caDoubleTabWidget::widget(int index)
{
    return viewPort->widget(index);
}

// set viewport according to the row
void caDoubleTabWidget::setRow(int r) {
    int pageIndex;
    row = r;
    if((pageIndex = lookupArrayIndex(row, col)) != -1) {
        setCurrentIndex(pageIndex);
    } else {
        tableIndex->setText("empty");
    }
    vTabBar->button(r)->setChecked(true);
}

// set viewport according to the column
void caDoubleTabWidget::setCol(int c) {
    int pageIndex;
    col = c;
    if((pageIndex = lookupArrayIndex(row, col)) != -1) {
        setCurrentIndex(pageIndex);
    }  else {
        tableIndex->setText("empty");
    }
}

// fontchange
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

//    setFont(1);
}

void caDoubleTabWidget::setItemsPadding(QString const &padding) {
    thisVerPadding= padding.split(";");
    setFont(1);
}


// set styles
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

        QString pushbtnColor = QString("QPushButton:checked {background-color: rgb( 255, 0, 255);}");
        style.append(pushbtnColor);

         style.append("QPushButton:default {border-color: navy; }");
        style.append("QTabBar::tab  {background-color: rgb( 255, 0, 255);}");
         this->setStyleSheet(style);
    }
#ifdef _MSC_VER
    delete[] padding;
#endif
}

void caDoubleTabWidget::editStyleSheet(QString styleSheet){
    if(firstExecution){
         if (styleSheet.length() == 0) {
            this->setStyleSheet("");
         }else{
            this->setStyleSheet(styleSheet);
         }
         firstExecution = false;
    }else{
         if(styleSheet.length() > 0){
         this->setStyleSheet(styleSheet);
         }else{
         }
    }
 }

