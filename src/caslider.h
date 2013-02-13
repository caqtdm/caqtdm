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

#ifndef CASLIDER_H
#define CASLIDER_H

#include <qtcontrols_global.h>
#include <qwt_slider.h>

class QTCON_EXPORT caSlider : public QwtSlider
{
    Q_OBJECT
    Q_PROPERTY(QString channel READ getPV WRITE setPV)
    Q_PROPERTY(Direction direction READ getDirection WRITE setDirection)
    Q_ENUMS(Direction)

    Q_PROPERTY(QColor foreground READ getForeground WRITE setForeground)
    Q_PROPERTY(QColor background READ getBackground WRITE setBackground)

    Q_PROPERTY(colMode colorMode READ getColorMode WRITE setColorMode)
    Q_ENUMS(colMode)

    Q_PROPERTY(SourceMode limitsMode READ getLimitsMode WRITE setLimitsMode)
    Q_ENUMS(SourceMode)

    Q_PROPERTY(double maxValue READ getMaxValue WRITE setMaxValue)
    Q_PROPERTY(double minValue READ getMinValue WRITE setMinValue)
    Q_PROPERTY(double value READ getSliderValue WRITE setSliderValue)

public:

    QString getPV() const;
    void setPV(QString const &newPV);

    enum Direction {Up, Down, Left, Right};
    Direction getDirection() const { return thisDirection; }
    void setDirection(Direction direction);

    QColor getForeground();
    void setForeground(QColor c);
    QColor getBackground();
    void setBackground(QColor c);

    enum colMode {Default, Static, Alarm};
    colMode getColorMode() const { return thisColorMode; }

    void setColorMode(colMode colormode) {thisColorMode = colormode;
                                          setBackground(thisBackColor);
                                          setForeground(thisForeColor);
                                          oldColorMode = thisColorMode;
                                           }

    enum SourceMode {Channel = 0, User};

    SourceMode getLimitsMode() const { return thisLimitsMode; }
    void setLimitsMode(SourceMode limitsmode) { thisLimitsMode = limitsmode;}

    int getAccessW() const {return thisAccessW;}
    void setAccessW(int access);

    double getMaxValue()  const {return thisMaximum;}
    void setMaxValue(double const &maxim);

    double getMinValue()  const {return thisMinimum;}
    void setMinValue(double const &minim);

    double getSliderValue()  const {return thisValue;}
    void setSliderValue(double const &value);

    caSlider(QWidget *parent);

    void setAlarmColors(short status);
    void setUserAlarmColors(double val);
    void setColors(QColor bg, QColor fg);
    void setNormalColors();

private slots:


protected:

private:
    QString thisPV;
    double thisMaximum, thisMinimum, thisValue;
    bool thisAccessW;
    Direction      thisDirection;
    bool eventFilter(QObject *obj, QEvent *event);

    QColor thisForeColor, oldForeColor;
    QColor thisBackColor, oldBackColor;
    QPalette thisPalette;
    colMode thisColorMode;
    colMode oldColorMode;
    SourceMode thisLimitsMode;

    QColor defaultBackColor;
    QColor defaultForeColor;

};

#endif
