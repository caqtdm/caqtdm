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

#include "qmutex.h"
#include "qtimer.h"
#include "urlhandlerhttp.h"
#include <QDebug>
#include <QObject>
#include <QNetworkReply>
#include <QTableWidget>
#include <QMessageBox>
#include <QEventLoop>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

// Used for debugging on Linux
#define PRINTFLUSH(...) \
    do { \
        printf(__VA_ARGS__); \
        printf("\n"); \
        std::fflush(stdout); \
    } while (0)


class QNetworkAccessManager;

class HttpRetrieval:public QObject
{
    Q_OBJECT

public:
    HttpRetrieval();
    ~HttpRetrieval();
    const QString lastError();
    int getCount();
    void getDataAppended(QVector<double> &x, QVector<double> &y);
    const QString getBackend();
    QString getRedirected_Url() const;
    bool is_Redirected() const;
    bool hasContinueAt() const;
    QDateTime continueAt() const;
    bool requestUrl(const QUrl downloadUrl, const QString backend, const int secondsPast, const bool binned, const bool timeAxis, const QString key);

signals:
    void networkError(const QString);
    void requestFinished();

protected slots:
    void finishReply(QNetworkReply*);
    const QString parseError(QNetworkReply::NetworkError error);
    int downloadFinished();
    void timeoutL();
    void cancelDownload();

private:
    bool getDoubleFromString(QString input, double &value);
    QByteArray gUncompress(const QByteArray &data);
    QTimer *m_timeoutHelper;
    QNetworkAccessManager *m_networkManager;
    QNetworkReply *m_networkReply;
    int m_isFinished;
    QUrl m_downloadUrl;
    QString m_errorString;
    QVector<double> m_vecX, m_vecY;
    int m_totalNumberOfPoints;
    int m_secondsPast;
    QEventLoop *m_eventLoop;
    bool m_isBinned;
    bool m_isAbsoluteTimeAxis;
    QString m_backend;
    bool m_isAborted;
    QString m_PV;
    bool m_isRedirected;
    QString m_redirectedUrl;
    QDateTime m_continueAt;
    QMutex m_globalMutex;
};

#endif
