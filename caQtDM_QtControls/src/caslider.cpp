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
#include <QEvent>
#include <QDebug>
#include <QMouseEvent>
#include <QApplication>
#include <QPainter>
#include <qnumeric.h>
#include "qwt_scale_draw.h"
#include "alarmdefs.h"

#if defined(_MSC_VER)
 #define fmax max
 #define fmin min
 #ifndef snprintf
  #define snprintf _snprintf
 #endif
#endif

// I need to overload the scaleengine of qwt in order to get the upper and lower scale ticks drawn
class mySliderScaleEngine: public QwtLinearScaleEngine
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

            if ( ticks.last() > sd.upperBound() || ticks.first() < sd.lowerBound()){
                if(ticks.last() > sd.upperBound()) ticks.append(sd.upperBound());
                if(ticks.first() < sd.lowerBound()) ticks.prepend(sd.lowerBound());
                sd.setTicks( QwtScaleDiv::MajorTick, ticks );
            }
        }
        return sd;
    }
};

//Overloaded to have the scale only draw the upper and lower values
class mySliderScaleDraw: public QwtScaleDraw {

public:
    double left;
    double right;
    bool scaleValueEnabled;

    void setLeft(double val) {
        left = val;
    }

    void setRight(double val) {
        right = val;
    }

    void setScaleValueEnabled(bool b) {
        scaleValueEnabled = b;
    }

    virtual void drawLabel (QPainter *painter, double value) const {
        if(value == left || value == right || !scaleValueEnabled)
            QwtScaleDraw::drawLabel(painter, value);
    }
};


caSlider::caSlider(QWidget *parent) : QwtSlider(parent)
{
    // to start with, clear the stylesheet, so that playing around
    // is not possible.
    setStyleSheet("");

    isShown = false;
    oldStyle = "";
    thisStyle = "";

    thisIncrement = 1.0;
    thisDirection = Up;
    thisMinimum = -50;
    thisMaximum = 50;
    thisCtrlMinimum = -50;
    thisCtrlMaximum = -50;
    pointSizePrv = 0.0;
    direction = 0;
    sliderSelected = false;

    setScalePosition(NoScale);
    setSpacing(0);
    setBorderWidth(1);
    setSliderValue(0.0);
    setAutoFocus(false);

    installEventFilter(this);

    setHandleSize(QSize(10,20));

    thisColorMode = Static;
    thisHighLimitMode = Channel;
    thisLowLimitMode = Channel;

    oldBackColor = QColor(Qt::white);
    oldForeColor = QColor(Qt::white);
    setScaleValueColor(Qt::black);

    setBackground(QColor(224,224,224));
    setForeground(Qt::black);

    setPrecision(1);
    setPrecisionMode(Channel);
    setFormatType(decimal);
    setScaleValueEnabled(false);

    setDirection(thisDirection);
    setAccessW(true);

    setFocusPolicy(Qt::ClickFocus);

    QwtLinearScaleEngine *scaleEngine = new mySliderScaleEngine();
    setScaleEngine( scaleEngine );

    repeatTimer = new QTimer(this);
    repeatTimer->setInterval(200);
    connect(repeatTimer, SIGNAL(timeout()), this, SLOT(repeater()));

    setElevation(on_top);

    configureScale();
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
    thisBackColor = c;
    setColors(thisBackColor, thisForeColor);
}

QColor caSlider::getForeground() {
    return thisForeColor;
}

QColor caSlider::getBackground() {
    return thisBackColor;
}

void caSlider::setForeground(QColor c)
{
    thisForeColor = c;
    setColors(thisBackColor, thisForeColor);
}

