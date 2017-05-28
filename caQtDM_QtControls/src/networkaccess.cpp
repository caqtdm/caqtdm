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

#include <QApplication>
#include <QNetworkAccessManager>
#include <QSslConfiguration>
#include <QFile>
#include <QDir>
#include <QTimer>
#include <QDebug>
#include "specialFunctions.h"
#include "networkaccess.h"

#ifdef MOBILE_ANDROID
#  include <unistd.h>
#endif

NetworkAccess::NetworkAccess()
{
    finished = false;
    manager = new QNetworkAccessManager;
    eventLoop = new QEventLoop(this);
    errorString = "";
    connect(this, SIGNAL(requestFinished()), this, SLOT(downloadFinished()) );
}

void NetworkAccess::timeoutL()
{
    errorString = tr("networkaccess: http request timeout for %1").arg(downloadUrl.toString());
    eventLoop->quit();
}

bool NetworkAccess::requestUrl(const QUrl url, const QString &file)
{
    finished = false;
    thisFile = file;
    //printf("caQtDM -- download %s\n", qasc(url.toString()));
    downloadUrl = url;

    QNetworkRequest *request = new QNetworkRequest(url);

    //for https we need some configuration (with no verify socket)
#ifndef QT_NO_SSL
    if(url.toString().toUpper().contains("HTTPS")) {
        QSslConfiguration config = request->sslConfiguration();
        config.setPeerVerifyMode(QSslSocket::VerifyNone);
        request->setSslConfiguration(config);
    }
#endif
    //request->setRawHeader("Content-Type", "application/json");
    //request->setRawHeader("Timeout", "86400");

    manager->get(*request);

    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(finishReply(QNetworkReply*)));

    finished = false;
    QTimer *timeoutHelper = new QTimer(this);
    timeoutHelper->setInterval(3000);
    timeoutHelper->start();
    connect(timeoutHelper, SIGNAL(timeout()), this, SLOT(timeoutL()));
    eventLoop->exec();

    if(finished) return true;
    else return false;
}

int NetworkAccess::downloadFinished()
{
    eventLoop->quit();
    return finished;
}

void NetworkAccess::finishReply(QNetworkReply *reply)
{
    //printf("network reply completed! thisFile=%s\n",  qasc(thisFile));

    QVariant status =  reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if(reply->error()) {
        errorString = tr("networkaccess: http status code %1 [%2] for %3").arg(status.toInt()).arg(reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString()).arg(downloadUrl.toString());
        emit requestFinished();
        reply->deleteLater();
        return;
    }

    // seems we want to download the file to a file with  the filename "thisFile"
    if(thisFile.length() > 0) {
        Specials specials;
        QString filePath = specials.getStdPath();

        // create directory if not exists
        QFileInfo fi(thisFile);
        QString newPath = filePath + "/" + fi.path();
        if(!QDir(newPath).exists()) QDir().mkpath(newPath);

        filePath.append("/");
        filePath.append(thisFile);

        QFile file(filePath);
        if(!file.open(QIODevice::ReadWrite)) {
            errorString = tr("networkaccess: %1 could not be opened for write").arg(filePath);
            emit requestFinished();
            reply->deleteLater();
            return;
        } else {
            file.write(reply->readAll());
            file.close();
        }

    }
    finished = true;
    reply->deleteLater();
    emit requestFinished();
}

const QString NetworkAccess::lastError()
{
    return errorString;
}

const QString NetworkAccess::parseError(QNetworkReply::NetworkError error)
{

    QString errstr = "";
    switch(error) {
    case QNetworkReply::ConnectionRefusedError:
        errstr = tr("ConnectionRefusedError");
        break;
    case QNetworkReply::RemoteHostClosedError:
        errstr = tr("RemoteHostClosedError");
        break;
    case QNetworkReply::HostNotFoundError:
        errstr = tr("HostNotFoundError");
        break;
    case QNetworkReply::TimeoutError:
        errstr = tr("TimeoutError");
        break;
    case QNetworkReply::OperationCanceledError:
        errstr = tr("OperationCanceledError");
        break;
    case QNetworkReply::SslHandshakeFailedError:
        errstr = tr("SslHandshakeFailedError");
        break;
#if QT_VERSION > QT_VERSION_CHECK(4, 8, 0)
    case QNetworkReply::TemporaryNetworkFailureError:
        errstr = tr("TemporaryNetworkFailureError");
        break;
#endif
    case QNetworkReply::ProxyConnectionRefusedError:
        errstr = tr("ProxyConnectionRefusedError");
        break;
    case QNetworkReply::ProxyConnectionClosedError:
        errstr = tr("ProxyConnectionClosedError");
        break;
    case QNetworkReply::ProxyNotFoundError:
        errstr = tr("ProxyNotFoundError");
        break;
    case QNetworkReply::ProxyTimeoutError:
        errstr = tr("ProxyTimeoutError");
        break;
    case QNetworkReply::ProxyAuthenticationRequiredError:
        errstr = tr("ProxyAuthenticationRequiredError");
        break;
    case QNetworkReply::ContentAccessDenied:
        errstr = tr("ContentAccessDenied");
        break;
    case QNetworkReply::ContentOperationNotPermittedError:
        errstr = tr("ContentOperationNotPermittedError");
        break;
    case QNetworkReply::ContentNotFoundError:
        errstr = tr("ContentNotFoundError");
        break;
    case QNetworkReply::AuthenticationRequiredError:
        errstr = tr("AuthenticationRequiredError");
        break;
    case QNetworkReply::ProtocolUnknownError:
        errstr = tr("ProtocolUnknownError");
        break;
    case QNetworkReply::ProtocolInvalidOperationError:
        errstr = tr("ProtocolInvalidOperationError");
        break;
    case QNetworkReply::UnknownNetworkError:
        errstr = tr("UnknownNetworkError");
        break;
    case QNetworkReply::UnknownProxyError:
        errstr = tr("UnknownProxyError");
        break;
    case QNetworkReply::UnknownContentError:
        errstr = tr("UnknownContentError");
        break;
    case QNetworkReply::ProtocolFailure:
        errstr = tr("ProtocolFailure");
        break;
    default:
        errstr = tr("unknownError %1").arg(error);
        break;
    }
    return errstr;
}
