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

void sliderDialog::setNewStyleSheet(QWidget* w, QSize size, QString myStyle, int pointSizeCorrection)
{
    int pointSize;
    if(size.height() > 500) pointSize = 16;
    else pointSize = 10;

    pointSize = pointSize + pointSizeCorrection;

    QString style = "font: %1pt; %2";
    style = style.arg(pointSize).arg(myStyle);
    w->setStyleSheet(style);
}

sliderDialog::sliderDialog(caSlider *w, MutexKnobData *data, const QString &title, QWidget *parent) : QWidget(parent)
{
    QString text;
    int thisWidth = 650;
    int thisHeight = 200;

    slider = w;
    thisParent = parent;

    monData = data;

    QGridLayout *Layout = new QGridLayout;
    setWindowModality (Qt::WindowModal);

#ifdef Q_OS_IOS
    if(qApp->desktop()->size().height() < 500) {
        thisWidth=430;  // normal for iphone
        thisHeight=150;
    }
    setNewStyleSheet(this, qApp->desktop()->size());
    QPalette palette;
    palette.setBrush(QPalette::Background, QColor(255,255,224,255));
    setPalette(palette);
    setAutoFillBackground(true);
    setGeometry(QStyle::alignedRect(Qt::LeftToRight,Qt::AlignCenter, QSize(thisWidth,thisHeight), qApp->desktop()->availableGeometry()));
#else
    Qt::WindowFlags flags = Qt::Dialog;
    setWindowFlags(flags);
    move(parent->x() + parent->width() / 2 - thisWidth / 2 , parent->y() + parent->height() /2 -thisHeight/2);
#endif

    QVBoxLayout *mainLayout = new QVBoxLayout;
    QGroupBox *groupBox = new QGroupBox("Increment and value change");

    QLabel *label1 = new QLabel("increment");
    text = text.setNum(slider->getIncrementValue());
    incrementEdit = new QLineEdit(text);

    QLabel *label2 = new QLabel("value");
    text = text.setNum(slider->getSliderValue());
    valueEdit = new QLineEdit(text);

    Layout->addWidget(label1,    1, 0);
    Layout->addWidget(incrementEdit,  1, 1);
    Layout->addWidget(label2,  2, 0);
    Layout->addWidget(valueEdit,  2, 1);

    buttonBox = new QDialogButtonBox( Qt::Horizontal );
    QPushButton *buttonReturn = new QPushButton( "Return" );
    connect( buttonReturn, SIGNAL(clicked()), this, SLOT(cancelClicked()) );
    buttonBox->addButton(buttonReturn, QDialogButtonBox::RejectRole );

    QPushButton *buttonApply = new QPushButton( "Apply" );
    connect(buttonApply, SIGNAL(clicked()), this, SLOT(applyClicked()) );
    buttonBox->addButton(buttonApply, QDialogButtonBox::ApplyRole);

    Layout->addWidget(buttonBox, 3, 0);

    groupBox->setLayout(Layout);
    mainLayout->addWidget(groupBox);

    setLayout(mainLayout);

    thisPV = slider->getPV();
    QLabel *Title = new QLabel(thisPV + " / " + w->objectName());
    Layout->addWidget(Title,0,0,1,-1);

    setWindowTitle(title);

    showNormal();
}

void sliderDialog::cancelClicked()
{
       close();
}

void sliderDialog::applyClicked()
{
    bool ok;

    QString text = incrementEdit->text();
    double value = text.toDouble(&ok);
    if(ok) slider->setIncrementValue(value);

    text = valueEdit->text();
    value = text.toDouble(&ok);
    if(ok) slider->setSliderValue(value);
}

void sliderDialog::exec()
{
    connect(buttonBox, SIGNAL(rejected()), &loop, SLOT(quit()) );
    connect(buttonBox, SIGNAL(accepted()), &loop, SLOT(quit()) );
    loop.exec();
    close();
    deleteLater();
}

void sliderDialog::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    loop.quit();
}

void sliderDialog::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    QPen pen(Qt::black, 3, Qt::SolidLine, Qt::FlatCap, Qt::RoundJoin);
    painter.setPen(pen);
    painter.drawRoundedRect(5, 5, width()-7, height()-7, 3, 3);

    QWidget::paintEvent(e);
}

