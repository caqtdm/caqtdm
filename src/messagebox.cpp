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

#include "messagebox.h"


MessageBox::MessageBox (Icon icon,
                        const QString & title,
                        const QString & text,
                        StandardButtons buttons,
                        QWidget * parent,
                        Qt::WindowFlags f,
                        bool autoClose)

    : QMessageBox (icon,title,text,buttons, parent,f)

{
    timeout = 5;
    _autoClose = autoClose;
    if (_autoClose) {
        currentTime = 0;
        this->startTimer(1000);
    }
}

MessageBox::~MessageBox() {

}

void MessageBox::timerEvent(QTimerEvent *event)
{
    currentTime++;
    if (currentTime >= timeout) {
        this->deleteLater();
    }
}
