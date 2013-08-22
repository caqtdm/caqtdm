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

caThermo::caThermo(QWidget *parent) : QwtThermoMarker(parent), m_externalEnabled(false)
{
    thisDirection = Up;
    defaultBackColor = QWidget::palette().background().color();
    defaultForeColor = palette().foreground().color();
    thisColorMode = Static;

    setSpacing(0);
    setBorderWidth(1);

    thisPalette = palette();

    thisLogScale = false;

    valPixOld = -999999;
    setLook(Outline);
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
    thisPalette.setColor(QPalette::Base, bg);
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
    case Channel:
        thisScale = true;
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
    case ALARM_INVALID:
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


