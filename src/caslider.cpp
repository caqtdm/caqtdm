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
#include <windows.h>
#define QWT_DLL
#endif


#include "caslider.h"
#include <QtDebug>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include <math.h>
#include "alarmdefs.h"

#if defined(_MSC_VER)
#define fmax max
#define fmin min
#endif


caSlider::caSlider(QWidget *parent) : QwtSlider(parent)
{
    thisIncrement = 1.0;
    thisDirection = Up;
    thisMinimum = -50;
    thisMaximum = 50;

    setScalePosition(NoScale);
    setSpacing(0);
    setBorderWidth(1);

    installEventFilter(this);

    setHandleSize(QSize(10,20));

    thisColorMode = Static;
    thisLimitsMode = Channel;
    defaultBackColor = palette().background().color();
    defaultForeColor = palette().foreground().color();
    oldBackColor = QColor(Qt::white);
    oldForeColor = QColor(Qt::white);

    setDirection(Up);
    setAccessW(true);

    QString style;
    style.append("  caSlider:focus  {border: 1px solid #f00;} ");
    setStyleSheet(style);

    setFocusPolicy(Qt::ClickFocus);

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

void caSlider::setIncrementValue(double const &value){
    thisIncrement = value;
    setDirection(thisDirection);
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
        setRange(thisMinimum, thisMaximum, thisIncrement, 1);
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
        if(thisMaximum > thisMinimum) setRange(thisMaximum, thisMinimum ,  thisIncrement, 1);
        else setRange(thisMinimum, thisMaximum , thisIncrement, 1);
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
        if(thisMaximum > thisMinimum) setRange(thisMaximum, thisMinimum , thisIncrement, 1);
        else setRange(thisMinimum, thisMaximum , thisIncrement, 1);
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
        setRange(thisMinimum, thisMaximum, thisIncrement, 1);
#endif
        break;
    }
    setScalePosition(scalepos);
    update();
}

void caSlider::keyPressEvent(QKeyEvent *e) {
      int increment;
    // keys supported by QwtAbstractSlider
    switch (e->key()) {
       case Qt::Key_Down:

            if(orientation() == Qt::Vertical) {
                if(e->modifiers() & Qt::ControlModifier) increment = -10; else increment = -1;
                QwtDoubleRange::incValue( increment );
                if(value() != prevValue()) Q_EMIT sliderMoved( value() );
            }
       break;

       case Qt::Key_Up:

        if(orientation() == Qt::Vertical) {
            if(e->modifiers() & Qt::ControlModifier) increment = 10; else increment = 1;
            QwtDoubleRange::incValue( increment );
            if(value() != prevValue()) Q_EMIT sliderMoved( value() );
        }
        break;

       case Qt::Key_Left:

        if(orientation() == Qt::Horizontal) {
            if(e->modifiers() & Qt::ControlModifier) increment = -10; else increment = -1;
            QwtDoubleRange::incValue( increment );
            if(value() != prevValue()) Q_EMIT sliderMoved( value() );
        }
        break;

       case Qt::Key_Right:

        if(orientation() == Qt::Horizontal) {
            if(e->modifiers() & Qt::ControlModifier) increment = 10; else increment = 1;
            QwtDoubleRange::incValue( increment );
            if(value() != prevValue()) Q_EMIT sliderMoved( value() );
        }
        break;

    }

}

void caSlider::keyReleaseEvent(QKeyEvent *e) {
     e->ignore();
     return;
}

void caSlider::mousePressEvent(QMouseEvent *e)
{
    if( e->button() == Qt::RightButton) {
        e->ignore();
        return;
    }
    else {
        e->accept();
        QwtAbstractSlider::mousePressEvent(e);
    }
}

void caSlider::mouseReleaseEvent( QMouseEvent *e )
{
    if( e->button() == Qt::LeftButton) {
        e->accept();
        stopMoving();
        QwtAbstractSlider::mouseReleaseEvent(e);
    }
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

