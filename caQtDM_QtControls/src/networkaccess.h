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

#ifndef NETWORKACCESS_H
#define NETWORKACCESS_H

#include <QObject>
#include <QNetworkReply>
#include <QTableWidget>
#include <QMessageBox>

class QNetworkAccessManager;

class NetworkAccess:public QObject
{
    Q_OBJECT

public:
    NetworkAccess();
    ~NetworkAccess(){}
    bool requestUrl(const QUrl url, const QString &file = QString::null);
    int downloadFinished();
    const QString lastError();

signals:
    void networkError(const QString);
    void requestFinished();

protected slots:
    void finishReply();
    const QString parseError(QNetworkReply::NetworkError error);

private:
    QNetworkAccessManager *manager;
    QTableWidget *thisTable;
    QString thisFile;
    int finished;
    QUrl downloadUrl;
    QString errorString;
};

#endif