void caSlider::setColors(QColor bg, QColor fg)
{
    if(!defaultBackColor.isValid() || !defaultForeColor.isValid()) return;

    if((bg != oldBackColor) || (fg != oldForeColor) || (thisColorMode != oldColorMode)) {

        QString thisStyle = "caSlider {background-color: rgba(%1, %2, %3, %4); color: rgba(%5, %6, %7, %8);} caSlider:focus  {border: 1px solid #f00;};";

        if(thisColorMode == Default) {
            thisStyle = thisStyle.arg(defaultBackColor.red()).arg(defaultBackColor.green()).arg(defaultBackColor.blue()).arg(defaultBackColor.alpha()).
                    arg(defaultForeColor.red()).arg(defaultForeColor.green()).arg(defaultForeColor.blue()).arg(defaultForeColor.alpha());
            if(thisStyle != oldStyle) setStyleSheet(thisStyle);
            oldStyle = thisStyle;
        } else if(thisColorMode == Static) {
            thisStyle = thisStyle.arg(bg.red()).arg(bg.green()).arg(bg.blue()).arg(bg.alpha()).arg(fg.red()).arg(fg.green()).arg(fg.blue()).arg(fg.alpha());
            if(thisStyle != oldStyle) setStyleSheet(thisStyle);
            oldStyle = thisStyle;
        } else  if(thisColorMode == Alarm_Static) {
            thisStyle = thisStyle.arg(thisBackColor.red()).arg(thisBackColor.green()).arg(thisBackColor.blue()).arg(thisBackColor.alpha()).
                    arg(thisForeColor.red()).arg(thisForeColor.green()).arg(thisForeColor.blue()).arg(thisForeColor.alpha());
            if(thisStyle != oldStyle) setStyleSheet(thisStyle);
            oldStyle = thisStyle;
            // will give the alarm colors for the handle and slider background (must be done after style sheet setting
            QColor bgs = bg.darker(125);
            bgs.setAlpha(255);
            QPalette thisPalette = palette();
            thisPalette.setColor(QPalette::Button, bg);
            thisPalette.setColor(QPalette::Mid, bgs);
            setPalette(thisPalette);
        } else  if(thisColorMode == Alarm_Default) {
            thisStyle = thisStyle.arg(defaultBackColor.red()).arg(defaultBackColor.green()).arg(defaultBackColor.blue()).arg(defaultBackColor.alpha()).
                    arg(defaultForeColor.red()).arg(defaultForeColor.green()).arg(defaultForeColor.blue()).arg(defaultForeColor.alpha());
            if(thisStyle != oldStyle) setStyleSheet(thisStyle);
            oldStyle = thisStyle;
            // will give the alarm colors for the handle and slider background (must be done after style sheet setting
            QColor bgs = bg.darker(125);
            bgs.setAlpha(255);
            QPalette thisPalette = palette();
            thisPalette.setColor(QPalette::Button, bg);
            thisPalette.setColor(QPalette::Mid, bgs);
            setPalette(thisPalette);
        }
    }

    oldBackColor = bg;
    oldForeColor = fg;
    oldColorMode = thisColorMode;
}

void caSlider::setMaxValue(double const &maxim){
    thisMaximum = maxim;
    configureScale();
    setDirection(thisDirection);
    update();
}

void caSlider::setMinValue(double const &minim){
    thisMinimum = minim;
    configureScale();
    setDirection(thisDirection);
    update();
}

void caSlider::setSliderValue(double const &value){
    thisValue = value;
    //qDebug() <<"setSliderValue"<<value<< this;
    setValue(value);

}

void caSlider::set_Max_Value(int max)
{
    set_Max_Value(double(max));
}

void caSlider::set_Max_Value(double max)
{
    if (fabs(thisMaximum-max)>std::numeric_limits<double>::epsilon()*10){
        //qDebug()<< "Max:"<< max;
        thisMaximum=max;
#if QWT_VERSION < 0x060100
        setRange(thisMinimum, thisMaximum, thisIncrement, 1);
#else
        setScale(thisMinimum, thisMaximum);
#endif
    }


}

void caSlider::set_Min_Value(int min)
{
    set_Min_Value(double(min));
}

void caSlider::set_Min_Value(double min)
{
    if (fabs(thisMinimum-min)>std::numeric_limits<double>::epsilon()*10){
        thisMinimum=min;
#if QWT_VERSION < 0x060100
        setRange(thisMinimum, thisMaximum, thisIncrement, 1);
#else
        setScale(thisMinimum, thisMaximum);
#endif
    }
}

void caSlider::setIncrementValue(double const &value){
    thisIncrement = value;
    setDirection(thisDirection);
    update();
}

void caSlider::setAccessW(bool access)
{
    thisAccessW = access;
}

