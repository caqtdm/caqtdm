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
#include "themecolors.h"
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

Q_LOGGING_CATEGORY(caThermoLog, "caqtdm.widgets.cathermo")

// colour set of caThermo (level 2), values come from themecolors.h
namespace caThermoColor {
    constexpr const ThemeColor &Background   = ThemeColors::Surface;
    constexpr const ThemeColor &Fill         = ThemeColors::TextPrimary;
    constexpr const ThemeColor &ScaleText    = ThemeColors::TextPrimary;
    constexpr const ThemeColor &FillDefault  = ThemeColors::ThermoFillDefault;
    constexpr const ThemeColor &PipeDefault  = ThemeColors::ThermoPipeDefault;
    constexpr const ThemeColor &FallbackBase = ThemeColors::SurfaceSunken;
    constexpr const ThemeColor &FallbackFill = ThemeColors::TextPrimary;
    constexpr const ThemeColor &Unset        = ThemeColors::White;
    constexpr const ThemeColor &AlarmNoAlarm = ThemeColors::AlarmNoAlarm;
    constexpr const ThemeColor &AlarmMinor   = ThemeColors::AlarmMinor;
    constexpr const ThemeColor &AlarmMajor   = ThemeColors::AlarmMajor;
    constexpr const ThemeColor &AlarmInvalid = ThemeColors::AlarmInvalid;
    constexpr const ThemeColor &AlarmDefault = ThemeColors::AlarmDefault;
    constexpr const ThemeColor &NotConnected = ThemeColors::AlarmInvalid;
    constexpr const ThemeColor &LimitOk      = ThemeColors::AlarmNoAlarm;
    constexpr const ThemeColor &LimitViolated= ThemeColors::AlarmMajor;
}

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
    appliedScheme = ThemeColors::Light;

    thisDirection = Up;
    thisColorMode = Static;
    thisLimitsMode = Channel;

    oldBackColor = ThemeColors::color(caThermoColor::Unset);
    oldForeColor = ThemeColors::color(caThermoColor::Unset);
    oldColorMode = Default;

    // these default could be changed by a stylesheet
    defaultForeColor = ThemeColors::color(caThermoColor::FillDefault);
    defaultBackColor = ThemeColors::color(caThermoColor::PipeDefault);
    defaultForeColorOld = defaultBackColor;
    defaultBackColorOld = defaultForeColor;

    setAutoFillBackground( true );
    setBackground(ThemeColors::color(caThermoColor::Background));
    setForeground(ThemeColors::color(caThermoColor::Fill));
    setTextColor(ThemeColors::color(caThermoColor::ScaleText));

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
    const ThemeColors::Scheme scheme = ThemeColors::schemeOf(this);

    if((bg != oldBackColor) || (fg != oldForeColor)  || (textColor != oldTextColor) || (mode != oldColorMode)
            || (defaultBackColorOld != defaultBackColor) || (defaultForeColorOld != defaultForeColor)
            || (scheme != appliedScheme)) {

        appliedScheme = scheme;
        QPalette thisPalette = palette();

        // only Default/Alarm_Default follow the panel theme; Static keeps the author colours
        const QColor themeBg = ThemeColors::themed(bg, caThermoColor::Background, scheme);
        const QColor themeText = ThemeColors::themed(textColor, caThermoColor::ScaleText, scheme);
        QColor themeDefaultBack = defaultBackColor;
        QColor themeDefaultFore = defaultForeColor;
        if(scheme != ThemeColors::Light) {
            themeDefaultBack = ThemeColors::color(caThermoColor::PipeDefault, scheme);
            themeDefaultFore = ThemeColors::color(caThermoColor::FillDefault, scheme);
        }

        if(thisColorMode == Default) {
            if(!themeDefaultBack.isValid() || !themeDefaultFore.isValid()) return;
            QColor bgs = themeDefaultBack.darker(125);
            bgs.setAlpha(bg.alpha());
            qCDebug(caThermoLog) << "default palette fg" << themeDefaultFore.red() << themeDefaultFore.green() << themeDefaultFore.blue() << objectName();
            qCDebug(caThermoLog) << "default palette bg" << themeDefaultBack.red() << themeDefaultBack.green() << themeDefaultBack.blue() << objectName();
            thisPalette.setColor(QPalette::ButtonText, themeDefaultFore);
            thisPalette.setColor(QPalette::Text, themeText);
            thisPalette.setColor(QPalette::WindowText, themeText);
            thisPalette.setColor(QPalette::Window, themeBg);
            thisPalette.setColor(QPalette::Base, bgs);
            setPalette(thisPalette);

        } else if(thisColorMode == Static) {
            QColor bgs = bg.darker(125);
            bgs.setAlpha(bg.alpha());
            qCDebug(caThermoLog) << "static set palette fg" << fg.red() << fg.green() << fg.blue();
            qCDebug(caThermoLog) << "static set palette bg" << bgs.red() << bgs.green() << bgs.blue();
            qCDebug(caThermoLog) << "static set palette txt" << textColor.red() << textColor.green() << textColor.blue();
            thisPalette.setColor(QPalette::ButtonText, fg);
            thisPalette.setColor(QPalette::Text, textColor);
            thisPalette.setColor(QPalette::WindowText, textColor);
            thisPalette.setColor(QPalette::Window, bg);
            thisPalette.setColor(QPalette::Base, bgs);
            setPalette(thisPalette);

        } else if(thisColorMode == Alarm_Static) {
            QColor bgs = bg.darker(125);
            bgs.setAlpha(bg.alpha());
            qCDebug(caThermoLog) << "alarm_static set palette" << fg.red() << fg.green() << fg.blue();
            thisPalette.setColor(QPalette::ButtonText, fg);
            thisPalette.setColor(QPalette::Text, textColor);
            thisPalette.setColor(QPalette::WindowText, textColor);
            thisPalette.setColor(QPalette::Window, bg);
            thisPalette.setColor(QPalette::Base, bgs);
            setPalette(thisPalette);

        } else  if(thisColorMode == Alarm_Default) {
            if(!themeDefaultBack.isValid() || !themeDefaultFore.isValid()) return;
            QColor bgs = themeDefaultBack.darker(125);
            bgs.setAlpha(bg.alpha());
            qCDebug(caThermoLog) << "alarm_default palette" << fg.red() << fg.green() << fg.blue();
            thisPalette.setColor(QPalette::ButtonText, fg);
            thisPalette.setColor(QPalette::Text, themeText);
            thisPalette.setColor(QPalette::WindowText, themeText);
            thisPalette.setColor(QPalette::Window, themeBg);
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

    ScalePosition scalepos = this->scalePosition();

    thisDirection = dir;

    switch (dir) {
    case Up:
    case Down:
        if(thisScale) {
            setScalePosition(LeftScale);
            setOrientation(Qt::Vertical);
        } else {

            setScalePosition(NoScale);
            setOrientation(Qt::Vertical);
        }
        break;
    case Left:
    case Right:
        if(thisScale) {
            setScalePosition(BottomScale);
            setOrientation(Qt::Horizontal);
        } else {
            setScalePosition(NoScale);
            setOrientation(Qt::Horizontal);
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
        c = ThemeColors::color(caThermoColor::AlarmNoAlarm);
        break;
    case MINOR_ALARM:
        c = ThemeColors::color(caThermoColor::AlarmMinor);
        break;
    case MAJOR_ALARM:
        c = ThemeColors::color(caThermoColor::AlarmMajor);
        break;
    case INVALID_ALARM:
        c = ThemeColors::color(caThermoColor::AlarmInvalid);
        break;
    case NOTCONNECTED:
        c = ThemeColors::color(caThermoColor::NotConnected);
        break;
    default:
        c = ThemeColors::color(caThermoColor::AlarmDefault);
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

    qCDebug(caThermoLog) << val << this->minValue() << this->maxValue() << thisDirection;
    switch (thisDirection) {
    case Up:
    case Right:
        if((thisValue < this->minValue()) || (thisValue > this->maxValue())) {
           setColors(thisBackColor, ThemeColors::color(caThermoColor::LimitViolated), thisTextColor, thisColorMode);
        } else {
           setColors(thisBackColor, ThemeColors::color(caThermoColor::LimitOk), thisTextColor, thisColorMode);
        }
        break;
    case Down:
    case Left:
        if((thisValue < this->maxValue()) || (thisValue > this->minValue())) {
           setColors(thisBackColor, ThemeColors::color(caThermoColor::LimitViolated), thisTextColor, thisColorMode);
        } else {
           setColors(thisBackColor, ThemeColors::color(caThermoColor::LimitOk), thisTextColor, thisColorMode);
        }
        break;
    }
}

// follow the panel theme when the scheme of the panel root changes
void caThermo::changeEvent(QEvent *e)
{
    QwtThermoMarker::changeEvent(e);
    if(e->type() == QEvent::PaletteChange || e->type() == QEvent::StyleChange) {
        if(ThemeColors::schemeOf(this) != appliedScheme) {
            setColors(thisBackColor, thisForeColor, thisTextColor, thisColorMode);
        }
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

            qCDebug(caThermoLog) << "default fore color" << defaultForeColor.red() << defaultForeColor.green() << defaultForeColor.blue() << "valid=" << defaultForeColor.isValid();
            qCDebug(caThermoLog) << "default back color" << defaultBackColor.red() << defaultBackColor.green() << defaultBackColor.blue() << "valid=" << defaultBackColor.isValid();

            if(!defaultBackColor.isValid()) defaultBackColor = ThemeColors::color(caThermoColor::FallbackBase);
            if(!defaultForeColor.isValid()) defaultForeColor = ThemeColors::color(caThermoColor::FallbackFill);

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
