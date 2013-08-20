//******************************************************************************
// Copyright (c) 2012 Paul Scherrer Institut PSI), Villigen, Switzerland
// Disclaimer: neither  PSI, nor any of their employees makes any warranty
// or assumes any legal liability or responsibility for the use of this software
//******************************************************************************
//******************************************************************************
//
//     Author : Anton Chr. Mezger
//
//********************************************************************************

#ifndef CAPOLYLINETASKMENU_H
#define CAPOLYLINETASKMENU_H

#include <QtDesigner/QDesignerTaskMenuExtension>
#include <QtDesigner/QExtensionFactory>
#include <qtcontrols_global.h>
#include <QAction>

QT_BEGIN_NAMESPACE
class QAction;
class QExtensionManager;
QT_END_NAMESPACE
class caPolyLine;

class QTCON_EXPORT caPolyLineTaskMenu : public QObject, public QDesignerTaskMenuExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerTaskMenuExtension)

public:
    caPolyLineTaskMenu(caPolyLine *tic, QObject *parent);

    QAction *preferredEditAction() const;
    QList<QAction *> taskActions() const;

private slots:
    void editState();

private:
    QAction *editStateAction;
    caPolyLine *capolyLine;
};

class QTCON_EXPORT caPolyLineTaskMenuFactory : public QExtensionFactory
{
    Q_OBJECT

public:
    caPolyLineTaskMenuFactory(QExtensionManager *parent = 0);

protected:
    QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};


#endif
