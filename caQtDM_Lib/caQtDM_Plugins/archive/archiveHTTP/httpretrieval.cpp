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
#include <QMutex>
#include <QNetworkAccessManager>
#include <QSslConfiguration>
#include <QTimer>
#include <QWaitCondition>
#include "urlhandlerhttp.h"
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

// =======================================================================================================================================================
//  public:

HttpRetrieval::HttpRetrieval()
{
    finished = false;
    totalCount = 0;
    requestInProgress = false;
    intern_is_Redirected = false;
    manager = new QNetworkAccessManager(this);
    eventLoop = new QEventLoop(this);
    errorString = "";
    //qDebug() << (__FILE__) << ":" << (__LINE__) << "|" << QTime::currentTime().toString() << this << "constructor";
    //const QUrl url, const QByteArray &json, int secondsPast, bool binned, bool timeAxis, QString key
    connect(this, SIGNAL(requestFinished()), this, SLOT(downloadFinished()));
    connect(this,
            SIGNAL(signalRequestUrl(const UrlHandlerHttp *, int, bool, bool, QString)),
            this,
            SLOT(requestUrl(const UrlHandlerHttp *, int, bool, bool, QString)));
    timeoutHelper = new QTimer(this);
    timeoutHelper->setInterval(60000);
    connect(timeoutHelper, SIGNAL(timeout()), this, SLOT(timeoutL()));
}

HttpRetrieval::~HttpRetrieval()
{
    X.clear();
    Y.clear();
    delete manager;
    delete eventLoop;
    delete timeoutHelper;
    //Debug() << this << "destructor" << PV;
}

bool HttpRetrieval::requestUrl(
    const UrlHandlerHttp *urlHandler, int secondsPast, bool binned, bool timeAxis, QString key)
{
    //requestInProgress = true;
    qDebug() << (__FILE__) << ":" << (__LINE__) << "|"
             << "httpRetrieval::requestUrl";
    aborted = false;
    finished = false;
    totalCount = 0;
    secndsPast = secondsPast;
    //printf("caQtDM -- request from %s with %s\n", qasc(url.toString()), qasc(out));
    downloadUrl = urlHandler->assembleUrl();
    isBinned = binned;
    timAxis = timeAxis;
    errorString = "";
    Backend = urlHandler->backend();
    PV = key;
    m_urlHandler.setUrl(downloadUrl);
    delete urlHandler;

    QNetworkRequest request(downloadUrl);

//for https we need some configuration (with no verify socket)
#ifndef CAQTDM_SSL_IGNORE
#ifndef QT_NO_SSL
    if (downloadUrl.toString().toUpper().contains("HTTPS")) {
        QSslConfiguration config = request.sslConfiguration();
#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
        config.setProtocol(QSsl::TlsV1);
#endif
        config.setPeerVerifyMode(QSslSocket::VerifyNone);
        request.setSslConfiguration(config);
    }

#endif
#endif

    request.setRawHeader("Content-Type", "application/json");
    request.setRawHeader("Timeout", "86400");
    request.setRawHeader("Accept-Encoding", "gzip, deflate");
    request.setRawHeader("Accept", "*/*");

    reply = manager->get(request);
    connect(manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(finishReply(QNetworkReply *)));
    qDebug() << __LINE__ << "sending GET request to " << downloadUrl
             << "TimeNow: " << QTime::currentTime();
    finished = false;

    // makes sure the timeout signal can be recieved and handled if eventLoop doesn't terminate before
    timeoutHelper->start();
    if (!eventLoop->isRunning()) {
        eventLoop->exec();
    }
    //qDebug() << (__FILE__) << ":" << (__LINE__) << "|" << QTime::currentTime().toString() << this << PV << "go on eventloop->exec";

    //downloadfinished will continue
    if (finished)
        return true;
    else
        return false;
}

const QString HttpRetrieval::lastError()
{
    return errorString;
}

int HttpRetrieval::getCount()
{
    return totalCount;
}

void HttpRetrieval::getData(QVector<double> &x, QVector<double> &y)
{
    x = X;
    y = Y;
}

const QString HttpRetrieval::getBackend()
{
    return Backend;
}

void HttpRetrieval::cancelDownload()
{
    totalCount = 0;
    aborted = true;

    disconnect(manager);
    if (reply != Q_NULLPTR) {
        //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << QTime::currentTime().toString() << this << PV << "!!!!!!!!!!!!!!!!! abort networkreply for";
        reply->abort();
        reply->deleteLater();
        reply = Q_NULLPTR;
    }

    downloadFinished();
    //deleteLater();
}

void HttpRetrieval::close()
{
    deleteLater();
}

QString HttpRetrieval::getRedirected_Url() const
{
    return Redirected_Url;
}

