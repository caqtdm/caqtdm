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

#ifndef CASHELLCOMMAND_H
#define CASHELLCOMMAND_H

#include <qtcontrols_global.h>
#include <carowcolmenu.h>

class QTCON_EXPORT caShellCommand : public caRowColMenu
{
    Q_OBJECT

public:

    caShellCommand(QWidget *parent);

};

#endif
