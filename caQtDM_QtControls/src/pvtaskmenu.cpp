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
#include <QtControls>
#include "pvdialog.h"
#include "pvtaskmenu.h"

PVTaskMenu::PVTaskMenu(QWidget *tic, QObject *parent) : QObject(parent)
{
    pvWidget = tic;
    editStateAction = new QAction(tr("Edit PV..."), this);
    connect(editStateAction, SIGNAL(triggered()), this, SLOT(editState()));
}

void PVTaskMenu::editState()
{
    PVDialog dialog(pvWidget);
    dialog.exec();
}

QAction *PVTaskMenu::preferredEditAction() const
{
    return editStateAction;
}

QList<QAction *> PVTaskMenu::taskActions() const
{
    QList<QAction *> list;
    list.append(editStateAction);
    return list;
}

PVTaskMenuFactory::PVTaskMenuFactory(QExtensionManager *parent)
    : QExtensionFactory(parent)
{
}

QObject *PVTaskMenuFactory::createExtension(QObject *object,
                                                   const QString &iid,
                                                   QObject *parent) const
{
    if (iid != Q_TYPEID(QDesignerTaskMenuExtension)) return 0;

    if (caLed *tic = qobject_cast<caLed*>(object))
            return new PVTaskMenu(tic, parent);
    else if (caLinearGauge *tic = qobject_cast<caLinearGauge*>(object))
        return new PVTaskMenu(tic, parent);
    else if (caCircularGauge *tic = qobject_cast<caCircularGauge*>(object))
        return new PVTaskMenu(tic, parent);
    else if (caMeter *tic = qobject_cast<caMeter*>(object))
        return new PVTaskMenu(tic, parent);
    else if (caLineEdit *tic = qobject_cast<caLineEdit*>(object))
        return new PVTaskMenu(tic, parent);
    else if (caMultiLineString *tic = qobject_cast<caMultiLineString*>(object))
        return new PVTaskMenu(tic, parent);
    else if (caThermo *tic = qobject_cast<caThermo*>(object))
        return new PVTaskMenu(tic, parent);
    else if (caByte *tic = qobject_cast<caByte*>(object))
        return new PVTaskMenu(tic, parent);
    else if (caWaveTable *tic = qobject_cast<caWaveTable*>(object))
        return new PVTaskMenu(tic, parent);
    else if (caCamera *tic = qobject_cast<caCamera*>(object))
        return new PVTaskMenu(tic, parent);
    else if (caWaterfallPlot *tic = qobject_cast<caWaterfallPlot*>(object))
        return new PVTaskMenu(tic, parent);
    else if (caLineDraw *tic = qobject_cast<caLineDraw*>(object))
        return new PVTaskMenu(tic, parent);

    else if (caNumeric *tic = qobject_cast<caNumeric*>(object))
        return new PVTaskMenu(tic, parent);
    else if (caApplyNumeric *tic = qobject_cast<caApplyNumeric*>(object))
        return new PVTaskMenu(tic, parent);
    else if (caSlider *tic = qobject_cast<caSlider*>(object))
        return new PVTaskMenu(tic, parent);
    else if (caMenu *tic = qobject_cast<caMenu*>(object))
        return new PVTaskMenu(tic, parent);
    else if (caChoice *tic = qobject_cast<caChoice*>(object))
        return new PVTaskMenu(tic, parent);
    else if (caTextEntry *tic = qobject_cast<caTextEntry*>(object))
        return new PVTaskMenu(tic, parent);
    else if (caMessageButton *tic = qobject_cast<caMessageButton*>(object))
        return new PVTaskMenu(tic, parent);
    else if (caToggleButton *tic = qobject_cast<caToggleButton*>(object))
        return new PVTaskMenu(tic, parent);
    else if (caSpinbox *tic = qobject_cast<caSpinbox*>(object))
        return new PVTaskMenu(tic, parent);
    else if (caByteController *tic = qobject_cast<caByteController*>(object))
        return new PVTaskMenu(tic, parent);

    return 0;
}

