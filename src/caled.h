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

#ifndef _CALED_H
#define _CALED_H

#include <QWidget>
#include <qtcontrols_global.h>
#include <eled.h>

class QTCON_EXPORT caLed : public ELed
{
Q_OBJECT

  Q_PROPERTY(QString channel READ getPV WRITE setPV)
  Q_PROPERTY(int bitNr READ getBitNr WRITE setBitNr)
  Q_PROPERTY(colMode colorMode READ getColorMode WRITE setColorMode)
  Q_PROPERTY(QColor falseColor READ getFalseColor WRITE setFalseColor)
  Q_PROPERTY(QColor trueColor READ getTrueColor WRITE setTrueColor)
  Q_ENUMS(colMode)

public:

    enum colMode {Static=0, Alarm};

	caLed(QWidget*);
        virtual ~caLed(){}

        QString getPV() const;
        void setPV(QString const &newPV);
        int getBitNr() const;
        void setBitNr(int const newBitNr);

        QColor getFalseColor() const {return thisFalseColor;}
        void setFalseColor(QColor c);

        QColor getTrueColor() const {return thisTrueColor;}
        void setTrueColor(QColor c);

        colMode getColorMode() const { return thisColorMode; }
        void setColorMode(colMode colormode);

        void setState(bool state);

        void setAlarmColors(short status);

private:
        QString thisPV;
        int thisBitNr;
        QColor thisFalseColor, thisTrueColor;
        colMode thisColorMode;
};

#endif
