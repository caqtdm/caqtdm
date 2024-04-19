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
#include <QMimeData>

caTextEntry::caTextEntry(QWidget *parent) : caLineEdit(parent)
{
  // this dis not really worked on ios, while the events had another order
  //connect(this, SIGNAL(returnPressed()), this, SLOT(dataInput()));
    clearFocus();
    setKeepFocus(false);
    setAccessW(true);
    installEventFilter(this);
    newFocusPolicy(Qt::StrongFocus);
    this->setAcceptDrops(false);
    setFromTextEntry();

    setElevation(on_top);

    setAcceptDrops(true);
}

void caTextEntry::setValue(double v)
{
    setText(QString::number(v));
    emit TextEntryChanged(QString::number(v));
}
void caTextEntry::setValue(QString string)
{
    setText(string);
    emit TextEntryChanged(string);
}

// routine not used any more
void caTextEntry::dataInput()
{
    //printf("dataInput %s\n", qasc(text()));
    emit TextEntryChanged(text());
}

void caTextEntry::setAccessW(bool access)
{
    _AccessW = access;
}

void caTextEntry::updateText(const QString &txt)
{
    //printf("text written by CS %s\n", qasc(txt));
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
                    //printf("keyPressEvent accept, set text to %s entered=%s ?\n", qasc(startText), qasc(text()));
                    emit TextEntryChanged(qasc(text()));
				}
			}
		}
    // move cursor with tab focus
    } else if(event->type() == QEvent::KeyRelease) {
        QKeyEvent *ev = static_cast<QKeyEvent *>(event);
        if (ev != (QKeyEvent *)0) {
            if(ev->key() == Qt::Key_Tab) {
                QCursor *cur = new QCursor;
                QPoint p = QWidget::mapToGlobal(QPoint(this->width()/2, this->height()/2));
                cur->setPos( p.x(), p.y());
                setFocus();
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

// The following workaround is only done on mobile as it isn't needed on desktop and only has unwanted side effects,
// such as the widget getting the focus while another application is focused, only by hovering over it with the mouse cursor.
#ifdef MOBILE
            this->activateWindow();  // I added this for ios while I could not get the focus
#endif

    } else if(event->type() == QEvent::Leave) {
        QApplication::restoreOverrideCursor();
        setReadOnly(false);
        if(!keepFocusOnLeave) clearFocus();
    } else if(event->type() == QEvent::FocusOut) {
        //printf("lost focus, set text to %s\n", qasc(startText));
        forceText(startText);
    } else if (event->type() == QEvent::FocusIn) {
        //printf("focus in\n");
    }
    return QObject::eventFilter(obj, event);
}

void caTextEntry::dragEnterEvent(QDragEnterEvent *event)
{
    setBackgroundRole(QPalette::Highlight);
    event->acceptProposedAction();
}

void caTextEntry::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void caTextEntry::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    setBackgroundRole(QPalette::Dark);
    if (mimeData->hasText()) {
        event->acceptProposedAction();
        emit TextEntryChanged(event->mimeData()->text());
    } else {

    }
}

void caTextEntry::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
}
