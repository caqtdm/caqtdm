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
 *    Yannick Wernle
 *  Contact details:
 *    anton.mezger@psi.ch
 *    yannick.wernle@psi.ch
 */

#include <stdio.h>
#include <QApplication>
#include <QWidget>
#include <QToolTip>
#include <QTimer>
#include "cadoubletabwidget.h"
#include "themecolors.h"

Q_LOGGING_CATEGORY(caDoubleTabWidgetLog, "caqtdm.widgets.cadoubletabwidget")

// colour set of caDoubleTabWidget (level 2), values come from themecolors.h
namespace caDoubleTabColor {
    constexpr const ThemeColor &Page           = ThemeColors::TabPage;
    constexpr const ThemeColor &ButtonBorder   = ThemeColors::TabButtonBorder;
    constexpr const ThemeColor &GradientTop    = ThemeColors::TabGradientTop;
    constexpr const ThemeColor &GradientBottom = ThemeColors::TabGradientBottom;
    constexpr const ThemeColor &Checked        = ThemeColors::TabChecked;
    constexpr const ThemeColor &DefaultBorder  = ThemeColors::TabDefaultBorder;
}

caDoubleTabWidget::caDoubleTabWidget(QWidget *parent) : QWidget(parent)

{
    refreshPending = false;
    applyingColors = false;
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

    // colorize horizontal bar
    applyVerticalStyle();

    setRow(0);
    setCol(0);

    installEventFilter(this);
}

bool caDoubleTabWidget::eventFilter(QObject *obj, QEvent *event)
{
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

    return QWidget::eventFilter(obj, event);
}

int caDoubleTabWidget::lookupArrayIndex(int row, int col)
{
    QMapIterator<int, twoInts> j(lookup);
    while (j.hasNext()) {
        j.next();
        qCDebug(caDoubleTabWidgetLog) << "viewportIndex" << j.key() << "in array row=" << j.value().r << "col=" << j.value().c;

    }
    qCDebug(caDoubleTabWidgetLog) << "number of viewport pages =" << count();


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
    qCDebug(caDoubleTabWidgetLog) << "store in array row=" << row << "col=" << col << "index of viewport=" << pageIndex;
    twoInts item;
    item.r = row;
    item.c = col;
    lookup.insert(pageIndex, item);
}

void  caDoubleTabWidget::deleteArrayIndex(int pageIndex)
{
    qCDebug(caDoubleTabWidgetLog) << "delete in array row=" << row << "col=" << col << "index of viewport=" << pageIndex;
    lookup.remove(pageIndex);
}


// add page
void caDoubleTabWidget::addPage(QWidget *page)
{
    qCDebug(caDoubleTabWidgetLog) << "add a new page" << page->objectName();
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
    qCDebug(caDoubleTabWidgetLog) << "we have to remove the page from array for row=" << row << "col=" << col;
    if((pageIndex =lookupArrayIndex(row, col)) != -1) {

        // delete this stackwidget page from the array
        deleteArrayIndex(pageIndex);
        QWidget *widget = viewPort->widget(index);
        qCDebug(caDoubleTabWidgetLog) << "remove widget at stacked widget index=" << index << "with name=" << widget->objectName();
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
        qCDebug(caDoubleTabWidgetLog) << "page not found, return";
        return;
    }
}

// insert a new viewport
void caDoubleTabWidget::insertPage(int index, QWidget *page)
{
    Q_UNUSED(index)
    page->setParent(viewPort);
    page->activateWindow();

    qCDebug(caDoubleTabWidgetLog) << "insert page for actual row=" << row << "column=" <<col;
    if(lookupArrayIndex(row, col) == -1 || addPages) {
        int viewPortIndex = viewPort->insertWidget((count()), page);
       qCDebug(caDoubleTabWidgetLog) << "stored with viewIndex=" << viewPortIndex;
        storeArrayIndex(viewPortIndex, row, col);
    } else {
        qCDebug(caDoubleTabWidgetLog) << "already done, return";
        return;
    }
    QString title = tr("Page_%1_%2").arg(row).arg(col);
    qCDebug(caDoubleTabWidgetLog) << "set page title to" << title;
    page->setObjectName(title);
    page->setAutoFillBackground(true);
    if(page->styleSheet().size() < 1) {          // pages without an authored sheet get ours
        ownedPages.append(page);
        ThemeColors::markSelfStyled(page);
        restylePages();
    }
}

