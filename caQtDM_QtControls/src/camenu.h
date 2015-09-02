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
    Q_ENUMS(colMode)
    Q_PROPERTY(QString channel READ getPV WRITE setPV)
    Q_PROPERTY(colMode colorMode READ getColorMode WRITE setColorMode)
    Q_PROPERTY(QColor foreground READ getForeground WRITE setForeground)
    Q_PROPERTY(QColor background READ getBackground WRITE setBackground)
    Q_PROPERTY(bool labelDisplay READ getLabelDisplay WRITE setLabelDisplay)
    Q_OBJECT

#include "caElevation.h"

public:

    enum colMode {Default, Static, Alarm};

    QString getPV() const;
    void setPV(QString const &newPV);

    QColor getForeground() const {return thisForeColor;}
    void setForeground(QColor c);

    QColor getBackground() const {return thisBackColor;}
    void setBackground(QColor c);

    bool getLabelDisplay() const { return thisLabelDisplay; }
    void setLabelDisplay(bool thisLabelDisplay);

    colMode getColorMode() const { return thisColorMode; }
    void setColorMode(colMode colormode) {thisColorMode = colormode; setColors(thisBackColor, thisForeColor, true);}

    QString getLabel() const;

     void populateCells(QStringList list);

    caMenu(QWidget *parent);

    int getAccessW() const {return thisAccessW;}
    void setAccessW(int access);

    void setColors(QColor bg, QColor fg, bool force);
    void setAlarmColors(short status);
    void setNormalColors();

private slots:


protected:

private:
    QString thisPV;
    QColor thisForeColor, oldForeColor;
    QColor thisBackColor, oldBackColor;
    QPalette thisPalette;
    bool thisLabelDisplay;
    int thisAccessW;
    colMode  thisColorMode;
    QPalette defaultPalette;

    bool eventFilter(QObject *obj, QEvent *event);
};

#endif
