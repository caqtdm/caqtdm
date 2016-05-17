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

#include "myMessageBox.h"

myMessageBox::myMessageBox(QWidget *parent) : QWidget(parent)
{
    bool showMax = false;
    Qt::WindowFlags flags = Qt::Dialog;
    setWindowFlags(flags);
    setWindowModality (Qt::WindowModal);

    thisText = new QTextEdit(this);
    thisText->setReadOnly(true);
    thisText->setTextInteractionFlags(Qt::TextSelectableByMouse);
    thisText->setLineWrapMode(QTextEdit::NoWrap);

#if defined(MOBILE_IOS)
    Specials special;
    QSize size = qApp->desktop()->size();
    if(size.height() < 500) {
        setGeometry(QStyle::alignedRect(Qt::LeftToRight,Qt::AlignCenter, QSize(250,250), qApp->desktop()->availableGeometry()));
        setMinimumSize(250,250);
    } else {
        setGeometry(QStyle::alignedRect(Qt::LeftToRight,Qt::AlignCenter, QSize(350,500), qApp->desktop()->availableGeometry()));
        setMinimumSize(350,500);
    }
    special.setNewStyleSheet(thisText, qApp->desktop()->size(), 16, 10);
#elif defined(MOBILE_ANDROID)
    showMax = true;
#else
    move(parent->x() + parent->width() / 2 - 175, parent->y()+25);
    setMinimumSize(350,500);
#endif

    QPushButton *cancelButton = new QPushButton(tr("Close"));

    buttonBox = new QDialogButtonBox(Qt::Horizontal);
    buttonBox->addButton(cancelButton, QDialogButtonBox::RejectRole);

    QVBoxLayout *lt = new QVBoxLayout;
    lt->addWidget(thisText);
    lt->addWidget(buttonBox);

    setLayout(lt);
    if(!showMax) showNormal();
    else showMaximized();
}

void myMessageBox::setText(QString strng) const
{
   thisText->setText(strng);
}

void myMessageBox::exec()
{
    connect(buttonBox, SIGNAL(rejected()), &loop, SLOT(quit()) );
    loop.exec();
    deleteLater();
}

void myMessageBox::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    loop.quit();
}

void myMessageBox::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    QPen pen(Qt::black, 3, Qt::SolidLine, Qt::FlatCap, Qt::RoundJoin);
    painter.setPen(pen);
    painter.drawRoundedRect(5, 5, width()-7, height()-7, 3, 3);

    QWidget::paintEvent(e);
}