// background of the pages we colour ourselves, by scheme
void caDoubleTabWidget::restylePages()
{
    const QString background = ThemeColors::css(caDoubleTabColor::Page, ThemeColors::schemeOf(this));
    foreach(const QPointer<QWidget> &page, ownedPages) {
        if(page.isNull()) continue;
        const QString style = QString("QWidget#%1 { background-color : %2; }").arg(page->objectName(), background);
        if(page->styleSheet() != style) page->setStyleSheet(style);
    }
}

// the panel palette changed (theme switch): regenerate the colours, deferred
void caDoubleTabWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    if(applyingColors || refreshPending) return;
    if(e->type() == QEvent::PaletteChange || e->type() == QEvent::StyleChange) {
        refreshPending = true;
        QTimer::singleShot(0, this, SLOT(refreshTheme()));
    }
}

void caDoubleTabWidget::refreshTheme()
{
    refreshPending = false;
    applyVerticalStyle();
    restylePages();
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
        applyVerticalStyle();
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
        qCDebug(caDoubleTabWidgetLog) << "found" << Index;
        viewPort->setCurrentIndex(Index);
        emit currentIndexChanged(Index);
    } else {
        qCDebug(caDoubleTabWidgetLog) << "not found";
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

}

void caDoubleTabWidget::setItemsPadding(QString const &padding) {
    thisVerPadding= padding.split(";");
    applyVerticalStyle();
}


// stylesheet of the vertical buttons: an authored sheet wins, otherwise it is
// generated from the theme (one sheet; the padding of the first button, as before)
void caDoubleTabWidget::applyVerticalStyle()
{
    if(!authoredStyleSheet.isEmpty()) {
        qCDebug(caDoubleTabWidgetLog) << "authored style" << authoredStyleSheet.left(80);
        ThemeColors::markSelfStyled(this, false);
        if(styleSheet() != authoredStyleSheet) {
            applyingColors = true;
            setStyleSheet(authoredStyleSheet);
            applyingColors = false;
        }
        return;
    }
    if(vTabBar->buttons().count() < 1) return;

    int padding = 0;
    if(thisVerPadding.count() > 0) padding = thisVerPadding.at(0).toInt();

    const ThemeColors::Scheme s = ThemeColors::schemeOf(this);
    QString style = "QPushButton {border: 2px solid " + ThemeColors::css(caDoubleTabColor::ButtonBorder, s)
            + "; border-radius: 6px ; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 "
            + ThemeColors::css(caDoubleTabColor::GradientTop, s) + ", stop: 1 "
            + ThemeColors::css(caDoubleTabColor::GradientBottom, s) + ");  ";

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

    style.append(tr("text-align: left; padding-left: %1px;").arg(padding));
    style.append("} ");
    style.append("QPushButton:checked {background-color: " + ThemeColors::css(caDoubleTabColor::Checked, s) + ";}");
    style.append("QPushButton:default {border-color: " + ThemeColors::css(caDoubleTabColor::DefaultBorder, s) + "; }");
    style.append("QTabBar::tab  {background-color: " + ThemeColors::css(caDoubleTabColor::Checked, s) + ";}");

    ThemeColors::markSelfStyled(this);
    qCDebug(caDoubleTabWidgetLog) << "vertical style" << (s == ThemeColors::Dark ? "dark" : "light") << (style != styleSheet()) << style.left(80);
    if(style != styleSheet()) {
        applyingColors = true;
        setStyleSheet(style);
        applyingColors = false;
    }
}

// authored stylesheet (designer/.ui); an empty one brings the generated style back
void caDoubleTabWidget::editStyleSheet(QString styleSheet){
    authoredStyleSheet = styleSheet;
    applyVerticalStyle();
}