bool HttpRetrieval::is_Redirected() const
{
    return intern_is_Redirected;
}

// =======================================================================================================================================================
//  protected slots:

void HttpRetrieval::finishReply(QNetworkReply *reply)
{
    if (aborted)
        return;
    qDebug() << (__FILE__) << ":" << (__LINE__) << "|" << QTime::currentTime() << this << PV
             << "reply received";
    int count = 0;
    struct timeb now;
    double seconds;

    QVariant status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    if (status.toInt() == 301 || status.toInt() == 302 || status.toInt() == 303
        || status.toInt() == 307 || status.toInt() == 308) {
        errorString
            = tr("Temporary Redirect status code %1 [%2] from %3")
                  .arg(status.toInt())
                  .arg(reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString())
                  .arg(downloadUrl.toString());
        qDebug() << (__FILE__) << ":" << (__LINE__) << "|" << QTime::currentTime().toString()
                 << this << PV << "finishreply" << errorString;
        QByteArray header = reply->rawHeader("location");
        qDebug() << "location" << header;
        finished = true;
        intern_is_Redirected = true;
        Redirected_Url = header;

        emit requestFinished();
        reply->deleteLater();

        return;
    }

    if (status.toInt() != 200) {
        errorString
            = tr("unexpected http status code %1 [%2] from %3")
                  .arg(status.toInt())
                  .arg(reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString())
                  .arg(downloadUrl.toString());
        qDebug() << (__FILE__) << ":" << (__LINE__) << "|" << QTime::currentTime().toString()
                 << this << PV << "finishreply" << errorString;
        emit requestFinished();
        reply->deleteLater();
        return;
    }

    if (reply->error()) {
        errorString = tr("%1: %2").arg(parseError(reply->error())).arg(downloadUrl.toString());
        qDebug() << (__FILE__) << ":" << (__LINE__) << "|" << QTime::currentTime().toString()
                 << this << PV << "finishreply" << errorString;
        emit requestFinished();
        reply->deleteLater();
        return;
    }
    qDebug() << "HTTP Response has lenght: " << reply->size();

    QByteArray outCompressed = reply->readAll();
    QByteArray out;
    // This sometimes fails for whatever reasons
    try {
        // Not using qUncompress, because we have zLib encrypted Data, so custom Function is used.
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
            return;
        }
    }

    reply->deleteLater();
    errorString = "";
    ftime(&now);
    seconds = (double) now.time + (double) now.millitm / (double) 1000;

