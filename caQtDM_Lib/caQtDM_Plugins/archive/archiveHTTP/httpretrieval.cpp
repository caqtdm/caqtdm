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
 *  Copyright (c) 2024
 *
 *  Author:
 *    Erik Schwarz
 *  Contact details:
 *    erik.schwarz@psi.ch
 */

#include <QApplication>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QNetworkAccessManager>
#include <QSslConfiguration>
#include <QTimer>
#include <QWaitCondition>
#include <iostream>
#include <time.h>

#ifdef MOBILE_ANDROID
#include <androidtimeb.h>
#else
#include <sys/timeb.h>
#endif

#include <QDebug>
#include <QThread>
#include <QTime>

#if defined(_MSC_VER)
#include <QtZlib/zlib.h>
#endif

#if defined(linux) || defined TARGET_OS_MAC
#include <zlib.h>
#endif

#include <fstream>
#include <httpretrieval.h>
#include <iostream>
#include <sstream>

#define qasc(x) x.toLatin1().constData()

#ifdef MOBILE_ANDROID
#include <unistd.h>
#endif

HttpRetrieval::HttpRetrieval()
{
    m_retryAfter = 0;
    m_isFinished = false;
    m_totalNumberOfPoints = 0;
    m_isRedirected = false;
    m_networkManager = new QNetworkAccessManager(this);
    m_eventLoop = new QEventLoop(this);
    m_errorString = "";
    connect(this, SIGNAL(requestFinished()), this, SLOT(downloadFinished()));
    m_timeoutHelper = new QTimer(this);
    m_timeoutHelper->setInterval(60000);
    connect(m_timeoutHelper, SIGNAL(timeout()), this, SLOT(timeoutRequest()));
}

HttpRetrieval::~HttpRetrieval()
{
    m_vecX.clear();
    m_vecY.clear();
    m_vecMinY.clear();
    m_vecMaxY.clear();
    delete m_networkManager;
    delete m_eventLoop;
    delete m_timeoutHelper;
}

bool HttpRetrieval::requestUrl(
    const QUrl downloadUrl, const QString backend, const int secondsPast, const bool binned, const bool timeAxis, const QString key)
{
    // Make sure that no cancelDownload is processed while we are still preparing the network request.
    m_globalMutex.lock();

    m_isAborted = false;
    m_isFinished = false;
    m_totalNumberOfPoints = 0;
    m_secondsPast = secondsPast;
    m_downloadUrl = downloadUrl;
    m_isBinned = binned;
    m_isAbsoluteTimeAxis = timeAxis;
    m_errorString = "";
    m_backend = backend;
    m_PV = key;

    QNetworkRequest request(m_downloadUrl);

//for https we need some configuration (with no verify socket)
#ifndef CAQTDM_SSL_IGNORE
#ifndef QT_NO_SSL
    if (m_downloadUrl.toString().toUpper().contains("HTTPS")) {
        QSslConfiguration config = request.sslConfiguration();
        config.setPeerVerifyMode(QSslSocket::VerifyNone);
        request.setSslConfiguration(config);
    }

#endif
#endif

    request.setRawHeader("Content-Type", "application/json");
    request.setRawHeader("Timeout", "86400");
    request.setRawHeader("Accept-Encoding", "gzip, deflate");
    request.setRawHeader("Accept", "*/*");

    // We want manual redirects to be able to save the redirected address.
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy);

    m_isFinished = false;

    if (!m_isAborted) {
        connect(m_networkManager, SIGNAL(finished(QNetworkReply *)), this, SLOT(finishReply(QNetworkReply *)));
        m_networkReply = m_networkManager->get(request);

        // Unlock the mutex so the cancelDownload function can now run as we have sent the request.
        m_globalMutex.unlock();

        // Makes sure the timeout signal can be recieved and handled if eventLoop doesn't terminate before
        m_timeoutHelper->start();
        m_eventLoop->exec();

        delete m_networkReply;
        m_networkReply = Q_NULLPTR;
    }

    if (m_isFinished) {
        return true;
    } else {
        return false;
    }
}

