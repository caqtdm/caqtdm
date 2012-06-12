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

#ifndef DMSEARCHFILE_H
#define DMSEARCHFILE_H

#include <QString>
#include <QFileDialog>

class dmsearchFile
{
public:
    dmsearchFile(QString FileName);
    QString findFile();
    QString displayPath();

private:
    QString _FileName;
};

#endif // DMSEARCHFILE_H
