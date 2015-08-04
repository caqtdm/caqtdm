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

#include "cagauge.h"
#include <QPainter>
#include <QPaintEvent>
#include <QApplication>
#include <eng_notation.h>

#define MIN_FONT_SIZE 3

caAbstractGauge::caAbstractGauge(QWidget *parent) : EAbstractGauge(parent)
{

}

QString caAbstractGauge::getPV() const
{
    return thisPV;
}

void caAbstractGauge::setPV(QString const &newPV)
{
    thisPV = newPV;
}

caLinearGauge::caLinearGauge(QWidget *parent, Qt::Orientation o) : caAbstractGauge(parent), m_orientation(o), m_fillMode(ALL)
{
    QFont font("Arial");
    setFont(font);
    setNumMajorTicks(LINEAR_MAJOR_TICKS);
    setNumMinorTicks(NUM_MINOR_TICKS);
    QFont f = this->font();
    f.setPointSize(10);
    this->setFont(f);
}

void caLinearGauge::configure()
{
    int shortSide, longSide;
    if (m_scaleEnabled)
    {
        longSide = 80;
        shortSide = 30;
    }
    else
    {
        longSide = 60;
        shortSide = 20;
    }


    if (m_orientation == Qt::Horizontal)
    {
        if (m_scaleEnabled)
            scalePos = 20;
        else
            scalePos = 0;
        labelsPos = 12;
        totalSize = 30;
        barSize = totalSize-scalePos;
        scaleSize = scalePos-labelsPos;
        labelsSize = labelsPos;
        setMinimumSize(longSide,shortSide);
    }
    else
    {
        if (m_scaleEnabled)
            scalePos = 10;
        else
            scalePos = 30;
        labelsPos = 18;
        totalSize = 30;
        barSize = scalePos;
        scaleSize = labelsPos-scalePos;
        labelsSize = totalSize-labelsPos;
        setMinimumSize(shortSide,longSide);
    }
    caAbstractGauge::configure();
}

void caLinearGauge::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    int size, w, h;
    QFontMetrics fm(painter.font());

    h = fm.height()+2;
    w = fm.width(labels[longestLabelIndex])+2;

    if (m_orientation == Qt::Horizontal) {
        size = qMin((int)(width()*totalSize/100.0), height());
    } else {
        size = qMin((int)(height()*totalSize/100.0), width());
    }

    painter.setRenderHint(QPainter::Antialiasing);

    if (m_orientation == Qt::Horizontal) {
        painter.setViewport((int)((width()-size*100.0/totalSize)*.5),(int)((height()-size)*.5), (int)(size*100.0/totalSize), size);
        painter.setWindow((int)(-w*.5), 0, 100+w, totalSize+2); /* border */
    } else {
        painter.setViewport((int)((width()-size)*.5),(int)((height()-size*100.0/totalSize)*.5), size, (int)(size*100.0/totalSize));
        painter.setWindow(-2, (int)(-h*.5), totalSize+2, 100+h); /* border */
    }
    painter.setViewport(0,0, width(), height()); // A.Mezger I do not like above behaviour when resizing

    drawColorBar(&painter);
    if (m_scaleEnabled)
    {
        drawScale(&painter);
        drawLabels(&painter);
    }
    if (isEnabled() && (m_fillMode == ALL))
        drawMarker(&painter, true);

    if (!isEnabled())
    {
        QColor c = palette().color(QPalette::Background);
        c.setAlpha(200);
        painter.fillRect(painter.window(), c);
    }
}

