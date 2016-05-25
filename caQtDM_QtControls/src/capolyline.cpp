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

#include "capolyline.h"
#include "alarmdefs.h"

caPolyLine::caPolyLine(QWidget *parent): QWidget(parent)
{
    QSizePolicy policy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    policy.setHeightForWidth( true );
    setSizePolicy( policy );
    thisLineStyle = Solid;
    thisLineSize = 1;
    thisLineColor = Qt::black;
    oldLineColor = Qt::white;
    thisForeColor = Qt::black;
    oldForeColor = Qt::white;
    thisColorMode = Static;
    thisVisibility = StaticV;

    thisFillStyle = Outline;
    thisPolyStyle = Polyline;
    initialize = true;
    inEditor = false;
    mouseMove = false;

    setHide(false);

    setAttribute(Qt::WA_TranslucentBackground, true );
    setWindowFlags(Qt::FramelessWindowHint);

    // called form designer ?
    QVariant source = qApp->property("APP_SOURCE").value<QVariant>();
    if(source.isValid()) inDesigner = true; else inDesigner = false;
}

void caPolyLine::setLineSize(int size )
{
    if (size > 0) {
        thisLineSize = size;
        update();
    }
}

void caPolyLine::setInEditor(bool in)
{
    inEditor = in;
}

void caPolyLine::setLineColor( QColor c )
{
    thisLineColor = c;
    if(oldLineColor != thisLineColor) update();
    oldLineColor = c;
}

void caPolyLine::setForeground(QColor c)
{
    thisForeColor = c;
    if(oldForeColor != thisForeColor) update();
    oldForeColor = thisForeColor;
}

void caPolyLine::setColorMode(colMode colormode)
{
    thisColorMode = colormode;
}

void caPolyLine::setAlarmColors(short status)
{
    switch (status) {

    case NO_ALARM:
        setForeground(AL_GREEN);
        setLineColor(AL_GREEN);
        break;
    case MINOR_ALARM:
        setForeground(AL_YELLOW);
        setLineColor(AL_YELLOW);
        break;
    case MAJOR_ALARM:
        setForeground(AL_RED);
        setLineColor(AL_RED);
        break;
    case INVALID_ALARM:
    case NOTCONNECTED:
        setForeground(AL_WHITE);
        setLineColor(AL_WHITE);
        break;
    default:
        setForeground(AL_DEFAULT);
        setLineColor(AL_DEFAULT);
        break;
    }
}

void caPolyLine::setLineStyle(LineStyle style)
{
    thisLineStyle = style;
    update();
}

void caPolyLine::setFillStyle(FillStyle style)
{
    thisFillStyle = style;
    update();
}

void caPolyLine::setPolyStyle(PolyStyle style)
{
    thisPolyStyle = style;
    update();
}

void caPolyLine::setPairs(const QString &newPairs)
{
    XYpairs = thisXYpairs = newPairs;
    update();
}

QString caPolyLine::pairs() const
{
    return thisXYpairs;
}

void caPolyLine::clearPairs()
{
    XYpairs = thisXYpairs = "";
    update();
}

void caPolyLine::setEditSize(int x, int y, int w, int h) {
    editSize = QRect(x,y,w,h);
}

void caPolyLine::mouseMoveEvent(QMouseEvent *event)
{
    if(!inEditor) return;
    actualPosition = QPointF(event->pos().x(),event->pos().y());
    update();
}

