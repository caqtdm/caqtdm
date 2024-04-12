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

#ifndef CAMETER_H
#define CAMETER_H

#include <QDebug>
#include <QtDebug>
#include <QEvent>
#include <QMouseEvent>
#include <QTimer>
#include <QApplication>
#include <QPainter>
#include <QString>
#include <math.h>
#include <qwt_dial.h>
#include <qtcontrols_global.h>
#include <qwt_dial_needle.h>
#include <qwt_round_scale_draw.h>
#include "alarmdefs.h"

/* a simple meter */

class QTCON_EXPORT caMeter : public QwtDial
{
    Q_OBJECT

    Q_PROPERTY(QString channel READ getPV WRITE setPV)

    Q_PROPERTY(double maxValue READ getMaxValue WRITE setMaxValue)
    Q_PROPERTY(double minValue READ getMinValue WRITE setMinValue)

    Q_PROPERTY(SourceMode limitsMode READ getLimitsMode WRITE setLimitsMode)
    Q_ENUMS(SourceMode)

    Q_PROPERTY(QColor baseColor READ getBaseColor WRITE setBaseColor)
    Q_PROPERTY(bool scaleDefaultColor READ getScaleDefaultColor WRITE setScaleDefaultColor)
    Q_PROPERTY(QColor scaleColor READ getScaleColor WRITE setScaleColor)

    Q_PROPERTY(colMode colorMode READ getColorMode WRITE setColorMode)
    Q_PROPERTY(bool scaleEnabled READ isScaleEnabled WRITE setScaleEnabled)
    Q_PROPERTY(bool valueDisplayed READ isValueDisplayed WRITE setValueDisplayed)

    Q_PROPERTY(int precision READ getPrecision WRITE setPrecision)
    Q_PROPERTY(SourceMode precisionMode READ getPrecisionMode WRITE setPrecisionMode)

    Q_PROPERTY(FormatType formatType READ getFormatType WRITE setFormatType)
    Q_PROPERTY(bool unitsEnabled READ getUnitsEnabled WRITE setUnitsEnabled)

    // this will prevent user interference
    Q_PROPERTY(QString styleSheet READ styleSheet WRITE noStyle DESIGNABLE false)

    Q_ENUMS(FormatType)
    Q_ENUMS(colMode)
    Q_ENUMS(SourceMode)
    Q_ENUMS(FormatType)
    Q_ENUMS(displayLims)

public:
    void noStyle(QString style) {Q_UNUSED(style);}

    caMeter( QWidget *parent = 0 );
    ~caMeter();

    QString getPV() const {return thisPV;}
    void setPV(QString const &newPV) {thisPV = newPV;}

    enum colMode {Static, Alarm};
    enum displayLims {Channel_Limits = 0 , User_Limits};

    enum SourceMode {Channel = 0, User};
    SourceMode getPrecisionMode() const { return thisPrecMode; }
    void setPrecisionMode(SourceMode precmode) {thisPrecMode = precmode;}
    int getPrecision() const {return thisPrecision;}
    void setPrecision(int prec) {thisPrecision = prec; setFormat(prec); invalidate();}

    void setFormat(int prec);
    void setValueUnits(double value, const QString& units);

    enum FormatType { decimal, exponential, engr_notation, compact, truncated};
    void setFormatType(FormatType m) { thisFormatType = m;  setFormat(thisPrecision); invalidate();}
    FormatType getFormatType() { return thisFormatType; }

    void setMinValue(double v);
    double getMinValue(){ return thisMinValue; }

    void setMaxValue(double v);
    double getMaxValue(){ return thisMaxValue; }

    void setScaleEnabled(bool b) {thisScaleEnabled = b; invalidate();}
    bool isScaleEnabled(){ return thisScaleEnabled; }

    SourceMode getLimitsMode() const { return thisLimitsMode; }
    void setLimitsMode(SourceMode limitsmode) { thisLimitsMode = limitsmode;}

    void setValueDisplayed(bool b){ thisValueDisplayed = b; update(); }
    bool isValueDisplayed(){ return thisValueDisplayed; }

    bool getUnitsEnabled() const { return thisUnitMode; }
    void setUnitsEnabled(bool check) { thisUnitMode = check;}

    void setAlarmColors(short status, bool force = false);
    void setNormalColors();

    QString setLabel(double value, const QString& units);
    QString setScaleLabel(double value);
    void updateMeter();

    colMode getColorMode() const { return thisColorMode; }
    void setColorMode(colMode colormode) {thisColorMode = colormode; setAlarmColors(NO_ALARM);}

    void setBaseColor(QColor c) { thisBaseColor = c; update(); setAlarmColors(NO_ALARM);}
    QColor getBaseColor() { return thisBaseColor; }

    void setScaleColor(QColor c) { thisScaleColor = c; setAlarmColors(NO_ALARM);}
    QColor getScaleColor() { return thisScaleColor; }

    void setScaleDefaultColor(bool b){ thisScaleDefaultColor= b; setAlarmColors(NO_ALARM);}
    bool getScaleDefaultColor(){ return thisScaleDefaultColor; }

    void invalidate();

public slots:
    void animation(QRect p) {
#include "animationcode.h"
    }

    void hideObject(bool hideit) {
#include "hideobjectcode.h"
    }

protected:
    virtual void drawScaleContents(QPainter *painter, const QPointF &center, double radius) const;
    void resizeEvent(QResizeEvent *e);

private:
   QPalette colorTheme( const QColor &base ) const;
   QString thisLabel;
   QString thisPV;
   colMode thisColorMode;
   QColor prevColor;
   double thisMinValue, thisMaxValue, lastValue;
   bool thisScaleEnabled, thisValueDisplayed;
   SourceMode thisLimitsMode;
   int thisPrecision;
   SourceMode thisPrecMode;
   FormatType thisFormatType;
   bool thisUnitMode;
   char thisFormat[SMALL_STRING_LENGTH];
   char thisFormatC[SMALL_STRING_LENGTH];
   QString lastUnits;
   QColor thisBaseColor, thisScaleColor, prevScaleColor;
   bool thisScaleDefaultColor, prevScaleDefaultColor;
};

#endif
