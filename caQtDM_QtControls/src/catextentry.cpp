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
#include <QMouseEvent>

caTextEntry::caTextEntry(QWidget *parent) : caLineEdit(parent)
{
  // this dis not really worked on ios, while the events had another order
  //connect(this, SIGNAL(returnPressed()), this, SLOT(dataInput()));
    clearFocus();
    setAccessW(true);
    installEventFilter(this);
    newFocusPolicy(Qt::ClickFocus);
    this->setAcceptDrops(false);
    setFromTextEntry();

    setElevation(on_top);
}

// routine not used any more
void caTextEntry::dataInput()
{
    //printf("dataInput %s\n", text().toLatin1().constData());
    emit TextEntryChanged(text());
}

void caTextEntry::setAccessW(bool access)
{
    _AccessW = access;
}

void caTextEntry::updateText(const QString &txt)
{
    //printf("text written by CS %s\n", txt.toLatin1().constData());
    startText = txt;
}

bool caTextEntry::eventFilter(QObject *obj, QEvent *event)
{
    // repeat enter or return key are not really wanted
	if (event->type() == QEvent::KeyPress){
		QKeyEvent *ev = static_cast<QKeyEvent *>(event);
		if (ev != (QKeyEvent *)0) {
			if (ev->key() == Qt::Key_Return || ev->key() == Qt::Key_Enter) {
				if (ev->isAutoRepeat()) {
                    //printf("keyPressEvent ignore\n");
                    event->ignore();
				}
				else {
					event->accept();
                    //printf("keyPressEvent accept, set text to %s entered=%s ?\n", startText.toLatin1().constData(), text().toLatin1().constData());
                    emit TextEntryChanged(text().toLatin1().constData());
				}
			}
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
        this->activateWindow();  // I added this for ios while I could not get the focus
    } else if(event->type() == QEvent::Leave) {
        QApplication::restoreOverrideCursor();
        setReadOnly(false);
        clearFocus();
    } else if(event->type() == QEvent::FocusOut) {
        //printf("lost focus, set text to %s\n", startText.toLatin1().constData());
        forceText(startText);
    } else if (event->type() == QEvent::FocusIn) {
        //printf("focus in\n");
    }
    return QObject::eventFilter(obj, event);
}
