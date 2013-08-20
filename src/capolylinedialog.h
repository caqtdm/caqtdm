//******************************************************************************
// Copyright (c) 2012 Paul Scherrer Institut PSI), Villigen, Switzerland
// Disclaimer: neither  PSI, nor any of their employees makes any warranty
// or assumes any legal liability or responsibility for the use of this software
//******************************************************************************
//******************************************************************************
//
//     Author : Anton Chr. Mezger
//
//********************************************************************************

#ifndef CAPOLYLINEDIALOG_H
#define CAPOLYLINEDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <qtcontrols_global.h>

QT_BEGIN_NAMESPACE
class QDialogButtonBox;
QT_END_NAMESPACE
class caPolyLine;

//! [0]
class QTCON_EXPORT caPolyLineDialog : public QDialog
{
    Q_OBJECT

public:
    explicit caPolyLineDialog(caPolyLine *plugin = 0, QWidget *parent = 0);

    QSize sizeHint() const;

private slots:
    void resetState();
    void saveState();

private:
    caPolyLine *editor;
    caPolyLine *capolyLine;
    QDialogButtonBox *buttonBox;
};
//! [0]

#endif