void caLinearGauge::drawColorBar(QPainter *p)
{
    if (m_colorMode == GRADIENT)
    {
        QPoint start, stop;
        QRect r;
        if (m_orientation == Qt::Horizontal)
        {
            start = QPoint(0,0);
            stop = QPoint(100,0);
            r = QRect(0, scalePos, 100, barSize);
        }
        else
        {
            start = QPoint(0,100);
            stop = QPoint(0,0);
            r = QRect(0, 0, barSize, 100);
        }
        QLinearGradient	grad(start, stop);
        for (int i = 0; i < v_p.size(); i++) {
            grad.setColorAt(v_p[i], v_c[i]);
    }
        p->setBrush(grad);
        p->drawRect(r);
    }
    else if (m_colorMode == COLORBAR)
    {
        p->setPen(Qt::NoPen);
        if (m_orientation == Qt::Horizontal)
        {
            QVector<qreal> widths;
            foreach(qreal w, v_p)
                widths << w * 100;
            for (int i = 1; i < v_p.size(); i++)
            {
                p->setBrush(v_c[i]);
                p->drawRect(QRectF(widths[i-1], scalePos, widths[i]-widths[i-1], barSize));
            }
            p->setBrush(Qt::NoBrush);
            p->setPen(Qt::black);
            p->drawRect(0, scalePos, 100, barSize);
        }
        else
        {
            QVector<qreal> heights;
            foreach(qreal h, v_p)
                heights << 100*(1-h);
            for (int i = 1; i < v_p.size(); i++)
            {
                p->setBrush(v_c[i]);
                p->drawRect(QRectF(0, heights[i], barSize, heights[i-1]-heights[i]));
            }
            p->setBrush(Qt::NoBrush);
            p->setPen(Qt::black);
            p->drawRect(0, 0, barSize, 100);
        }
    }
    else if (m_colorMode == SINGLECOLOR)
    {
        p->setBrush(v_c[0]);
        if (m_orientation == Qt::Horizontal)
            p->drawRect(0, scalePos, 100, barSize);
        else
            p->drawRect(0, 0, barSize, 100);
    }

    if (m_fillMode != ALL)
    {
        double v1, v2, min = 0.0, max = 0.0;
        v1 = (m_value-m_minValue)/(m_maxValue-m_minValue);

        p->setPen(Qt::black);
        p->setBrush(palette().background());
        if (m_fillMode == FROM_MIN)
        {
            max = v1;
            min = 0.0;
        }
        else if (m_fillMode == FROM_ZERO)
        {
            v2  = (-m_minValue)/(m_maxValue-m_minValue);
            min = qMin(v1, v2);
            max = qMax(v1, v2);
        }
        else if (m_fillMode == FROM_REF)
        {
            v2  = (m_reference-m_minValue)/(m_maxValue-m_minValue);
            min = qMin(v1, v2);
            max = qMax(v1, v2);
        }

        if (m_orientation == Qt::Horizontal)
        {
            p->drawRect(QRectF(0, scalePos, 100.0*min, barSize));
            p->drawRect(QRectF(100.0*max, scalePos, 100.0*(1-max), barSize));
        }
        else
        {
            p->drawRect(QRectF(0.0, 100*(1-min), barSize, 100*min));
            p->drawRect(QRectF(0.0, 0.0, barSize, 100.0*(1-max)));
        }
    }
}

void caLinearGauge::drawScale(QPainter *p)
{
    QVector<QLineF> lines;

    if (m_orientation == Qt::Horizontal)
    {
        qreal x1=0, y1=scalePos, wM=scaleSize, wm=wM*.5;
        QLineF majorTickLine(x1,y1,x1,y1-wM);
        qreal interval = 100.0/(m_numMajorTicks-1)/(m_numMinorTicks); /* distance between 2 minor ticks */

        for (int i = 0; i < (m_numMajorTicks-1); i++)
        {
            lines << majorTickLine;
            QLineF minorTickLine(majorTickLine.x1(), majorTickLine.y1(), majorTickLine.x2(), majorTickLine.y1()-wm);
            for (int j = 0; j < (m_numMinorTicks-1); j++)
            {
                minorTickLine.translate(QPointF(interval,0));
                lines << minorTickLine;
            }
            majorTickLine.translate(QPointF(100.0/(m_numMajorTicks-1),0));
        }
        lines << majorTickLine;
    }
    else
    {
        qreal x1=scalePos, y1=0, wM=scaleSize, wm=wM*.5;
        QLineF majorTickLine(x1, y1, x1+wM, y1);
        qreal interval = 100.0/(m_numMajorTicks-1)/(m_numMinorTicks); /* distance between 2 minor ticks */
        for (int i = 0; i < (m_numMajorTicks-1); i++)
        {
            lines << majorTickLine;
            QLineF minorTickLine(majorTickLine.x1(), majorTickLine.y1(), majorTickLine.x1()+wm, majorTickLine.y1());
            for (int j = 0; j < (m_numMinorTicks-1); j++)
            {
                minorTickLine.translate(QPointF(0, interval));
                lines << minorTickLine;
            }
            majorTickLine.translate(QPointF(0, 100.0/(m_numMajorTicks-1)));
        }
        lines << majorTickLine;
    }
    p->drawLines(lines);
    if (m_referenceEnabled)
        drawMarker(p, false);
}

