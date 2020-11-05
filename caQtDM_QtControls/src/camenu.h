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

#ifndef CAMENU_H
#define CAMENU_H
#include <QComboBox>
#include <qtcontrols_global.h>

class QTCON_EXPORT caMenu : public QComboBox
{
    Q_OBJECT

    Q_ENUMS(colMode)
    Q_PROPERTY(QString channel READ getPV WRITE setPV)

    Q_PROPERTY(QString channelMask READ getMaskPV WRITE setMaskPV)

    Q_PROPERTY(colMode colorMode READ getColorMode WRITE setColorMode)
    Q_PROPERTY(QColor foreground READ getForeground WRITE setForeground)
    Q_PROPERTY(QColor background READ getBackground WRITE setBackground)
    Q_PROPERTY(bool labelDisplay READ getLabelDisplay WRITE setLabelDisplay)

    // this will prevent user interference
    Q_PROPERTY(QString styleSheet READ styleSheet WRITE noStyle DESIGNABLE false)



#include "caElevation.h"

public:
    void noStyle(QString style) {Q_UNUSED(style);}

    enum colMode {Default, Static, Alarm};

    QString getPV() const;
    void setPV(QString const &newPV);

    QString getMaskPV() const;
    void setMaskPV(QString const &newPV);

    QColor getForeground() const {return thisForeColor;}
    void setForeground(QColor c);

    QColor getBackground() const {return thisBackColor;}
    void setBackground(QColor c);

    bool getLabelDisplay() const { return thisLabelDisplay; }
    void setLabelDisplay(bool thisLabelDisplay);

    colMode getColorMode() const { return thisColorMode; }
    void setColorMode(colMode colormode) {thisColorMode = colormode; setColors(thisBackColor, thisForeColor);}

    QString getLabel() const;

    void populateCells(QStringList list);
    void setMaskValue(const int &mask);
    void setIndex(int const &indx);

    caMenu(QWidget *parent);

    int getAccessW() const {return thisAccessW;}
    void setAccessW(int access);

    void setColors(QColor bg, QColor fg);
    void setAlarmColors(short status);
    void setNormalColors();
public slots:
    void alarmrewrite();
    void animation(QRect p) {
#include "animationcode.h"
    }

    void hideObject(bool hideit) {
#include "hideobjectcode.h"
    }

protected:
     virtual bool event(QEvent *);
private:
    QString thisPV, thisMaskPV;
    QColor thisForeColor, oldForeColor;
    QColor thisBackColor, oldBackColor;
    QColor defBackColor, defForeColor, defSelectColor;
    QPalette thisPalette;
    bool thisLabelDisplay;
    int thisAccessW;
    short alarmstatus;
    colMode  thisColorMode, oldColorMode;
    QPalette defaultPalette;
    QString thisStyle, oldStyle;
    QStringList nonMaskedStrings, maskedStrings;
    int thisMaskValue;
    bool isShown;
    bool updateAlarmStatus_once_Later;
    QColor updateAlarmStatus_bg;
    QColor updateAlarmStatus_fg;
    bool eventFilter(QObject *obj, QEvent *event);
    int lastIndex;
};

#endif
