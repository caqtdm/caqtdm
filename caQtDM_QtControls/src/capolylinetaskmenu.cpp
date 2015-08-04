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

#include <QtDesigner/QtDesigner>
#include "capolyline.h"
#include "capolylinedialog.h"
#include "capolylinetaskmenu.h"

caPolyLineTaskMenu::caPolyLineTaskMenu(caPolyLine *tic, QObject *parent)
    : QObject(parent)
{
    capolyLine = tic;

    editStateAction = new QAction(tr("Edit PolyLine..."), this);
    connect(editStateAction, SIGNAL(triggered()), this, SLOT(editState()));
}

void caPolyLineTaskMenu::editState()
{
    caPolyLineDialog dialog(capolyLine);
    dialog.exec();
}

QAction *caPolyLineTaskMenu::preferredEditAction() const
{
    return editStateAction;
}

QList<QAction *> caPolyLineTaskMenu::taskActions() const
{
    QList<QAction *> list;
    list.append(editStateAction);
    return list;
}

caPolyLineTaskMenuFactory::caPolyLineTaskMenuFactory(QExtensionManager *parent)
    : QExtensionFactory(parent)
{
}

QObject *caPolyLineTaskMenuFactory::createExtension(QObject *object,
                                                   const QString &iid,
                                                   QObject *parent) const
{
    if (iid != Q_TYPEID(QDesignerTaskMenuExtension))
        return 0;

    if (caPolyLine *tic = qobject_cast<caPolyLine*>(object))
        return new caPolyLineTaskMenu(tic, parent);

    return 0;
}

