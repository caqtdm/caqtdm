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

#ifndef CAFRAME_H
#define CAFRAME_H

#include <QFrame>
#include <qtcontrols_global.h>

class QTCON_EXPORT caFrame : public QFrame
{
    Q_PROPERTY(QString macro READ getMacro WRITE setMacro)
    Q_PROPERTY(QColor background READ getBackground WRITE setBackground)
    Q_PROPERTY(BackgroundMode backgroundMode READ getBackgroundMode WRITE setBackgroundMode)

    Q_PROPERTY(Visibility visibility READ getVisibility WRITE setVisibility)
    Q_PROPERTY(VisibilityMode visibilityMode READ getVisibilityMode WRITE setVisibilityMode)
    Q_PROPERTY(QString visibilityCalc READ getVisibilityCalc WRITE setVisibilityCalc)
    Q_PROPERTY(QString channel READ getChannelA WRITE setChannelA)
    Q_PROPERTY(QString channelB READ getChannelB WRITE setChannelB)
    Q_PROPERTY(QString channelC READ getChannelC WRITE setChannelC)
    Q_PROPERTY(QString channelD READ getChannelD WRITE setChannelD)
    Q_ENUMS(Visibility)
    Q_ENUMS(VisibilityMode)
    Q_ENUMS(BackgroundMode)

    Q_OBJECT

public:

    enum  Visibility { StaticV,
         IfNotZero,
         IfZero,
         Calc
       };

    enum VisibilityMode {All, Background};
    enum BackgroundMode {Outline, Filled};

       Visibility getVisibility() const {return thisVisibility;}
       void setVisibility(Visibility s) {thisVisibility = s;}

       VisibilityMode getVisibilityMode() const {return thisVisibilityMode;}
       void setVisibilityMode(VisibilityMode s) {thisVisibilityMode = s;}

       BackgroundMode getBackgroundMode() const {return thisBackgroundMode;}
       void setBackgroundMode(BackgroundMode s) {thisBackgroundMode = s;
                                                 setBackground(thisBackColor);}

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


    caFrame(QWidget * parent = 0);

    QString getMacro() const {return thisMacro.join(";");}
    void setMacro(QString const &newMacro) {thisMacro = newMacro.split(";");}
    void setBackground(QColor c);
    QColor getBackground() const {return thisBackColor;}

protected:

public slots:

signals:

private:

   QString thisChannelA, thisChannelB, thisChannelC, thisChannelD;
   Visibility thisVisibility;
   BackgroundMode thisBackgroundMode;
   VisibilityMode thisVisibilityMode;
   QString thisVisibilityCalc;
   QStringList thisMacro;
   QColor thisBackColor;
};

#endif
