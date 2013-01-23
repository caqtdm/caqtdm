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

// used for about, icon will be changed here

#include "messagebox.h"


QTDMMessageBox::QTDMMessageBox (Icon icon,
                        const QString & title,
                        const QString & text,
                        StandardButtons buttons,
                        QWidget * parent,
                        Qt::WindowFlags f,
                        bool autoClose)

    : QMessageBox (icon,title,text,buttons, parent,f)

{
    QPixmap pixmap = QPixmap(":/caQtDM.png");
    QPixmap scaledPixmap = pixmap.scaledToWidth(100);
    setIconPixmap(scaledPixmap);
    timeout = 5;
    _autoClose = autoClose;
    if (_autoClose) {
        currentTime = 0;
        this->startTimer(1000);
    }
}

QTDMMessageBox::~QTDMMessageBox() {

}

void QTDMMessageBox::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    currentTime++;
    if (currentTime >= timeout) {
        this->deleteLater();
    }
}
