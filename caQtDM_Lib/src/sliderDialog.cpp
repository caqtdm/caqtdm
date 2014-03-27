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
#include "sliderDialog.h"

sliderDialog::sliderDialog(caSlider *w, MutexKnobData *data, const QString &title, QWidget *parent) : QDialog(parent)
{
    QString text;
    slider = w;
    monData = data;
    QGridLayout *Layout = new QGridLayout;

    QLabel *label1 = new QLabel("increment");
    text = text.setNum(slider->getIncrementValue());
    incrementEdit = new QLineEdit(text);

    QLabel *label2 = new QLabel("value");
    text = text.setNum(slider->getSliderValue());
    valueEdit = new QLineEdit(text);

    Layout->addWidget(label1,    0, 0);
    Layout->addWidget(incrementEdit,  0, 1);
    Layout->addWidget(label2,  1, 0);
    Layout->addWidget(valueEdit,  1, 1);

    QDialogButtonBox *box = new QDialogButtonBox( Qt::Horizontal );
    QPushButton *button = new QPushButton( "Return" );
    connect( button, SIGNAL(clicked()), this, SLOT(cancelClicked()) );

    box->addButton(button, QDialogButtonBox::RejectRole );

    button = new QPushButton( "Apply" );
    connect( button, SIGNAL(clicked()), this, SLOT(applyClicked()) );
    box->addButton(button, QDialogButtonBox::ApplyRole );

    Layout->addWidget(box, 2, 0);

    setLayout(Layout);

    setWindowTitle(title);

}

void sliderDialog::cancelClicked()
{
    reject();
}

void sliderDialog::applyClicked()
{
    bool ok;

    QString text = incrementEdit->text();
    double value = text.toDouble(&ok);

    if(ok) slider->setIncrementValue(text.toDouble(&ok));

    text = valueEdit->text();
    value = text.toDouble(&ok);

    if(ok) slider->setSliderValue(text.toDouble(&ok));
}

