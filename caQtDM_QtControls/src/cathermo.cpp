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

#if defined(_WIN32) || defined(_WIN64)
#define QWT_DLL
#endif

#include <math.h>
#include <QtDebug>
#include <QPainter>
#include <QPaintEvent>
#include "alarmdefs.h"
#include "qwt_scale_map.h"
#include "cathermo.h"

#define MIN_FONT_SIZE 5
#define MAX_FONT_SIZE 20


// I need to overload the scaleengine of qwt in order to get the upper and lower scale ticks drawn
class myScaleEngine: public QwtLinearScaleEngine
{
    virtual QwtScaleDiv divideScale(double x1, double x2, int maxMajorSteps, int maxMinorSteps, double stepSize  ) const {
        QwtScaleDiv sd = QwtLinearScaleEngine::divideScale(x1, x2, maxMajorSteps, maxMinorSteps, stepSize );
        QList<double> ticks = sd.ticks( QwtScaleDiv::MajorTick );
        if(ticks.count() > 0) {
            if ( ticks.last() < sd.upperBound() || ticks.first() > sd.lowerBound()){
                if(ticks.last() < sd.upperBound()) ticks.append(sd.upperBound());
                if(ticks.first() > sd.lowerBound()) ticks.prepend(sd.lowerBound());
                sd.setTicks( QwtScaleDiv::MajorTick, ticks );
            } 
        }
        return sd;
    }
};

caThermo::caThermo(QWidget *parent) : QwtThermoMarker(parent), m_externalEnabled(false)
{

    thisDirection = Up;
    defaultBackColor = QWidget::palette().background().color();
    defaultForeColor = palette().foreground().color();
    thisColorMode = Static;
    thisLimitsMode = Channel;

    pointSizePrv = 0.0;

    setBackground(QColor(224,224,224));
    setForeground(Qt::black);

    setSpacing(0);
    setBorderWidth(1);

    thisLogScale = false;

    valPixOld = -999999;
    setLook(noLabel);

    QwtLinearScaleEngine *scaleEngine = new myScaleEngine();
    setScaleEngine( scaleEngine );
}

QString caThermo::getPV() const
{
    return thisPV;
}

void caThermo::setPV(QString const &newPV)
{
    thisPV = newPV;
}

void caThermo::setColors(QColor bg, QColor fg)
{

    if((oldBackColor == bg) && (oldForeColor == fg)) return;

    QPalette thisPalette = palette();
    QColor bgs = bg.darker(125);
    bgs.setAlpha(255);
    setAutoFillBackground(true);
    thisPalette.setColor(QPalette::Background, bg);
    thisPalette.setColor(QPalette::Base, bgs);
    thisPalette.setColor(QPalette::ButtonText, fg);
    setPalette(thisPalette);
    oldBackColor = bg;
    oldForeColor = fg;
}

void caThermo::setBackground(QColor c)
{
    QColor color = c;
    if(thisColorMode == Default) {
        thisBackColor = defaultBackColor;
    } else {
      thisBackColor = color;
    }
    setColors(thisBackColor, thisForeColor);
}

void caThermo::setForeground(QColor c)
{
    QColor color = c;
    if(thisColorMode == Default) {
       thisForeColor= defaultForeColor;
    } else {
      thisForeColor = color;
    }
    setColors(thisBackColor, thisForeColor);
}

void caThermo::setLook(Look look)
{
    thisLook = look;
    switch (look) {
    case Outline:
    case Limits:
    case ChannelV:
        thisScale = true;
        break;
    case noLabel:
    case noDeco:
        thisScale = false;
        break;
    default:
        thisScale = false;
        break;
    }

    setDirection(thisDirection);
}

void caThermo::setValue(double val)
{
    int valPix;
    double thisValue;
    if(thisLogScale && val > 0.0) {
       thisValue = log10(val);
    } else {
       thisValue = val;
    }

    valPix = qRound(scaleMap().transform(thisValue));
    if(valPixOld != valPix) {
      QwtThermoMarker::setValue(thisValue);
    }
}

void caThermo::setDirection(Direction dir)
{
    thisDirection = dir;

    switch (dir) {
    case Up:
    case Down:
        if(thisScale) {
            setScalePosition(LeftScale);
#if QWT_VERSION < 0x060100
            setOrientation(Qt::Vertical, LeftScale);
#else
            setOrientation(Qt::Vertical);
#endif
        } else {

            setScalePosition(NoScale);
#if QWT_VERSION < 0x060100
            setOrientation(Qt::Vertical, NoScale);
#else
            setOrientation(Qt::Vertical);
#endif
        }
        break;
    case Left:
    case Right:
        if(thisScale) {
            setScalePosition(BottomScale);
#if QWT_VERSION < 0x060100
            setOrientation(Qt::Horizontal, BottomScale);
#else
            setOrientation(Qt::Horizontal);
#endif
        } else {
            setScalePosition(NoScale);
#if QWT_VERSION < 0x060100
            setOrientation(Qt::Horizontal, NoScale);
#else
            setOrientation(Qt::Horizontal);
#endif
        }
        break;
    }
    update();
}

