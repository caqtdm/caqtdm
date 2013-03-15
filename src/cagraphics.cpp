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

#include <QPaintEvent>
#include <QPainter>
#include <QDebug>
#include "alarmdefs.h"
/*
#include <QtDesigner/abstractformwindow.h>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerPropertySheetExtension>
#include <iostream>
*/
#include <math.h>
#include "cagraphics.h"

caGraphics::caGraphics( QWidget *parent) :  QWidget(parent)
{
    QSizePolicy policy( QSizePolicy::Preferred, QSizePolicy::Preferred );
    policy.setHeightForWidth( true );
    setSizePolicy( policy );
    thisChannelA="";
    thisChannelB="";
    thisChannelC="";
    thisChannelD="";
    thisForm = Rectangle;
    thisLineSize = 1;
    thisLineColor = Qt::black;
    thisForeColor = Qt::black;

    thisLineColorOld = Qt::white;
    thisForeColorOld = Qt::white;

    thisColorMode = Static;
    thisVisibility = StaticV;
    thisArrowSize=10;
    thisFillStyle = Outline;

    thisTiltAngle = 0;
    thisStartAngle = 0;
    thisSpanAngle = 90;

    setLineStyle(Solid);
    setHidden(false);

    setAttribute(Qt::WA_TranslucentBackground );
    setWindowFlags(Qt::FramelessWindowHint);
}

void caGraphics::setLineSize(int size ) {
    if (size > 0) {
        thisLineSize = size;
        update();
    }
}

void caGraphics::setLineColor( QColor c ) {
    thisLineColor = c;
    if(thisLineColorOld != c) {
        update();
    }
    thisLineColorOld = c;
}

void caGraphics::setForeground(QColor c)
{
    thisForeColor = c;
    if(thisForeColorOld != c) {
        update();
    }
    thisForeColorOld = c;
}

void caGraphics::setColorMode(colMode colormode)
{
    thisColorMode = colormode;
}

void caGraphics::setForm(Form form)
{
    thisForm = form;
/*
    QStringList List;
    List << "arrowSize";
    setPropertyEditorItems((QWidget*) this, List, false);
*/
    update();
}

void caGraphics::setFillStyle(FillStyle fillstyle)
{
    thisFillStyle = fillstyle;
    update();
}

void caGraphics::setLineStyle(LineStyle linestyle)
{
    thisLineStyle = linestyle;
    update();
}

void caGraphics::setTiltAngle( int angle)
{
    if (angle >= 0 && angle <= 360) {
        thisTiltAngle = angle;
        update();
    }
}


void caGraphics::setStartAngle( int angle)
{
    if (angle >= 0 && angle <= 360) {
        thisStartAngle = angle;
        update();
    }
}

void caGraphics::setSpanAngle( int angle)
{
    if (angle >= 0 && angle <= 360) {
        thisSpanAngle = angle;
        update();
    }
}

void caGraphics::setArrowSize( int size )
{
    if (size >= 0) {
        thisArrowSize = size;
        update();
    }
}

void caGraphics::setArrowMode( ArrowMode mode )
{
    thisArrowMode = mode;
    update();
}

QPolygonF caGraphics::getHead( QPointF p1, QPointF p2 ) {
    qreal Pi = 3.14;
    QPolygonF arrowHead;
    QLineF m_line = QLineF( p1, p2 );
    double angle = ::acos( m_line.dx() / m_line.length() );

    if ( m_line.dy() >= 0 )  angle = ( Pi * 2 ) - angle;

    QPointF arrowP1 = m_line.p1() + QPointF( sin( angle + Pi / 3 ) * getArrowSize(),
                      cos( angle + Pi / 3 ) * getArrowSize() );
    QPointF arrowP2 = m_line.p1() + QPointF( sin( angle + Pi - Pi / 3 ) * getArrowSize(),
                      cos( angle + Pi - Pi / 3 ) * getArrowSize() );

    arrowHead.clear();
    arrowHead << m_line.p1() << arrowP1 << arrowP2 << m_line.p1();
    return arrowHead;
}

void caGraphics::setAlarmColors(short status)
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

