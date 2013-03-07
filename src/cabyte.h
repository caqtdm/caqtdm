//******************************************************************************
// Copyright (c) 2012 Paul Scherrer Institut PSI), Villigen, Switzerland
// Disclaimer: neither  PSI, nor any of their employees makes any warranty
// or assumes any legal liability or responsibility for the use of this software
//******************************************************************************
//******************************************************************************
//
//     Author : Anton Chr. Mezger
//
//******************************************************************************

#ifndef CABYTE_H
#define CABYTE_H

#include <QString>
#include <QColor>
#include <qtcontrols_global.h>
#include <rectangle.h>

class QGridLayout;

class QTCON_EXPORT caByte : public QWidget
{
    Q_OBJECT

    Q_ENUMS(Direction)
    Q_ENUMS(colMode)

    Q_PROPERTY(QString channel READ getPV WRITE setPV)

    Q_PROPERTY(Direction direction READ getDirection WRITE setDirection)

    Q_PROPERTY(int startBit READ getStartBit WRITE setStartBit)
    Q_PROPERTY(int endBit   READ getEndBit   WRITE setEndBit)

    Q_PROPERTY(colMode colorMode READ getColorMode WRITE setColorMode)

    Q_PROPERTY(QColor foreground   READ getTrueColor     WRITE setTrueColor)
    Q_PROPERTY(QColor background  READ getFalseColor    WRITE setFalseColor)

public:

    enum colMode {Static=0, Alarm};
    enum Direction {Up, Down, Left, Right};

    Direction getDirection() const { return thisDirection; }
    void setDirection(Direction direction);

    colMode getColorMode() const { return thisColorMode; }
    void setColorMode(colMode colormode) {thisColorMode = colormode;}

    caByte(QWidget *parent);

    QString getPV() const;
    void setPV(QString const &newPV);

    void setEnumDisplay(int cell, unsigned int value, QString corresponding_str, QColor corresponding_color);

    void setTrueColor(QColor c);
    QColor getTrueColor() const {return thisTrueColor;}

    void setFalseColor(QColor c);
    QColor getFalseColor() const {return thisFalseColor;}

    QList<rectangle*> cells;
    void setValue(int value);

    int getStartBit()  const {return thisStartBit;}
    void setStartBit(int const &bit);

    int getEndBit()  const {return thisEndBit;}
    void setEndBit(int const &bit);

    bool bitState(long value, int bitNr);

    void setAlarmColors(short status);

     void drawByte(long lvalue, QColor trueColor, QColor falseColor);

protected:

    void arrangeCells();
    void resizeEvent(QResizeEvent *e);

private:
    int                 numRows;
    int 	            numColumns;

    QColor		        thisFalseColor;
    QColor		        thisTrueColor;
    QGridLayout 	   *grid;
    int                 thisStartBit;
    int                 thisEndBit;
    QString             thisPV;
    Direction           thisDirection;
    colMode             thisColorMode;
    long                thisValue;

};

#endif  /* CABYTE */