const QString HttpRetrieval::lastError()
{
    return m_errorString;
}

int HttpRetrieval::getCount()
{
    return m_totalNumberOfPoints;
}

void HttpRetrieval::getDataAppended(QVector<double> &x, QVector<double> &y)
{
    x.append(m_vecX);
    y.append(m_vecY);
}

void HttpRetrieval::getBinnedDataAppended(QVector<double> &x, QVector<double> &avgY, QVector<double> &minY, QVector<double> &maxY)
{
    x.append(m_vecX);
    avgY.append(m_vecY);
    minY.append(m_vecMinY);
    maxY.append(m_vecMaxY);
}

const QString HttpRetrieval::getBackend()
{
    return m_backend;
}

void HttpRetrieval::cancelDownload()
{
    // Make sure we don't try to cancel the download while it is being prepared.
    m_globalMutex.lock();
    m_totalNumberOfPoints = 0;
    m_isAborted = true;

    if (m_networkReply != Q_NULLPTR) {
        m_networkReply->abort();
        m_networkReply->deleteLater();
        m_networkReply = Q_NULLPTR;
    }
    if (m_networkManager != Q_NULLPTR) {
        disconnect(m_networkManager);
    }

    downloadFinished();
    m_globalMutex.unlock();
}

QString HttpRetrieval::getRedirectedUrl() const
{
    return m_redirectedUrl;
}

bool HttpRetrieval::isRedirected() const
{
    return m_isRedirected;
}

