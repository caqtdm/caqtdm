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

class Sleep
{
public:
    static void msleep(unsigned long msecs)
    {
        QMutex mutex;
        mutex.lock();
        QWaitCondition waitCondition;
        waitCondition.wait(&mutex, msecs);
        mutex.unlock();
    }
};


fileFunctions::fileFunctions()
{
}
#include <unistd.h>
int fileFunctions::checkFileAndDownload(const QString &fileName, const QString &url)
{
    QString displayPath;
    QFileInfo fi(fileName);

    //printf("filename to download %s\n", fileName.toAscii().constData());
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

    displayPath.append("/");
    displayPath.append(fileName);
    QUrl displayUrl(displayPath);

    NetworkAccess *displayGet = new NetworkAccess(0, fileName);
    displayGet->requestUrl(displayUrl);

    //wait until download was done (up to 3 seconds)
    int looped = 0;
    for(int i=0; i<10; i++) {
        qApp->processEvents();
#ifndef MOBILE_ANDROID
        Sleep::msleep(300);
#else // not nice, but the above does not work on android now (does not wait)
        usleep(500000);
#endif
        qApp->processEvents();
        if(displayGet->downloadFinished()) {
            qDebug() << "download finished succesfully\n";
            break;
        }
        looped++;
    }
    if(!displayGet->downloadFinished()) {
        qDebug() << "download not finished\n";
        return false;
    }
    displayGet->deleteLater();
    return true;
}
