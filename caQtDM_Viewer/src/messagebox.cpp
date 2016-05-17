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

// used for about, icon will be changed here

#include "messagebox.h"


QTDMMessageBox::QTDMMessageBox (Icon icon,
                        const QString & title,
                        const QString & text,
                        const QString & iconString,
                        StandardButtons buttons,
                        QWidget * parent,
                        Qt::WindowFlags f,
                        bool autoClose)

    : QMessageBox (icon,title,text,buttons, parent,f)

{
    QPixmap pixmap = QPixmap(iconString);
    QPixmap scaledPixmap = pixmap.scaledToWidth(150);
    setIconPixmap(scaledPixmap);
    timeout = 10;
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
