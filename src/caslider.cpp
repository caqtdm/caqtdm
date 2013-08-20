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

#if defined(_MSC_VER)
#include <windows.h>
#define QWT_DLL
#endif


#include "caslider.h"
#include <QtDebug>
#include <QEvent>
#include <QApplication>
#include <math.h>
#include "alarmdefs.h"

#if defined(_MSC_VER)
#define fmax max
#define fmin min
#endif


caSlider::caSlider(QWidget *parent) : QwtSlider(parent)
{
    setScalePosition(NoScale);
    setSpacing(0);
    setBorderWidth(1);

    thisDirection = Up;
    thisMinimum = -50;
    thisMaximum = 50;

    setAccessW(true);
    setDirection(Up);

    installEventFilter(this);

    setHandleSize(QSize(10,20));

    thisColorMode = Static;
    thisLimitsMode = Channel;
    defaultBackColor = palette().background().color();
    defaultForeColor = palette().foreground().color();
    oldBackColor = QColor(Qt::white);
    oldForeColor = QColor(Qt::white);
}

QString caSlider::getPV() const
{
    return thisPV;
}

void caSlider::setPV(QString const &newPV)
{
    thisPV = newPV;
}

void caSlider::setBackground(QColor c)
{
    QColor color = c;
    if(thisColorMode == Default) {
        thisBackColor = defaultBackColor;
    } else {
      thisBackColor = color;
    }
    setColors(thisBackColor, thisForeColor);
}

QColor caSlider::getForeground() {
    if(thisColorMode == Default) {
        return defaultForeColor;
    } else {
      return thisForeColor;
    }
}

QColor caSlider::getBackground() {
    if(thisColorMode == Default) {
        return defaultBackColor;
    } else {
      return thisBackColor;
    }
}

void caSlider::setForeground(QColor c)
{
    QColor color = c;
    if(thisColorMode == Default) {
       thisForeColor= defaultForeColor;
    } else {
      thisForeColor = color;
    }
    setColors(thisBackColor, thisForeColor);
}

void caSlider::setColors(QColor bg, QColor fg)
{
    if((oldBackColor == bg) && (oldForeColor == fg)) return;
    QPalette thisPalette = palette();
    thisPalette.setColor(QPalette::Mid, bg);
    thisPalette.setColor(QPalette::Text, thisForeColor);
    thisPalette.setColor(QPalette::Button, fg);
    setPalette(thisPalette);
    oldBackColor = bg;
    oldForeColor = fg;
}

void caSlider::setMaxValue(double const &maxim){
    thisMaximum = maxim;
    setDirection(thisDirection);
    update();
}

void caSlider::setMinValue(double const &minim){
    thisMinimum = minim;
    setDirection(thisDirection);
    update();
}

void caSlider::setSliderValue(double const &value){
    thisValue = value;
    setValue(value);
    update();
}

void caSlider::setAccessW(int access)
{
     thisAccessW = access;
}

void caSlider::setDirection(Direction dir)
{
#if QWT_VERSION < 0x060100
    ScalePos scalepos = this->scalePosition();
#else
    ScalePosition scalepos = this->scalePosition();
#endif
    thisDirection = dir;

    switch (dir) {
    case Up:
#if QWT_VERSION < 0x060100
        setScalePosition(QwtSlider::LeftScale);
#else
        setScalePosition(QwtSlider::TrailingScale);
#endif
        setOrientation(Qt::Vertical);
#if QWT_VERSION < 0x060100
        setRange(thisMinimum, thisMaximum, fmin(1.0,(fabs(thisMaximum- thisMinimum) / 100.0)), 1);
#endif
        break;
    case Down:
#if QWT_VERSION < 0x060100
        setScalePosition(QwtSlider::LeftScale);
#else
        setScalePosition(QwtSlider::LeadingScale);
#endif
        setOrientation(Qt::Vertical);
#if QWT_VERSION < 0x060100
        if(thisMaximum > thisMinimum) setRange(thisMaximum, thisMinimum ,  fmin(1.0,(fabs(thisMaximum- thisMinimum) / 100.0)), 1);
        else setRange(thisMinimum, thisMaximum , fmin(1.0,(fabs(thisMaximum- thisMinimum) / 100.0)), 1);
#endif
        break;
    case Left:

#if QWT_VERSION < 0x060100
        setScalePosition(QwtSlider::BottomScale);
#else
        setScalePosition(QwtSlider::TrailingScale);
#endif
        setOrientation(Qt::Horizontal);
 #if QWT_VERSION < 0x060100
        if(thisMaximum > thisMinimum) setRange(thisMaximum, thisMinimum , fmin(1.0,(fabs(thisMaximum- thisMinimum) / 100.0)), 1);
        else setRange(thisMinimum, thisMaximum , fmin(1.0,(fabs(thisMaximum- thisMinimum) / 100.0)), 1);
#endif
        break;
    case Right:
 #if QWT_VERSION < 0x060100
        setScalePosition(QwtSlider::BottomScale);
#else
        setScalePosition(QwtSlider::LeadingScale);
#endif
        setOrientation(Qt::Horizontal);
#if QWT_VERSION < 0x060100
        setRange(thisMinimum, thisMaximum, fmin(1.0,(fabs(thisMaximum- thisMinimum) / 100.0)), 1);
#endif
        break;
    }
    setScalePosition(scalepos);
    update();
}

bool caSlider::eventFilter(QObject *obj, QEvent *event)
{
        if (event->type() == QEvent::Enter) {
            if(!thisAccessW) {
                QApplication::setOverrideCursor(QCursor(Qt::ForbiddenCursor));
                setReadOnly(true);
            } else {
                QApplication::restoreOverrideCursor();
            }
        } else if(event->type() == QEvent::Leave) {
            QApplication::restoreOverrideCursor();
            setReadOnly(false);
            clearFocus();
        }
        return QObject::eventFilter(obj, event);
}

void caSlider::setAlarmColors(short status)
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
    setColors(c, thisForeColor);
}

void caSlider::setUserAlarmColors(double val)
{
//     if((val< this->minValue()) || (val > this->maxValue())) {
   if((val< thisMinimum) || (val > thisMaximum)) {
        setColors(thisBackColor, QColor(Qt::red));
     } else {
        setColors(thisBackColor, QColor(Qt::green));
     }
}

void caSlider::setNormalColors()
{
    setColors(thisBackColor, thisForeColor);
}

#include "moc_caslider.cpp"