void HttpRetrieval::finishReply(QNetworkReply *reply)
{
    if (m_isAborted) {
        m_errorString = "Retrieval was aborted";
        emit requestFinished();
        return;
    }
    int count = 0;
    struct timeb now;
    double seconds;

    QVariant status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    m_httpStatusCode = status.toInt();

    // Handle redirects
    if (status.toInt() == 301 || status.toInt() == 302 || status.toInt() == 303
        || status.toInt() == 307 || status.toInt() == 308) {
        m_errorString
            = QString("Got redirected with status code: \"%1\" [\"%2\"] from: \"%3\"")
                  .arg(status.toString(), reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString(), m_downloadUrl.toString());
        QByteArray header = reply->rawHeader("Location");
        m_isFinished = true;
        m_isRedirected = true;
        m_redirectedUrl = header;

        emit requestFinished();
        reply->deleteLater();
        return;
    }

    // Handle 'too many requests'
    if (status.toInt() == 429) {
        m_errorString
            = QString("Received status code: \"%1\" [\"%2\"], indicating too many requests to the server from: \"%3\"")
                  .arg(status.toString(), reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString(), m_downloadUrl.toString());
        if (reply->hasRawHeader("Retry-After")) {
            QByteArray retryAfterRawValue = reply->rawHeader("Retry-After");
            bool conversionOk = false;
            int retryAfterValue = retryAfterRawValue.toInt(&conversionOk);
            if (conversionOk) {
                m_retryAfter = retryAfterValue;
                m_errorString.append(QString("Response contains Retry-After Header with value: \"%1\"").arg(retryAfterValue));
            }
        }
        // Set this to false so requestUrl returns false and the worker knows the request failed
        m_isFinished = false;
        emit requestFinished();
        reply->deleteLater();
        return;
    }

    if (status.toInt() != 200) {
        if (status.toInt() > 0) {
            m_errorString
                = QString("Received unexpected HTTP status code: \"%1\" [\"%2\"] from: \"%3\"")
                      .arg(status.toString(), reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString(), m_downloadUrl.toString());
            emit requestFinished();
            reply->deleteLater();
            return;
        } else if (reply->error() == QNetworkReply::HostNotFoundError) {
            m_errorString
                = QString("Hostname: \"%1\" could not be resolved, check dynamic property and environment variables.")
                      .arg(m_downloadUrl.host());
            emit requestFinished();
            reply->deleteLater();
            return;
        }
    }

    if (reply->error()) {
        m_errorString = QString("Received error: \"%1\" for URL: \"%2\"").arg(parseError(reply->error()), m_downloadUrl.toString());
        emit requestFinished();
        reply->deleteLater();
        return;
    }

    m_requestSizeKB = reply->size();
    QByteArray outCompressed = reply->readAll();
    QByteArray out;
    // This sometimes fails for whatever reasons
    try {
        // Not using qUncompress because we have zLib encrypted data, so custom function is used.
        out = gUncompress(outCompressed);
    } catch (...) {
        qDebug() << (__FILE__) << ":" << (__LINE__) << "|"
                 << "failed to uncompress Data, treating it as plain json";
        out = outCompressed;
    }

    if (out.isEmpty()) { // Must have been uncompressed data
        out = outCompressed;
        if (out.isEmpty()) { // well...
            qDebug() << (__FILE__) << ":" << (__LINE__) << "|"
                     << "Response is empty, aborting request.";
            emit requestFinished();
            reply->deleteLater();
            m_errorString += "HTTP response was empty";
            return;
        }
    }

    reply->deleteLater();
    m_errorString = "";
    ftime(&now);
    seconds = (double) now.time + (double) now.millitm / (double) 1000;

    bool conversionOk = true;
    QJsonObject rootObject;
    try {
        rootObject = QJsonDocument::fromJson(out).object();
    } catch (...) {
        conversionOk = false;
    }

    // Did it go wrong?
    if (rootObject.isEmpty() || !conversionOk) {
        m_errorString = QString("could not parse json string left=%1 right=%2")
                          .arg(QString(out).left(20), QString(out).right(20));
        emit requestFinished();
        return;
    }

    // Set continueAt so the worker can figure out whether to send another request or not
    if (rootObject.contains("continueAt")) {
        m_continueAt = QDateTime::fromString(rootObject.value("continueAt").toString(), Qt::ISODate);
    }

    // If we got a valid reponse but a retry-after statement is present, save that
    QByteArray retryAfterRawValue = reply->rawHeader("Retry-After");
    conversionOk = false;
    int retryAfterValue = retryAfterRawValue.toInt(&conversionOk);
    if (conversionOk) {
        m_retryAfter = retryAfterValue;
        // If it isn't convertible to an integer, we also cannot wait for that amount...
    }

    // set count to zero, it will be incremented according to values
    count = 0;
    int secondsAnchor = rootObject.value("tsAnchor").toInt();
    double archiveTime = 0;
    if (m_isBinned) {
        QJsonValue minsObject = rootObject["mins"];
        QJsonValue maxsObject = rootObject["maxs"];
        QJsonValue avgsObject = rootObject["avgs"];
        QJsonValue FirstMsJson = rootObject["ts1Ms"];
        QJsonValue LastMsJson = rootObject["ts2Ms"];
        double min = 0;
        double max = 0;
        double avg = 0;
        bool isDouble = avgsObject[0].isDouble();
        for (quint32 i = 0; i < avgsObject.toArray().size(); i++) {
            // Get Values
            if (isDouble) { // We have to check for datatype because QJsonValue wont convert Int to Double and vice versa...
                min = minsObject[i].toDouble();
                max = maxsObject[i].toDouble();
                avg = avgsObject[i].toDouble();
            } else {
                min = minsObject[i].toInt();
                max = maxsObject[i].toInt();
                avg = avgsObject[i].toInt();
            }
            // get average timestamp in seconds
            archiveTime = secondsAnchor
                          + ((FirstMsJson[i].toInt() + LastMsJson[i].toInt()) / 2000);

            // fill in our data, yes this step is redundant (same code for binned and non binned), but to do this in a seperate loop would butcher performance
            if (archiveTime) {
                // fill in our data
                if ((seconds - archiveTime) < m_secondsPast) {
                    if (!m_isAbsoluteTimeAxis) {
                        m_vecX.append(-(seconds - archiveTime) / 3600.0);
                    } else {
                        m_vecX.append(archiveTime * 1000);
                    }
                    m_vecY.append(avg);
                    m_vecMinY.append(min);
                    m_vecMaxY.append(max);
                    count++;
                }
            }
        }
    } else {
        QJsonValue MsJson = rootObject["tsMs"];
        QJsonValue ValueJson = rootObject["values"];
        double value = 0;
        bool isDouble = ValueJson[0].isDouble();
        for (quint32 i = 0; i < ValueJson.toArray().size(); i++) {
            // look for mean (or simply the value for non-binned data)
            if (isDouble) { // We have to check for datatype because QJsonValue wont convert Int to Double and vice versa...
                value = ValueJson[i].toDouble();
            } else {
                value = ValueJson[i].toInt();
            }

            // get timestamp in seconds
            archiveTime = secondsAnchor + (MsJson[i].toInt() / 1000);

            // fill in our data, yes this step is redundant (same code for binned and non binned), but to do this in a seperate loop would butcher performance
            if (archiveTime) {
                // fill in our data
                if ((seconds - archiveTime) < m_secondsPast) {
                    if (!m_isAbsoluteTimeAxis) {
                        m_vecX.append(-(seconds - archiveTime) / 3600.0);
                    } else {
                        m_vecX.append(archiveTime * 1000);
                    }
                    m_vecY.append(value);
                    count++;
                }
            }
        }
    }

    m_totalNumberOfPoints = count;
    m_isFinished = true;
    emit requestFinished();
}

