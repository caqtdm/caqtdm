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
#include <qnumeric.h>
//#include <QMetaEnum>
#if defined(_MSC_VER)
    #ifndef snprintf
     #define snprintf _snprintf
    #endif
#endif


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

    thisDirection = Up;
    thisColorMode = Static;
    thisLimitsMode = Channel;

    oldBackColor = QColor(Qt::white);
    oldForeColor = QColor(Qt::white);
    oldColorMode = Default;

    // these default could be changed by a stylesheet
    defaultForeColor = QColor(Qt::darkRed);
    defaultBackColor = QColor(Qt::lightGray);
    defaultForeColorOld = defaultBackColor;
    defaultBackColorOld = defaultForeColor;

    setAutoFillBackground( true );
    setBackground(QColor(224,224,224));
    setForeground(Qt::black);
    setTextColor(Qt::black);

    pointSizePrv = 0.0;

    setSpacing(0);
    setBorderWidth(1);

    thisLogScale = false;

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

void caThermo::setColors(QColor bg, QColor fg, QColor textColor, colMode mode)
{

    if((bg != oldBackColor) || (fg != oldForeColor)  || (textColor != oldTextColor) || (mode != oldColorMode)
            || (defaultBackColorOld != defaultBackColor) || (defaultForeColorOld != defaultForeColor)) {

        QPalette thisPalette = palette();

        if(thisColorMode == Default) {
            if(!defaultBackColor.isValid() || !defaultForeColor.isValid()) return;
            QColor bgs = defaultBackColor.darker(125);
            bgs.setAlpha(bg.alpha());
            //printf("default palette fg %d %d %d %s\n", defaultForeColor.red(), defaultForeColor.green(), defaultForeColor.blue(), qasc(objectName()));
            //printf("default palette bg %d %d %d %s\n", defaultBackColor.red(), defaultBackColor.green(), defaultBackColor.blue(), qasc(objectName()));
            thisPalette.setColor(QPalette::ButtonText, defaultForeColor);
            thisPalette.setColor(QPalette::Text, textColor);
            thisPalette.setColor(QPalette::WindowText, textColor);
            thisPalette.setColor(QPalette::Window, bg);
            thisPalette.setColor(QPalette::Base, bgs);
            setPalette(thisPalette);

        } else if(thisColorMode == Static) {
            QColor bgs = bg.darker(125);
            bgs.setAlpha(bg.alpha());
            //printf("static set palette fg  %d %d %d\n", fg.red(), fg.green(), fg.blue());
            //printf("static set palette bg  %d %d %d\n", bgs.red(), bgs.green(), bgs.blue());
            //printf("static set palette txt %d %d %d\n", textColor.red(), textColor.green(), textColor.blue());
            thisPalette.setColor(QPalette::ButtonText, fg);
            thisPalette.setColor(QPalette::Text, textColor);
            thisPalette.setColor(QPalette::WindowText, textColor);
            thisPalette.setColor(QPalette::Window, bg);
            thisPalette.setColor(QPalette::Base, bgs);
            setPalette(thisPalette);

        } else if(thisColorMode == Alarm_Static) {
            QColor bgs = bg.darker(125);
            bgs.setAlpha(bg.alpha());
            //printf("alarm_static set palette %d %d %d\n", fg.red(), fg.green(), fg.blue());
            thisPalette.setColor(QPalette::ButtonText, fg);
            thisPalette.setColor(QPalette::Text, textColor);
            thisPalette.setColor(QPalette::WindowText, textColor);
            thisPalette.setColor(QPalette::Window, bg);
            thisPalette.setColor(QPalette::Base, bgs);
            setPalette(thisPalette);

        } else  if(thisColorMode == Alarm_Default) {
            if(!defaultBackColor.isValid() || !defaultForeColor.isValid()) return;
            QColor bgs = defaultBackColor.darker(125);
            bgs.setAlpha(bg.alpha());
            //printf("alarm_default palette %d %d %d\n", fg.red(), fg.green(), fg.blue());
            thisPalette.setColor(QPalette::ButtonText, fg);
            thisPalette.setColor(QPalette::Text, textColor);
            thisPalette.setColor(QPalette::WindowText, textColor);
            thisPalette.setColor(QPalette::Window, bg);
            thisPalette.setColor(QPalette::Base, bgs);
            setPalette(thisPalette);
        }
    }

    oldBackColor = bg;
    oldForeColor = fg;
    oldTextColor = textColor;
    oldColorMode = mode;
    defaultBackColorOld = defaultBackColor;
    defaultForeColorOld = defaultForeColor;
}

void caThermo::setBackground(QColor c)
{
    thisBackColor = c;
    setColors(thisBackColor, thisForeColor, thisTextColor, thisColorMode);
}

void caThermo::setForeground(QColor c)
{
    thisForeColor = c;
    setColors(thisBackColor, thisForeColor, thisTextColor, thisColorMode);
}

void caThermo::setTextColor(QColor c)
{
    thisTextColor = c;
    setColors(thisBackColor, thisForeColor, thisTextColor, thisColorMode);
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
    double thisValue;
    if(thisLogScale && val > 0.0) {
       thisValue = log10(val);
    } else {
       thisValue = val;
    }

    QwtThermoMarker::setValue(thisValue);
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

    // update() did not really do what was expected here
    resize(width()+1, height());
    resize(width()-1, height());

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
       setColors(c, c, c, thisColorMode);
    } else {
       setColors(thisBackColor, c, thisTextColor, thisColorMode);
    }
}

