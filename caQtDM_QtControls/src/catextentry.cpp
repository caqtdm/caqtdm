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

#include "catextentry.h"
#include <QApplication>
#include <QtDebug>
#include <QMouseEvent>

caTextEntry::caTextEntry(QWidget *parent) : caLineEdit(parent)
{
    connect(this, SIGNAL(returnPressed()), this, SLOT(dataInput()));
    clearFocus();
    setAccessW(true);
    installEventFilter(this);
    newFocusPolicy(Qt::ClickFocus);
    this->setAcceptDrops(false);
}

void caTextEntry::dataInput()
{
    //printf("return pressed %s\n", text().toAscii().constData());
    emit TextEntryChanged(text());
}

void caTextEntry::setAccessW(int access)
{
    _AccessW = access;
}

void caTextEntry::updateText(const QString &txt)
{
    //printf("text written by CS %s\n", txt.toAscii().constData());
    startText = txt;
}

bool caTextEntry::eventFilter(QObject *obj, QEvent *event)
{
    // repeat enter or return key are not really wanted
    QKeyEvent *ev = static_cast<QKeyEvent *>(event);
    if( ev->key()==Qt::Key_Return || ev->key()==Qt::Key_Enter ) {
        if(ev->isAutoRepeat() ) {
            //printf("keyPressEvent ignore\n");
            event->ignore();
        } else {
            //printf("keyPressEvent accept\n");
            event->accept();
        }
    }

    // treat mouse enter and leave as well as focus out
    if (event->type() == QEvent::Enter) {
        if(!_AccessW) {
            QApplication::setOverrideCursor(QCursor(Qt::ForbiddenCursor));
            setReadOnly(true);
        } else {
            QApplication::restoreOverrideCursor();
        }
    } else if(event->type() == QEvent::Leave) {
        QApplication::restoreOverrideCursor();
        setReadOnly(false);
        clearFocus();
    } else if(event->type() == QEvent::FocusOut) {
        //printf("lost focus, set text to %s\n", startText.toAscii().constData());
        forceText(startText);
    }
    return QObject::eventFilter(obj, event);
}