#if QT_VERSION > QT_VERSION_CHECK(5, 1, 0)
    // Performance Measurement
    QElapsedTimer myTimer;
    myTimer.start();
    qDebug() << __LINE__ << "parsing qJson reply";
    bool conversionOk = true;
    QJsonObject rootObject;
    try {
        rootObject = QJsonDocument::fromJson(out).object();
    } catch (...) {
        conversionOk = false;
    }
    qDebug() << "finished parsing qJson reply" << myTimer.restart();
    //printf("\n\nout: %s\n\n", qasc(out));

    // Did it go wrong?
    if (rootObject.isEmpty() || !conversionOk) {
        errorString = tr("could not parse json string left=%1 right=%2")
                          .arg(QString(out).left(20))
                          .arg(QString(out).right(20));
        qDebug() << (__FILE__) << ":" << (__LINE__) << "|" << QTime::currentTime().toString()
                 << this << PV << "finishreply" << errorString;
        emit requestFinished();
        return;
    }

    // wait until previous request(s) have been processed
    //    while (requestInProgress) {
    //        continue;
    //    }
    // Here, rather just retur everything as usual and then make the workerHttp create a new request for the following data
    if (rootObject.contains("continueAt")) {
        QDateTime newBeginTime = QDateTime::fromString(rootObject.value("continueAt").toString(),
                                                       Qt::ISODate);
        qDebug() << "continueAt present:" << rootObject.value("continueAt").toString()
                 << "previous beginTime: " << m_urlHandler.beginTime();
        m_urlHandler.setBeginTime(newBeginTime);
        UrlHandlerHttp *urlHandler = new UrlHandlerHttp();
        urlHandler->setUrl(m_urlHandler.assembleUrl());
        qDebug() << "new beginTime:" << urlHandler->beginTime();

        //        emit signalRequestUrl(urlHandler, secndsPast, isBinned, timAxis, PV);
        //        qDebug() << "-----REQUESTFINISHED; CONTINUING----------------";
        //        requestInProgress = true;
    }

    // const QJsonObject &root0 = root["channel"].toObject();

    // const QJsonArray &array0 = root[L"data"].toArray();

    QJsonArray ValueArray;
    if (isBinned) {
        ValueArray = rootObject["avgs"].toArray();
    } else {
        ValueArray = rootObject["values"].toArray();
    }

    // set array size
    X.resize(ValueArray.size());
    Y.resize(ValueArray.size());

    qDebug() << __LINE__ << "starting" << ValueArray.size()
             << "iterations over qJsonArray with convertions";
    int secondsAnchor = rootObject.value("tsAnchor").toInt();
    bool isDouble = ValueArray[0].isDouble();
    if (isBinned) {
        qDebug() << "data is binned";
        QJsonArray FirstMsArray = rootObject["ts1Ms"].toArray();
        QJsonArray LastMsArray = rootObject["ts2Ms"].toArray();
        for (unsigned int i = 0; i < ValueArray.size(); i++) {
            double mean;
            double archiveTime;

            // look for mean (or simply the value for non-binned data)
            if (isDouble) { // We have to check for datatype because QJsonValue wont convert Int to Double and vice versa...
                mean = ValueArray[i].toDouble();
            } else {
                mean = ValueArray[i].toInt();
            }

            archiveTime = secondsAnchor
                          + ((FirstMsArray[i].toInt() + LastMsArray[i].toInt()) * 0.0005);
            if (archiveTime)

                // fill in our data
                if ((seconds - archiveTime) < secndsPast) {
                    if (!timAxis) {
                        X[count] = -(seconds - archiveTime) / 3600.0;
                    } else {
                        X[count] = archiveTime * 1000;
                        Y[count] = mean;
                        //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << "binned" << X[count] << Y[count];
                        count++;
                    }
                }
        }
    } else {
        QJsonArray MsArray = rootObject["tsMs"].toArray();
        for (unsigned int i = 0; i < ValueArray.size(); i++) {
            double mean;
            double archiveTime;

            // look for mean (or simply the value for non-binned data)
            if (isDouble) { // We have to check for datatype because QJsonValue wont convert Int to Double and vice versa...
                mean = ValueArray[i].toDouble();
            } else {
                mean = ValueArray[i].toInt();
            }

            archiveTime = secondsAnchor + (MsArray[i].toInt() * 0.001);

            // fill in our data
            if ((seconds - archiveTime) < secndsPast) {
                if (!timAxis) {
                    X[count] = -(seconds - archiveTime) / 3600.0;
                } else {
                    X[count] = archiveTime * 1000;
                    Y[count] = mean;
                    //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << "binned" << X[count] << Y[count];
                    count++;
                }
            }
        }
    }
    qDebug() << "finished iterations over qJsonArray with convertions" << myTimer.restart();

