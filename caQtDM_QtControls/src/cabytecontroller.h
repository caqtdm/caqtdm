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

#ifndef CABYTECONTROLLER_H
#define CABYTECONTROLLER_H

#include <QString>
#include <QColor>
#include <QSignalMapper>
#include <QApplication>
#include <qtcontrols_global.h>
#include "epushbutton.h"

class QGridLayout;

class QTCON_EXPORT caByteController : public QWidget
{
    Q_OBJECT

    Q_ENUMS(Direction)
    Q_ENUMS(colMode)

    Q_PROPERTY(QString channel READ getPV WRITE setPV)

    Q_PROPERTY(Direction direction READ getDirection WRITE setDirection)

    Q_PROPERTY(int startBit READ getStartBit WRITE setStartBit)
    Q_PROPERTY(int endBit   READ getEndBit   WRITE setEndBit)

    Q_PROPERTY(colMode colorMode READ getColorMode WRITE setColorMode)

    Q_PROPERTY(QColor foreground  READ getTrueColor  WRITE setTrueColor)
    Q_PROPERTY(QColor background  READ getFalseColor WRITE setFalseColor)
    Q_PROPERTY(QColor textColor  READ getTextColor   WRITE setTextColor)

    Q_PROPERTY(EPushButton::ScaleMode fontScaleMode READ fontScaleMode WRITE setFontScaleModeL)

#include "caElevation.h"

public:

    enum colMode {Static=0, Alarm};
    enum Direction {Up, Down, Left, Right};

    Direction getDirection() const { return thisDirection; }
    void setDirection(Direction direction);

    colMode getColorMode() const { return thisColorMode; }
    void setColorMode(colMode colormode) {thisColorMode = colormode;}

    caByteController(QWidget *parent);

    QString getPV() const;
    void setPV(QString const &newPV);

    void setTrueColor(QColor c);
    QColor getTrueColor() const {return thisTrueColor;}

    void setFalseColor(QColor c);
    QColor getFalseColor() const {return thisFalseColor;}

    void setTextColor(QColor c);
    QColor getTextColor() const {return thisTextColor;}

    QList<EPushButton*> cells;
    void setValue(long value);
    long getValue();

    int getStartBit()  const {return thisStartBit;}
    void setStartBit(int const &bit);

    int getEndBit()  const {return thisEndBit;}
    void setEndBit(int const &bit);

    bool bitState(long value, int bitNr);

    void setAlarmColors(short status);

     void drawByte(long lvalue, QColor trueColor, QColor falseColor);

     void setFontScaleModeL(EPushButton::ScaleMode m);
     EPushButton::ScaleMode fontScaleMode();

     bool getAccessW() const {return _AccessW;}
     void setAccessW(bool access);

signals:
    void clicked(int bit);

protected:
    void arrangeCells();
    void resizeEvent(QResizeEvent *e);

private:
    void   setColor(EPushButton *button, QColor c, QColor text);
    bool   eventFilter(QObject *obj, QEvent *event);
    int                 numRows;
    QSignalMapper      *signalMapper;
    QColor		        thisFalseColor;
    QColor		        thisTrueColor;
     QColor		        thisTextColor;
    QGridLayout 	   *grid;
    int                 thisStartBit;
    int                 thisEndBit;
    QString             thisPV;
    Direction           thisDirection;
    colMode             thisColorMode;
    long                thisValue;
    bool               _AccessW;
    EPushButton::ScaleMode thisScaleMode;
};

#endif  /* CABYTECONTROLLER */
