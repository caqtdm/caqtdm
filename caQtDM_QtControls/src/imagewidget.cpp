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
#include <QPainter>
#ifndef QT_NO_CONCURRENT
#include <qtconcurrentrun.h>
#endif
#include <math.h>
#include "imagewidget.h"

ImageWidget::ImageWidget(QWidget *parent) : QWidget(parent)
{
    imageOffset.setX(0);
    imageOffset.setY(0);
    for(int i=0; i<4; i++) {
        drawValues[i] =false;
        geoValues[i] = 0;
    }
}

void ImageWidget::paintEvent(QPaintEvent * event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    if (imageNew.isNull()) {
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, tr("Rendering initial image, please wait..."));
        return;
    }

    painter.drawImage(imageOffset,imageNew);

       painter.setPen(Qt::red);
       painter.drawRoundedRect(0,0,this->rect().width()-1, this->rect().height()-1, 10.0, 10.0);

    for(int i=0; i<2; i++) {
        if(i==0) painter.setPen( QPen( Qt::white )); else painter.setPen( QPen( Qt::black ));
        if(!drawValues[0]) return;
        if(drawValues[0]) {
            if(i==0)painter.drawLine(geoValues[0], 0, geoValues[0], this->height());
            else painter.drawLine(geoValues[0]+1, 0, geoValues[0]+1, this->height());
        }
        if(geoValues[1] <= 0) return;
        if(drawValues[1]) {
            if(i==0) painter.drawLine(0, geoValues[1], this->width(), geoValues[1]);
            else painter.drawLine(0, geoValues[1]+1, this->width(), geoValues[1]+1);
        }
        if((geoValues[0] -geoValues[2]/2) <= 1) return;
        if((geoValues[1] -geoValues[3]/2) <= 1) return;
        if(drawValues[0] && drawValues[1] && drawValues[2] && drawValues[3]) {
            if(i==0)painter.drawRect(geoValues[0] -geoValues[2]/2, geoValues[1]-geoValues[3]/2, geoValues[2], geoValues[3]);
            else painter.drawRect(geoValues[0] -geoValues[2]/2 - 1, geoValues[1]-geoValues[3]/2 - 1, geoValues[2] + 2, geoValues[3] + 2);
        }
    }
}

void ImageWidget::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e);
}

QImage ImageWidget::scaleImage(const QImage &image, const double &scaleFactor, const bool &FitToSize) {
    if(FitToSize) {
        return image.scaled(this->size(), Qt::KeepAspectRatio, Qt::FastTransformation);
     } else {
        return image.scaled(image.size() * scaleFactor, Qt::KeepAspectRatio, Qt::FastTransformation);
    }
}

void ImageWidget::updateImage(bool FitToSize, const QImage &image, bool valuesPresent[], int values[], const double &scaleFactor)
{
    // in case of fit to parent widget, we calculate concurrently if possible
    if((FitToSize) || (qAbs(scaleFactor-1) > 0.01)) {
#ifndef QT_NO_CONCURRENT
        QFuture<QImage> future = QtConcurrent::run(this, &ImageWidget::scaleImage, image, scaleFactor, FitToSize);
        imageNew = future.result();
#else
        imageNew = scaleImage(image, scalefactor, FitToSize);
#endif
    // no scaling, just take the pointer
    } else {
        imageNew = image;
    }
    for(int i=0; i<4; i++) {
        drawValues[i] = valuesPresent[i];
        if(drawValues[i]) geoValues[i] = values[i];
        else geoValues[i] = 0;
    }
    update();
}


