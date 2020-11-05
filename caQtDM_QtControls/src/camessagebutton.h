

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

#ifndef caMessageMenu_H
#define caMessageMenu_H

#include <QString>
#include <QPushButton>
#include <QEvent>
#include <qtcontrols_global.h>
#include <epushbutton.h>

class QTCON_EXPORT caMessageButton : public EPushButton
{

    Q_OBJECT

    Q_PROPERTY(QString channel READ getPV WRITE setPV)
    Q_PROPERTY(QString label READ getLabel WRITE setLabel)
    Q_PROPERTY(QColor foreground READ getForeground WRITE setForeground)
    Q_PROPERTY(QColor background READ getBackground WRITE setBackground)
    Q_PROPERTY(QColor disabledForeground READ getDisabledForeground WRITE setDisabledForeground)
    Q_PROPERTY(QString disableChannel READ getDisablePV WRITE setDisablePV)
    Q_PROPERTY(QString releaseMessage READ getReleaseMessage WRITE setReleaseMessage)
    Q_PROPERTY(QString pressMessage READ getPressMessage WRITE setPressMessage)
    Q_PROPERTY(colMode colorMode READ getColorMode WRITE setColorMode)

    // this will prevent user interference
    Q_PROPERTY(QString styleSheet READ styleSheet WRITE noStyle DESIGNABLE false)

    Q_ENUMS(colMode)

#include "caElevation.h"

public:
    void noStyle(QString style) {Q_UNUSED(style);}

    caMessageButton(QWidget *parent);

    enum colMode {Static=0, Alarm, Default};

    void setColors(QColor bg, QColor fg,  QColor hover, QColor border, QColor disabledFg);
    void setNormalColors();
    void setAlarmColors(short status);

    colMode getColorMode() const { return thisColorMode; }

    void setColorMode(colMode colormode) {thisColorMode = colormode;
                                          setBackground(thisBackColor);
                                          setForeground(thisForeColor);
                                          oldColorMode = thisColorMode;
                                           }

    QColor getForeground() const {return thisForeColor;}
    void setForeground(QColor c);

    QColor getBackground() const {return thisBackColor;}
    void setBackground(QColor c);


    QColor getDisabledForeground() const {return thisDisabledForeColor;}
    void setDisabledForeground(QColor c);

    QString getLabel() const {return thisLabel;}
    void setLabel(QString const &label);

    QString getPressMessage() const {return thisPressMessage;}
    QString getReleaseMessage() const {return thisReleaseMessage;}
    void setReleaseMessage(QString const &m) {thisReleaseMessage = m;}
    void setPressMessage(QString const &m) {thisPressMessage = m;}

    QString getPV() const {return thisPV;}
    void setPV(QString const &newPV){ thisPV = newPV;}

    QString getDisablePV() const {return thisDisablePV;}
    void setDisablePV(QString const &newPV){ thisDisablePV = newPV;}

    bool getAccessW() const {return _AccessW;}
    void setAccessW(bool access);

    void buttonhandle(int type);


public slots:
    void animation(QRect p) {
#include "animationcode.h"
    }

    void hideObject(bool hideit) {
#include "hideobjectcode.h"
    }

    void sendMssage(int i) {
        emit animateClick();
        if(_AccessW) buttonhandle(i);
    }



signals:
    void messageButtonSignal(int type);
    void releaseClicked(int val);
    void pressClicked(int val);

private:

    QString thisPV, thisDisablePV;
    QString thisLabel;
    QColor thisForeColor, oldForeColor;
    QColor thisBackColor, oldBackColor;
    QColor thisHoverColor, oldHoverColor;
    QColor thisDisabledForeColor, oldDisabledForeColor;
    QColor thisBorderColor;
    QPalette thisPalette;

    colMode thisColorMode;
    colMode oldColorMode;

    QString thisReleaseMessage, thisPressMessage;
    bool eventFilter(QObject *obj, QEvent *event);
    bool _AccessW;
    bool renewStyleSheet;
};

#endif
