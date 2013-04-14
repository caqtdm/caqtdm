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

