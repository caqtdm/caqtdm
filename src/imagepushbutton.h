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

#ifndef IMAGEPUSHBUTTON_H
#define IMAGEPUSHBUTTON_H

#include <QString>
#include "epushbutton.h"
#include "searchfile.h"

class ImagePushButton : public EPushButton
{

public:

    ImagePushButton(const QString& text, const QString& image, QWidget *parent = 0);
    ~ImagePushButton(){}

    void setLabelText(const QString& text);
    void setIconVisible(bool b);
    void setInVisible(QColor bg, QColor fg, QColor bc);

protected:

  void paintEvent( QPaintEvent* event);
  void resizeEvent(QResizeEvent *e);

private:
   QString myText;
   QString myImage;
   bool iconPresent;
   QPixmap pixmap;
   bool iconOK;
   bool invisible;
   QColor thisbg, thisfg, thisbc;
};

#endif
