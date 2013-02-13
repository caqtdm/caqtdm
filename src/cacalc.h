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

#ifndef CACALC_H
#define CACALC_H

#include <QWidget>
#include <QLabel>
#include <qtcontrols_global.h>


class  QTCON_EXPORT caCalc : public  QLabel
{
    Q_OBJECT

    Q_PROPERTY(QString variable READ getVariable WRITE setVariable)
    Q_PROPERTY(QString calc READ getCalc WRITE setCalc)
    Q_PROPERTY(QString channel READ getChannelA WRITE setChannelA)
    Q_PROPERTY(QString channelB READ getChannelB WRITE setChannelB)
    Q_PROPERTY(QString channelC READ getChannelC WRITE setChannelC)
    Q_PROPERTY(QString channelD READ getChannelD WRITE setChannelD)
    Q_PROPERTY(double initialValue READ getInitialValue WRITE setInitialValue)

public:


    QString getVariable() const {return thisVariable;}
    void setVariable(QString const &var) {thisVariable = var;}

    QString getCalc() const {return thisCalc;}
    void setCalc(QString const &calc) {thisCalc = calc;}

    QString getChannelA() const {return thisChannelA;}
    void setChannelA(QString const &pv) {thisChannelA = pv;}

    QString getChannelB() const {return thisChannelB;}
    void setChannelB(QString const &pv) {thisChannelB = pv;}

    QString getChannelC() const {return thisChannelC;}
    void setChannelC(QString const &pv) {thisChannelC = pv;}

    QString getChannelD() const {return thisChannelD;}
    void setChannelD(QString const &pv) {thisChannelD = pv;}

    double getInitialValue() const {return thisValue;}
    void setInitialValue(double const &value) {thisValue = value;}

    caCalc( QWidget *parent = 0 );

    void setValue(double value);
    void setText(const QString &txt);
    void setForeAndBackground(QColor fg, QColor bg);


public slots:


signals:


protected:


private:

    QString thisChannelA, thisChannelB, thisChannelC, thisChannelD, thisVariable;
    QString thisCalc;
    QString keepText;
    double thisValue;
};

#endif // CACALC_H
