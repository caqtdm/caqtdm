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
#include "qwt_scale_draw.h"
#include "cathermo.h"

#define MIN_FONT_SIZE 3
#define MAX_FONT_SIZE 20


// I need to overload the scaleengine of qwt in order to get the upper and lower scale ticks drawn
class myThermoScaleEngine: public QwtLinearScaleEngine
{
    virtual QwtScaleDiv divideScale(double x1, double x2, int maxMajorSteps, int maxMinorSteps, double stepSize  ) const {
        QwtScaleDiv sd = QwtLinearScaleEngine::divideScale(x1, x2, maxMajorSteps, maxMinorSteps, stepSize );
        QList<double> ticks = sd.ticks( QwtScaleDiv::MajorTick );
        if(ticks.count() > 0) {
            if ( ticks.last() < sd.upperBound() || ticks.first() > sd.lowerBound()) {
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
    // to start with, clear the stylesheet, so that playing around
    // is not possible.
    setStyleSheet("");

    isShown = false;
    oldStyle = "";
    thisStyle = "";

    thisDirection = Up;
    thisColorMode = Static;
    thisLimitsMode = Channel;

    oldBackColor = QColor(Qt::white);
    oldForeColor = QColor(Qt::white);

    setBackground(QColor(224,224,224));
    setForeground(Qt::black);
    setTextColor(Qt::black);

    pointSizePrv = 0.0;

    setSpacing(0);
    setBorderWidth(1);

    thisLogScale = false;

    valPixOld = -999999;
    setLook(noLabel);

    QwtLinearScaleEngine *scaleEngine = new myThermoScaleEngine();
    setScaleEngine( scaleEngine );

    setPrecision(1);
    setPrecisionMode(Channel);
    setFormatType(decimal);
    setScaleValueEnabled(false);
}

QString caThermo::getPV() const
{
    return thisPV;
}

void caThermo::setPV(QString const &newPV)
{
    thisPV = newPV;
}

void caThermo::setColors(QColor bg, QColor fg, QColor textColor)
{
    if(!defaultBackColor.isValid() || !defaultForeColor.isValid()) return;

    if((bg != oldBackColor) || (fg != oldForeColor)  || (textColor != oldTextColor) || (thisColorMode != oldColorMode)) {

        QPalette thisPalette = palette();

        QString thisStyle = "caThermo {background-color: rgba(%1, %2, %3, %4); color: rgba(%5, %6, %7, %8)};";

        if(thisColorMode == Default) {
            thisStyle = thisStyle.arg(defaultBackColor.red()).arg(defaultBackColor.green()).arg(defaultBackColor.blue()).arg(defaultBackColor.alpha()).
                    arg(defaultForeColor.red()).arg(defaultForeColor.green()).arg(defaultForeColor.blue()).arg(defaultForeColor.alpha());
            if(thisStyle != oldStyle) setStyleSheet(thisStyle);
            oldStyle = thisStyle;
            QColor bgs = defaultBackColor.darker(125);
            bgs.setAlpha(255);
            thisPalette.setColor(QPalette::ButtonText, defaultForeColor);
            thisPalette.setColor(QPalette::Text, textColor);
            thisPalette.setColor(QPalette::Base, bgs);

            setPalette(thisPalette);
        } else if(thisColorMode == Static) {
            thisStyle = thisStyle.arg(bg.red()).arg(bg.green()).arg(bg.blue()).arg(bg.alpha()).arg(fg.red()).arg(fg.green()).arg(fg.blue()).arg(fg.alpha());
            if(thisStyle != oldStyle) setStyleSheet(thisStyle);
            oldStyle = thisStyle;
            QColor bgs = bg.darker(125);
            bgs.setAlpha(255);
            thisPalette.setColor(QPalette::ButtonText, fg);
            thisPalette.setColor(QPalette::Text, textColor);
            thisPalette.setColor(QPalette::Base, bgs);
            setPalette(thisPalette);

        } else if(thisColorMode == Alarm_Static) {
            thisStyle = thisStyle.arg(thisBackColor.red()).arg(thisBackColor.green()).arg(thisBackColor.blue()).arg(thisBackColor.alpha()).
                    arg(thisForeColor.red()).arg(thisForeColor.green()).arg(thisForeColor.blue()).arg(thisForeColor.alpha());
            if(thisStyle != oldStyle) setStyleSheet(thisStyle);
            oldStyle = thisStyle;
            QColor bgs = bg.darker(125);
            bgs.setAlpha(255);
            thisPalette.setColor(QPalette::ButtonText, fg);
            thisPalette.setColor(QPalette::Text, textColor);
            thisPalette.setColor(QPalette::Base, bgs);
            setPalette(thisPalette);

        } else  if(thisColorMode == Alarm_Default) {
            thisStyle = thisStyle.arg(defaultBackColor.red()).arg(defaultBackColor.green()).arg(defaultBackColor.blue()).arg(defaultBackColor.alpha()).
                    arg(defaultForeColor.red()).arg(defaultForeColor.green()).arg(defaultForeColor.blue()).arg(defaultForeColor.alpha());
            if(thisStyle != oldStyle) setStyleSheet(thisStyle);
            oldStyle = thisStyle;
            QColor bgs = defaultBackColor.darker(125);
            bgs.setAlpha(255);
            thisPalette.setColor(QPalette::ButtonText, fg);
            thisPalette.setColor(QPalette::Text, textColor);
            thisPalette.setColor(QPalette::Base, bgs);
            setPalette(thisPalette);
        }
    }

    oldBackColor = bg;
    oldForeColor = fg;
    oldTextColor = textColor;
    oldColorMode = thisColorMode;
}

void caThermo::setBackground(QColor c)
{
    thisBackColor = c;
    setColors(thisBackColor, thisForeColor, thisTextColor);
}

void caThermo::setForeground(QColor c)
{
    thisForeColor = c;
    setColors(thisBackColor, thisForeColor, thisTextColor);
}

void caThermo::setTextColor(QColor c)
{
    thisTextColor = c;
    setColors(thisBackColor, thisForeColor, thisTextColor);
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

#if QWT_VERSION < 0x060100
    ScalePos scalepos = this->scalePosition();
#else
    ScalePosition scalepos = this->scalePosition();
#endif

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

    if(scalepos != NoScale) setScalePosition(scalepos);
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
       setColors(c, c, c);
    } else {
       setColors(thisBackColor, c, thisTextColor);
    }
}

void caThermo::setNormalColors()
{
    setColors(thisBackColor, thisForeColor, thisTextColor);
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
           setColors(thisBackColor, AL_RED, thisTextColor);
        } else {
           setColors(thisBackColor, AL_GREEN, thisTextColor);
        }
        break;
    case Down:
    case Left:
        if((thisValue < this->maxValue()) || (thisValue > this->minValue())) {
           setColors(thisBackColor, AL_RED, thisTextColor);
        } else {
           setColors(thisBackColor, AL_GREEN, thisTextColor);
        }
        break;
    }
}


