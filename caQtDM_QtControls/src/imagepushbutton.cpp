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

#include "imagepushbutton.h"
#include <QResizeEvent>
#include <QPainter>

ImagePushButton::ImagePushButton(const QString & text, const QString& image, QWidget *parent): EPushButton(text, parent)
{
    myText = text;
    myImage= image;
    iconPresent = false;
    invisible = false;

    //  load from the resources
    iconOK = true;
    QString fileName =  ":/pixmaps/%1";
    fileName = fileName.arg(image);
    pixmap = QPixmap(fileName);
    resize(pixmap.width(), pixmap.height());
}

void ImagePushButton:: setLabelText(const QString& text) {
    myText = text;
    update();
}

void ImagePushButton::setIconVisible(bool b) {
    iconPresent = b;
    update();
}

void ImagePushButton::setInVisible(QColor bg, QColor fg, QColor bc) {
    invisible = true;
    thisbg = bg;
    thisfg = fg;
    thisbc = bc;
    update();
}

void ImagePushButton::resizeEvent(QResizeEvent *e) {
    QSize size = e->size();
    setGeometry(0, 0, size.width(), size.height());
    EPushButton::resizeEvent(e);
}

void ImagePushButton::paintEvent( QPaintEvent* event) {

    int x, y, w, h;
    EPushButton::paintEvent(event);
    QPainter p(this);

    p.setRenderHint( QPainter::Antialiasing );

    QRect r = this->geometry();
    x=r.x(); y=r.y(); w=r.width(); h=r.height();

    if(invisible) {
        thisbg.setAlpha(0);
        thisfg.setAlpha(0);
        p.setBackground(thisbg);
        p.setPen(thisfg);
        p.drawRect(QRect(0, 0, w, h));
    }  else {
        if(iconPresent && iconOK) {
            int hpix = h * 0.9;
            QPixmap pixnew1 = pixmap.scaledToHeight(hpix, Qt::SmoothTransformation);
            int wpix = pixnew1.width() * 0.85;
            QPixmap pixnew2 = pixnew1.scaled(wpix, hpix, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            int pixw = pixnew2.width();
            int pixh = pixnew2.height();
            p.drawPixmap( 1, y+h/2-pixh/2, pixnew2);
            x += pixw + 4;
            w -= pixw + 4;
        }
        r.setRect(x, y, w, h);
    }
}

