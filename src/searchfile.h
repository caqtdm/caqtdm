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

#ifndef SEARCHFILE_H
#define SEARCHFILE_H

#include <QString>
#include <QFileDialog>
#include <qtcontrols_global.h>

class QTCON_EXPORT searchFile
{
public:
    searchFile(QString FileName);
    QString findFile();

private:
    QString _FileName;
};

#endif // SEARCHFILE_H
