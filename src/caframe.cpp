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

#include <QtGui>

#include "caframe.h"

caFrame::caFrame(QWidget *parent) : QFrame(parent)
{
    thisBackColor = Qt::gray;
    thisBackgroundMode = Outline;
    setVisibility(StaticV);
    setVisibilityMode(All);
    setBackground(thisBackColor);
}


void caFrame::setBackground(QColor c)
{
    thisBackColor = c;
    QColor thisBackgroundColor = c;
    QColor thisLightColor = c.lighter();
    QColor thisDarkColor = c.darker();

    QPalette thisPalette = palette();
    thisPalette.setColor(QPalette::WindowText, thisBackColor);
    thisPalette.setColor(QPalette::Light, thisLightColor);
    thisPalette.setColor(QPalette::Dark, thisDarkColor);
    thisPalette.setColor(QPalette::Window, thisBackgroundColor);
    setPalette(thisPalette);
    if(thisBackgroundMode == Filled) {
        setAutoFillBackground(true);
    } else {
        setAutoFillBackground(false);
    }
    update();
}


