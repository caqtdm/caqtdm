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

#ifndef CASLIDER_H
#define CASLIDER_H

#if defined(_MSC_VER)
 #define QWT_DLL
#endif

#include <QtCore>
#include <limits>

#include <qtcontrols_global.h>
#include <qwt_slider.h>
#include <qwt_scale_draw.h>
#include <qwt_scale_engine.h>

#include <QTimer>

class QTCON_EXPORT caSlider : public QwtSlider
{
    Q_OBJECT

    Q_PROPERTY( Qt::Orientation orientation
                   READ orientation WRITE setOrientation  DESIGNABLE false)

    Q_PROPERTY(QString channel READ getPV WRITE setPV)
    Q_PROPERTY(Direction direction READ getDirection WRITE setDirection)

    Q_PROPERTY(QColor foreground READ getForeground WRITE setForeground)
    Q_PROPERTY(QColor background READ getBackground WRITE setBackground)

    Q_PROPERTY(colMode colorMode READ getColorMode WRITE setColorMode)
    Q_ENUMS(colMode)

    Q_PROPERTY(SourceMode highLimitMode READ getHighLimitMode WRITE setHighLimitMode)
    Q_PROPERTY(double maxValue READ getMaxValue WRITE setMaxValue)
    Q_PROPERTY(SourceMode lowLimitMode READ getLowLimitMode WRITE setLowLimitMode)
    Q_PROPERTY(double minValue READ getMinValue WRITE setMinValue)

    Q_PROPERTY(SourceMode precisionMode READ getPrecisionMode WRITE setPrecisionMode)
    Q_PROPERTY(int precision READ getPrecision WRITE setPrecision)

    Q_PROPERTY(double incrementValue READ getIncrementValue WRITE setIncrementValue)
    Q_PROPERTY(double value READ getSliderValue WRITE setSliderValue)

    // this will prevent user interference
    Q_PROPERTY(QString styleSheet READ styleSheet WRITE noStyle DESIGNABLE false)

    Q_PROPERTY(FormatType formatType READ getFormatType WRITE setFormatType)
    Q_PROPERTY(bool scaleValueEnabled READ isScaleValueEnabled WRITE setScaleValueEnabled)
    Q_PROPERTY(QColor scaleValueColor READ getScaleValueColor WRITE setScaleValueColor)

    Q_PROPERTY(SourceMode limitsMode READ getLegacyLimit WRITE setLegacyLimit DESIGNABLE false)

    Q_PROPERTY(bool autoFocus READ getAutoFocus WRITE setAutoFocus)

    Q_ENUMS(Direction)
    Q_ENUMS(SourceMode)
    Q_ENUMS(FormatType)

#include "caElevation.h"

public:
    void noStyle(QString style) {Q_UNUSED(style);}

    QString getPV() const;
    void setPV(QString const &newPV);

    enum Direction {Up, Down, Left, Right};
    Direction getDirection() const { return thisDirection; }
    void setDirection(Direction direction);

    QColor getForeground();
    void setForeground(QColor c);
    QColor getBackground();
    void setBackground(QColor c);

    enum colMode {Default, Static, Alarm_Default, Alarm_Static, Alarm=Alarm_Default};
    colMode getColorMode() const { return thisColorMode; }

    void setColorMode(colMode colormode) {thisColorMode = colormode;
                                          setBackground(thisBackColor);
                                          setForeground(thisForeColor);
                                          oldColorMode = thisColorMode;
                                           }

    enum SourceMode {Channel = 0, User};

    SourceMode getLimitsMode() const { return thisLimitsMode; }
    void setLimitsMode(SourceMode limitsmode) { thisLimitsMode = limitsmode;}

    SourceMode getHighLimitMode() const { return thisHighLimitMode; }
    void setHighLimitMode(SourceMode limit) { thisHighLimitMode = limit;}
    SourceMode getLowLimitMode() const { return thisLowLimitMode; }
    void setLowLimitMode(SourceMode limit) { thisLowLimitMode = limit;}

    SourceMode getLegacyLimit() const {return thisHighLimitMode; }
    void setLegacyLimit(SourceMode limit)  { thisLowLimitMode = limit; thisHighLimitMode = limit; }

