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
    if(_FileName.isNull()) return NULL;

    QString path = (QString)  getenv("CAQTDM_DISPLAY_PATH");
#ifdef _MSC_VER
    QStringList paths = path.split(";");
#else
    QStringList paths = path.split(":");
#endif
    // first search in current directory
    QString FileName = _FileName;
    bool fileFound = false;
    QFileInfo fi(FileName);

    // file was not found, go through path list
    if(!fi.exists()) {
       for(int i=0; i< paths.count(); i++) {
           FileName = paths[i] + "/" + _FileName;
           QFileInfo fi(FileName);
           if(fi.exists()) {
             fileFound = true;
             break;
           }
        }

    // file was found in current directory
    } else {
        fileFound = true;
    }

    // return filename or null
    if(fileFound) {
        //printf("dmsearchFile %s\n", FileName.toAscii().constData());
        return FileName;
    }
    else return NULL;
}

QString dmsearchFile::displayPath()
{
    return (QString)  getenv("CAQTDM_DISPLAY_PATH");
}
