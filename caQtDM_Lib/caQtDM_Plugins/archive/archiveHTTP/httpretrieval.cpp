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
#include "qjsonarray.h"
#include "qjsondocument.h"
#include "qjsonobject.h"
#include "qjsonvalue.h"
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
#include <QtZlib/zlib.h>
#include "httpRetrieval.h"
#include <fstream>
#include <iostream>
#include <sstream>

#define qasc(x) x.toLatin1().constData()

#ifdef MOBILE_ANDROID
#include <unistd.h>
#endif

// =======================================================================================================================================================
//  public:

httpRetrieval::httpRetrieval()
{
    finished = false;
    intern_is_Redirected = false;
    manager = new QNetworkAccessManager(this);
    eventLoop = new QEventLoop(this);
    errorString = "";
    //qDebug() << (__FILE__) << ":" << (__LINE__) << "|" << QTime::currentTime().toString() << this << "constructor";
    connect(this, SIGNAL(requestFinished()), this, SLOT(downloadFinished()));

    timeoutHelper = new QTimer(this);
    timeoutHelper->setInterval(60000);
    connect(timeoutHelper, SIGNAL(timeout()), this, SLOT(timeoutL()));
}

httpRetrieval::~httpRetrieval()
{
    X.clear();
    Y.clear();
    delete manager;
    delete eventLoop;
    delete timeoutHelper;
    //Debug() << this << "destructor" << PV;
}

