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
      QString path = (QString) qgetenv("CAQTDM_DISPLAY_PATH");
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
    return (QString)  qgetenv("CAQTDM_DISPLAY_PATH");
}
