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

#include "configDialog.h"

configDialog::configDialog(const QList<QString> &urls, const QList<QString> &files, QWidget *parent): QDialog(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;

    setWindowTitle("caQtDM tablet configuration");
    mainLayout->setSizeConstraint(QLayout::SetNoConstraint);
    mainLayout->addWidget(new QLabel("<h1>Configuration data</h1>"), 0, Qt::AlignCenter);

    QPushButton* clearButton = new QPushButton("Clear and prepare local files");
    mainLayout->addWidget(clearButton);

    QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal );
    connect(box, SIGNAL(rejected()), this, SLOT(reject()));
    connect(box, SIGNAL(accepted()), this, SLOT(accept()));

    QGridLayout* urlLayout = new QGridLayout;
    QGroupBox* urlBox = new QGroupBox("Choose your url where your config file is located");
    for(int i=0; i< qMin(10,urls.length()); i++) {
           urlEdit[i] = new QLineEdit(urls.at(i));
           urlLayout->addWidget(urlEdit[i], i, 0);
           urlRadio[i] = new QRadioButton();
           urlLayout->addWidget(urlRadio[i], i, 1);
    }
    urlRadio[0]->setChecked(true);
    urlBox->setLayout(urlLayout);
    mainLayout->addWidget(urlBox);

    QGridLayout* fileLayout = new QGridLayout;
    QGroupBox* fileBox = new QGroupBox("Choose your config file at the above url");

    for(int i=0; i< qMin(10,files.length()); i++) {
           fileEdit[i] = new QLineEdit(files.at(i));
           fileLayout->addWidget(fileEdit[i], i, 0);
           fileRadio[i] = new QRadioButton();
           fileLayout->addWidget(fileRadio[i], i, 1);
    }
    fileRadio[0]->setChecked(true);
    fileBox->setLayout(fileLayout);
    mainLayout->addWidget(fileBox);

    mainLayout->addWidget(box);

    setLayout(mainLayout);
}

void configDialog::getChoice(QString &url, QString &file)
{
   for(int i=0; i< 10; i++) {
       if(urlRadio[i]->isChecked()) {
           url = urlEdit[i]->text();
           break;
       }
   }
   for(int i=0; i< 10; i++) {
       if(fileRadio[i]->isChecked()) {
           file = fileEdit[i]->text();
           break;
       }
   }
}


