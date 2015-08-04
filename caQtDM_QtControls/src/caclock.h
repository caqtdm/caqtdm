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

#ifndef CACLOCK_H
#define CACLOCK_H

#include <QEvent>
#include <QMouseEvent>
#include <QTimer>
#include <QApplication>

#include <math.h>
#include <qwt_analog_clock.h>
#include <qwt_round_scale_draw.h>
#include <qtcontrols_global.h>
#include "alarmdefs.h"

class QTCON_EXPORT caClock : public QwtAnalogClock
{
    Q_OBJECT

    Q_ENUMS(timeType)
    Q_PROPERTY(QString channel READ getPV WRITE setPV)
    Q_PROPERTY(timeType TimeType READ getTimeType WRITE setTimeType)

    Q_PROPERTY(QColor baseColor READ getBaseColor WRITE setBaseColor)
    Q_PROPERTY(bool scaleDefaultColor READ getScaleDefaultColor WRITE setScaleDefaultColor)
    Q_PROPERTY(QColor scaleColor READ getScaleColor WRITE setScaleColor)
    Q_PROPERTY(colMode colorMode READ getColorMode WRITE setColorMode)
    Q_ENUMS(colMode)

public:

    enum timeType {InternalTime = 0, ReceiveTime};

    caClock(QWidget *parent);

    QString getPV() const {return thisPV;}
    void setPV(QString const &newPV) {thisPV = newPV;}
    timeType getTimeType() { return thisTimeType; }
    void setTimeType(timeType type) {thisTimeType = type;
                                     if(type == InternalTime) {
                                         timer->start(1000);
                                     } else {
                                         timer->stop();
                                     }}
    void updateClock(QTime time);
    void setAlarmColors(short status);

    enum colMode {Static, Alarm};
    colMode getColorMode() const { return thisColorMode; }
    void setColorMode(colMode colormode) {thisColorMode = colormode; setAlarmColors(NO_ALARM);}

    void setBaseColor(QColor c) { thisBaseColor = c; setAlarmColors(NO_ALARM);}
    QColor getBaseColor() { return thisBaseColor; }

    void setScaleColor(QColor c) { thisScaleColor = c; setAlarmColors(NO_ALARM);}
    QColor getScaleColor() { return thisScaleColor; }

    void setScaleDefaultColor(bool b){ thisScaleDefaultColor= b; setAlarmColors(NO_ALARM);}
    bool getScaleDefaultColor(){ return thisScaleDefaultColor; }

signals:
    void updateTime(QTime);

private slots:
    void setClockTime(QTime);

protected:
    void resizeEvent(QResizeEvent *e);

private:

    QPalette colorTheme( const QColor &base ) const;

    QString thisPV;
    timeType thisTimeType;
    QTimer *timer;
    QTime prevTime;
    colMode thisColorMode;
    QColor prevColor;
    QColor thisBaseColor;
    QColor thisScaleColor, prevScaleColor;
    bool thisScaleDefaultColor, prevScaleDefaultColor;
};

#endif
