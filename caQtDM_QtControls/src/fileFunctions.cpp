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

#include "fileFunctions.h"
#include "networkaccess.h"
#include "searchfile.h"
#include "specialFunctions.h"

fileFunctions::fileFunctions()
{
}

const QString fileFunctions::lastError()
{
    if(errorString.length() > 0) return errorString;
    else return QString::null;
}

const QString fileFunctions::lastInfo()
{
    if(infoString.length() > 0) return "Info: " + infoString;
    else return QString::null;
}

int fileFunctions::checkFileAndDownload(const QString &fileName, const QString &url)
{
    QString displayPath;
    errorString = "";
    infoString = "";

    //QString Path = (QString)  qgetenv("CAQTDM_DISPLAY_PATH");
    //printf("<%s>\n", Path.toLatin1().constData());
    //printf("checkFileAndDownload <%s>\n", fileName.toLatin1().constData());

    searchFile *s = new searchFile(fileName);
    QString fileNameFound = s->findFile();
    if(!fileNameFound.isNull()) return true;

    // use specified url
    if(url.size() > 0) {
       displayPath = url;
    // otherwise use url from environment variable
    } else {
       displayPath = (QString)  qgetenv("CAQTDM_URL_DISPLAY_PATH");
    }

    if(displayPath.length() < 1) return false;

    //printf("filename to download %s\n", fileName.toLatin1().constData());

    displayPath.append("/");
    displayPath.append(fileName);
    QUrl displayUrl(displayPath);
    infoString = "download file " + displayPath;

    NetworkAccess *displayGet = new NetworkAccess();
    if(!displayGet->requestUrl(displayUrl, fileName)) {
        errorString = displayGet->lastError();
        displayGet->deleteLater();
        return false;
    }
    displayGet->deleteLater();
    return true;
}
