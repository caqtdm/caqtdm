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

    Q_PROPERTY(EPushButton::ScaleMode fontScaleMode READ fontScaleMode WRITE setFontScaleMode)

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

     void setFontScaleMode(EPushButton::ScaleMode m);
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
    QStringList   texts, labels, lastLabels;
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
