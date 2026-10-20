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

#if defined(_MSC_VER)
#define NOMINMAX
#include <windows.h>
#define QWT_DLL
#endif

#include "cameter.h"

// colour set of caMeter (level 2), values come from themecolors.h
namespace caMeterColor {
    constexpr const ThemeColor &AlarmNoAlarm = ThemeColors::AlarmNoAlarm;
    constexpr const ThemeColor &AlarmMinor   = ThemeColors::AlarmMinor;
    constexpr const ThemeColor &AlarmMajor   = ThemeColors::AlarmMajor;
    constexpr const ThemeColor &AlarmInvalid = ThemeColors::AlarmInvalid;
    constexpr const ThemeColor &NotConnected = ThemeColors::AlarmInvalid;
    constexpr const ThemeColor &Base      = ThemeColors::ClockBase;        // Qt::gray
    constexpr const ThemeColor &ScaleText = ThemeColors::ClockScaleText;
    constexpr const ThemeColor &Needle    = ThemeColors::AlarmMajor;       // Qt::red
}

#if defined(_MSC_VER)
 #define fmax max
 #define fmin min
 #ifndef snprintf
  #define snprintf _snprintf
 #endif
#endif

#include <qnumeric.h>
#include <qwt_text.h>

class myDialScaleDraw: public QwtRoundScaleDraw
{
public:
    explicit myDialScaleDraw(caMeter *w) {
        thisParent = w;
    }

    virtual QwtText label(double v) const
    {
        return QwtText(thisParent->setLabel(v,""));
    }

private:
    caMeter *thisParent;
};

caMeter::~caMeter() {
}

caMeter::caMeter(QWidget *parent) : QwtDial(parent)
{

    thisColorMode = Default;
    appliedScheme = ThemeColors::Light;
    lastStatus = NO_ALARM;
    prevScaleDefaultColor = thisScaleDefaultColor = true;
    thisBaseColor = ThemeColors::color(caMeterColor::Base);
    thisScaleColor = prevScaleColor = thisBaseColor.darker( 200 ).lighter( 800 );

    myDialScaleDraw *ScaleDraw = new myDialScaleDraw(this);
    setScaleDraw(ScaleDraw);

    setWrapping(false);
    setReadOnly(true);
    setFocusPolicy(Qt::NoFocus);
    setPalette( colorTheme(effectiveBase().darker( 150 ) ) );

    setOrigin(135.0);
    setScaleArc(0.0, 270.0);
    thisLabel = "V";
	thisMinValue = 0;
	thisMaxValue = 300;

    setScale( thisMinValue, thisMaxValue);
    setScaleStepSize((thisMinValue - thisMaxValue)/10.0);

#if QWT_VERSION < 0x060200
    ScaleDraw->setPenWidth(1);
#else
    ScaleDraw->setPenWidthF(1);
#endif
    setLineWidth(1);
    setFrameShadow(QwtDial::Sunken);

    ScaleDraw->setTickLength( QwtScaleDiv::MajorTick, 6);
    ScaleDraw->setSpacing(3.0);

    setNeedleColors(effectiveBase());

    setColorMode(Default);
    setScaleEnabled(true);
    setValueDisplayed(true);
    setUnitsEnabled(true);
    setPrecision(0);
    setPrecisionMode(Channel);
    setFormatType(decimal);
    setLimitsMode(Channel);
}

void caMeter::setMinValue(double v) {
    thisMinValue = v;
    setScale( thisMinValue, thisMaxValue);
    setScaleStepSize((thisMinValue - thisMaxValue)/10.0);
}

void caMeter::setMaxValue(double v) {
    thisMaxValue = v;
    setScale( thisMinValue, thisMaxValue);
     setScaleStepSize((thisMinValue - thisMaxValue)/10.0);
    update();
}

void caMeter::drawScaleContents(QPainter *painter, const QPointF &center, double radius) const
{
    Q_UNUSED(center);
    Q_UNUSED(radius);
    QRectF rect( 2./10.*width(), 8.5/10.0*height(), 6.0/10.0*width(), 1.0/10.0*height());

    const QColor color = palette().color(QPalette::Text);
    painter->setPen(color);

    const int flags = Qt::AlignBottom | Qt::AlignHCenter;
    QFont f;

    f.setPointSizeF(80.0);
    painter->setFont(f);
    QRect fontBoundRect = painter->fontMetrics().boundingRect(rect.toRect(), flags, thisLabel);
    while(rect.width() < fontBoundRect.width() || rect.height() < fontBoundRect.height()) {
        if(f.pointSizeF() < 3) break;
        f.setPointSizeF(f.pointSizeF()*0.95);
        painter->setFont(f);
        fontBoundRect = painter->fontMetrics().boundingRect(rect.toRect(), flags, thisLabel);
    }

    CorrectFontIfAndroid(f);

    painter->setBrush(QColor(0,0,0,0));
    painter->setPen(QColor(0,0,0,0));
    painter->drawRect(rect);

    painter->setPen(color);                  // the value follows the scale text colour
    if(thisValueDisplayed) {
       painter->drawText(fontBoundRect, Qt::AlignHCenter , thisLabel);
    }
}

void caMeter::setFormat(int prec)
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
        qstrncpy(thisFormat, "%d",SMALL_STRING_LENGTH);
        break;
    default:
        sprintf(thisFormat, "%s.%dlf", "%", precision);
    }
}