void caSlider::setDirection(Direction dir)
{
#if QWT_VERSION < 0x060100
    ScalePos scalepos = this->scalePosition();

    // when ui file was made with version Qwt6.1.1 and here we use Qwt6.0.1 then
    // we are in trouble, so make some defaults in this case
    if(scalepos == -1) {
        switch (dir) {
            case Up:
            case Down:
               scalepos = QwtSlider::LeftScale;
               break;
            case Left:
            case Right:
               scalepos = QwtSlider::BottomScale;
               break;
         }
    }

#else
    ScalePosition scalepos = this->scalePosition();
    if(scalepos == -1) {
        switch (dir) {
            case Up:
            case Left:
               scalepos = QwtSlider::TrailingScale;
               break;
            case Down:
            case Right:
               scalepos = QwtSlider::LeadingScale;
               break;
         }
    }
#endif

    thisDirection = dir;

#if QWT_VERSION >= 0x060100
    incrementValue((int) thisIncrement);
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
    setValue(thisValue);
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
        setStepAlignment(false);
        QwtAbstractSlider::setValue(value() + increment * thisIncrement);
#else
        QwtDoubleRange::setValue(value() + increment * step());
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
        if(!thisAccessW) {
            e->ignore();
            return;
        }

        const QPoint &p = e->pos();
        const int markerPos = transform( value() );

        // I have to do the work myself due to the unwanted snapping
#if QWT_VERSION >= 0x060100
        if(isScrollPosition(e->pos())) {
            QwtSlider::mousePressEvent(e);
            sliderSelected = true;
        } else if(sliderRect().contains(e->pos())) {
#else
        QwtAbstractSlider::ScrollMode scrollMode;
        getScrollMode(p,  scrollMode, direction);
        if(scrollMode == QwtAbstractSlider::ScrMouse) {
            QwtSlider::mousePressEvent(e);
            sliderSelected = true;
        } else {
#endif
            if (orientation() == Qt::Horizontal ) {
                if(thisDirection == Right) {
                    if ( p.x() < markerPos ) direction = -1;
                    else direction = 1;
                } else {
                    if ( p.x() < markerPos ) direction = 1;
                    else direction = -1;
                }
            } else {
                if(thisDirection == Up) {
                    if ( p.y() < markerPos ) direction = 1;
                    else direction = -1;
                } else {
                    if ( p.y() < markerPos ) direction = -1;
                    else direction = 1;
                }
            }

            moveSlider();
            repeatTimer->start();
            e->ignore();
        }
    }
}

void caSlider::mouseReleaseEvent( QMouseEvent *e )
{
    sliderSelected = false;
    if( e->button() == Qt::LeftButton) {
#if QWT_VERSION < 0x060100
        stopMoving();
        QwtSlider::mouseReleaseEvent(e);
#else
        QwtSlider::mouseReleaseEvent(e);
#endif
    }
    repeatTimer->stop();
}

void caSlider::wheelEvent(QWheelEvent *e)
{
    if(!thisAccessW) {
        e->ignore();
        return;
    }
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    int delta = e->delta();
#else
    int delta = e->pixelDelta().manhattanLength();
#endif


    if(thisDirection == Right || thisDirection == Up) {
        if ( delta > 0 ) direction = 1;
        else direction = -1;
    } else {
        if ( delta > 0 ) direction = -1;
        else direction = 1;
    }

    moveSlider();
    e->ignore();
}

void caSlider::repeater( )
{
    moveSlider();
}

void caSlider::moveSlider()
{
    if(!thisAccessW) return;
    double oldVal = thisValue;

#if QWT_VERSION >= 0x060100
    double step = thisIncrement;
    thisValue = thisValue + double(direction) * step;
#else
    thisValue = thisValue + double(direction) * step();
#endif

    if(thisValue > thisCtrlMaximum) thisValue = thisCtrlMaximum;
    else if (thisValue <thisCtrlMinimum) thisValue = thisCtrlMinimum;
    else if(thisValue < thisMinimum && oldVal >= thisMinimum) thisValue = thisMinimum;
    else if(thisValue > thisMaximum && oldVal <= thisMaximum) thisValue = thisMaximum;

    setValue(thisValue);
    Q_EMIT sliderMoved( thisValue );
    Q_EMIT valueChanged( thisValue );
}

bool caSlider::timerActive()
{
    return repeatTimer->isActive();
}

void caSlider::stopUpdating()
{
    repeatTimer->stop();
}

void caSlider::mouseMoveEvent( QMouseEvent *e )
{
    if(!thisAccessW) {
        e->ignore();
        return;
    }
    if(sliderSelected) setPosition(e->pos());
    e->ignore();
}

void caSlider::setPosition(const QPoint &p)
{
#if QWT_VERSION >= 0x060100
       QwtAbstractSlider::setValue( scrolledTo( p ));
#else
    QwtDoubleRange::setValue( getValue( p ));
#endif
}

void caSlider::setValue( double val )
{
    QwtAbstractSlider::setValue(val);
    Q_EMIT sliderMoved( thisValue );
    Q_EMIT valueChanged( thisValue );
    update();
}