void caThermo::setAlarmColors(short status)
{
    QColor c;
    switch (status) {

    case NO_ALARM:
        c=AL_GREEN;
        break;
    case MINOR_ALARM:
        c=AL_YELLOW;
        break;
    case MAJOR_ALARM:
        c=AL_RED;
        break;
    case INVALID_ALARM:
    case NOTCONNECTED:
        c=AL_WHITE;
        break;
    default:
        c=AL_DEFAULT;
        break;
    }
    if(status == NOTCONNECTED) {
       setColors(c, c);
    } else {
       setColors(thisBackColor, c);
    }
}

void caThermo::setNormalColors()
{
    setColors(thisBackColor, thisForeColor);
}

void caThermo::setUserAlarmColors(double val)
{
    double thisValue;
    if(thisLogScale && val > 0.0) {
       thisValue = log10(val);
    } else {
        thisValue = val;
    }

    //printf("%lf %lf %lf %d\n", val, this->minValue(), this->maxValue(), thisDirection);
    switch (thisDirection) {
    case Up:
    case Right:
        if((thisValue < this->minValue()) || (thisValue > this->maxValue())) {
           setColors(thisBackColor, AL_RED);
        } else {
           setColors(thisBackColor, AL_GREEN);
        }
        break;
    case Down:
    case Left:
        if((thisValue < this->maxValue()) || (thisValue > this->minValue())) {
           setColors(thisBackColor, AL_RED);
        } else {
           setColors(thisBackColor, AL_GREEN);
        }
        break;
    }
}


// here we adapt the scale font size and the handle size to the size of the widget
bool caThermo::event(QEvent *e)
{
    if(e->type() == QEvent::Resize || e->type() == QEvent::Show || e->type() == QEvent::Paint) {

        const caThermo* that = this;

        switch (thisScale) {

        case false:
            switch (thisDirection) {
            case Up:
            case Down: {
                int pipewidth = width()-4;
                if(pipewidth != this->pipeWidth()) {
                    this->setPipeWidth(pipewidth);
                }
                break;
            }

            case Right:
            case Left: {
                int pipewidth = height() -4;
                if(pipewidth != this->pipeWidth()) {
                    this->setPipeWidth(pipewidth);
                }
                break;
            }
            default:
                break;
            }
            break;

        case  true:
            switch (thisDirection) {

                case Up:
                case Down: {
                    int pipewidth = width()*2/5-4;
                    if(pipewidth != this->pipeWidth()) {
                        this->setPipeWidth(pipewidth);
                    }
                    QFont f = font();
                    int size = that->scaleDraw()->maxLabelWidth(f);
                    float xFactor = (float) size  / ((float) width() * 3.0/5.0 - 15.0);

                    if(xFactor < 0.1) break;

                    float pointSize = f.pointSizeF() / xFactor;
                    if(pointSize < MIN_FONT_SIZE) pointSize = MIN_FONT_SIZE;
                    if(pointSize > MAX_FONT_SIZE) pointSize = MAX_FONT_SIZE;

                    if(qAbs(pointSize - pointSizePrv) >= 2.0) {
                        f.setPointSizeF(pointSize);
                        pointSizePrv = pointSize;
                        setFont(f);
                        update();
                    }
                }
                break;

                case Right:
                case Left: {
                   int pipewidth = height()*2/5-4;
                   if(pipewidth != this->pipeWidth()) {
                       this->setPipeWidth(pipewidth);
                   }
                   QFont f = font();
                   int size = that->scaleDraw()->maxLabelWidth(f);
                   float yFactor = (float) size  / ((float) height()*3.0/5.0 - 10.0);

                   if(yFactor < 0.1) break;

                   float pointSize = f.pointSizeF() / yFactor;
                   if(pointSize < MIN_FONT_SIZE) pointSize = MIN_FONT_SIZE;
                   if(pointSize > MAX_FONT_SIZE) pointSize = MAX_FONT_SIZE;

                   if(qAbs(pointSize - pointSizePrv) >= 2.0) {
                       f.setPointSizeF(pointSize);
                       pointSizePrv = pointSize;
                       setFont(f);
                       update();
                   }
                }
                break;
            }

        }

    }

    return QwtThermoMarker::event(e);
}
