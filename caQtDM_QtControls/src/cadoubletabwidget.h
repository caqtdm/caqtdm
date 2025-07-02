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

#ifndef DOUBLEWIDGET_H
#define DOUBLEWIDGET_H
#include <QtGui>
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

#include <QWidget>
#include <QTabBar>
#include <QPushButton>
#include <QButtonGroup>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QStackedWidget>

#include <qtcontrols_global.h>

typedef char string40[40];

class QTCON_EXPORT caDoubleTabWidget : public QWidget
{

    Q_OBJECT

    Q_PROPERTY(QString itemsHorizontal READ getItemsHor WRITE setItemsHor)
    Q_PROPERTY(QString itemsVertical READ getItemsVer WRITE setItemsVer)
    Q_PROPERTY(QString itemsVerticalPadding READ getItemsPadding WRITE setItemsPadding)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex)

    // this will prevent user interference
    Q_PROPERTY(QString styleSheet READ styleSheet WRITE editStyleSheet) // DESIGNABLE false

public:
    void noStyle(QString style) {Q_UNUSED(style);}

    caDoubleTabWidget(QWidget *parent);

    QString getItemsHor() const {return thisHorItems.join(";");}
    void setItemsHor(QString const &items) {thisHorItems = items.split(";"); removeTabs(0); addSampleWidget(0);}

    QString getItemsVer() const {return thisVerItems.join(";");}
    void setItemsVer(QString const &items) {thisVerItems = items.split(";"); removeTabs(1); addSampleWidget(1);}

    QString getItemsPadding() const {return thisVerPadding.join(";");}
    void setItemsPadding(QString const &items);

    void addSampleWidget(int);
    void removeTabs(int);

    int count() const;
    int currentIndex() const;
    QWidget *widget(int index);

    // array handling routines
    int  lookupArrayIndex(int row, int col);
    void storeArrayIndex(int index, int row, int col);
    void deleteArrayIndex(int pageIndex);

    void setFont(int dir);

public slots:
    void animation(QRect p) {
#include "animationcode.h"
    }
    void editStyleSheet(QString styleSheet);

    void hideObject(bool hideit) {
#include "hideobjectcode.h"
    }

public slots:
    void setCurrentIndex(int index);
    void setRow(int r);
    void setCol(int c);
    void addPage(QWidget *page);
    void insertPage(int index, QWidget *page);
    void removePage(int index);

signals:
    void currentChanged(int indx);
    void currentIndexChanged(int index);

protected:
    virtual void fontChange(const QFont & oldFont);

private:
    struct twoInts {
        int r;
        int c;
    };

    bool eventFilter(QObject *obj, QEvent *event);

    QStringList	thisHorItems;
    QStringList	thisVerItems;
    QStringList	thisVerPadding;
    int row;
    int col;
    QTabBar *hTabBar;
    QButtonGroup *vTabBar;
    QStackedWidget *viewPort;
    QMap<int, twoInts> lookup;
    QLineEdit *tableIndex;
    bool addPages;
    QVBoxLayout*  buttonLayout;
    int vCount;
};

#endif
