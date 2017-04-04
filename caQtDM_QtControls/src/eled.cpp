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

#include "eled.h"
#include <QPainter>
#include <QRadialGradient>
#include <QtDebug>
#include <math.h>

ELed::ELed(QWidget *parent) : QWidget(parent), ledColor(Qt::gray)
{
    setMinimumHeight(4);
    setMinimumWidth(4);
    d_width = d_height = 18;
    d_angleDeg = 0;
    d_rectangular = false;
    d_gradientEnabled = true;
    d_linearGradient = false;
    d_scaleContents = false;
    d_gradientStart = 0.10;
    d_gradientStop = 0.95;
    d_alphaChannel = 255;

    QSizePolicy p(sizePolicy());
    p.setHeightForWidth(true);
    setSizePolicy(p);
    setBorderColor(Qt::black);
    return;
}


void ELed::setColor(const QColor &c, bool up)
{
    if(c == ledColor)
        return;

    ledColor = c;
    ledColor.setAlpha(d_alphaChannel);
    if (up)
        update();
}

void ELed::setBorderColor(const QColor &c)
{
    borderColor = c;
    update();
}

void ELed::paintEvent(QPaintEvent *)
{
    QPainter	painter(this);
    QMatrix m;
    int ledWidth, ledHeight;
    int w, h, w1, h1;

    // in case of a gradient, we will have to keep the correct aspect ratio
    if(d_gradientEnabled) {
        w = h = qMin(d_width, d_height);
        w1 = h1 = qMin(width() - 2, height() - 2);
    } else {
        w = d_width;
        h = d_height;
        w1 = width() - 2;
        h1 = height() - 2;
    }

    d_scaleContents ? ledWidth = w1: ledWidth = w;
    d_scaleContents ? ledHeight = h1: ledHeight = h;

    //painter.rotate(d_angleDeg);
    qreal		radius  = .45 * qMin(ledWidth, ledHeight);
    qreal x1, y1;
    x1 = width()/2.0 - ledWidth/2.0;
    y1 = height()/2.0 - ledHeight/2.0;

    //QPointF  topLeft(x1, y1);
    QRect rect((int)x1, (int)y1, ledWidth, ledHeight);
    //QPointF center(rect.center());

    if(!d_linearGradient && d_gradientEnabled)
    {
        QRadialGradient	gradient(.5 * width(), .5 * height(), radius, .5 * (width() - radius), .5 * (height() - radius));
        gradient.setColorAt(d_gradientStart, QColor(Qt::white));
        if (isEnabled()) {
            gradient.setColorAt(d_gradientStop, ledColor);
        } else {
            gradient.setColorAt(d_gradientStop, QColor(Qt::gray));
        }

        QColor color( 200,200,200,50);
        gradient.setColorAt(1, borderColor); //QColor(Qt::black));
        painter.setPen(borderColor);
        painter.setBrush(gradient);
    }
    else if(d_gradientEnabled && d_linearGradient)
    {
        QLinearGradient	gradient(rect.topLeft(), rect.bottomRight());
        gradient.setColorAt(d_gradientStart, QColor(Qt::white));
        if (isEnabled()) {
            gradient.setColorAt(d_gradientStop, ledColor);
        } else {
            gradient.setColorAt(d_gradientStop, QColor(Qt::gray));
        }
        gradient.setColorAt(1, borderColor); //QColor(Qt::black));
        painter.setPen(borderColor);
        painter.setBrush(gradient);
    }
    else if(!d_gradientEnabled)
    {
        QBrush b(ledColor);
        painter.setBrush(b);
        painter.setPen(borderColor);
    }
    painter.setRenderHint(QPainter::Antialiasing);

    // we do not use angle any more
    /*
    m.translate(center.x(), center.y());
    m.rotate(d_angleDeg);
    m.translate(-center.x(), -center.y());
    painter.setWorldMatrix(m, true);
    */
    if(d_rectangular) {
        painter.drawRect(rect);
    } else {
        painter.drawEllipse(rect);
    }
}

void ELed::setAlphaChannel(int a)
{
    if(a < 0)
    {
        printf("alpha channel %d is not valid: it must be an integer between 0 and 255. Setting to 0", a);
        d_alphaChannel = 0;
    }
    else if(a > 255)
    {
        printf("alpha channel %d is not valid: it must be an integer between 0 and 255. Setting to 255", a);
        d_alphaChannel = 255;
    }
    d_alphaChannel = a;
    ledColor.setAlpha(a);
    update();
}

void ELed::setLinearGradient(bool l)
{
    d_linearGradient = l;
    update();
}

void ELed::setGradientEnabled(bool grad)
{
    d_gradientEnabled = grad;
    update();
}

void ELed::setAngle(int deg)
{
    d_angleDeg = deg;
    update();
}

void ELed::setLedWidth(int w)
{
    setMinimumWidth(w + 2);
    if(maximumWidth() < minimumWidth())
        setMaximumWidth(minimumWidth());
    if(w < this->width())
        d_width = w;
    else
        d_width = this->width();
    update();
}

void ELed::setLedHeight(int h)
{
    setMinimumHeight(h + 2);
    if(maximumHeight() < minimumHeight())
        setMaximumHeight(minimumHeight());
    if(h < this->height())
        d_height = h;
    else
        d_height = this->height();
    update();
}

void ELed::setRectangular(bool re)
{
    d_rectangular = re;
    update();
}

void ELed::setScaleContents(bool s)
{
    d_scaleContents = s;
    update();
}

void ELed::setGradientStop(double val)
{
    d_gradientStop = val;
    update();
}

void ELed::setGradientStart(double val)
{
    d_gradientStart = val;
    update();
}

QSize ELed::sizeHint() const
{
    return QSize(d_width, d_height);
}

QSize ELed::minimumSizeHint() const
{
    return this->sizeHint();
}