#else
    // Performance Measurement
    QElapsedTimer myTimer;
    myTimer.start();
    qDebug() << __LINE__ << "parsing qJson reply";
    bool conversionOk = true;
    QJsonArray array;
    try {
        array = QJsonArray(QJsonDocument::fromJson(out).array());
    } catch (...) {
        conversionOk = false;
    }
    qDebug() << "finished parsing qJson reply" << myTimer.restart();
    //printf("\n\nout: %s\n\n", qasc(out));

    // Did it go wrong?
    if (array.isEmpty() || !conversionOk) {
        errorString = tr("could not parse json string left=%1 right=%2")
                          .arg(QString(out).left(20))
                          .arg(QString(out).right(20));
        //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << QTime::currentTime().toString() << this << PV << "finishreply" << errorString;
        emit requestFinished();
        return;
    }

    // pseudo code for handling continueAt when it's implemented in the backend
    //    double newStartSeconds = continueAt;
    //    QJsonObject newPayLoad = payLoad.object();
    //    QJsonObject newRange = newPayLoad.value("range").toObject();
    //    QJsonObject::Iterator iterator = newRange.find("startSeconds");
    //    newRange.erase(iterator);
    //    newRange.insert("startSeconds", QString::number(newStartSeconds, 'g', 10));
    //    iterator = newPayLoad.find("range");
    //    newPayLoad.erase(iterator);
    //    newPayLoad.insert("range", newRange);
    //    if (continueAtExists){
    //        emit signalRequestUrl(downloadUrl, QJsonDocument(newPayLoad), secndsPast, isBinned, timAxis, PV);
    //        // wait until previous request(s) have been processed
    //        while (requestInProgress) {
    //            continue;
    //        }
    //        qDebug() << "-----REQUESTFINISHED; CONTINUING----------------";
    //        requestInProgress = true;
    //    }
    //    //rest of code
    //    if (continueAtExists){
    //        requestInProgress = false;
    //        return;
    //    }
    //    emit requestFinished();
    //    return;

    for (unsigned int i = 0; i < array.size(); i++) {
        //qDebug() << __LINE__ << "converting qJsonArray Item to qJsonObject";
        const QJsonObject &root = array[i].toObject();
        //qDebug() << "finished converting qJsonArray Item to qJsonObject" << myTimer.restart();

        // find channel data inside this part of array
        if (root.contains(L"channel") && root[L"channel"].isObject()) {
            //qDebug() << "converting JSONObject Item to JSONObject";
            const QJsonObject &root0 = root["channel"].toObject();
            //qDebug() << "finished converting JSONObject Item to JSONObject";

            // get backend name
            if (root0.contains(L"backend") && root0[L"backend"].isString()) {
                Backend = root0[L"backend"].toString().replace("\"", "");
            }

            // find data array inside this part of array
            if (root.contains(L"data") && root[L"data"].isArray()) {
                //qDebug() << "converting JSONObject Item to JSONArray";
                const QJsonArray &array0 = root[L"data"].toArray();
                //qDebug() << "finished converting JSONObject Item to JSONArray";
                //qDebug() << (__FILE__) << ":" << (__LINE__) << "|" << "\ndata part found as array" << array.size();

                // scan the data part (big array)
                if (array0.size() < 1) {
                    errorString = tr("no data from %1 : %2").arg(downloadUrl.toString()).arg(Backend);
                    //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << QTime::currentTime().toString() << this << PV << "finishreply" << errorString;
                    emit requestFinished();
                    return;
                }

                // set array size
                X.resize(array0.size());
                Y.resize(array0.size());

                qDebug() << __LINE__ << "starting" << array0.size()
                         << "iterations over qJsonArray with convertions";
                for (unsigned int i = 0; i < array0.size(); i++) {
                    bool valueFound = false;
                    bool timeFound = false;
                    double mean;
                    double archiveTime;

                    // find value part now
                    const QJsonObject &root1 = array0[i].toObject();
                    if (root1.contains(L"value") && root1[L"value"].isObject()) {
                        const QJsonObject &root2 = root1["value"].toObject();
                        // look for mean
                        if (root2.contains(L"mean") && root2[L"mean"].isDouble()) {
                            //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << "mean part found";
                            //stat = swscanf(root2[L"mean"]->Stringify().c_str(), L"%lf", &mean);
                            mean = root2[L"mean"].toDouble();
                            valueFound = true;
                        }
                    }

                    // look for globalSeconds
                    if (root1.contains(L"globalSeconds") && root1[L"globalSeconds"].isString()) {
                        //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << "globalSeconds part found";
                        if (getDoubleFromString(root1[L"globalSeconds"].toString(), archiveTime)) {
                            timeFound = true;
                        } else {
                            //QDebug() << tr("could not decode globalSeconds ????");
                            break;
                        }
                    }

                    // fill in our data
                    if (timeFound && valueFound && (seconds - archiveTime) < secndsPast) {
                        if (!timAxis)
                            X[count] = -(seconds - archiveTime) / 3600.0;
                        else
                            X[count] = archiveTime * 1000;
                        Y[count] = mean;
                        //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << "binned" << X[count] << Y[count];
                        count++;
                    }
                }
                qDebug() << "finished iterations over qJsonArray with convertions"
                         << myTimer.restart();
            }
        }
    }
#endif
    totalCount = count;
    //qDebug() << (__FILE__) << ":" << (__LINE__) << "|" << QTime::currentTime().toString() << this << PV << "finishreply totalcount =" << count << reply;

    //    if (rootObject.contains("continueAt")){
    //        requestInProgress = false;
    //        return;
    //    }

    finished = true;
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

int HttpRetrieval::downloadFinished()
{
    //qDebug() << (__FILE__) << ":" << (__LINE__) << "|" << QTime::currentTime().toString() << this << PV << "download finished";
#if QT_VERSION > QT_VERSION_CHECK(4, 8, 0)
    eventLoop->quit();
#else
    eventLoop->exit();
#endif
    return finished;
}

void HttpRetrieval::timeoutL()
{
    errorString = "http request timeout";
    //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << QTime::currentTime().toString() << this << PV << "timeout" << errorString;
    cancelDownload();
}

// =======================================================================================================================================================
//  private:

bool HttpRetrieval::getDoubleFromString(QString input, double &value)
{
    bool ok;
    value = input.toDouble(&ok);
    if (ok) {
        return true;
    } else {
        return false;
    }
}

// gzip uncompress function from https://stackoverflow.com/a/7351507
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

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = data.size();
    strm.next_in = (Bytef *) (data.data());

    ret = inflateInit2(&strm, 15 + 32); // gzip decoding
    if (ret != Z_OK)
        return QByteArray();

    // run inflate()
    do {
        strm.avail_out = CHUNK_SIZE;
        strm.next_out = (Bytef *) (out);

        ret = inflate(&strm, Z_NO_FLUSH);
        Q_ASSERT(ret != Z_STREAM_ERROR); // state not clobbered

        switch (ret) {
        case Z_NEED_DICT:
            ret = Z_DATA_ERROR; // and fall through
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