void caLinearGauge::drawMarker(QPainter *p, bool drawValue)
{
    QPolygonF triangolo;
    QColor c;
    QFont f, old;

    f = old = p->font();
    f.setPointSize(3);
    p->setFont(f);
    p->setPen(Qt::red);//p->setPen(EColor(Elettra::red));

    if (m_orientation == Qt::Horizontal)
    {
        if (drawValue)
        {
            QPointF	vertice(100*(m_value-m_minValue)/(m_maxValue-m_minValue), scalePos);
            triangolo << vertice << (vertice + QPointF(4,8)) << (vertice + QPointF(-4,8));
        }
        else
        {
            QPointF	vertice(100*(m_reference-m_minValue)/(m_maxValue-m_minValue), scalePos);
            triangolo << vertice << (vertice + QPointF(2,-4)) << (vertice + QPointF(-2,-4));
            p->drawText(vertice+QPoint(-4,-5), "REF");
        }
    }
    else
    {
        if (drawValue)
        {
            QPointF	vertice(scalePos, 100*(1-(m_value-m_minValue)/(m_maxValue-m_minValue)));
            triangolo << vertice << (vertice + QPointF(-8,-4)) << (vertice + QPointF(-8,4));
        }
        else
        {
            QPointF	vertice(scalePos, 100*(1-(m_reference-m_minValue)/(m_maxValue-m_minValue)));
            triangolo << vertice << (vertice + QPointF(4,-2)) << (vertice + QPointF(4,2));
            p->drawText(vertice+QPointF(5,1.5), "REF");
        }
    }
    p->setFont(old);
    p->setPen(Qt::black);

    if (drawValue)
        c = QColor(Qt::darkBlue);
    else
        c = QColor(Qt::red);
    c.setAlpha(100);
    p->setBrush(c);
    p->drawPolygon(triangolo);
}

void caLinearGauge::drawLabels(QPainter *p)
{
    double x, y, w, h;
    bool check = true;
    QFont f = p->font();
    QFontMetrics fm(f);
    h = fm.height();
    w = fm.width(labels[longestLabelIndex]);

    if (m_orientation == Qt::Horizontal)
    {
        y = 0;
        h = labelsSize;
        while (check && f.pointSize() > 2)
        {
            if ((p->fontMetrics().width(labels[longestLabelIndex]) > (p->window().width()/m_numMajorTicks)))
            {
                f.setPointSize(f.pointSize()-1);
                p->setFont(f);
            }
            else
                check = false;
        }

        for (int i = 0; i < m_numMajorTicks; i++)
        {
            QRectF br(i*100.0/(m_numMajorTicks-1)-w*.5, y, w, h);
            p->drawText(br, Qt::AlignBottom | Qt::AlignHCenter | Qt::TextDontClip, labels[i]);
        }
    }
    else
    {
        x = labelsPos;
        w = labelsSize;
        while (check && f.pointSize() > 2)
        {
            if (((p->fontMetrics().height()>(p->window().height()/m_numMajorTicks))||p->fontMetrics().width(labels[longestLabelIndex])>labelsSize))
            {
                f.setPointSize(f.pointSize()-1);
                p->setFont(f);
            }
            else
                check = false;
        }

        for (int i = 0; i < m_numMajorTicks; i++)
        {
            QRectF br(x, i*100.0/(m_numMajorTicks-1)-h*.5, w, h);
            p->drawText(br, Qt::AlignRight | Qt::AlignVCenter | Qt::TextDontClip, labels[m_numMajorTicks-1-i]);
        }
    }
}

caCircularGauge::caCircularGauge(QWidget *parent) : caAbstractGauge(parent),
    m_startAngle(240), m_stopAngle(300), m_arcLength(300),
    m_innerRadius(25), m_outerRadius(30), m_labelRadius(45),
    m_label(""), m_valueDisplayed(true)
{
    setExternalScale(false);
    setNumMajorTicks(CIRCULAR_MAJOR_TICKS);
    setNumMinorTicks(NUM_MINOR_TICKS);
    setMinimumSize(70,70);

    /* it is important that the font is installed since it has to scale well */
    QFont::insertSubstitutions("FreeSans", QStringList()  << "Verdana" << "Sans Serif");

    QFont f = this->font();
    f.setFamily("FreeSans"); /* Free sans scales ugly */
    // 	printf("Circular gauge font family %s\n", qstoc(f.family()));
    setFont(f);
}

void caCircularGauge::configure()
{
    if (m_scaleEnabled)
    {
        if (m_externalScale)
        {
            m_innerRadius = 25;
            m_outerRadius = 30;
            m_labelRadius = 46;
            setMinimumSize(80,80);
        }
        else
        {
            if (m_referenceEnabled)
            {
                m_innerRadius = 37;
                m_outerRadius = 46;
                m_labelRadius = 32;
            }
            else
            {
                m_innerRadius = 40;
                m_outerRadius = 49;
                m_labelRadius = 35;
            }
            setMinimumSize(70,70);
        }
    }
    else
    {
        m_innerRadius = 40;
        m_outerRadius = 49;
        m_labelRadius = 50;
        setMinimumSize(50,50);
    }
    caAbstractGauge::configure();
}

