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

#include <QtGui>
#include <QtDesigner/QtDesigner>

#include "capolyline.h"
#include "capolylinedialog.h"

caPolyLineDialog::caPolyLineDialog(caPolyLine *tic, QWidget *parent) : QDialog(parent)
{
    capolyLine = tic;
    editor = new caPolyLine;

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel
                                     | QDialogButtonBox::Reset);

    connect(buttonBox->button(QDialogButtonBox::Reset), SIGNAL(clicked()),
            this, SLOT(resetState()));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(saveState()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(editor);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    setWindowTitle(tr("Edit PolyLine"));
    int dialogWidth  = capolyLine->width()+40;
    int dialogHeight = capolyLine->height()+80;
    if(dialogWidth < 300) dialogWidth = 300;
    if(dialogHeight < 300) dialogHeight = 300;
    setFixedWidth(dialogWidth);
    setFixedHeight(dialogHeight);

    editor->setEditSize(0,0, capolyLine->width(), capolyLine->height());

    editor->setPairs(capolyLine->pairs());
    editor->setLineSize(tic->getLineSize());
    editor->setLineStyle(tic->getLineStyle());
    editor->setLineColor(tic->getLineColor());
    editor->setForeground(tic->getForeground());
    editor->setFillStyle(tic->getFillStyle());
    editor->setPolyStyle(tic->getPolyStyle());

    editor->setInEditor(true);
}

QSize caPolyLineDialog::sizeHint() const
{
    return QSize(250, 250);
}

void caPolyLineDialog::resetState()
{
    editor->clearPairs();
}

void caPolyLineDialog::saveState()
{
    if (QDesignerFormWindowInterface *formWindow
            = QDesignerFormWindowInterface::findFormWindow(capolyLine)) {
        formWindow->cursor()->setProperty("xyPairs", editor->pairs());
    }

    accept();
}

