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
        readValuesPresentL[i] =false;
        georeadValues[i] = 0;
        geowriteValues[i] = 0;
    }
    firstImage = true;
    scaleFactorL = 1.0;
    firstSelection = true;
    selectionInProgress = false;
}

void ImageWidget::getImageDimensions(int &width, int &height)
{
    double correction = scaleFactorL;
    width = imageNew.size().width() * correction;
    height = imageNew.size().height() * correction;
}

void ImageWidget::updateDisconnected()
{
    disconnected = true;
    update();
}

QPolygonF ImageWidget::getHead( QPointF p1, QPointF p2, int arrowSize ) {
    qreal Pi = 3.14;
    QPolygonF arrowHead;
    QLineF m_line = QLineF( p1, p2 );
    double angle = ::acos( m_line.dx() / m_line.length() );

    if ( m_line.dy() >= 0 )  angle = ( Pi * 2 ) - angle;

    QPointF arrowP1 = m_line.p1() + QPointF( sin( angle + Pi / 3 ) * arrowSize,
                                             cos( angle + Pi / 3 ) * arrowSize );
    QPointF arrowP2 = m_line.p1() + QPointF( sin( angle + Pi - Pi / 3 ) * arrowSize,
                                             cos( angle + Pi - Pi / 3 ) * arrowSize );

    arrowHead.clear();
    arrowHead << m_line.p1() << arrowP1 << arrowP2 << m_line.p1();
    return arrowHead;
}