void caCircularGauge::paintEvent(QPaintEvent *)
{
    QPainter	painter(this);
    int size = qMin(height(), width());

    /* see http://doc.trolltech.com/4.4/coordsys.html#window-viewport-conversion for a analogous example */
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setViewport((int)((width()-size)*.5),(int)((height()-size)*.5),size,size);
    painter.setWindow(-50,-50,100,100);

    drawColorBar(&painter);

    if (isEnabled())
        drawNeedle(&painter);

    if (m_valueDisplayed)
        drawValue(&painter);

    if (!isEnabled())
    {
        QColor c = palette().color(QPalette::Background);
        c.setAlpha(200);
        painter.fillRect(painter.window(), c);
    }
    if (m_scaleEnabled)
    {
        drawScale(&painter);
        drawLabels(&painter);
    }
}

void caCircularGauge::drawColorBar(QPainter *p)
{
    p->setPen(Qt::NoPen);
    if (m_colorMode == GRADIENT)
    {
        QConicalGradient grad(0, 0, m_stopAngle);
        for (int i = 0; i < v_p.size(); i++)
            grad.setColorAt((1-v_p[i])*m_arcLength/360.0, v_c[i]);
        p->setBrush(grad);
        p->drawPie(-m_outerRadius,-m_outerRadius,m_outerRadius*2,m_outerRadius*2, (int)m_startAngle*16, (int)-m_arcLength*16);
    }
    else if (m_colorMode == COLORBAR)
    {
        for (int i = 1; i < v_p.size(); i++)
        {
            p->setBrush(v_c[i]);
            p->drawPie(-m_outerRadius,-m_outerRadius,m_outerRadius*2,m_outerRadius*2, (int)((m_startAngle-m_arcLength*v_p[i])*16), (int)((m_arcLength*v_p[i]-m_arcLength*v_p[i-1])*16));
        }
    }
    else
    {
        p->setBrush(v_c[0]);
        p->drawPie(-m_outerRadius,-m_outerRadius,m_outerRadius*2,m_outerRadius*2, (int)(m_startAngle*16), (int)-(m_arcLength*16));
    }
    p->setBrush(palette().background());
    p->drawEllipse(-m_innerRadius,-m_innerRadius,m_innerRadius*2,m_innerRadius*2);
}

void caCircularGauge::drawNeedle(QPainter *p)
{
    double angle = (m_startAngle-(m_value-m_minValue)/(m_maxValue-m_minValue)*m_arcLength)*3.1415927/180.0;

    QPolygonF tr1, tr2;
    QPointF longArm, shortArm, side1, side2;

    longArm = QPointF(m_outerRadius*cos(angle),-m_outerRadius*sin(angle));
    shortArm = QPointF(-2*cos(angle),2*sin(angle));
    side1 = QPointF(-2*sin(angle),-2*cos(angle));
    side2 = -side1;

    tr1 << longArm << side1 << shortArm;
    tr2 << longArm << side2 << shortArm;

    p->setPen(Qt::NoPen);
    QRadialGradient grad1(QPointF(0,0),m_outerRadius,side1*.5);
    grad1.setColorAt(0.0, palette().color(QPalette::Mid));
    grad1.setColorAt(1.0, palette().color(QPalette::Dark));
    QRadialGradient grad2(QPointF(0,0),m_outerRadius,side2*.5);
    grad2.setColorAt(0.0, palette().color(QPalette::Midlight));
    grad2.setColorAt(1.0, palette().color(QPalette::Dark));
    p->setBrush(grad1);
    p->drawPolygon(tr1);
    p->setBrush(grad2);
    p->drawPolygon(tr2);

    QPen pen(Qt::black);
    pen.setJoinStyle(Qt::RoundJoin);
    p->setPen(pen);
    p->drawLine(longArm,side1);
    p->drawLine(side1,shortArm);
    p->drawLine(shortArm,side2);
    p->drawLine(side2,longArm);

    p->setBrush(palette().color(QPalette::Dark));
    p->drawEllipse(QRectF(-.5,-.5,1,1));
}

