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

#include <QNetworkAccessManager>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QEventLoop>
#include "specialFunctions.h"
#include "networkaccess.h"
#ifdef MOBILE_ANDROID
#  include <unistd.h>
#endif

// this sleep will not block the GUI and QThread::msleep is protected in Qt4.8 (so do not use that)
class Sleep
{
public:
    static void msleep(unsigned long msecs)
    {
#ifndef MOBILE_ANDROID
        QMutex mutex;
        mutex.lock();
        QWaitCondition waitCondition;
        waitCondition.wait(&mutex, msecs);
        mutex.unlock();
#else
        // not nice, but the above does not work on android now (does not wait)
        usleep(msecs * 100);
#endif
    }
};

NetworkAccess::NetworkAccess()
{
    finished = false;
    manager = new QNetworkAccessManager;
    errorString = "";
}

bool NetworkAccess::requestUrl(const QUrl url, const QString &file)
{
    finished = false;
    thisFile = file;
    //printf("caQtDM -- download %s\n", url.toString().toLatin1().constData());
    downloadUrl = url;
    QNetworkReply* reply = manager->get(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()), this, SLOT(finishReply()));

    //wait until download was done (up to 3 seconds)
    int looped = 0;
    for(int i=0; i<10; i++) {
        qApp->processEvents();
        Sleep::msleep(300);
        qApp->processEvents();
        if(downloadFinished()) {
            return true;
        }
        looped++;
    }
    if(!downloadFinished()) {
        return false;
    }
    return false;
}

int NetworkAccess::downloadFinished()
{
    return finished;
}

void NetworkAccess::finishReply()
{
    //printf("newtwork reply completed! thisFile=%s\n",  thisFile.toLatin1().constData());
    QObject* obj = sender();
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(obj);

    if(reply->error()) {
        errorString = tr("%1: %2").arg(parseError(reply->error())).arg(downloadUrl.toString());
        return;
    }

    // seems we want to download the file to a file with  the filename "thisFile"
    if(thisFile.length() > 0) {
        Specials specials;
        QString filePath = specials.getStdPath();

#ifndef MOBILE
        if(!QDir(filePath).exists()) QDir().mkdir(filePath);
#endif

        filePath.append("/");
        filePath.append(thisFile);

        QFile file(filePath);
        if(!file.open(QIODevice::ReadWrite)) {
            errorString = tr("network error: %1 could not be opened for write").arg(filePath);
            return;
        } else {
            file.write(reply->readAll());
            file.close();
        }

    }
    finished = true;
}

const QString NetworkAccess::lastError()
{
    return errorString;
}

const QString NetworkAccess::parseError(QNetworkReply::NetworkError error)
{
    QString errstr = "";
    switch(error)
    {
    case QNetworkReply::ConnectionRefusedError:
        errstr = tr("ConnectionRefusedError");
        break;
    case QNetworkReply::HostNotFoundError:
        errstr = tr("HostNotFoundError");
        break;
    case QNetworkReply::RemoteHostClosedError:
        errstr = tr("RemoteHostClosedError");
        break;
    case QNetworkReply::TimeoutError:
        errstr = tr("TimeoutError");
        break;
    case QNetworkReply::ContentAccessDenied:
        errstr = tr("ContentAccessDenied");
        break;
    case QNetworkReply::ProtocolFailure:
        errstr = tr("ProtocolFailure");
        break;
    case QNetworkReply::ContentNotFoundError:
        errstr = tr("ContentNotFoundError");
        break;
    default:
        break;
    }
    return errstr;
}