void caPolyLine::mousePressEvent(QMouseEvent *event)
{
    if(!inEditor) {
        if(event->type() == QEvent::MouseButtonPress) {
                QMouseEvent *ev = event;
#if QT_VERSION< QT_VERSION_CHECK(4, 8, 0)
                 if(ev->button() == Qt::MidButton) {
#else
                if(ev->button() == Qt::MiddleButton) {
#endif
                     QWidget::mousePressEvent(event);
                }
        }
        return;
    }
    // start move
    actualPosition = QPointF(event->pos().x(),event->pos().y());
    if (event->button() == Qt::LeftButton) {
        mouseMove = true;
        // add first point if empty
        if(thisXYpairs.size() == 0) {
            thisXYpairs.append(QString::number(event->pos().x()));
            thisXYpairs.append(",");
            thisXYpairs.append(QString::number(event->pos().y()));
        }

    // remove last point
    } else
    if(event->button() == Qt::RightButton) {
        int pos = thisXYpairs.lastIndexOf(";");
        int len = thisXYpairs.count();
        if(pos > 0) thisXYpairs.chop(len-pos);
        else thisXYpairs.clear();  // get rid of last point
    }
    XYpairs = thisXYpairs;
    update();
}

void caPolyLine::mouseReleaseEvent(QMouseEvent *event)
{
   if(!inEditor) return;
    mouseMove = false;
    // add point
    if (event->button() == Qt::LeftButton) {
        thisXYpairs.append(";");
        thisXYpairs.append(QString::number(event->pos().x()));
        thisXYpairs.append(",");
        thisXYpairs.append(QString::number(event->pos().y()));
    }
    XYpairs = thisXYpairs;
}

void caPolyLine::setHide(bool hide)
{
    thisHide = hide;
    repaint();
}

void caPolyLine::paintEvent(QPaintEvent * /* event */)
{
    QStringList pairs;
    if(thisHide) return;

    QPainter painter(this);

    painter.setRenderHint( QPainter::Antialiasing );
    int nbPoints = 0;

    if(inEditor) {
      painter.setBrush(QColor(Qt::white));
      painter.drawRect(editSize);
    }

    if(thisLineStyle == Dash) {
        painter.setPen( QPen( getLineColor(), getLineSize(), Qt::DotLine,  Qt::FlatCap));
    } else if (thisLineStyle == BigDash) {
        painter.setPen( QPen( getLineColor(), getLineSize(), Qt::DashLine, Qt::FlatCap ));
    } else {
        painter.setPen( QPen( getLineColor(), getLineSize(), Qt::SolidLine, Qt::FlatCap));
    }

    if(inDesigner) {
        pairs = thisXYpairs.split(";", QString::SkipEmptyParts);
    } else {
        pairs = XYpairs.split(";", QString::SkipEmptyParts);
    }
    QPolygon polygon(pairs.count());

    for(int i=0; i< pairs.count(); i++) {
        QStringList xy = pairs.at(i).split(",", QString::SkipEmptyParts);
        if(xy.count() == 2) {
            polygon.putPoints(i, 1, atoi(qasc(xy.at(0))), atoi(qasc(xy.at(1))));
            lastPosition =  QPointF(atoi(qasc(xy.at(0))), atoi(qasc(xy.at(1))));
            nbPoints++;
        }
    }

    // when polygon, close the line
    if((thisPolyStyle == Polygon) && (nbPoints > 2)) {
        QStringList xy = pairs.at(0).split(",", QString::SkipEmptyParts);
        if(xy.count() == 2) {
            polygon.putPoints(nbPoints, 1, atoi(qasc(xy.at(0))), atoi(qasc(xy.at(1))));
            nbPoints++;
        }
    }

    if(thisFillStyle == Filled) {
        painter.setBrush(getForeground());
    }

    if(nbPoints > 0) {
        if(thisPolyStyle == Polygon) {
            painter.drawPolygon(polygon);
        } else {
            painter.drawPolyline(polygon);
        }
    }

    if(inEditor && mouseMove) {
      painter.setPen( QPen(QColor(Qt::red), getLineSize(), Qt::SolidLine, Qt::FlatCap ) );
      if(actualPosition != lastPosition)
         painter.drawLine(actualPosition, lastPosition);
    }
}

void caPolyLine::setActualSize(QSize size)
{
    actualWidth = size.width();
    actualHeight = size.height();
    initialize = false;
    //printf("set actual size %d %d\n", actualWidth, actualHeight);
}

void caPolyLine::resizeEvent(QResizeEvent *e)
 {
    if(initialize && inDesigner) {
        actualWidth= width();
        actualHeight = height();
        //printf("first resize of polyline %d %d\n", width(), height());
        initialize = false;
    } else if(initialize) {
        actualWidth= width();
        actualHeight = height();
        //printf("first resize of polyline %d %d\n", width(), height());
        initialize = false;
    }

    //printf("resize of polyline %d %d\n", e->size().width(), e->size().height());

    double resizeX = (double) e->size().width() / (double) actualWidth;
    double resizeY = (double) e->size().height() / (double) actualHeight;

    QStringList pairs = thisXYpairs.split(";", QString::SkipEmptyParts);

    if(inDesigner) {

        thisXYpairs.clear();
        for(int i=0; i< pairs.count(); i++) {
            QStringList xy = pairs.at(i).split(",", QString::SkipEmptyParts);
            if(xy.count() == 2) {
                double x = atof(qasc(xy.at(0))) * resizeX;
                double y = atof(qasc(xy.at(1))) * resizeY;
                if(i!=0) thisXYpairs.append(";");
                thisXYpairs.append(QString::number(qRound(x)));
                thisXYpairs.append(",");
                thisXYpairs.append(QString::number(qRound(y)));
            }
        }

        actualWidth= e->size().width();
        actualHeight = e->size().height();

    } else {
        // resize coordinates when not in designer, we do not resize the linewidth actually
        // while we are not keeping the aspect ratio
        XYpairs.clear();
        for(int i=0; i< pairs.count(); i++) {
            QStringList xy = pairs.at(i).split(",", QString::SkipEmptyParts);
            if(xy.count() == 2) {
                 double x = atof(qasc(xy.at(0))) * resizeX;
                 double y = atof(qasc(xy.at(1))) * resizeY;
                 if(i!=0) XYpairs.append(";");
                 XYpairs.append(QString::number(qRound(x)));
                 XYpairs.append(",");
                 XYpairs.append(QString::number(qRound(y)));
            }
        }
    }

}