    SourceMode getPrecisionMode() const { return thisPrecMode; }
    void setPrecisionMode(SourceMode precmode) { thisPrecMode = precmode; }
    int getPrecision() const { return thisPrecision; }
    void setPrecision(int prec) { thisPrecision = prec; setFormat(prec); configureScale();}

    void setFormat(int prec);

    enum FormatType { decimal, exponential, engr_notation, compact, truncated};
    void setFormatType(FormatType m) { thisFormatType = m; setFormat(thisPrecision); }
    FormatType getFormatType() { return thisFormatType; }

    QString setScaleLabel (double value) const;

    void setScaleValueEnabled(bool b);
    bool isScaleValueEnabled(){ return thisScaleValueEnabled; }

    QColor getScaleValueColor() {return thisScaleValueColor;}
    void setScaleValueColor(QColor c) {thisScaleValueColor = c; configureScale();}

    bool getAccessW() const {return thisAccessW;}
    void setAccessW(bool access);

    bool getAutoFocus() const {return thisAutoFocus;}
    void setAutoFocus(bool autofocus) {thisAutoFocus = autofocus;}

    double getMaxValue()  const {return thisMaximum;}
    void setMaxValue(double const &maxim);

    double getMinValue()  const {return thisMinimum;}
    void setMinValue(double const &minim);

    double getSliderValue()  const {return thisValue;}

    double getIncrementValue()  const {return thisIncrement;}
    void setIncrementValue(double const &value);

    double getCtrlMaximum() const {return thisCtrlMaximum;}
    void setCtrlMaximum(double max) {thisCtrlMaximum = max;}

    double getCtrlMinimum() const {return thisCtrlMinimum;}
    void setCtrlMinimum(double max) {thisCtrlMinimum = max;}

    caSlider(QWidget *parent);

    void setAlarmColors(short status);
    void setUserAlarmColors(double val);
    void setColors(QColor bg, QColor fg);
    void setNormalColors();
    bool timerActive();
    void stopUpdating();
    void setPosition (const QPoint &);
    void setValue( double val );
    void moveSlider();

public slots:
    void animation(QRect p) {
#include "animationcode.h"
    }

    void hideObject(bool hideit) {
#include "hideobjectcode.h"
    }

public slots:
    void setSliderValue(double const &value);
    void set_Max_Value(int max);
    void set_Max_Value(double max);
    void set_Min_Value(int min);
    void set_Min_Value(double min);

private slots:
    void repeater();

protected:
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void keyReleaseEvent(QKeyEvent *e);
    virtual void wheelEvent(QWheelEvent *e);
    virtual bool event(QEvent *);
    virtual void drawSlider ( QPainter *, const QRect & ) const;

private:
    bool eventFilter(QObject *obj, QEvent *event);

    QString thisPV;

    bool thisAccessW;
    Direction thisDirection;
    QColor thisForeColor, oldForeColor;
    QColor thisBackColor, oldBackColor;
    QColor thisScaleValueColor;
    colMode thisColorMode;
    colMode oldColorMode;

    SourceMode thisLimitsMode;
    SourceMode thisHighLimitMode;
    SourceMode thisLowLimitMode;

    int thisPrecision;
    SourceMode thisPrecMode;
    FormatType thisFormatType;
    char thisFormat[SMALL_STRING_LENGTH];
    char thisFormatC[SMALL_STRING_LENGTH];
    bool thisScaleValueEnabled;
    QString thisStyle, oldStyle;

    QColor defaultBackColor;
    QColor defaultForeColor;

    double thisMaximum;
    double thisMinimum;
    double thisCtrlMinimum;
    double thisCtrlMaximum;
    double thisValue;
    double thisIncrement;
    float  pointSizePrv;
    QTimer *repeatTimer;
    int direction;
    bool thisAutoFocus;
    bool sliderSelected;

    void configureScale();
    QRect createValueRect(QRect sliderRect) const ;
    void paintValue ( QPainter *painter, QRect valueRect) const;
    bool isShown;
};

#endif

