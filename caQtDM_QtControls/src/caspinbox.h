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

#ifndef caSpin_H
#define caSpin_H

#include <QWidget>
#include <qtcontrols_global.h>
#include <snumeric.h>

class QTCON_EXPORT caSpinbox : public SNumeric
{
Q_OBJECT

    Q_PROPERTY(QString channel READ getPV WRITE setPV)

    Q_PROPERTY(QColor foreground READ getForeground WRITE setForeground)
    Q_PROPERTY(QColor background READ getBackground WRITE setBackground)
    Q_PROPERTY(colMode colorMode READ getColorMode WRITE setColorMode)
    Q_PROPERTY(SourceMode precisionMode READ getPrecisionMode WRITE setPrecisionMode)
    Q_PROPERTY(int maxChannelPrecision READ getMaxChannelPrecision WRITE setMaxChannelPrecision)
    Q_PROPERTY(bool fixedFormat READ getFixedFormat WRITE setFixedFormat)
    Q_PROPERTY(bool autoDigitShift READ getAutoDigitShift WRITE setAutoDigitShift)

    Q_PROPERTY(SourceMode limitsMode READ getLimitsMode WRITE setLimitsMode)
    Q_PROPERTY(double maxValue READ getMaxValue WRITE setMaxValue)
    Q_PROPERTY(double minValue READ getMinValue WRITE setMinValue)

    // this will prevent user interference
    Q_PROPERTY(QString styleSheet READ styleSheet WRITE noStyle DESIGNABLE false)



#include "caElevation.h"

public:
    void noStyle(QString style) {Q_UNUSED(style);}

    enum colMode {Static=0, Default, Alarm};
    Q_ENUM(colMode)

     caSpinbox(QWidget *parent);
    ~caSpinbox(){}

    QString getPV() const;
    void setPV(QString const &newPV);

    bool getAccessW() const {return thisAccessW;}
    void setAccessW(bool access);

    colMode getColorMode() const { return thisColorMode; }

    void setColorMode(colMode colormode) {thisColorMode = colormode;
                                          setBackground(thisBackColor);
                                          renewStyleSheet = true;
                                          setForeground(thisForeColor);
                                          oldColorMode = thisColorMode;
                                           }

    enum SourceMode {Channel = 0, User};
    Q_ENUM(SourceMode)
    SourceMode getPrecisionMode() const { return thisPrecMode; }
    void setPrecisionMode(SourceMode precmode) {thisPrecMode = precmode;}

    int getMaxChannelPrecision() const {return thisMaxChannelPrecision;}
    void setMaxChannelPrecision(int prec) {thisMaxChannelPrecision = qBound(0, prec, (int) SNumeric::MaxTotalDigits);}

    SourceMode getLimitsMode() const { return thisLimitsMode; }

    bool getFixedFormat()  const {return SNumeric::getFixedFormat();}
    void setFixedFormat(bool const &fixed) {SNumeric::setFixedFormat(fixed);}

    /* delegated to the base class, a local copy would never reach it */
    bool getAutoDigitShift() const {return SNumeric::getAutoDigitShift();}
    void setAutoDigitShift(bool const &shift) {SNumeric::setAutoDigitShift(shift);}

    void setLimitsMode(SourceMode limitsmode) { thisLimitsMode = limitsmode;}

    double getMaxValue()  const {return thisMaximum;}
    void setMaxValue(double const &maxim) {thisMaximum=maxim; setMaximum(maxim);}

    double getMinValue()  const {return thisMinimum;}
    void setMinValue(double const &minim) {thisMinimum=minim; setMinimum(minim);}

    QColor getForeground() const {return thisForeColor;}
    void setForeground(QColor c);

    QColor getBackground() const {return thisBackColor;}
    void setBackground(QColor c);

    void setColors(QColor bg, QColor fg, bool init = false);
    void setConnectedColors(bool connected);
    void setAlarmColors(short status);

public slots:
    void animation(QRect p) {
#include "animationcode.h"
    }

    void hideObject(bool hideit) {
#include "hideobjectcode.h"
    }

protected:
   void paintEvent(QPaintEvent *event);

public slots:
    void setSpinValue(double v) {setValue(v);}

private:
    QString thisPV;
    bool thisAccessW;
    double thisMaximum, thisMinimum;
    SourceMode thisPrecMode;
    SourceMode thisLimitsMode;
    /* 4 was the hard coded limit before it became configurable, keeping it as
     * default leaves existing panels unchanged */
    int thisMaxChannelPrecision = 4;
    QColor thisForeColor, oldForeColor;
    QColor thisBackColor, oldBackColor;

    colMode thisColorMode;
    colMode oldColorMode;
    bool renewStyleSheet;
    bool oldConnected = true;

};
#endif
