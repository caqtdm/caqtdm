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

#include "caclock.h"
#include "themecolors.h"
#include <qwt_dial_needle.h>

// colour set of caClock (level 2), values come from themecolors.h
namespace caClockColor {
    constexpr const ThemeColor &Base         = ThemeColors::ClockBase;        // Qt::gray
    constexpr const ThemeColor &ScaleText    = ThemeColors::ClockScaleText;
    constexpr const ThemeColor &SecondHand   = ThemeColors::AlarmMajor;       // Qt::red
    constexpr const ThemeColor &AlarmNoAlarm = ThemeColors::AlarmNoAlarm;
    constexpr const ThemeColor &AlarmMinor   = ThemeColors::AlarmMinor;
    constexpr const ThemeColor &AlarmMajor   = ThemeColors::AlarmMajor;
    constexpr const ThemeColor &AlarmInvalid = ThemeColors::AlarmInvalid;
    constexpr const ThemeColor &NotConnected = ThemeColors::AlarmInvalid;
}

#if defined(_MSC_VER)
#define fmax max
#define fmin min
#endif

caClock::caClock(QWidget *parent) : QwtAnalogClock(parent)
{
    setReadOnly(true);
    setFocusPolicy(Qt::NoFocus);

    thisUpdateRate = Normal;
    thisColorMode = Default;
    appliedScheme = ThemeColors::Light;
    lastStatus = NO_ALARM;
    prevScaleDefaultColor = thisScaleDefaultColor = true;
    thisBaseColor = ThemeColors::color(caClockColor::Base);
    thisScaleColor = prevScaleColor = thisBaseColor.darker( 200 ).lighter( 800 );
    setPalette( colorTheme( effectiveBase().darker( 150 ) ) );


#if QWT_VERSION < QT_VERSION_CHECK(6, 2, 0)
    scaleDraw()->setPenWidth(3);
#else
    scaleDraw()->setPenWidthF(3);
#endif

    setLineWidth(3);
    setFrameShadow(QwtDial::Sunken);

    scaleDraw()->setTickLength( QwtScaleDiv::MajorTick, 6);
    scaleDraw()->setSpacing(3.0);

    setHands(effectiveBase());

    // update the clock every second
    timer = new QTimer(this);
    timer->connect(timer, SIGNAL(timeout()), this, SLOT(myCurrentTime()));
    connect(this, SIGNAL(updateTime(QTime)), this, SLOT(setClockTime(QTime)));

    setTimeType (InternalTime);
    setColorMode(Default);
}

// hands and knob are derived from the dial base
void caClock::setHands(const QColor &base)
{
    handBase = base;
    const QColor knobColor = base.lighter(130);
    for ( int i = 0; i < QwtAnalogClock::NHands; i++) {
        QColor handColor = base.lighter(150);
        int width = 5;
        if ( i == QwtAnalogClock::SecondHand ) {
            handColor = ThemeColors::color(caClockColor::SecondHand);
            width = 3;
        }
        QwtDialSimpleNeedle *hand = new QwtDialSimpleNeedle(QwtDialSimpleNeedle::Arrow, true, handColor, knobColor);
        hand->setWidth(width);
        setHand((QwtAnalogClock::Hand)i, hand);
    }
}

QColor caClock::effectiveBase() const
{
    if(thisColorMode == Default) return ThemeColors::themed(thisBaseColor, caClockColor::Base, ThemeColors::schemeOf(this));
    return thisBaseColor;
}

// the panel palette changed (theme switch): rebuild dial and hands for the new scheme
void caClock::changeEvent(QEvent *e)
{
    QwtAnalogClock::changeEvent(e);
    if(e->type() != QEvent::PaletteChange && e->type() != QEvent::StyleChange) return;
    const ThemeColors::Scheme scheme = ThemeColors::schemeOf(this);
    if(scheme == appliedScheme) return;
    appliedScheme = scheme;
    setAlarmColors(lastStatus, true);
}

QPalette caClock::colorTheme( const QColor &base ) const
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
           palette.setColor( QPalette::Text, ThemeColors::color(caClockColor::ScaleText, ThemeColors::Dark) );
       else
           palette.setColor( QPalette::Text, base.darker( 200 ).lighter( 800 ) );
    } else {
       palette.setColor( QPalette::Text, thisScaleColor );
    }
    palette.setColor( QPalette::WindowText, base.darker( 200 ) );

    return palette;
}

void caClock::setClockTime(QTime time) {
    setTime(time);
}

void caClock::updateClock(QTime time) {
    if(time == prevTime) return;
    prevTime = time;
    emit updateTime(time);
}

void caClock::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e);
    int Width = width();
    QFont f = font();
#ifdef MOBILE_IOS
    f.setPointSizeF(Width/12.0);
#else
    f.setPointSizeF(Width/20.0);
#endif
    setFont(f);
    update();
}

void caClock::setAlarmColors(short status, bool force)
{
    lastStatus = status;
    const QColor base = effectiveBase();
    const bool ownColour = (thisColorMode != Alarm);       // Static and Default ignore the alarm
    QColor c;
    switch (status) {

    case NO_ALARM:
        c = ThemeColors::color(caClockColor::AlarmNoAlarm);
        if(ownColour) c = base.darker( 150 );
        break;
    case MINOR_ALARM:
        c = ThemeColors::color(caClockColor::AlarmMinor);
        if(ownColour) c = base.darker( 150 );
        break;
    case MAJOR_ALARM:
        c = ThemeColors::color(caClockColor::AlarmMajor);
        if(ownColour) c = base.darker( 150 );
        break;
    case INVALID_ALARM:
        c = ThemeColors::color(caClockColor::AlarmInvalid);
        if(ownColour) c = base.darker( 150 );
        break;
    case NOTCONNECTED:
        c = ThemeColors::color(caClockColor::NotConnected);
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
    if(base != handBase) setHands(base);
}
//#ifndef MOBILE
#include "moc_caclock.cpp"
//#endif