const QString HttpRetrieval::parseError(QNetworkReply::NetworkError error)
{
    QString errstr = "";
    switch (error) {
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
    case QNetworkReply::TemporaryNetworkFailureError:
        errstr = tr("TemporaryNetworkFailureError");
        break;
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

int HttpRetrieval::downloadFinished()
{
    if (m_eventLoop->isRunning()) {
        m_eventLoop->quit();
    }
    return m_isFinished;
}

void HttpRetrieval::timeoutRequest()
{
    m_errorString = "http request timeout";
    cancelDownload();
}

QByteArray HttpRetrieval::gUncompress(const QByteArray &data)
{
    if (data.size() <= 4) {
        qWarning("gUncompress: Input data is truncated");
        return QByteArray();
    }

    QByteArray result;

    int ret;
    z_stream strm;
    static const int CHUNK_SIZE = 1024;
    char out[CHUNK_SIZE];

    // allocate inflate state
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = data.size();
    strm.next_in = (Bytef *) (data.data());

    ret = inflateInit2(&strm, 15 + 32); // gzip decoding
    if (ret != Z_OK) {
        return QByteArray();
    }

    // run inflate()
    do {
        strm.avail_out = CHUNK_SIZE;
        strm.next_out = (Bytef *) (out);

        ret = inflate(&strm, Z_NO_FLUSH);
        Q_ASSERT(ret != Z_STREAM_ERROR); // state not clobbered

        switch (ret) {
        case Z_NEED_DICT:
        case Z_DATA_ERROR:
        case Z_MEM_ERROR:
            (void) inflateEnd(&strm);
            return QByteArray();
        }

        result.append(out, CHUNK_SIZE - strm.avail_out);
    } while (strm.avail_out == 0);

    // clean up and return
    inflateEnd(&strm);
    return result;
}

int HttpRetrieval::retryAfter() const
{
    return m_retryAfter;
}

quint64 HttpRetrieval::responseSizeKB() const
{
    return m_requestSizeKB;
}

int HttpRetrieval::httpStatusCode() const
{
    return m_httpStatusCode;
}

bool HttpRetrieval::isAborted() const
{
    return m_isAborted;
}

QDateTime HttpRetrieval::continueAt() const
{
    return m_continueAt;
}

bool HttpRetrieval::hasContinueAt() const
{
    return !m_continueAt.isNull();
}