bool caSlider::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Enter) {
        if(!thisAccessW) {
            QApplication::setOverrideCursor(QCursor(Qt::ForbiddenCursor));
            setReadOnly(true);
        } else {
            if(thisAutoFocus) QWidget::setFocus();
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
    case INVALID_ALARM:
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
        setAlarmColors(MAJOR_ALARM);
    } else {
        setAlarmColors(NO_ALARM);
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

        if(!isShown) {
            QString c=  palette().color(QPalette::Base).name();
            defaultBackColor = QColor(c);
            c =  palette().color(QPalette::Text).name();
            defaultForeColor = QColor(c);

            if(!defaultBackColor.isValid()) defaultBackColor = QColor(255, 248, 220, 255);
            if(!defaultForeColor.isValid()) defaultForeColor = Qt::black;

            setColors(thisBackColor, thisForeColor);
            isShown = true;
        }

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
                QSize handlesize = QSize(width()*2/5-4, height()/10);
                if(handlesize != this->handleSize()) {
                    this->setHandleSize(handlesize);
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

            case Qt::Horizontal: {
                QSize handlesize = QSize(width()/10, height()*2/5-4);
                if(handlesize != this->handleSize()) {
                    this->setHandleSize(handlesize);
                }
                QFont f = font();
                int size = that->scaleDraw()->maxLabelWidth(f);
                float yFactor = (float) size  / ((float) height()*3.0/5.0 -10.0);

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

//Overridden from QwtSlider to add the display of the current value
void caSlider::drawSlider(QPainter *painter, const QRect &sliderRect ) const
{
    QwtSlider::drawSlider(painter, sliderRect);

    if(this->scalePosition() != NoScale && thisScaleValueEnabled) {
        QRect valueRect = createValueRect(sliderRect);
        paintValue (painter,  valueRect);
    }
}

//Created to calculate where the QRect for the value should be placed
QRect caSlider::createValueRect(QRect sliderRect) const
{
    QRect valueRect;
    int valHeight = pointSizePrv + 5;

    if(orientation() == Qt::Horizontal) {
        switch (this->scalePosition()) {
        //Horizontal and scale on top
#if QWT_VERSION < 0x060100
        case TopScale:
#else
        case TrailingScale:
#endif
            valueRect.setRect(sliderRect.x(), sliderRect.y() - 12 - (valHeight), sliderRect.width(), valHeight);
            break;
        //Horizontal and scale on bottom
#if QWT_VERSION < 0x060100
        case BottomScale:
#else
        case LeadingScale:
#endif
            valueRect.setRect(sliderRect.x(), sliderRect.height() + 12 , sliderRect.width(), valHeight);
            break;

        default:
            break;

        }
    } else {
        switch (this->scalePosition()) {

        //Vertical and scale on left
#if QWT_VERSION < 0x060100
        case  LeftScale:
#else
        case TrailingScale:
#endif
            valueRect.setRect(0, height() * .5 - valHeight / 2, width() - sliderRect.width(), valHeight);
            break;

        //Verticle and scale on right
#if QWT_VERSION < 0x060100
        case  RightScale:
#else
        case LeadingScale:
#endif
            valueRect.setRect(sliderRect.width() + 6, height() * .5 - valHeight / 2, width() - sliderRect.width() - 6, valHeight);
            break;

        default:
            break;

        }
    }
    return valueRect;
}

// Draws the label with the value on the widget
void caSlider::paintValue(QPainter *painter, QRect valueRect) const
{
    painter->setPen(thisScaleValueColor);
    if(orientation() == Qt::Horizontal) {
        painter->drawText( valueRect, Qt::AlignCenter, setScaleLabel(thisValue) );
    } else {
        painter->drawText( valueRect, Qt::AlignLeft , setScaleLabel(thisValue) );
    }
}

// Configures the scale for the widget
void caSlider::configureScale()
{
    mySliderScaleDraw *scaleDraw = new mySliderScaleDraw();
    scaleDraw->setLeft(thisMinimum);
    scaleDraw->setRight(thisMaximum);
    scaleDraw->setScaleValueEnabled(thisScaleValueEnabled);
    setScaleDraw ( scaleDraw );
}

// Creates the precision format the value label will use
void caSlider::setFormat(int prec)
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
QString caSlider::setScaleLabel(double value) const
{
    char asc[MAX_STRING_LENGTH];
    QString label;

    if(thisFormatType == compact) {
        if ((value < 1.e4 && value > 1.e-4) || (value > -1.e4 && value < -1.e-4) || value == 0.0) {
            snprintf(asc, MAX_STRING_LENGTH, thisFormatC, value);
        } else {
            snprintf(asc, MAX_STRING_LENGTH,  thisFormat, value);
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

void caSlider::setScaleValueEnabled(bool b)
{
    thisScaleValueEnabled = b;
    configureScale();
    setDirection(thisDirection);
}

#include "moc_caslider.cpp"

