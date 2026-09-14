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

#ifndef CALABEL_H
#define CALABEL_H

#include <qtcontrols_global.h>
#include <ELabel>

class QTCON_EXPORT caLabel : public ESimpleLabel
{
    Q_OBJECT

    Q_PROPERTY(QColor foreground READ getForeground WRITE setForeground)
    Q_PROPERTY(QColor background READ getBackground WRITE setBackground)
    Q_PROPERTY(QColor borderColor READ getBorderColor WRITE setBorderColor)
    Q_PROPERTY(int borderWidth READ getBorderWidth WRITE setBorderWidth)

    Q_PROPERTY(colMode colorMode READ getColorMode WRITE setColorMode)

    Q_PROPERTY(Shape frameShape READ frameShape WRITE setFrameShape DESIGNABLE false)
    Q_PROPERTY(Shadow frameShadow READ frameShadow WRITE setFrameShadow DESIGNABLE false)
    Q_PROPERTY(int lineWidth READ lineWidth WRITE setLineWidth DESIGNABLE false)
    Q_PROPERTY(int midLineWidth READ midLineWidth WRITE setMidLineWidth DESIGNABLE false)
    Q_PROPERTY(int frameWidth READ frameWidth DESIGNABLE false)

    Q_ENUMS(colMode)

#include "caVisibProps.h"
#include "caVisibDefs.h"



public:
    caLabel( QWidget *parent = 0 );

    QColor getForeground() const {return thisForeColor;}
    void setForeground(QColor c);

    QColor getBackground() const {return thisBackColor;}
    void setBackground(QColor c);

    QColor getBorderColor() const {return thisBorderColor;}
    void setBorderColor(QColor c);

    enum colMode {Static, Alarm, Default};
    colMode getColorMode() const { return thisColorMode; }

    void setColors(QColor bg, QColor fg);

    void setBorderWidth(int width) {thisBorderWidth = width; setColors(thisBackColor, thisForeColor);}
    int getBorderWidth() const {return thisBorderWidth;}

    void setAlarmColors(short status);

    void setColorMode(colMode colormode) {thisColorMode = colormode;
                                          setBackground(thisBackColor);
                                          setForeground(thisForeColor);}
public slots:
    void animation(QRect p) {
#include "animationcode.h"
    }

    void hideObject(bool hideit) {
#include "hideobjectcode.h"
    }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QColor thisForeColor;
    QColor thisBackColor;
    QColor thisBorderColor;
    int thisBorderWidth;
    colMode thisColorMode;
};

#endif