// here we adapt the scale font size and the handle size to the size of the widget
bool caThermo::event(QEvent *e)
{
    if(e->type() == QEvent::Resize || e->type() == QEvent::Show || e->type() == QEvent::Paint) {

        if(!isShown) {
            QString c=  palette().color(QPalette::Base).name();
            defaultBackColor = QColor(c);
            c =  palette().color(QPalette::Text).name();
            defaultForeColor = QColor(c);

            if(!defaultBackColor.isValid()) defaultBackColor = QColor(255, 248, 220, 255);
            if(!defaultForeColor.isValid()) defaultForeColor = Qt::black;

            setColors(thisBackColor, thisForeColor, thisTextColor);
            isShown = true;
        }

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

//Overridden from QwtSlider to add the display of the current value
void caThermo::drawLiquid(QPainter *painter, const QRect &sliderRect ) const
{
    QwtThermoMarker::drawLiquid(painter, sliderRect);

    if(this->scalePosition() != NoScale && thisScaleValueEnabled) {
        paintValue (painter,  sliderRect);
    }
}

// Draws the label with the value on the widget
void caThermo::paintValue(QPainter *painter, QRect valueRect) const
{
    if(orientation() == Qt::Horizontal) {
        painter->drawText( valueRect, Qt::AlignCenter, setScaleLabel(value()) );
    } else {
        QFontMetrics fm(font());
        QString label = setScaleLabel(value());
        int w = fm.width(label);
        int h = fm.height();
        switch (this->scalePosition()) {
        case LeftScale:
            painter->rotate(270);
            painter->drawText(QPoint(-valueRect.height()/2-w/2, width() - valueRect.width() + h - fm.descent()), label);
            break;
        case RightScale:
            painter->rotate(270);
            painter->drawText(QPoint(-valueRect.height()/2-w/2, valueRect.width()/2 + h/2 - fm.descent()), label);
            break;
        case TopScale:
        case BottomScale:
        case NoScale:
            break;
        }
    }
}

// Creates the precision format the value label will use
void caThermo::setFormat(int prec)
{
    int precision = prec;
    if(precision > 17) precision = 17;
    if(thisPrecMode == User) {
        precision = getPrecision();
    }
    switch (thisFormatType) {
    case decimal:
        if(precision >= 0) {
            sprintf(thisFormat, "%s.%dlf", "%", precision);
        } else {
            sprintf(thisFormat, "%s.%dle", "%", -precision);
        }
        break;
    case compact:
        sprintf(thisFormat, "%s.%dle", "%", qAbs(precision));
        sprintf(thisFormatC, "%s.%dlf", "%", qAbs(precision));
        break;
    case exponential:
    case engr_notation:
        sprintf(thisFormat, "%s.%dle", "%", qAbs(precision));
        break;
    case truncated:
        strcpy(thisFormat, "%d");
        break;
    default:
        sprintf(thisFormat, "%s.%dlf", "%", precision);
    }

}

// Creates the QString that will be displayed for the value label
QString caThermo::setScaleLabel(double value) const
{
    char asc[1024];
    QString label;

    if(thisFormatType == compact) {
        if ((value < 1.e4 && value > 1.e-4) || (value > -1.e4 && value < -1.e-4) || value == 0.0) {
            sprintf(asc, thisFormatC, value);
        } else {
            sprintf(asc, thisFormat, value);
        }
    } else if(thisFormatType == truncated) {
        sprintf(asc, thisFormat, (int) value);
    } else {
        sprintf(asc, thisFormat, value);
    }
    label = QString::fromAscii(asc);

    return label;
}

void caThermo::setScaleValueEnabled(bool b)
{
    thisScaleValueEnabled = b;
    update();
}