QString caMeter::setLabel(double value, const QString& units)
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

    if(thisUnitMode) {
        strcat(asc, " ");
        strcat(asc, units.toUtf8().constData());
    }

    label = QString::fromLatin1(asc);

    return label;
}

// something that has to be thought over
QString caMeter::setScaleLabel(double value)
{
    char asc[MAX_STRING_LENGTH];
    QString label;

    if(!thisScaleEnabled) return "";

    if(thisFormatType == compact) {
      if ((value < 1.e4 && value > 1.e-4) || (value > -1.e4 && value < -1.e-4) || value == 0.0) {
        snprintf(asc, MAX_STRING_LENGTH,thisFormatC, value);
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

    label = QString::fromLatin1(asc);

    return label;
}

void caMeter::updateMeter()
{
    setValueUnits(lastValue, lastUnits);
}

void caMeter::setValueUnits(double value, const QString &units)
{
    if(lastValue == value && lastUnits == units) return;
    setValue(value);
    thisLabel = setLabel(value, units);
    lastValue = value;
    lastUnits = units;
    invalidate();
}

QPalette caMeter::colorTheme( const QColor &base ) const
{
    QPalette palette;
    palette.setColor( QPalette::Base, base );
    palette.setColor( QPalette::Window, base.darker( 150 ) );
    palette.setColor( QPalette::Mid, base.darker( 110 ) );
    palette.setColor( QPalette::Light, base.lighter( 170 ) );
    palette.setColor( QPalette::Dark, base.darker( 170 ) );
    if(thisScaleDefaultColor) {
       // light: today's derived value; dark Default dial: the theme's digit colour
       if(thisColorMode == Default && ThemeColors::schemeOf(this) == ThemeColors::Dark)
           palette.setColor( QPalette::Text, ThemeColors::color(caMeterColor::ScaleText, ThemeColors::Dark) );
       else
           palette.setColor( QPalette::Text, base.darker( 200 ).lighter( 800 ) );
    } else {
       palette.setColor( QPalette::Text, thisScaleColor );
    }
    palette.setColor( QPalette::WindowText, base.darker( 200 ) );
    return palette;
}

QColor caMeter::effectiveBase() const
{
    if(thisColorMode == Default) return ThemeColors::themed(thisBaseColor, caMeterColor::Base, ThemeColors::schemeOf(this));
    return thisBaseColor;
}

// needle and knob are derived from the dial base
void caMeter::setNeedleColors(const QColor &base)
{
    needleBase = base;
    setNeedle(new QwtDialSimpleNeedle( QwtDialSimpleNeedle::Arrow, true, ThemeColors::color(caMeterColor::Needle), base.lighter(130)));
}

// the panel palette changed (theme switch): rebuild dial and needle for the new scheme
void caMeter::changeEvent(QEvent *e)
{
    QwtDial::changeEvent(e);
    if(e->type() != QEvent::PaletteChange && e->type() != QEvent::StyleChange) return;
    const ThemeColors::Scheme scheme = ThemeColors::schemeOf(this);
    if(scheme == appliedScheme) return;
    appliedScheme = scheme;
    setAlarmColors(lastStatus, true);
}

void caMeter::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e);

    int Width = width();
    QFont f = font();
    float size = (float) (Width/25.0);
    if(size < 3.0) size = 3.0;
    f.setPointSizeF(size);

    CorrectFontIfAndroid(f);

    setFont(f);
    invalidate();
}

void caMeter::invalidate()
{
    invalidateCache();
    setScale(getMinValue()-1, getMaxValue()+1);
    setScale(getMinValue(), getMaxValue());
    update();
}

void caMeter::setNormalColors()
{
    setAlarmColors(NO_ALARM);
}

void caMeter::setAlarmColors(short status, bool force)
{
    lastStatus = status;
    const QColor base = effectiveBase();
    const bool ownColour = (thisColorMode != Alarm);       // Static and Default ignore the alarm
    QColor c;

    switch (status) {

    case NO_ALARM:
        c = ThemeColors::color(caMeterColor::AlarmNoAlarm);
        if(ownColour) c = base.darker( 150 );
        break;
    case MINOR_ALARM:
        c = ThemeColors::color(caMeterColor::AlarmMinor);
        if(ownColour) c = base.darker( 150 );
        break;
    case MAJOR_ALARM:
        c = ThemeColors::color(caMeterColor::AlarmMajor);
        if(ownColour) c = base.darker( 150 );
        break;
    case INVALID_ALARM:
        c = ThemeColors::color(caMeterColor::AlarmInvalid);
        if(ownColour) c = base.darker( 150 );
        break;
    case NOTCONNECTED:
        c = ThemeColors::color(caMeterColor::NotConnected);
        break;
    default:
        c = base.darker( 150 );
        break;
    }

    if((prevColor != c) || (thisScaleDefaultColor != prevScaleDefaultColor) || (thisScaleColor != prevScaleColor) || force){
      prevColor = c;
      prevScaleDefaultColor = thisScaleDefaultColor;
      prevScaleColor = thisScaleColor;
      setPalette( colorTheme( c) );
    }
    if(base != needleBase) setNeedleColors(base);
}
//#ifndef MOBILE
#include "moc_cameter.cpp"
//#endif
