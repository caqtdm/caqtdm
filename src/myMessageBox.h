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


#ifndef MYMESSAGEBOX_H
#define MYMESSAGEBOX_H


#include <QDialog>
#include <QTextEdit>

class myMessageBox : public QDialog
{
public:
    explicit myMessageBox(QWidget *parent = 0);
    void setText(QString strng) const;

private:
    QTextEdit *thisText;
};



#endif
