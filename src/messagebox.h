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

#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <QMessageBox>

class MessageBox : public QMessageBox {

public:

MessageBox ( Icon icon,
             const QString & title,
             const QString & text,
             StandardButtons buttons = NoButton,
             QWidget * parent = 0,
             Qt::WindowFlags f = Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint,
             bool autoClose = false);

~MessageBox() {}

protected:

void timerEvent(QTimerEvent *event);

private:

int timeout;
bool _autoClose;
int currentTime;

};
#endif
