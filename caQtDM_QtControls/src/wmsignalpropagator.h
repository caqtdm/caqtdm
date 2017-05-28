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

#ifndef WMSIGNALPROPAGATOR_H
#define WMSIGNALPROPAGATOR_H

#include <QWidget>
#include "esimplelabel.h"
#include <qtcontrols_global.h>


class  QTCON_EXPORT wmSignalPropagator : public  ESimpleLabel
{
    Q_OBJECT

public:
    wmSignalPropagator( QWidget *parent = 0 );

public slots:
    void animation(QRect p) {
#include "animationcode.h"
    }

public slots:
    void closewindow();
    void shownormal();
    void showmaximized();
    void showminimized();
    void showfullscreen();

signals:
    void wmCloseWindow();
    void wmShowNormal();
    void wmShowMaximized();
    void wmShowMinimized();
    void wmShowFullScreen();

private:
    void setForeAndBackground(QColor fg, QColor bg);
};

#endif 