void caThermo::setNormalColors()
{
    setColors(thisBackColor, thisForeColor, thisTextColor, thisColorMode);
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
           setColors(thisBackColor, AL_RED, thisTextColor, thisColorMode);
        } else {
           setColors(thisBackColor, AL_GREEN, thisTextColor, thisColorMode);
        }
        break;
    case Down:
    case Left:
        if((thisValue < this->maxValue()) || (thisValue > this->minValue())) {
           setColors(thisBackColor, AL_RED, thisTextColor, thisColorMode);
        } else {
           setColors(thisBackColor, AL_GREEN, thisTextColor, thisColorMode);
        }
        break;
    }
}

// here we adapt the scale font size and the handle size to the size of the widget
bool caThermo::event(QEvent *e)
{
    if (e->type() == QEvent::StyleChange) {

        if(!isShown) {
            QString c=  palette().color(QPalette::Base).name();
            defaultBackColor = QColor(c);
            c =  palette().color(QPalette::Text).name();
            defaultForeColor = QColor(c);

            //printf("default fore color %d %d %d valid=%d\n", defaultForeColor.red(), defaultForeColor.green(), defaultForeColor.blue(), defaultForeColor.isValid());
            //printf("default back color %d %d %d valid=%d\n", defaultBackColor.red(), defaultBackColor.green(), defaultBackColor.blue(), defaultBackColor.isValid());

            if(!defaultBackColor.isValid()) defaultBackColor = QColor(255, 248, 220, 255);
            if(!defaultForeColor.isValid()) defaultForeColor = Qt::black;

            setColors(thisBackColor, thisForeColor, thisTextColor, thisColorMode);
            isShown = true;
        }

    } else if(e->type() == QEvent::Resize || e->type() == QEvent::Show) {

        const caThermo* that = this;

        switch (thisScale) {

        case false:
        {
            int pipewidth;
            if(thisDirection == Up || thisDirection == Down) {
                pipewidth = width();
            } else {
                pipewidth = height();
            }
            if(pipewidth != this->pipeWidth()) this->setPipeWidth(pipewidth);

            QFont f = font();
            f.setPointSizeF(MAX_FONT_SIZE);
            QFontMetricsF tmpFm(f);
            float txtHeight = tmpFm.height();
            while((txtHeight > pipewidth-1.0) && f.pointSizeF() > MIN_FONT_SIZE) {
                if(f.pointSizeF() <= 0.0) f.setPointSizeF(1.0);
                f.setPointSizeF(f.pointSizeF() - 0.5);
                QFontMetricsF tmpFm(f);
                txtHeight = tmpFm.height();
            }
            float pointSize = f.pointSizeF();
            if(pointSize < MIN_FONT_SIZE) pointSize = MIN_FONT_SIZE;
            if(pointSize > MAX_FONT_SIZE) pointSize = MAX_FONT_SIZE;

            if(qAbs(pointSize - pointSizePrv) >= 0.1) {
                f.setPointSizeF(pointSize);
                pointSizePrv = pointSize;
                setFont(f);
                update();
            }
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
    if(thisScaleValueEnabled) paintValue (painter,  sliderRect);
}

// Draws the label with the value on the widget
void caThermo::paintValue(QPainter *painter, QRect valueRect) const
{
    QString label = setScaleLabel(value());
    if(orientation() == Qt::Horizontal) {
        painter->drawText( valueRect, Qt::AlignCenter, label );
    } else {
        QFontMetrics fm(font());
        int w = QMETRIC_QT456_FONT_WIDTH(fm,label);
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
            break;
        case NoScale:
            painter->rotate(270);
            painter->drawText(QPoint(-valueRect.height()/2-w/2, width() - valueRect.width() + h - fm.descent()), label);
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
            snprintf(thisFormat,SMALL_STRING_LENGTH, "%s.%dlf", "%", precision);
        } else {
            snprintf(thisFormat,SMALL_STRING_LENGTH, "%s.%dle", "%", -precision);
        }
        break;
    case compact:
        snprintf(thisFormat,SMALL_STRING_LENGTH, "%s.%dle", "%", qAbs(precision));
        snprintf(thisFormatC,SMALL_STRING_LENGTH, "%s.%dlf", "%", qAbs(precision));
        break;
    case exponential:
    case engr_notation:
        snprintf(thisFormat,SMALL_STRING_LENGTH, "%s.%dle", "%", qAbs(precision));
        break;
    case truncated:
        qstrncpy(thisFormat, "%d",SMALL_STRING_LENGTH);
        break;
    default:
        snprintf(thisFormat,SMALL_STRING_LENGTH, "%s.%dlf", "%", precision);
    }

}

// Creates the QString that will be displayed for the value label
QString caThermo::setScaleLabel(double value) const
{
    char asc[MAX_STRING_LENGTH];
    QString label;

    if(thisFormatType == compact) {
        if ((value < 1.e4 && value > 1.e-4) || (value > -1.e4 && value < -1.e-4) || value == 0.0) {
            snprintf(asc, MAX_STRING_LENGTH, thisFormatC, value);
        } else {
            snprintf(asc, MAX_STRING_LENGTH, thisFormat, value);
        }
    } else if(thisFormatType == truncated) {
        snprintf(asc, MAX_STRING_LENGTH, thisFormat, (int) value);
    } else {
        snprintf(asc, MAX_STRING_LENGTH, thisFormat, value);
    }

    if(qIsNaN(value)){
      snprintf(asc, MAX_STRING_LENGTH,  "nan");
    }
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    label = QString::fromAscii(asc);
#else
    label = QString::fromLatin1(asc);
#endif

    return label;
}

void caThermo::setScaleValueEnabled(bool b)
{
    thisScaleValueEnabled = b;
    update();
}
