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

#ifndef caRelatedDisplay_H
#define caRelatedDisplay_H

#include <qtcontrols_global.h>
#include <carowcolmenu.h>
#include "caPropHandleDefs.h"

class QTCON_EXPORT caRelatedDisplay : public caRowColMenu
{

    Q_OBJECT

    Q_PROPERTY(Stacking stackingMode READ getStacking WRITE setStacking)
    Q_PROPERTY(QStringList removeParentList READ getRemoveList WRITE setRemoveList STORED false)
    Q_PROPERTY(QString removeParent READ getReplaceModes WRITE setReplaceModes DESIGNABLE inactiveButVisible())

    // this will prevent user interference
    Q_PROPERTY(QString styleSheet READ styleSheet WRITE noStyle DESIGNABLE false)

#include "caElevation.h"

public:

#include "caPropHandle.h"

    void noStyle(QString style) {Q_UNUSED(style);}

    caRelatedDisplay(QWidget *parent);

    QString getReplaceModes() const {return replacemodes.join(";");}
    void setReplaceModes(QString const &newL) {replacemodes = newL.split(";");}
    QStringList getRemoveList() const {return replacemodes;}
    void setRemoveList(QStringList list) {replacemodes = list; updatePropertyEditorItem(this, "removeParent");}

private:

    QStringList  replacemodes;

};

#endif
