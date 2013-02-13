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

#ifndef CALABEL_H
#define CALABEL_H

#include <qtcontrols_global.h>
#include <ELabel>

class QTCON_EXPORT caLabel : public ESimpleLabel
{

    Q_PROPERTY(QColor foreground READ getForeground WRITE setForeground)
    Q_PROPERTY(QColor background READ getBackground WRITE setBackground)

    Q_PROPERTY(colMode colorMode READ getColorMode WRITE setColorMode)
    Q_ENUMS(colMode)

#include "caVisib.h"

    Q_OBJECT

public:

#include "caVisibPublic.h"

    caLabel( QWidget *parent = 0 );

    QColor getForeground() const {return thisForeColor;}
    void setForeground(QColor c);

    QColor getBackground() const {return thisBackColor;}
    void setBackground(QColor c);

    enum colMode {Static, Alarm};
    colMode getColorMode() const { return thisColorMode; }

    void setColors(QColor bg, QColor fg);

    void setAlarmColors(short status);

    void setColorMode(colMode colormode) {thisColorMode = colormode;
                                          setBackground(thisBackColor);
                                          setForeground(thisForeColor);
                                          oldColorMode = thisColorMode;
                                           }

private slots:


protected:


private:

#include "caVisibPrivate.h"

    QColor thisForeColor, thisForeColorOld;
    QColor thisBackColor, thisBackColorOld;
    colMode thisColorMode;
    colMode oldColorMode;
    QString thisStyle, oldStyle;
};

#endif
