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
#include "qstandardpaths.h"
#include "networkaccess.h"

NetworkAccess::NetworkAccess(QTableWidget *w, const QString &file)
{
        finished = false;
        manager = new QNetworkAccessManager;
        thisTable = w;
        thisFile = file;
}

QIODevice* NetworkAccess::requestUrl(const QUrl url)
{
        finished = false;
        printf("download %s", url.toString().toAscii().constData());
        downloadUrl = url;
        QNetworkReply* reply = manager->get(QNetworkRequest(url));
        connect(reply, SIGNAL(finished()), this, SLOT(finishReply()));
        return reply;
}

int NetworkAccess::downloadFinished()
{
    return finished;
}

void NetworkAccess::finishReply()
{
    qDebug() << "Finished!";
    QObject* obj = sender();
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(obj);

    if(reply->error()) {
        qDebug() << "reply error";
        //QMessageBox::warning(0, tr("caQtDM"), tr("network error: %1 for file %2").arg(parseError(reply->error())).arg(downloadUrl.toString()));
        return;
    }

    // seems we want to download the file to a file with  the filename "thisFile"
    qDebug() << "open and write the file";
    if(thisTable == 0 && thisFile.length() > 0) {
        QString filePath=QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        filePath.append("/");
        filePath.append(thisFile);
        QFile file(filePath);
        if(!file.open(QIODevice::ReadWrite)) {
            QMessageBox::warning(0, tr("caQtDM"), tr("error: file %1 could not be opened for write").arg(filePath));
            return;
        } else {
            file.write(reply->readAll());
            file.close();
        }

    // seems we want to fill the dialog with filenames
    } else {
        QString myString(reply->readAll());
        qDebug() << myString;
        QStringList list = myString.split(QRegExp("[\r\n]"),QString::SkipEmptyParts);
        qDebug() << list;

        for(int i=0; i< list.count(); i++) {
            QTableWidgetItem *FileItem = new QTableWidgetItem(list.at(i));
            FileItem->setFlags(FileItem->flags() ^ Qt::ItemIsEditable);
            thisTable->insertRow(i);
            thisTable->setItem(i, 0, FileItem);
        }
    }
    finished = true;
    qDebug() << "exit finishreply";
}

const QString NetworkAccess::parseError(QNetworkReply::NetworkError error)
{
    QString errstr = "";
        switch(error)
        {
                case QNetworkReply::ConnectionRefusedError:
                errstr = tr("Connection refused!");
                break;
                case QNetworkReply::HostNotFoundError:
                errstr = tr("Host not found!");
                break;
                case QNetworkReply::RemoteHostClosedError:
                errstr = tr("Remote host closed!");
                break;
                case QNetworkReply::TimeoutError:
                errstr = tr("Timeout!");
                break;
                case QNetworkReply::ContentAccessDenied:
                errstr = tr("Content access denied!");
                break;
                case QNetworkReply::ProtocolFailure:
                errstr = tr("Protocol failure!");
                break;
                case QNetworkReply::ContentNotFoundError:
                errstr = tr("Content not found!");
                break;
                default:
                break;
        }
        qDebug() << "error" << errstr;
        return errstr;
}
