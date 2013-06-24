//******************************************************************************
// Copyright (c) 2012 Paul Scherrer Institut PSI), Villigen, Switzerland
// Disclaimer: neither  PSI, nor any of their employees makes any warranty
// or assumes any legal liability or responsibility for the use of this software
//******************************************************************************
//******************************************************************************
//
//     Author : Anton Chr. Mezger
//
//******************************************************************************

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
    thisForeColor = Qt::black;
    thisColorMode = Static;
    thisVisibility = StaticV;

    thisFillStyle = Outline;
    thisPolyStyle = Polyline;
    initialize = true;
    inEditor = false;
    mouseMove = false;

    setHidden(false);

    setAttribute(Qt::WA_TranslucentBackground, true );
    setWindowFlags(Qt::FramelessWindowHint);
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
    update();
}

void caPolyLine::setForeground(QColor c)
{
    thisForeColor = c;
    update();
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
    case ALARM_INVALID:
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
    thisXYpairs = newPairs;
    update();
}

QString caPolyLine::pairs() const
{
    return thisXYpairs;
}

void caPolyLine::clearPairs()
{
    thisXYpairs = "";
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
    if(!inEditor) return;
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
}

void caPolyLine::setHidden(bool hide)
{
    thisHide = hide;
    repaint();
}

void caPolyLine::paintEvent(QPaintEvent * /* event */)
{

    if(thisHide) return;

    QPainter painter(this);

    painter.setRenderHint( QPainter::Antialiasing );
    int nbPoints = 0;

    if(inEditor) {
      painter.setBrush(QColor(Qt::white));
      painter.drawRect(editSize);
    }

    if(thisLineStyle == Dash) {
        painter.setPen( QPen( getLineColor(), getLineSize(), Qt::DotLine ));
    } else if (thisLineStyle == BigDash) {
        painter.setPen( QPen( getLineColor(), getLineSize(), Qt::DashLine ));
    } else {
        painter.setPen( QPen( getLineColor(), getLineSize(), Qt::SolidLine ));
    }

    QStringList pairs = thisXYpairs.split(";", QString::SkipEmptyParts);
    QPolygon polygon(pairs.count());

    for(int i=0; i< pairs.count(); i++) {
        QStringList xy = pairs.at(i).split(",", QString::SkipEmptyParts);
        if(xy.count() == 2) {
            polygon.putPoints(i, 1, atoi(xy.at(0).toAscii().constData()), atoi(xy.at(1).toAscii().constData()));
            lastPosition =  QPointF(atoi(xy.at(0).toAscii().constData()), atoi(xy.at(1).toAscii().constData()));
            nbPoints++;
        }
    }

    // when polygon, close the line
    if((thisPolyStyle == Polygon) && (nbPoints > 2)) {
        QStringList xy = pairs.at(0).split(",", QString::SkipEmptyParts);
        if(xy.count() == 2) {
            polygon.putPoints(nbPoints, 1, atoi(xy.at(0).toAscii().constData()), atoi(xy.at(1).toAscii().constData()));
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
      painter.setPen( QPen(QColor(Qt::red), getLineSize(), Qt::SolidLine, Qt::FlatCap, Qt::RoundJoin ) );
      if(actualPosition != lastPosition)
         painter.drawLine(actualPosition, lastPosition);
    }
}

void caPolyLine::resizeEvent(QResizeEvent *e)
 {
    if(initialize) {
        actualWidth= e->size().width();
        actualHeight = e->size().height();
        initialize = false;
    }
    double resizeX = (double) e->size().width() / (double) actualWidth;
    double resizeY = (double) e->size().height() / (double) actualHeight;

    QStringList pairs = thisXYpairs.split(";", QString::SkipEmptyParts);
    thisXYpairs.clear();

    for(int i=0; i< pairs.count(); i++) {
        QStringList xy = pairs.at(i).split(",", QString::SkipEmptyParts);
        if(xy.count() == 2) {
             double x = atof(xy.at(0).toAscii().constData()) * resizeX;
             double y = atof(xy.at(1).toAscii().constData()) * resizeY;
             if(i!=0) thisXYpairs.append(";");
             thisXYpairs.append(QString::number((int)(x+0.5)));
             thisXYpairs.append(",");
             thisXYpairs.append(QString::number((int)(y+0.5)));
        }
    }

    actualWidth= e->size().width();
    actualHeight = e->size().height();
}
