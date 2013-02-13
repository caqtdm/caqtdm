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

    enum  Visibility { StaticV,
      IfNotZero,
      IfZero,
      Calc
    };

    Visibility getVisibility() const {return thisVisibility;}
    void setVisibility(Visibility s) {thisVisibility = s;}

    QString getVisibilityCalc() const {return thisVisibilityCalc;}
    void setVisibilityCalc(QString const &calc) {thisVisibilityCalc = calc;}

    QString getChannelA() const {return thisChannelA;}
    void setChannelA(QString const &pv) {thisChannelA = pv;}

    QString getChannelB() const {return thisChannelB;}
    void setChannelB(QString const &pv) {thisChannelB = pv;}

    QString getChannelC() const {return thisChannelC;}
    void setChannelC(QString const &pv) {thisChannelC = pv;}

    QString getChannelD() const {return thisChannelD;}
    void setChannelD(QString const &pv) {thisChannelD = pv;}

