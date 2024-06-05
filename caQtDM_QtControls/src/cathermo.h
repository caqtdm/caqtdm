
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

#ifndef CATHERMO_H
#define CATHERMO_H

#include <qwt_global.h>

#include <qtcontrols_global.h> 
#if QWT_VERSION < 0x060100
   #include <qwt_thermo_marker.h>
#else
   #include <qwt_thermo_marker_61.h>
#endif
#include <qwt_scale_draw.h>
#include <qwt_scale_engine.h>

class QTCON_EXPORT caThermo : public QwtThermoMarker
{
    Q_OBJECT

    Q_ENUMS(Direction)
    Q_ENUMS(Look)
    Q_ENUMS(FormatType)
    Q_ENUMS(SourceMode)

    Q_PROPERTY(QString channel READ getPV WRITE setPV)
    Q_PROPERTY(Direction direction READ getDirection WRITE setDirection)
    Q_PROPERTY(Look look READ getLook WRITE setLook)
    Q_PROPERTY(bool logScale READ getLogScale WRITE setLogScale)

    Q_PROPERTY(QColor foreground READ getForeground WRITE setForeground)
    Q_PROPERTY(QColor background READ getBackground WRITE setBackground)
    Q_PROPERTY(QColor textColor READ getTextColor WRITE setTextColor)

    Q_PROPERTY(colMode colorMode READ getColorMode WRITE setColorMode)
    Q_ENUMS(colMode)

    Q_PROPERTY(SourceMode limitsMode READ getLimitsMode WRITE setLimitsMode)

    // this will prevent user interference
    Q_PROPERTY(QString styleSheet READ styleSheet WRITE noStyle DESIGNABLE false)

    Q_PROPERTY(SourceMode precisionMode READ getPrecisionMode WRITE setPrecisionMode)
    Q_PROPERTY(int precision READ getPrecision WRITE setPrecision)

    Q_PROPERTY(FormatType formatType READ getFormatType WRITE setFormatType)
    Q_PROPERTY(bool scaleValueEnabled READ isScaleValueEnabled WRITE setScaleValueEnabled)

public:
    enum FormatType { decimal, exponential, engr_notation, compact, truncated};

    void noStyle(QString style) {Q_UNUSED(style);}

    QString getPV() const;
    void setPV(QString const &newPV);

    enum Direction {Up, Down, Left, Right};
    Direction getDirection() const { return thisDirection; }
    void setDirection(Direction direction);

    enum Look {noLabel, noDeco, Outline, Limits, ChannelV};
    Look getLook() const { return thisLook; }
    void setLook(Look look);

    void setValue(double value);

    bool getLogScale() const { return thisLogScale; }
    void setLogScale(bool LogScale) {thisLogScale = LogScale;}

    QColor getForeground() const {return thisForeColor;}
    void setForeground(QColor c);
    QColor getBackground() const {return thisBackColor;}
    void setBackground(QColor c);
    QColor getTextColor() const {return thisTextColor;}
    void setTextColor(QColor c);

    enum colMode {Default, Static, Alarm_Default, Alarm_Static, Alarm=Alarm_Default};
    colMode getColorMode() const { return thisColorMode; }

    void setColorMode(colMode colormode) {thisColorMode = colormode;
                                          setBackground(thisBackColor);
                                          setForeground(thisForeColor);
                                         }

    enum SourceMode {Channel = 0, User};
    SourceMode getLimitsMode() const { return thisLimitsMode; }
    void setLimitsMode(SourceMode limitsmode) {thisLimitsMode = limitsmode;}

    SourceMode getPrecisionMode() const { return thisPrecMode; }
    void setPrecisionMode(SourceMode precmode) { thisPrecMode = precmode; }
    int getPrecision() const { return thisPrecision; }
    void setPrecision(int prec) { thisPrecision = prec; setFormat(prec); update();}

    void setFormatType(FormatType m) { thisFormatType = m; setFormat(thisPrecision);}
    FormatType getFormatType() { return thisFormatType; }

    void setAlarmColors(short status);
    void setUserAlarmColors(double val);
    void setColors(QColor bg, QColor fg, QColor barColor, colMode mode);
    void setNormalColors();

    caThermo(QWidget *parent);

    QString setScaleLabel (double value) const;
    void setFormat(int prec);

    void setScaleValueEnabled(bool b);
    bool isScaleValueEnabled(){ return thisScaleValueEnabled; }

public slots:
    void animation(QRect p) {
#include "animationcode.h"
    }

    void hideObject(bool hideit) {
#include "hideobjectcode.h"
    }

protected:
    virtual bool event(QEvent *);
    virtual void drawLiquid ( QPainter *, const QRect & ) const;

private:
    QString thisPV;
    bool    m_externalEnabled;

    QColor thisForeColor, oldForeColor;
    QColor thisBackColor, oldBackColor;
    QColor thisTextColor, oldTextColor;
    QColor defaultBackColorOld, defaultForeColorOld;

    colMode thisColorMode;
    colMode oldColorMode;
    SourceMode thisLimitsMode;
    Direction      thisDirection;
    Look thisLook;
    int thisScale;
    bool thisLogScale;
    QColor defaultBackColor;
    QColor defaultForeColor;
    float  pointSizePrv;
    bool isShown;

    int thisPrecision;
    SourceMode thisPrecMode;
    FormatType thisFormatType;
    char thisFormat[SMALL_STRING_LENGTH];
    char thisFormatC[SMALL_STRING_LENGTH];
    double thisMaximum;
    double thisMinimum;
    double thisValue;
    bool thisScaleValueEnabled;

    void paintValue ( QPainter *painter, QRect valueRect) const;
};

#endif