void ImageWidget::paintEvent(QPaintEvent * event)
{
    int xnew, ynew, width , height;
    QPolygonF lines, head1, head2;
    QPointF p1, p2;
    QRect selectionRect;
    bool present[4];

    Q_UNUSED(event);
    QPainter painter(this);
    painter.save();

    if (imageNew.isNull()) {
        painter.setPen(Qt::white);
        painter.setBrush(QBrush(QColor(100,100,100,255)));
        painter.drawRect(rect());
        painter.drawText(rect(), Qt::AlignCenter, tr("Rendering initial image, please wait..."));
        painter.restore();
        return;
    }
    if(disconnected) {
        painter.setPen(Qt::white);
        painter.setBrush(QBrush(QColor(255,255,255,255)));
        painter.drawRect(rect());
        painter.restore();
        return;
    }

    // rescale
    painter.scale(scaleFactorL, scaleFactorL);

    // exposed rectangle
    QRect exposedRect = painter.matrix().inverted().mapRect(event->rect()).adjusted(-1, -1, 1, 1);

    // and draw

    painter.drawImage(exposedRect, imageNew, exposedRect);

    if(selectSimpleViewL) {
        painter.restore();
        return;
    }

    // draw a rounded rectangle around the image
    width = imageNew.size().width();
    height = imageNew.size().height();
    painter.setPen(Qt::blue);
    painter.drawRoundedRect(0, 0, width, height, 2.0, 2.0);
    //painter.drawRect(0, 0, width, height);

    painter.restore();

    // grey selection area during mouse selection
    if(selectionInProgress && ((writemarkerTypeL == box) || writemarkerTypeL == box_crosshairs)) {
        painter.setPen(QPen(QBrush(QColor(0,0,0,180)),1,Qt::DashLine));
        painter.setBrush(QBrush(QColor(255,255,255,120)));
    } else {
        painter.setPen(QPen(QBrush(QColor(0,0,0,0)),1,Qt::DashLine));
        painter.setBrush(QBrush(QColor(255,255,255,0)));
    }
    selectionRect.setCoords(selectionPointsL[0].x(), selectionPointsL[0].y(), selectionPointsL[1].x(), selectionPointsL[1].y());
    painter.drawRect(selectionRect);

    painter.setBrush(Qt::NoBrush);
    painter.setCompositionMode(QPainter::RasterOp_SourceAndNotDestination);

    // loop on readback values and selection rectangle
    for(int j=0; j<2; j++) {
        ROI_type TypeL;
        ROI_markertype markerTypeL;
        int values[4];

        lines.clear();
        head1.clear();
        head2.clear();

        // readback values display
        if(j==0) {
            if(!readValuesPresentL[0]) continue;
            if(georeadValues[1] <= 0) continue;
            if(georeadValues[0] <= 0) continue;
            TypeL = readTypeL;
            markerTypeL = readmarkerTypeL;
            for(int i=0; i<4; i++) {
                present[i] = readValuesPresentL[i];
                values[i] = georeadValues[i];
            }
            painter.setPen(QPen(Qt::red,2, Qt::SolidLine));

            // selection rectangle display
        } else {
            TypeL = writeTypeL;
            markerTypeL = writemarkerTypeL;
            // selection is present
            for(int i=0; i<4; i++) present[i] = true;
            painter.setPen(QPen(Qt::green,2, Qt::SolidLine));

            // prepare from the selection rectangle the requested values
            switch (TypeL) {
            case none:
                return;
            case xy_only:
                values[0] = selectionRect.center().x();
                values[1] = selectionRect.center().y();
                break;
            case xy1_xy2:
                values[0] = selectionPointsL[0].x();
                values[1] = selectionPointsL[0].y();
                values[2] = selectionPointsL[1].x();
                values[3] = selectionPointsL[1].y();
                break;
            case xyUpleft_xyLowright:
                values[0] = selectionRect.topLeft().x();
                values[1] = selectionRect.topLeft().y();
                values[2] = selectionRect.bottomRight().x();
                values[3] = selectionRect.bottomRight().y();
                break;
            case xycenter_width_height:
                values[0] = selectionRect.center().x();
                values[1] = selectionRect.center().y();
                values[2] = selectionRect.width();
                values[3] = selectionRect.height();
                break;
            default:
                return;
            }
            for(int i=0; i<4; i++) {
                values[i] = qRound(values[i] * scaleFactorL / selectionFirstFactor);
            }
        }

        switch (TypeL) {

        //-----------------------------------------------------------------------------------------------------------------
        case none:
            break;

            //-----------------------------------------------------------------------------------------------------------------
        case xy_only:
            if(!present[0] || !present[1]) break;
            // vertical and horizontal
            painter.drawLine(values[0], 0, values[0], this->height()*scaleFactorL);
            painter.drawLine(0, values[1], this->width()*scaleFactorL, values[1]);

            switch (markerTypeL) {
            case box:
            case box_crosshairs:
            case line:
            case arrow:
                break;
            default:
                break;
            }

            break;

            //-----------------------------------------------------------------------------------------------------------------
        case xy1_xy2:

            // vertical and horizontal
            xnew = (values[0] + values[2]) / 2;
            ynew = (values[1] + values[3]) / 2;
            if(!present[0] || !present[1] || !present[2] || !present[3]) break;

            switch (markerTypeL) {
            case box_crosshairs:
                // vertical and horizontal
                painter.drawLine(xnew, 0, xnew, this->height()*scaleFactorL);
                painter.drawLine(0, ynew, this->width()*scaleFactorL, ynew);
            case box:
                selectionRect.setCoords(values[0], values[1], values[2], values[3]);
                painter.drawRect(selectionRect);
                break;

            case line:
                p1 = QPointF( values[0], values[1]);
                p2 = QPointF( values[2], values[3]);
                lines.append(p1);
                lines.append(p2);
                painter.drawPolygon(lines);
                break;

            case arrow:
                p1 = QPointF( values[0], values[1]);
                p2 = QPointF( values[2], values[3]);
                lines.append(p1);
                lines.append(p2);

                head1 = getHead(p1,p2, qRound(sqrt(0.5 * qMin(width,  height))));
                head2 = getHead(p2,p1, qRound(sqrt(0.5 * qMin(width,  height))));
                for(int j=0; j<head1.count(); j++) lines.append(head1.at(j));
                for(int j=0; j<head2.count(); j++) lines.append(head2.at(j));
                painter.drawPolygon(lines);
                break;

            default:
                // do nothing
                break;
            }
            break;

            //-----------------------------------------------------------------------------------------------------------------
        case xyUpleft_xyLowright:

            // vertical and horizontal
            xnew = (values[0] + values[2]) / 2;
            ynew = (values[1] + values[3]) / 2;
            width = qAbs(values[0] - values[2]);
            height =  qAbs(values[1] - values[3]);

            if(!present[0] || !present[1] || !present[2] || !present[3]) break;

            switch (markerTypeL) {
            case box_crosshairs:
                // vertical and horizontal
                painter.drawLine(xnew, 0, xnew, this->height()*scaleFactorL);
                painter.drawLine(0, ynew, this->width()*scaleFactorL, ynew);
            case box:
                if(width <= 1) break;
                if((height) <= 1) break;
                painter.drawRect(values[0],  values[1], width, height);
                break;

            case line:
                p1 = QPointF( values[0], values[1]);
                p2 = QPointF( values[2], values[3]);
                lines.append(p1);
                lines.append(p2);
                painter.drawPolygon(lines);
                break;
            case arrow:
                p1 = QPointF( values[0], values[1]);
                p2 = QPointF( values[2], values[3]);
                lines.append(p1);
                lines.append(p2);

                head1 = getHead(p1,p2, qRound(sqrt(0.5 * qMin(width,  height))));
                head2 = getHead(p2,p1, qRound(sqrt(0.5 * qMin(width,  height))));
                for(int j=0; j<head1.count(); j++) lines.append(head1.at(j));
                for(int j=0; j<head2.count(); j++) lines.append(head2.at(j));
                painter.drawPolygon(lines);
                break;
            default:
                // do nothing
                break;
            }
            break;

            //-----------------------------------------------------------------------------------------------------------------
        case xycenter_width_height:

            switch (markerTypeL) {
            case box_crosshairs:
                if(!present[0] || !present[1]) break;
                // vertical and horizontal
                painter.drawLine(values[0], 0, values[0], this->height()*scaleFactorL);
                painter.drawLine(0, values[1], this->width()*scaleFactorL, values[1]);
            case box:
                if(!present[0] || !present[1] || !present[2] || !present[3]) break;
                if((values[0] - values[2]/2) <= 1) break;
                if((values[1] - values[3]/2) <= 1) break;
                painter.drawRect(values[0] - values[2]/2 ,  values[1]-values[3]/2,
                        values[2], values[3]);
                break;

            case line:
                if(!present[0] || !present[1] || !present[2] || !present[3]) break;
                if((values[0] - values[2]/2) <= 1) break;
                if((values[1] - values[3]/2) <= 1) break;
                p1 = QPointF( values[0] - values[2]/2 ,  values[1]-values[3]/2);
                p2 = QPointF( values[0] + values[2]/2 ,  values[1]+values[3]/2);
                lines.append(p1);
                lines.append(p2);
                painter.drawPolygon(lines);
                break;

            case arrow:
                if(!present[0] || !present[1] || !present[2] || !present[3]) break;
                if((values[0] - values[2]/2) <= 1) break;
                if((values[1] - values[3]/2) <= 1) break;
                p1 = QPointF( values[0] - values[2]/2 ,  values[1]-values[3]/2);
                p2 = QPointF( values[0] + values[2]/2 ,  values[1]+values[3]/2);
                lines.append(p1);
                lines.append(p2);

                head1 = getHead(p1,p2, qRound(sqrt(0.5 * qMin(width,  height))));
                head2 = getHead(p2,p1, qRound(sqrt(0.5 * qMin(width,  height))));
                for(int j=0; j<head1.count(); j++) lines.append(head1.at(j));
                for(int j=0; j<head2.count(); j++) lines.append(head2.at(j));
                painter.drawPolygon(lines);
                break;

            default:
                // do nothing
                break;
            }
            break;
        }
    }

}