QPolygonF caGraphics::rotateObject(int degrees, int w, int h, int linesize, const QPolygonF& object) {
    // rotate
    double resizeWidth=999.0, resizeHeight=999.0;
    QTransform transform = QTransform().rotate(degrees);
    QPolygonF rotated = transform.map(object);
    //printf("w,h of object  %f %f\n",object.boundingRect().width(), object.boundingRect().height());
    //printf("w,h of rotated %f %f\n",rotated.boundingRect().width(), rotated.boundingRect().height());

    // calculate resize factor so that polygon will fit into the designer rectangle
    if(rotated.boundingRect().width() > 0) {
        resizeWidth = (double)(w) / rotated.boundingRect().width();
    }
    if(rotated.boundingRect().height() > 0) {
      resizeHeight = (double)(h) / rotated.boundingRect().height();
    }
    double resize = qMin(resizeWidth, resizeHeight);
    //printf("resize %f %f final=%f\n",resizeWidth, resizeHeight, resize);
    transform = QTransform().scale(resize, resize);
    rotated = transform.map(rotated);

    // get center of rotated polygon
    double centerX = rotated.boundingRect().x() + rotated.boundingRect().width()/2.0;
    double centerY = rotated.boundingRect().y() + rotated.boundingRect().height()/2.0;
    //printf("center of rotated polygon %f %f\n", centerX, centerY);

    // get center of canvas
    double centerXorg = w/2.0;
    double centerYorg = h/2.0;
    //printf("center of canvas %f %f\n", centerXorg, centerYorg);

    // calculate translation
    double translateX = centerX - centerXorg - linesize/2;
    double translateY = centerY - centerYorg - linesize/2;
    //printf("translate %f %f\n", translateX, translateY);

    transform = QTransform().translate(-translateX, -translateY);
    rotated = transform.map(rotated);
    return rotated;
}

QPolygonF caGraphics::drawCircle(int x1, int x2, int y1, int y2) {
    double x, y;
    QPolygonF points;

    double rx = (double) (x2 - x1) / 2.0;
    double ry = (double) (y2 - y1) / 2.0;
    double cx = (double) (x2 + x1) / 2.0;
    double cy = (double) (y2 + y1) / 2.0;

    for (double Angle = 0; Angle <=6.283185307; Angle=Angle+0.05) {
              x = cx + cos(Angle) * rx;
              y = cy + sin(Angle) * ry;
              points.append(QPointF(x,y));
    }
    return points;
}

void caGraphics::setHidden(bool hide)
{
    thisHide = hide;
    repaint();
}

void caGraphics::paintEvent( QPaintEvent *event )
{
    Q_UNUSED(event);

    if(thisHide) return;

    int m_margin = 3;
    QPointF p1,p2;
    QPainter painter( this );
    painter.setRenderHint( QPainter::Antialiasing );

    int margin = thisLineSize/2;
    int w = width() - 2 * margin;
    int h = height() - 2 * margin;

// do not increase linesize beyond the canvas size
    if(w <= 0 || h <= 0) {
        setLineSize(thisLineSize-1);
    }
    margin = thisLineSize/2;
    w = width() - 2 * margin;
    h = height() - 2 * margin;
    int x = margin;
    int y = margin;

    if(thisLineStyle == Dash) {
        painter.setPen( QPen( getLineColor(), getLineSize(), Qt::DotLine) );
    } else if (thisLineStyle == BigDash) {
       painter.setPen( QPen( getLineColor(), getLineSize(), Qt::DashLine ) );
    } else {
       painter.setPen( QPen( getLineColor(), getLineSize(), Qt::SolidLine ) );
    }

    if(thisFillStyle == Filled) {
        painter.setBrush(getForeground());
    } else {
        //painter.setBrush(QColor(0,0,0,0));
    }

    if(getForm() == Rectangle) {
        QPolygonF rectangle;
        rectangle.append(QPointF(x,y));
        rectangle.append(QPointF(x+w,y));
        rectangle.append(QPointF(x+w,y+h));
        rectangle.append(QPointF(x,y+h));
        rectangle.append(QPointF(x,y));
        // rotate
        QPolygonF rotated = rotateObject(thisTiltAngle, w, h, thisLineSize, rectangle);
        painter.drawPolygon(rotated);

    } else if(getForm() == Circle) {
        // rotate my calculated ellipse
        QPolygonF rotated = rotateObject(thisTiltAngle, w, h, thisLineSize, drawCircle(x, x+w, y, y+h));
        painter.drawPolygon(rotated);

    } else if(getForm() == Triangle) {
        QPolygonF triangle;
        triangle.append(QPointF(x+w/2,0));
        triangle.append(QPointF(x,y+h));
        triangle.append(QPointF(x+w,y+h));
        triangle.append(QPointF(x+w/2,0));

        // rotate
        QPolygonF rotated = rotateObject(thisTiltAngle, w, h, thisLineSize, triangle);
        painter.drawPolygon(rotated);

    } else if(getForm() == Arc) {
         if(thisFillStyle == Filled) {
             painter.drawPie (x, y, w, h, thisStartAngle * 16, thisSpanAngle * 16);
         } else {
             painter.drawArc (x, y, w, h, thisStartAngle * 16, thisSpanAngle * 16);
         }

    } else if(getForm() == Line) {
        QPolygonF line;
        line.append(QPointF(x,y+h/2));
        line.append(QPointF(x+w,y+h/2));

        // rotate
        QPolygonF rotated = rotateObject(thisTiltAngle, w, h, thisLineSize, line);
        painter.drawPolygon(rotated);

    } else if(getForm() == Arrow) {
         QPolygonF lines;
         QPolygonF head1;
         QPolygonF head2;

         p1 = QPointF( m_margin, height()/2 );
         p2 = QPointF( width()-m_margin, height()/2 );
         lines.append(p1);
         lines.append(p2);

         head1 = getHead(p1,p2);
         if (getArrowMode() == Double) {
            head2 = getHead(p2,p1);
         }

         for(int i=0; i<head1.count(); i++) lines.append(head1.at(i));
         for(int i=0; i<head2.count(); i++) lines.append(head2.at(i));

        // rotate
        QPolygonF rotated = rotateObject(thisTiltAngle, w, h, thisLineSize, lines);
        painter.drawPolygon(rotated);
    }

}

