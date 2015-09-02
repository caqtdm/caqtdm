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

#ifndef CACHOICE_H
#define CACHOICE_H

#include <QString>
#include <QSignalMapper>
#include <qtcontrols_global.h>
#include "epushbutton.h"
#include "elabel.h"

class QGridLayout;

class QTCON_EXPORT caChoice : public QWidget
{
    Q_OBJECT

    Q_ENUMS(Stacking)
    Q_ENUMS(colMode)
    Q_ENUMS(alignmentHor)

    Q_PROPERTY(QString channel READ getPV WRITE setPV)

    Q_PROPERTY(QColor foreground READ getForeground WRITE setForeground)
    Q_PROPERTY(QColor background READ getBackground WRITE setBackground)
    Q_PROPERTY(QColor bordercolor READ getBordercolor WRITE setBordercolor)
    Q_PROPERTY(alignmentHor alignment READ getAlignment WRITE setAlignment)

    Q_PROPERTY(colMode colorMode READ getColorMode WRITE setColorMode)
    Q_PROPERTY(Stacking stackingMode READ getStacking WRITE setStacking)

    Q_PROPERTY(int startBit READ getStartBit WRITE setStartBit)
    Q_PROPERTY(int endBit   READ getEndBit   WRITE setEndBit)

    Q_PROPERTY(EPushButton::ScaleMode fontScaleMode READ fontScaleMode WRITE setFontScaleModeL)

#include "caElevation.h"

public:

    enum alignmentHor {left, right, center};
    enum colMode {Default, Static, Alarm};
    enum Stacking {Row, Column, RowColumn};

    Stacking getStacking() const { return thisStacking; }
    void setStacking(Stacking stacking);

    caChoice(QWidget *parent);

    QString getPV() const;
    void setPV(QString const &newPV);

    void populateCells(QStringList list, int indx);

    int getAccessW() const {return thisAccessW;}
    void setAccessW(int access);

    int getStartBit()  const {return thisStartBit;}
    void setStartBit(int const &bit);

    int getEndBit()  const {return thisEndBit;}
    void setEndBit(int const &bit);

    colMode getColorMode() const { return thisColorMode; }
    void setColorMode(colMode colormode);

     void setAlarmColors(short status);
     void setColors(QColor back, QColor fore, QColor border, alignmentHor alignment);
     void setNormalColors();

     QColor getForeground() const {return thisForeColor;}
     void setForeground(QColor c);

     QColor getBackground() const {return thisBackColor;}
     void setBackground(QColor c);

     QColor getBordercolor() const {return thisBorderColor;}
     void setBordercolor(QColor c);

     alignmentHor getAlignment() const {return thisAlignment;}
     void setAlignment(alignmentHor alignment);

     void setFontScaleModeL(EPushButton::ScaleMode m);
     EPushButton::ScaleMode fontScaleMode();

     void updateChoice();

protected:

    void arrangeCells(QStringList list, int indx);
    void resizeEvent(QResizeEvent *e);

signals:

    void clicked(QString text);

private:

    QGridLayout  *grid;
    QString      thisPV;
    Stacking     thisStacking;
    int          numCells;
    bool         thisAccessW;
    bool eventFilter(QObject *obj, QEvent *event);
    QSignalMapper *signalMapper;
    QStringList   texts, labels;
    QList<EPushButton*> cells;
    int         thisStartBit;
    int         thisEndBit;

    colMode     thisColorMode, oldColorMode;
    QColor      thisForeColor, oldForeColor;
    QColor      thisBackColor, oldBackColor;
    QColor      thisBorderColor, oldBorderColor;

    alignmentHor thisAlignment, oldAlignment;
    EPushButton::ScaleMode thisScaleMode;

    int lastValue;
};

#endif  /* CACHOICE */
