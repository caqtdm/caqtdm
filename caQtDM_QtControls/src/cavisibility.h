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

#ifndef CAVISIBILITY_H
#define CAVISIBILITY_H

#include <QWidget>
#include <qtcontrols_global.h>

class  QTCON_EXPORT caVisibility : public QWidget
{
    Q_OBJECT
    
    Q_PROPERTY(Visibility visibility READ getVisibility WRITE setVisibility)
    Q_PROPERTY(QString visibilityCalc READ getVisibilityCalc WRITE setVisibilityCalc)
    Q_PROPERTY(QString channel READ getChannelA WRITE setChannelA)
    Q_PROPERTY(QString channelB READ getChannelB WRITE setChannelB)
    Q_PROPERTY(QString channelC READ getChannelC WRITE setChannelC)
    Q_PROPERTY(QString channelD READ getChannelD WRITE setChannelD)
    Q_ENUMS(Visibility)

public:
  
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
    
    void setHidden(bool hide);
    
    bool getHidden();

    caVisibility( QWidget *parent = 0 );

private:

    QString thisChannelA, thisChannelB, thisChannelC, thisChannelD;
    Visibility thisVisibility;
    QString thisVisibilityCalc;
    bool thisHide;
};

#endif 