bool httpRetrieval::requestUrl(
    const QUrl url, const QByteArray &json, int secondsPast, bool binned, bool timeAxis, QString key)
{
    //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << "httpRetrieval::requestUrl" << json;
    aborted = false;
    finished = false;
    totalCount = 0;
    secndsPast = secondsPast;
    //printf("caQtDM -- request from %s with %s\n", qasc(url.toString()), qasc(out));
    downloadUrl = url;
    isBinned = binned;
    timAxis = timeAxis;
    errorString = "";
    PV = key;

    QNetworkRequest request(url);

//for https we need some configuration (with no verify socket)
#ifndef CAQTDM_SSL_IGNORE
#ifndef QT_NO_SSL
    if (url.toString().toUpper().contains("HTTPS")) {
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
    request.setRawHeader("Accept-Encoding", "gzip,deflate");

    reply = manager->post(request, json);
    qDebug() << __LINE__ << "sending POST request to " << url
             << "TimeNow: " << QTime::currentTime();

    connect(reply, SIGNAL(readyRead()), this, SLOT(readPartialReply()));

    //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << "requesturl reply" << reply;

    connect(manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(finishReply(QNetworkReply *)));
    finished = false;

    // makes sure the timeout signal can be recieved and handled if eventLoop doesn't terminate before
    timeoutHelper->start();
    eventLoop->exec();
    //qDebug() << (__FILE__) << ":" << (__LINE__) << "|" << QTime::currentTime().toString() << this << PV << "go on eventloop->exec";

    //downloadfinished will continue
    if (finished)
        return true;
    else
        return false;
}

const QString httpRetrieval::lastError()
{
    return errorString;
}

int httpRetrieval::getCount()
{
    return totalCount;
}

void httpRetrieval::getData(QVector<double> &x, QVector<double> &y)
{
    x = X;
    y = Y;
}

const QString httpRetrieval::getBackend()
{
    return Backend;
}

void httpRetrieval::cancelDownload()
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

void httpRetrieval::close()
{
    deleteLater();
}

QString httpRetrieval::getRedirected_Url() const
{
    return Redirected_Url;
}

bool httpRetrieval::is_Redirected() const
{
    return intern_is_Redirected;
}

// =======================================================================================================================================================
//  protected slots:

void httpRetrieval::finishReply(QNetworkReply *reply)
{
    if (aborted)
        return;
    qDebug() << (__FILE__) << ":" << (__LINE__) << "|" << QTime::currentTime().toString() << this
             << PV << "reply received";
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
        //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << QTime::currentTime().toString() << this << PV << "finishreply" << errorString;
        QByteArray header = reply->rawHeader("location");
        //QDebug() << "location" << header;
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
        //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << QTime::currentTime().toString() << this << PV << "finishreply" << errorString;
        emit requestFinished();
        reply->deleteLater();
        return;
    }

    if (reply->error()) {
        errorString = tr("%1: %2").arg(parseError(reply->error())).arg(downloadUrl.toString());
        //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << QTime::currentTime().toString() << this << PV << "finishreply" << errorString;
        emit requestFinished();
        reply->deleteLater();
        return;
    }
    qDebug() << "HTTP Response has lenght: " << reply->size();
    // This sometimes fails for whatever reasons
    QByteArray out;
    try {
        // Not using qUncompress, because we have zLib encrypted Data, so custom Function is used.
        out = gUncompress(reply->readAll());
    } catch (...) {
        errorString = tr("%1: %2").arg("failed to uncompress data from").arg(downloadUrl.toString());
        qDebug() << (__FILE__) << ":" << (__LINE__) << "|" << QTime::currentTime().toString()
                 << this << PV << "finishreply" << errorString;
        emit requestFinished();
        reply->deleteLater();
        return;
    }

    qDebug() << "received Data in archiveHTTP";
    reply->deleteLater();

    errorString = "";
    ftime(&now);
    seconds = (double) now.time + (double) now.millitm / (double) 1000;

#ifdef CSV
    QStringList result = out.split("\n", SKIP_EMPTY_PARTS);
    //printf("number of values received = %d\n",  result.count());

    if (result.count() < 2) {
        if (result.count() == 1)
            errorString
                = tr("result too small %1:[%2]").arg(QString::number(result.count())).arg(result[0]);
        else
            errorString = tr("result too small %1").arg(QString::number(result.count()));
        emit requestFinished();
        return;
    }

    X.resize(result.count() - 1);
    Y.resize(result.count() - 1);

    bool ok1, ok2;
    for (int i = 1; i < result.count(); ++i) {
        QStringList line = result[i].split(";", SKIP_EMPTY_PARTS);
        if (line.count() != expected) {
            errorString = tr("dataline has not the expected number of items %1: [%2]")
                              .arg(QString::number(line.count()))
                              .arg(expected);
            emit requestFinished();
            return;
        } else {
            //qDebug() << "i=" << i <<  "linecount" << line.count() << line[1];
            double archiveTime = line[1].toDouble(&ok1);
            if (ok1) {
                if ((seconds - archiveTime) < secndsPast) {
                    X[count] = -(seconds - archiveTime) / 3600.0;
                    Y[count] = line[valueIndex].toDouble(&ok2);
                    if (ok2)
                        count++;
                    else {
                        errorString = tr("could not decode value %1 at position %2")
                                          .arg(line[valueIndex].arg(valueIndex));
                        emit requestFinished();
                        return;
                    }
                }
            } else {
                errorString = tr("could not decode time %1 at position").arg(line[1].arg(1));
                emit requestFinished();
                return;
            }
        }
    }
    totalCount = count;

#else

    totalCount = 0;
    Backend = "";

#if QT_VERSION > QT_VERSION_CHECK(5, 1, 0)
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

#else
    // Performance Measurement
    QElapsedTimer myTimer;
    myTimer.start();
    qDebug() << __LINE__ << "parsing JSON reply";
    JSONValue *value = JSON::Parse(qasc(out));
    qDebug() << "finished parsing JSON reply" << myTimer.restart();
    //printf("\n\nout: %s\n\n", qasc(out));

    // Did it go wrong?
    if (value == Q_NULLPTR) {
        errorString = tr("could not parse json string left=%1 right=%2")
                          .arg(out.left(20))
                          .arg(out.right(20));
        //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << QTime::currentTime().toString() << this << PV << "finishreply" << errorString;
        emit requestFinished();
        return;
    } else {
        if (!value->IsArray()) {
            //QDebug() << QTime::currentTime().toString() << "finishreply the json root element is not an array, something has changed";
        }

        if (value->IsArray()) {
            //qDebug() << "converting JSONValue to JSONArray";
            JSONArray array = value->AsArray();
            //qDebug() << "finished converting JSONValue to JSONArray with Size: " << array.size();

            for (unsigned int i = 0; i < array.size(); i++) {
                qDebug() << __LINE__ << "converting JSONArray Item to JSONValue";
                JSONValue *value1 = JSON::Parse(array[i]->Stringify().c_str());
                qDebug() << "finished converting JSONArray Item to JSONValue" << myTimer.restart();

                if (value1->IsObject()) {
                    JSONObject root;
                    //qDebug() << __LINE__ << "converting JSONValue to JSONObject";
                    root = value1->AsObject();
                    //qDebug() << "finished converting JSONValue to JSONObject";

                    // find channel data inside this part of array
                    if (root.find(L"channel") != root.end() && root[L"channel"]->IsObject()) {
                        //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << "\nchannel part found as object";
                        //qDebug() << "converting JSONObject Item to JSONObject";
                        JSONValue *value2 = JSON::Parse(root[L"channel"]->Stringify().c_str());
                        JSONObject root0 = value2->AsObject();
                        //qDebug() << "finished converting JSONObject Item to JSONObject";
                        // get channel name
                        if (root0.find(L"name") != root0.end() && root0[L"name"]->IsString()) {
                            std::wstring data = root0[L"name"]->Stringify();
                            char *channel = new char[data.size() + 1];
                            sprintf(channel, "%ls", data.c_str());
                            //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << "channel name found" << root0[L"name"]->AsString().c_str() << channel;
                            delete[] channel;
                        }

                        // get backend name
                        if (root0.find(L"backend") != root0.end() && root0[L"backend"]->IsString()) {
                            //char backend[800];
                            std::wstring data = root0[L"backend"]->Stringify();
                            char *backend = new char[data.size() + 1];
                            sprintf(backend, "%ls", data.c_str());
                            Backend = QString(backend);
                            Backend = Backend.replace("\"", "");
                            //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << "backend name found" << root0[L"backend"]->AsString().c_str() << backend;
                            delete[] backend;
                        }
                        delete value2;
                    }

                    // find data array inside this part of array
                    if (root.find(L"data") != root.end() && root[L"data"]->IsArray()) {
                        //qDebug() << "converting JSONObject Item to JSONArray";
                        JSONArray array = root[L"data"]->AsArray();
                        //qDebug() << "finished converting JSONObject Item to JSONArray";
                        //qDebug() << (__FILE__) << ":" << (__LINE__) << "|" << "\ndata part found as array" << array.size();

                        // scan the data part (big array)
                        if (array.size() < 1) {
                            errorString = tr("no data from %1 : %2")
                                              .arg(downloadUrl.toString())
                                              .arg(Backend);
                            //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << QTime::currentTime().toString() << this << PV << "finishreply" << errorString;
                            emit requestFinished();
                            delete value;
                            delete value1;
                            return;
                        }

                        // set array size
                        X.resize(array.size());
                        Y.resize(array.size());

                        // binned data
                        if (isBinned) {
                            qDebug() << __LINE__ << "starting" << array.size()
                                     << "iterations over JSONArray with convertions";
                            for (unsigned int i = 0; i < array.size(); i++) {
                                bool valueFound = false;
                                bool timeFound = false;
                                double mean;
                                double archiveTime;

                                // find value part now
                                JSONObject root1 = array[i]->AsObject();
                                if (root1.find(L"value") != root1.end()
                                    && root1[L"value"]->IsObject()) {
                                    JSONValue *value2 = JSON::Parse(
                                        root1[L"value"]->Stringify().c_str());
                                    JSONObject root2 = value2->AsObject();

                                    // look for mean
                                    if (root2.find(L"mean") != root2.end()
                                        && root2[L"mean"]->IsNumber()) {
                                        //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << "mean part found";
                                        //stat = swscanf(root2[L"mean"]->Stringify().c_str(), L"%lf", &mean);
                                        mean = root2[L"mean"]->AsNumber();
                                        valueFound = true;
                                    }
                                    delete value2;
                                }

                                // look for globalSeconds
                                if (root1.find(L"globalSeconds") != root1.end()
                                    && root1[L"globalSeconds"]->IsString()) {
                                    //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << "globalSeconds part found";
                                    if (getDoubleFromString(QString::fromWCharArray(
                                                                root1[L"globalSeconds"]
                                                                    ->AsString()
                                                                    .c_str()),
                                                            archiveTime)) {
                                        timeFound = true;
                                    } else {
                                        //QDebug() << tr("could not decode globalSeconds ????");
                                        break;
                                    }
                                }

                                // fill in our data
                                if (timeFound && valueFound
                                    && (seconds - archiveTime) < secndsPast) {
                                    if (!timAxis)
                                        X[count] = -(seconds - archiveTime) / 3600.0;
                                    else
                                        X[count] = archiveTime * 1000;
                                    Y[count] = mean;
                                    //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << "binned" << X[count] << Y[count];
                                    count++;
                                }
                            }
                            qDebug() << "finished iterations over JSONArray with convertions"
                                     << myTimer.restart();

                            // non binned data
                        } else {
                            bool valueFound = false;
                            bool timeFound = false;
                            double mean;
                            double archiveTime;
                            qDebug() << __LINE__ << "starting to process non binned data";

                            for (unsigned int i = 0; i < array.size(); i++) {
                                // simple value
                                JSONObject root1 = array[i]->AsObject();
                                if (root1.find(L"value") != root1.end()
                                    && root1[L"value"]->IsNumber()) {
                                    //qDebug() << "value found";
                                    //stat = swscanf(root1[L"value"]->Stringify().c_str(), L"%lf", &mean);
                                    mean = root1[L"value"]->AsNumber();
                                    valueFound = true;
                                } else

                                    // an array
                                    if (root1.find(L"value") != root1.end()
                                        && root1[L"value"]->IsArray()) {
                                        //qDebug() << "\nvalue part found as array, not yet supported" << array.size();
                                        errorString = tr("waveforms not supported");
                                        //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << QTime::currentTime().toString() << this << PV << "finishreply" << errorString;
                                        emit requestFinished();
                                        return;
                                    }

                                // look for globalSeconds
                                if (root1.find(L"globalSeconds") != root1.end()
                                    && root1[L"globalSeconds"]->IsString()) {
                                    //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << "globalSeconds part found";
                                    if (getDoubleFromString(QString::fromWCharArray(
                                                                root1[L"globalSeconds"]
                                                                    ->AsString()
                                                                    .c_str()),
                                                            archiveTime)) {
                                        timeFound = true;
                                        //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << "time found" << archiveTime;
                                    } else {
                                        //QDebug() << tr("could not decode globalSeconds ????");
                                        break;
                                    }
                                }

                                // fill in our data
                                if (timeFound && valueFound
                                    && (seconds - archiveTime) < secndsPast) {
                                    if (!timAxis)
                                        X[count] = -(seconds - archiveTime) / 3600.0;
                                    else
                                        X[count] = archiveTime * 1000;
                                    Y[count] = mean;
                                    //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << "not binned" << X[count] << Y[count];
                                    count++;
                                }
                            }
                            qDebug() << "finished processing non binned data" << myTimer.restart();
                        }
                    }
                }
                qDebug() << __LINE__ << "starting to delete value1";
                delete value1;
                qDebug() << "finished deleting value1" << myTimer.restart();
            }
        }
        qDebug() << __LINE__ << "starting to delete value";
        delete value;
        qDebug() << "finished deleting value" << myTimer.restart();
    }
#endif
    totalCount = count;
    //qDebug() << (__FILE__) << ":" << (__LINE__) << "|" << QTime::currentTime().toString() << this << PV << "finishreply totalcount =" << count << reply;

#endif

    finished = true;
    emit requestFinished();
}

const QString httpRetrieval::parseError(QNetworkReply::NetworkError error)
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

int httpRetrieval::downloadFinished()
{
    //qDebug() << (__FILE__) << ":" << (__LINE__) << "|" << QTime::currentTime().toString() << this << PV << "download finished";
#if QT_VERSION > QT_VERSION_CHECK(4, 8, 0)
    eventLoop->quit();
#else
    eventLoop->exit();
#endif
    return finished;
}

void httpRetrieval::timeoutL()
{
    errorString = "http request timeout";
    //QDebug() << (__FILE__) << ":" << (__LINE__) << "|" << QTime::currentTime().toString() << this << PV << "timeout" << errorString;
    cancelDownload();
}

void httpRetrieval::readPartialReply()
{
    return;
    QByteArray compressedData = reply->readAll();
    QString partialReply = QString(gUncompress(compressedData));

    //JSONValue *value = JSON::Parse(qasc(out));
    //JSONArray array = value->AsArray();
    qDebug() << partialReply << "\n\n---------------------------------------------------\n\n";
}

// =======================================================================================================================================================
//  private:

bool httpRetrieval::getDoubleFromString(QString input, double &value)
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
QByteArray httpRetrieval::gUncompress(const QByteArray &data)
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
