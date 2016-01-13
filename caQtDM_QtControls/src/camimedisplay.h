

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

#ifndef caMimeDisplay_H
#define caMimeDisplay_H

#include <QString>
#include <QMessageBox>
#include <QDesktopServices>
#include <QEvent>
#include <QUrl>
#include <qtcontrols_global.h>
#include <carowcolmenu.h>
#include "pathdefinitions.h"

class QTCON_EXPORT caMimeDisplay : public caRowColMenu
{
    Q_OBJECT

    Q_PROPERTY(Stacking stackingMode READ getStacking WRITE setStacking)
    Q_PROPERTY(QString args READ getArgs WRITE setArgs DESIGNABLE false)

#include "caElevation.h"

public:
    caMimeDisplay(QWidget *parent);

private:
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void Callback_Clicked(int);
};

#endif
