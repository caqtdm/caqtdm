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

class HttpRetrieval:public QObject
{
    Q_OBJECT

public:
    HttpRetrieval();
    ~HttpRetrieval();
    const QString lastError();

    /*
     * Returns how many points were parsed and saved successfully per array.
     * */
    int getCount();

    /*
     * Appends the parsed raw data to the given vectors.
     * */
    void getDataAppended(QVector<double> &x, QVector<double> &y);

    /*
     * Appends the parsed binned data to the given vectors.
     * */
    void getBinnedDataAppended(QVector<double> &x, QVector<double> &avgY, QVector<double> &minY, QVector<double> &maxY);

    /*
     * Returns the backend specified in the api call.
     * */
    const QString getBackend();

    /*
     * Returns the url the request was redirected to, if it was.
     * */
    QString getRedirectedUrl() const;

    /*
     * Returns whether or not the request has been redirected.
     * */
    bool isRedirected() const;

    /*
     * Returns whether or not the response contained a continueAt time stamp.
     * */
    bool hasContinueAt() const;

    /*
     * Returns the continueAt time stamp if one has been sent with the response.
     * */
    QDateTime continueAt() const;

    /*
     * Starts a network request to the specified downloadUrl and returns true if the reply was received and parsed successfully, and otherwise false.
     * If the retrieval was successful, the data can be accessed using getDataAppended and getBinnedDataAppended.
     * */
    bool requestUrl(const QUrl downloadUrl, const QString backend, const int secondsPast, const bool binned, const bool timeAxis, const QString key);

    /*
     * Specifies whether or not the request has been aborted, e.g through parent thread.
     * */
    bool isAborted() const;

    /*
     * Returns the HTTP status code of the response.
     * */
    int httpStatusCode() const;

    /*
     * Returns the size of the response in KB.
     * */
    quint64 responseSizeKB() const;

    /*
     * Returns the cooldown to wait until another request is sent, if suggested by the api in the response, in seconds.
     * */
    int retryAfter() const;

signals:
    /*
     * Signal to indicate the processing of the network request has finished.
     * */
    void requestFinished();

protected slots:
    /*
     * Parses the reply and saves the data
     * */
    void finishReply(QNetworkReply*);

    /*
     * Stops the eventloop and returns whether the download was finished successfully
     * */
    int downloadFinished();

    /*
     * Slot to cancel the download once the timeout has elapsed.
     * */
    void timeoutRequest();

public slots:

    /*
     * Cancels the download and makes other functions currently processing a request return early.
     * */
    void cancelDownload();

private:
    /*
     * Returns a QString with the name of the network error.
     * */
    const QString parseError(QNetworkReply::NetworkError error);

    /*
     * gzip uncompress function from https://stackoverflow.com/a/7351507
     * */
    QByteArray gUncompress(const QByteArray &data);

    QTimer *m_timeoutHelper;
    QNetworkAccessManager *m_networkManager;
    QNetworkReply *m_networkReply;
    int m_isFinished;
    QUrl m_downloadUrl;
    QString m_errorString;
    QVector<double> m_vecX, m_vecY, m_vecMinY, m_vecMaxY;
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
    int m_httpStatusCode;
    quint64 m_requestSizeKB;
    int m_retryAfter;
};

#endif
