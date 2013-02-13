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

#include <QtDesigner/QtDesigner>
#include <QtGui>

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

