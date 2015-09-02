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

#include "camenu.h"
#include "alarmdefs.h"
#include <QLineEdit>
#include <QMouseEvent>
#include<QApplication>

caMenu::caMenu(QWidget *parent) : QComboBox(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    defaultPalette = palette();
    setLabelDisplay(false);
    setAccessW(true);
    installEventFilter(this);

    setColorMode(Default);
    setBackground(QColor(230,230,230));
    setForeground(QColor(0,0,0));

    setElevation(on_top);
}

QString caMenu::getPV() const
{
    return thisPV;
}

void caMenu::setPV(QString const &newPV)
{
    thisPV = newPV;
    if(thisLabelDisplay) {
        setLabelDisplay(true);
    }
}

void caMenu::setBackground(QColor c)
{
    thisBackColor = c;
    setColors(thisBackColor, thisForeColor, false);
}

void caMenu::setForeground(QColor c)
{
    thisForeColor = c;
    setColors(thisBackColor, thisForeColor, false);
}

void caMenu::setColors(QColor bg, QColor fg, bool force)
{
    if((bg != oldBackColor) || (fg != oldForeColor) || force) {
        thisPalette = palette();
        thisPalette.setColor(QPalette::ButtonText, fg);
        thisPalette.setColor(QPalette::Button, bg);
        if(thisColorMode != Default) {
           setPalette(defaultPalette);
           setPalette(thisPalette);
        } else {
           setPalette(defaultPalette);
        }
        oldBackColor = bg;
        oldForeColor = fg;
    }
}

void caMenu::setAlarmColors(short status)
{
    QColor bg, fg;
    fg = thisForeColor;
    switch (status) {

    case NO_ALARM:
        bg = AL_GREEN;
        break;
    case MINOR_ALARM:
        bg = AL_YELLOW;
        break;
    case MAJOR_ALARM:
        bg = AL_RED;
        break;
    case INVALID_ALARM:
    case NOTCONNECTED:
        bg = AL_WHITE;
        fg = bg;
        break;
    default:
        bg = AL_DEFAULT;
        fg = thisForeColor;
        break;
    }
    setColors(bg, fg, false);
}

void caMenu::setNormalColors()
{
    setColors(thisBackColor, thisForeColor, false);
}

QString caMenu::getLabel() const
{
    if(thisLabelDisplay) {
        if(thisPV.contains(":")) {
            QStringList list = thisPV.split(":");
            return list[0];
        } else {
            return "";
        }
    } else {
        return "";
    }
}

// this is to add a label on the menu with the first part of a device name
void caMenu::setLabelDisplay(bool c)
{
    thisLabelDisplay= c;
    clear();
    if(thisLabelDisplay) {
            addItem(getLabel());
            setCurrentIndex(0);
    }
}

void caMenu::setAccessW(int access)
{
     thisAccessW = access;
}

void caMenu::populateCells(QStringList stringlist)
{

    // remove first event filter for the actual items
    QList<QWidget*> widgets = this->findChildren<QWidget*>();
    foreach (QWidget* widget, widgets) widget->removeEventFilter(this);

    clear();
    if(getLabelDisplay()) {
        addItem(getLabel());
    }
    addItems(stringlist);

    // add now the event filter for the new items
    widgets = this->findChildren<QWidget*>();
    foreach (QWidget* widget, widgets) widget->installEventFilter(this);
}

bool caMenu::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Enter) {
        if(!thisAccessW) {
            // disable also menu items
            for (int i=0; i < this->count(); i++) {
              QModelIndex index = model()->index(i, 0);
              QVariant v(0);   // disable flag
              model()->setData(index, v, Qt::UserRole - 1);
            }
            QApplication::setOverrideCursor(QCursor(Qt::ForbiddenCursor));

        } else {
            QApplication::restoreOverrideCursor();
        }
    } else if(event->type() == QEvent::Leave) {
        QApplication::restoreOverrideCursor();
        // enable menu items again
        for (int i=0; i < this->count(); i++) {
          QModelIndex index = model()->index(i, 0);
          QVariant v(1 | 32);  // enable flag
          model()->setData(index, v, Qt::UserRole - 1);
        }

    } else if(event->type() == QEvent::Wheel) {
        event->ignore();
        return true;
    }
    return QObject::eventFilter(obj, event);
}

