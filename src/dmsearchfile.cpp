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

#include <QString>
#include "dmsearchfile.h"
#include <QDebug>

dmsearchFile::dmsearchFile(QString filename)
{
   _FileName = filename;
}

QString dmsearchFile::findFile()
{
    QString path = (QString)  getenv("CAQTDM_DISPLAY_PATH");
    QString FileName = _FileName;
    bool fileFound = false;
    QFileInfo fi(FileName);
    if(!fi.exists()) {
       FileName = path + "/" + _FileName;
       QFileInfo fi(FileName);
       if(fi.exists()) {
         fileFound = true;
       }
    } else {
        fileFound = true;
    }
    if(fileFound) return FileName;
    else return NULL;
}

QString dmsearchFile::displayPath()
{
    return (QString)  getenv("CAQTDM_DISPLAY_PATH");
}
