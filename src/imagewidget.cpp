#include <QtGui>
#ifndef QT_NO_CONCURRENT
#include <qtconcurrentrun.h>
#endif
#include <math.h>
#include "imagewidget.h"

ImageWidget::ImageWidget(QWidget *parent) : QWidget(parent)
{
    pixmapOffset.setX(0);
    pixmapOffset.setY(0);
    for(int i=0; i<4; i++) {
        drawValues[i] =false;
        geoValues[i] = 0;
    }
}

void ImageWidget::paintEvent(QPaintEvent * event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    if (pixmap.isNull()) {
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, tr("Rendering initial image, please wait..."));
        return;
    }
    painter.drawPixmap(pixmapOffset, pixmap);

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

QImage ImageWidget::scaleImage(const QImage &image) {
    return image.scaled(this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
}


void ImageWidget::updateImage(bool zoom, const QImage &image, bool valuesPresent[], int values[])
{
    if(zoom) {
        QImage imageNew = scaleImage(image);
        if(imageNew.isNull()) return;

#ifndef QT_NO_CONCURRENT
        QFuture<QImage> future = QtConcurrent::run(this, &ImageWidget::scaleImage, image);
        imageNew = future.result();
#else
        QImage imageNew = scaleImage(image);

#endif

        pixmap = QPixmap::fromImage(imageNew);
    } else {
        pixmap = QPixmap::fromImage(image);
    }
    for(int i=0; i<4; i++) {
        drawValues[i] = valuesPresent[i];
        if(drawValues[i]) geoValues[i] = values[i];
        else geoValues[i] = 0;
    }
    update();
}