/*
//================================================== ===================
// METHOD : setPropertyEditorItems()
//
// DESCRIPTION:
// Used by plugins in order to hide/show specific property editor properties
//
// PRECONDITIONS:
// A valid list of properties
//
// POSTCONDITIONS:
// NONE
//
// EXCEPTIONS:
// NONE
//
// PARAMETERS:
// pTheWidget: A Pointer to the widget whos properties in designer are to be updated
// QStringListToChange: the QStringList list of the names of property editor properties to hide or show
// bShowProperties: A bool, when true, will show the properties in QStringListToHide, when false will hide them
//
// RETURN VALUE:
// True if able to complete operation, false otherwise
//================================================== ===================
bool caGraphics::setPropertyEditorItems(QWidget *pTheWidget, QStringList QStringListToChange, bool bShowProperties)
{
    bool bRetVal = false;
    int iLoopVal, iPropertyIndex;
    QDesignerFormWindowInterface *pFormWindow = 0;

    // attempting to get a pointer to the property editor control methods
    pFormWindow = QDesignerFormWindowInterface::findFormWindow(pTheWidget);
    if (pFormWindow && false == QStringListToChange.isEmpty())
    {
        bRetVal = true;

        QDesignerFormEditorInterface *pFormEditor = pFormWindow->core();
        QExtensionManager *pExtensionManager = pFormEditor->extensionManager();
        QDesignerPropertySheetExtension *pPropertySheet;
        pPropertySheet = qt_extension<QDesignerPropertySheetExtension*>(pExtensionManager, pTheWidget);

        // now traversing our list and setting the properties accordingly
        for (iLoopVal = 0; iLoopVal < QStringListToChange.size(); iLoopVal++)
        {
            iPropertyIndex = pPropertySheet->indexOf(QStringListToChange[iLoopVal]);
            std::cout << "name is " << pPropertySheet->propertyName(iPropertyIndex).toStdString() << std::endl;
            std::cout << "Found index of " << iPropertyIndex << " for string " << QStringListToChange[iLoopVal].toStdString() << std::endl;
            std::cout << "Setting value to " << bShowProperties << std::endl;

            // visibility does not work, however changing value works

            pPropertySheet->setVisible(iPropertyIndex, true);
            pPropertySheet->setProperty(iPropertyIndex, 100);
            pPropertySheet->setChanged(iPropertyIndex, true);

        }
        delete pPropertySheet;

    }
    return bRetVal;
}
*/

