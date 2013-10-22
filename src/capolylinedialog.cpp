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

