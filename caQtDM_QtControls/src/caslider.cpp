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

#define MIN_FONT_SIZE 3
#define MAX_FONT_SIZE 20

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
    pointSizePrv = 0.0;

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

    setBackground(QColor(224,224,224));
    setForeground(Qt::black);

    setDirection(Up);
    setAccessW(true);

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

    QColor bgs = bg.darker(125);
    bgs.setAlpha(255);

    QString style = "caSlider {background-color: rgba(%1, %2, %3, %4); } caSlider:focus  {border: 1px solid #f00;}; ";
    style = style.arg(bg.red()).arg(bg.green()).arg(bg.blue()).arg(bg.alpha());
    setStyleSheet(style);

    thisPalette.setColor(QPalette::Mid, bgs);
    thisPalette.setColor(QPalette::Text, thisForeColor);
    thisPalette.setColor(QPalette::Button, bg);
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

#if QWT_VERSION >= 0x060100
    incrementValue(thisIncrement);
    setPageSteps(1);
#endif

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
#else
        setScale(thisMinimum, thisMaximum);
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
#else
        if(thisMaximum > thisMinimum) setScale(thisMaximum, thisMinimum);
        else setScale(thisMinimum, thisMaximum);
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
#else
        if(thisMaximum > thisMinimum) setScale(thisMaximum, thisMinimum);
        else setScale(thisMinimum, thisMaximum);
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
#else
        setScale(thisMinimum, thisMaximum);
#endif
        break;
    }
    setScalePosition(scalepos);
    update();
}

void caSlider::keyPressEvent(QKeyEvent *e) {
    int increment = 0;
    bool doIt = false;

    if(isReadOnly()) return;
    if ( !isValid()) return;

    switch (e->key()) {
    case Qt::Key_Down:

        if(orientation() == Qt::Vertical) {
            if(e->modifiers() & Qt::ControlModifier) increment = -10; else increment = -1;
        }
        doIt = true;
        break;

    case Qt::Key_Up:

        if(orientation() == Qt::Vertical) {
            if(e->modifiers() & Qt::ControlModifier) increment = 10; else increment = 1;

        }
        doIt = true;
        break;

    case Qt::Key_Left:

        if(orientation() == Qt::Horizontal) {
            if(e->modifiers() & Qt::ControlModifier) increment = -10; else increment = -1;
        }
        doIt = true;
        break;

    case Qt::Key_Right:

        if(orientation() == Qt::Horizontal) {
            if(e->modifiers() & Qt::ControlModifier) increment = 10; else increment = 1;
        }
        doIt = true;
        break;

    }

    if(doIt) {
#if QWT_VERSION >= 0x060100
        incrementValue(increment);
        Q_EMIT sliderMoved( value() );
        Q_EMIT valueChanged( value() );
#else
        QwtDoubleRange::setValue(value() + increment * step());
        //QwtDoubleRange::incValue( increment );  // this rounds to step value
#endif
    } else {
        e->ignore();
    }

}

void caSlider::keyReleaseEvent(QKeyEvent *e) {
     e->ignore();
     return;
}

void caSlider::mousePressEvent(QMouseEvent *e)
{

#if QT_VERSION< QT_VERSION_CHECK(4, 8, 0)
    if( e->button() == Qt::RightButton || e->button() == Qt::MidButton) {
#else

    if( e->button() == Qt::RightButton || e->button() == Qt::MiddleButton) {
#endif
        e->ignore();
        return;
    }
    else {
        QwtSlider::mousePressEvent(e);
    }
}

void caSlider::mouseReleaseEvent( QMouseEvent *e )
{
    if( e->button() == Qt::LeftButton) {
#if QWT_VERSION < 0x060100
        stopMoving();
        QwtSlider::mouseReleaseEvent(e);
#else
        QwtSlider::mouseReleaseEvent(e);
#endif
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

// here we adapt the scale font size and the handle size to the size of the widget
bool caSlider::event(QEvent *e)
{
    if(e->type() == QEvent::Resize || e->type() == QEvent::Show || e->type() == QEvent::Paint) {

        const caSlider* that = this;

        switch (this->scalePosition()) {

        case NoScale:
            switch (orientation()) {
            case Qt::Vertical: {
                QSize handlesize = QSize(width()-4, height()/10);
                if(handlesize != this->handleSize()) {
                    this->setHandleSize(handlesize);
                }
                break;
            }

            case Qt::Horizontal: {
                QSize handlesize = QSize(width()/10, height() -4);
                if(handlesize != this->handleSize()) {
                    this->setHandleSize(handlesize);
                }
                break;
            }
            default:
                break;
            }
            break;
#if QWT_VERSION < 0x060100
        case  LeftScale:
        case  RightScale:
        case  TopScale:
        case  BottomScale:
#else
        case LeadingScale:
        case TrailingScale:
#endif
            switch (orientation()) {

                case Qt::Vertical: {
                    QSize handlesize = QSize(width()/3-4, height()/10);
                    if(handlesize != this->handleSize()) {
                        this->setHandleSize(handlesize);
                    }
                    QFont f = font();
                    int size = that->scaleDraw()->maxLabelWidth(f);
                    float xFactor = (float) size  / ((float) width() * 2.0/3.0 - 15.0);

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

                case Qt::Horizontal: {
                   QSize handlesize = QSize(width()/10, height()/3-4);
                   if(handlesize != this->handleSize()) {
                       this->setHandleSize(handlesize);
                   }
                   QFont f = font();
                   int size = that->scaleDraw()->maxLabelWidth(f);
                   float yFactor = (float) size  / ((float) height()*2.0/3.0 -10.0);

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
    return QwtSlider::event(e);
}


#include "moc_caslider.cpp"