void ImageWidget::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e);
}

void ImageWidget::rescaleReadValues(const bool &fitToSize, const QImage &image, const double &scaleFactor,
                                    bool readvaluesPresent[], double readvalues[] )
{
    double factorX = (double) this->size().width() / (double) image.size().width();
    double factorY = (double) this->size().height() /(double) image.size().height();
    double factor = qMin(factorX, factorY);
    for(int i=0; i<4; i++) {
        readValuesPresentL[i] = readvaluesPresent[i];
        if(!fitToSize) {
            georeadValues[i] = readvalues[i] * scaleFactor;
        } else {
            georeadValues[i] = readvalues[i] * factor;
        }
        readValuesL[i] = georeadValues[i];
    }
    update();
}

void ImageWidget::initSelectionBox(const double &scaleFactor)
{
    selectionInProgress = true;
    selectionFirstFactor = scaleFactor;
}

void ImageWidget::rescaleSelectionBox(const double &scaleFactor)
{
    scaleFactorL = scaleFactor;
    update();
}

void ImageWidget::updateSelectionBox(QPoint selectionPoints[], const bool &selectInProgress)
{
    selectionPointsL[0] = selectionPoints[0];
    selectionPointsL[1] = selectionPoints[1];
    selectionInProgress = selectInProgress;
    update();
}

void  ImageWidget::updateImage(bool FitToSize, const QImage &image, bool readvaluesPresent[], double readvalues[],
                               double scaleFactor, bool selectSimpleView,
                               short readmarkerType, short readType, short writemarkerType, short writeType)
{
    disconnected = false;
    selectSimpleViewL = selectSimpleView;
    readmarkerTypeL = (ROI_markertype) readmarkerType;
    writemarkerTypeL = (ROI_markertype) writemarkerType;
    readTypeL = (ROI_type) readType;
    writeTypeL = (ROI_type) writeType;
    if(FitToSize) {
        double factorX = (double) this->size().width() / (double) image.size().width();
        double factorY = (double) this->size().height() /(double) image.size().height();
        scaleFactorL = qMin(factorX, factorY);
    } else {
        scaleFactorL = scaleFactor;
    }

    imageNew = image;

    if(firstImage) {
        imageFirstFactor = scaleFactor;
        firstImage = false;
    }

    if(selectSimpleViewL) {
        update();
        return;
    }

    rescaleReadValues(FitToSize, image, scaleFactor, readvaluesPresent, readvalues);
    update();
}


