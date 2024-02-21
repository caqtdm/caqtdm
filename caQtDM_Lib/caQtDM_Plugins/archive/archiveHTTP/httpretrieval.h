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
 *  Copyright (c) 2010 - 2024
 *
 *  Author:
 *    Erik Schwarz
 *  Contact details:
 *    erik.schwarz@psi.ch
 */

#ifndef HTTPRETRIEVAL_H
#define HTTPRETRIEVAL_H

#include "qtimer.h"
#include "urlhandlerhttp.h"
#include <QDebug>
#include <QObject>
#include <QNetworkReply>
#include <QTableWidget>
#include <QMessageBox>
#include <QEventLoop>

//#define CSV 1

#ifndef CSV
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#endif

class QNetworkAccessManager;

class HttpRetrieval:public QObject
{
    Q_OBJECT

public:
    HttpRetrieval();
    ~HttpRetrieval();
    const QString lastError();
    int getCount();
    void getData(QVector<double> &x, QVector<double> &y);
    const QString getBackend();
    void cancelDownload();
    void close();

    QString getRedirected_Url() const;
    bool is_Redirected() const;

signals:
    void networkError(const QString);
    void requestFinished();
    void signalRequestUrl(const UrlHandlerHttp*, int, bool, bool, QString);

public slots:
    bool requestUrl(const UrlHandlerHttp *urlHandler, int secondsPast, bool binned, bool timeAxis, QString key);

protected slots:
    void finishReply(QNetworkReply*);
    const QString parseError(QNetworkReply::NetworkError error);
    int downloadFinished();
    void timeoutL();

private:
    bool getDoubleFromString(QString input, double &value);
    QByteArray gUncompress(const QByteArray &data);
    QTimer *timeoutHelper;
    QNetworkAccessManager *manager;
    QNetworkReply *reply;
    QTableWidget *thisTable;
    QString thisFile;
    int finished;
    bool requestInProgress;
    QUrl downloadUrl;
    QString errorString;
    QVector<double> X,Y;
    int totalCount;
    UrlHandlerHttp m_urlHandler;
    int secndsPast;
    QEventLoop *eventLoop;
    bool isBinned, timAxis;
    QString Backend;
    bool aborted;
    QString PV;
    bool intern_is_Redirected;
    QString Redirected_Url;
};

#endif
