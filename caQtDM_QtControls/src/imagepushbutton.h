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