void caCircularGauge::drawScale(QPainter *p)
{
    p->setPen(Qt::black);
    p->drawArc(-(m_outerRadius+0), -(m_outerRadius+0), 2*(m_outerRadius+0), 2*(m_outerRadius+0), (int)m_startAngle*16, (int)-m_stopAngle*16);

    qreal start = m_startAngle*3.1415927/180.0;
    QPointF startPoint = QPointF((m_outerRadius+0)*cos(start),-(m_outerRadius+0)*sin(start));
    QPointF stopPoint, stopPoint2;
    if (m_externalScale)
    {
        stopPoint = startPoint + QPointF(8*cos(start),-8*sin(start));
        stopPoint2 = startPoint + QPointF(4*cos(start),-4*sin(start));
    }
    else
    {
        stopPoint = startPoint - QPointF(8*cos(start),-8*sin(start));
        stopPoint2 = startPoint - QPointF(4*cos(start),-4*sin(start));
    }
    QLineF majorTickLine(startPoint, stopPoint);
    QLineF minorTickLine(startPoint, stopPoint2);

    p->save();
    for (int i = 0; i < m_numMajorTicks; i++)
    {
        p->drawLine(majorTickLine);
        if (i < (m_numMajorTicks-1))
        {
            for (int j = 0; j < (m_numMinorTicks-1); j++)
            {
                p->rotate(m_arcLength/(m_numMajorTicks-1)/m_numMinorTicks);
                p->drawLine(minorTickLine);
            }
            p->rotate(m_arcLength/(m_numMajorTicks-1)/m_numMinorTicks);
        }
    }
    p->restore();
    if (m_referenceEnabled)
    {
        double angle = (m_startAngle-(m_reference-m_minValue)/(m_maxValue-m_minValue)*m_arcLength)*3.1415927/180.0;
        double shift = 2.5*3.1415927/180.0;
        QPolygonF tr;
        QPointF p1, p2, p3;
        p1 = QPointF(m_outerRadius*cos(angle),-m_outerRadius*sin(angle));
        p2 = QPointF((m_outerRadius+3)*cos(angle+shift),-(m_outerRadius+3)*sin(angle+shift));
        p3 = QPointF((m_outerRadius+3)*cos(angle-shift),-(m_outerRadius+3)*sin(angle-shift));
        tr << p1 << p2 << p3;

        QColor c = QColor(Qt::red);//EColor(Elettra::red);
        c.setAlpha(100);
        p->setBrush(c);
        p->setPen(Qt::black);
        p->drawPolygon(tr);
    }
}

void caCircularGauge::drawLabels(QPainter *p)
{
    double angle = m_startAngle*3.1415927/180.0;
    double step = m_arcLength/(m_numMajorTicks-1)*3.1415927/180.0;

    QFont f = p->font();
    if (m_externalScale)
        f.setPointSizeF(5.5);
    else
        f.setPointSizeF(5.4);
    p->setFont(f);
    for (int i = 0; i < m_numMajorTicks; i++)
    {
        QPointF vertice(m_labelRadius*cos(angle), -m_labelRadius*sin(angle));
        vertice -= QPointF(13,10);
        QRectF br(vertice, QSize(26, 20));
        br = QRectF(vertice, QSize(26, 20));
        p->drawText(br, Qt::AlignCenter, labels[i]);
        angle -= step;
    }
}

void caCircularGauge::drawValue(QPainter *p)
{
    int x, y, w, h;
    QString s;
    if (isEnabled())
    {
        EngString engString(QString().sprintf(m_valueFormat.toLatin1(), d_naturalValue), m_valueFormat, d_naturalValue);
        s = engString;
    }
    else
        s = "####";
    QFont f = p->font();
    if (m_scaleEnabled)
    {
        f.setPointSize(7);
        x = -16; y = 5; w = 32; h = 10;
    }
    else
    {
        f.setPointSize(10);
        x = -20; y = 8; w = 40; h = 14;
    }
    p->setFont(f);
    bool check = true;
    while (check)
    {
        if ((p->fontMetrics().width(s) > (w-2)))
        {
            qreal fontSize = f.pointSizeF()-.5;
            if(fontSize < MIN_FONT_SIZE) fontSize = MIN_FONT_SIZE;
            f.setPointSizeF(fontSize);
            p->setFont(f);
        }
        else
            check = false;
    }
    QRect textRect(x,y+2,w,h);
    p->setBrush(QColor(255,255,255,100));
    p->drawRect(textRect);
    p->drawText(x,y+1,w,h, Qt::AlignCenter|Qt::TextDontClip, s);

    f.setPointSize(8);
    p->setFont(f);
    check = true;
    while (check)
    {
        if ((p->fontMetrics().width(m_label) > (38)))
        {
            f.setPointSizeF(f.pointSizeF()-.5);
            p->setFont(f);
        }
        else
            check = false;
    }
    p->drawText(-20,35,40,10, Qt::AlignCenter|Qt::TextDontClip, m_label);
}
