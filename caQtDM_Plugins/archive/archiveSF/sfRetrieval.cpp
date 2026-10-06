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
#include <iostream>
#include <QFile>
#include <QDir>
#include <QMutex>
#include <QWaitCondition>
#include <QEventLoop>
#include <QTimer>

#include "sfRetrieval.h"
#include "sslverifyhelper.h"
#include "caQtDM_Plugins_global.h"
#include <QDebug>
#include <QThread>
#include <QTime>
#include <iostream>
#include <sstream>

#define qasc(x) x.toLatin1().constData()

//#define CSV 1

#ifndef CSV
#include "QJsonDocument"
#include "QJsonArray"
#include "QJsonObject"
#endif

#ifdef MOBILE_ANDROID
#  include <unistd.h>
#endif


sfRetrieval::sfRetrieval()
{
    finished = false;
    intern_is_Redirected = false;
    manager = new QNetworkAccessManager(this);
    eventLoop = new QEventLoop(this);
    errorString = "";
    qCDebug(archiveSFLog) << QTime::currentTime().toString() << this << "constructor";
    connect(this, SIGNAL(requestFinished()), this, SLOT(downloadFinished()) );
}

void sfRetrieval::timeoutL()
{
    errorString = "http request timeout";
    qCDebug(archiveSFLog) << QTime::currentTime().toString() << this << PV << "timeout" << errorString;
    cancelDownload();
}

bool sfRetrieval::requestUrl(const QUrl url, const QByteArray &json, int secondsPast, bool binned, bool timeAxis, QString key)
{
    qCDebug(archiveSFLog) << "sfRetrieval::requestUrl" << json;
    aborted = false;
    finished = false;
    totalCount = 0;
    secndsPast = secondsPast;
    QString out = QString(json);
    qCDebug(archiveSFLog) << "caQtDM -- request from" << url << "with" << out;
    downloadUrl = url;
    isBinned = binned;
    timAxis = timeAxis;
    errorString = "";
    PV = key;

    QNetworkRequest *request = new QNetworkRequest(url);

    caQtDM_applySslPolicy(*request);

    request->setRawHeader("Content-Type", "application/json");
    request->setRawHeader("Timeout", "86400");

    reply = manager->post(*request, json);

    qCDebug(archiveSFLog) << "requesturl reply" << reply;

    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(finishReply(QNetworkReply*)));

    finished = false;
    QTimer *timeoutHelper = new QTimer(this);
    timeoutHelper->setInterval(60000);
    timeoutHelper->start();
    connect(timeoutHelper, SIGNAL(timeout()), this, SLOT(timeoutL()));
    qCDebug(archiveSFLog) << QTime::currentTime().toString() << this << PV << "go on eventloop->exec";
    eventLoop->exec();

    //downloadfinished will continue
    if(finished) return true;
    else return false;
}

void sfRetrieval::close()
{
    deleteLater();
}

void sfRetrieval::cancelDownload()
{
    totalCount = 0;
    aborted = true;

    disconnect(manager);
    if( reply != Q_NULLPTR ) {
        qCDebug(archiveSFLog) << QTime::currentTime().toString() << this << PV << "!!!!!!!!!!!!!!!!! abort networkreply for";
        reply->abort();
        reply->deleteLater();
        reply = Q_NULLPTR;
    }

    downloadFinished();
    //deleteLater();
}

int sfRetrieval::downloadFinished()
{
    qCDebug(archiveSFLog) << QTime::currentTime().toString() << this << PV << "download finished";
    //eventLoop->processEvents();
#if QT_VERSION > QT_VERSION_CHECK(4, 8, 0)
    eventLoop->quit();
#else
    eventLoop->exit();
#endif
    return finished;
}

void sfRetrieval::finishReply(QNetworkReply *reply)
{
    if(aborted) return;
    qCDebug(archiveSFLog) << QTime::currentTime().toString() << this << PV << "reply received";
    int count = 0;
    double seconds;
#ifdef CSV
    int valueIndex = 2;
    int expected = 3;
    if(isBinned) {
        valueIndex = 3;
        expected = 5;
    }
#endif

    QVariant status =  reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    if(status.toInt() == 301||status.toInt() == 302||status.toInt() == 303||status.toInt() == 307||status.toInt() == 308) {
        errorString = tr("Temporary Redirect status code %1 [%2] from %3").arg(status.toInt()).arg(reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString()).arg(downloadUrl.toString());
        qCDebug(archiveSFLog) << QTime::currentTime().toString() << this << PV << "finishreply" << errorString;
        QByteArray header = reply->rawHeader("location");
        qCDebug(archiveSFLog) << "location" << header;
        finished = true;
        intern_is_Redirected=true;
        Redirected_Url=header;

        emit requestFinished();
        reply->deleteLater();

        return;
    }

    if(status.toInt() != 200) {
        errorString = tr("unexpected http status code %1 [%2] from %3").arg(status.toInt()).arg(reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString()).arg(downloadUrl.toString());
        qCDebug(archiveSFLog) << QTime::currentTime().toString() << this << PV << "finishreply" << errorString;
        emit requestFinished();
        reply->deleteLater();
        return;
    }

    if(reply->error()) {
        errorString = tr("%1: %2").arg(parseError(reply->error())).arg(downloadUrl.toString());
        qCDebug(archiveSFLog) << QTime::currentTime().toString() << this << PV << "finishreply" << errorString;
        emit requestFinished();
        reply->deleteLater();
        return;
    }

    QString out = QString(reply->readAll());
    qCDebug(archiveSFLog) << "received Data in archiveSF";
    reply->deleteLater();

    errorString = "";
    seconds = QDateTime::currentMSecsSinceEpoch() / 1000.0;


#ifdef CSV
    QStringList result = out.split("\n", SKIP_EMPTY_PARTS);
    //printf("number of values received = %d\n",  result.count());

    if(result.count() < 2) {
        if(result.count() == 1) errorString = tr("result too small %1:[%2]").arg(QString::number(result.count())).arg(result[0]);
        else errorString = tr("result too small %1").arg(QString::number(result.count()));
        emit requestFinished();
        return;
    }

    X.resize(result.count()-1);
    Y.resize(result.count()-1);

    bool ok1, ok2;
    for(int i=1; i< result.count(); ++i) {
        QStringList line = result[i].split(";", SKIP_EMPTY_PARTS);
        if(line.count() != expected) {
            errorString = tr("dataline has not the expected number of items %1: [%2]").arg(QString::number(line.count())).arg(expected);
            emit requestFinished();
            return;
        } else {
            //qCDebug(archiveSFLog) << "i=" << i <<  "linecount" << line.count() << line[1];
            double archiveTime = line[1].toDouble(&ok1);
            if(ok1) {
                if((seconds - archiveTime) < secndsPast) {
                    X[count] = -(seconds - archiveTime) / 3600.0;
                    Y[count] = line[valueIndex].toDouble(&ok2);
                    if(ok2) count++;
                    else {
                        errorString = tr("could not decode value %1 at position %2").arg(line[valueIndex].arg(valueIndex));
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

    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(out.toUtf8(), &parseError);

    qCDebug(archiveSFLog) << "\n\nout:" << out << "\n";

    // Did it go wrong?
    if (parseError.error != QJsonParseError::NoError) {
        errorString = tr("could not parse json string left=%1 right=%2 with error %3").arg(out.left(20)).arg(out.right(20)).arg(parseError.errorString());
        qCDebug(archiveSFLog) << QTime::currentTime().toString() << this << PV << "finishreply" << errorString;
        emit requestFinished();
        return;
    } else {

        if(!document.isArray()) {
            qCDebug(archiveSFLog) << "the json root element is not an array, something has changed";
        } else {
            QJsonArray array = document.array();

            for (unsigned int i = 0; i < array.size(); i++) {
                QJsonValue value = array.at(i);

                if(value.isObject()) {

                    QJsonObject root1 = value.toObject();

                    // find channel data inside this part of array
                    if (root1["channel"].isObject()) {
                        //qCDebug(archiveSFLog) << "\nchannel part found as object";
                        QJsonObject root2 = root1["channel"].toObject();

                        // get channel name
                        if (root2["name"].isString()) {
                            QString data = root2["name"].toString();
                            char *channel = new char[data.size()+1];
                            snprintf(channel,data.size()+1,"%s", qasc(data));
                            //qCDebug(archiveSFLog) << "channel name found" << root2["name"]->toString() << channel;
                            delete[] channel;
                        }

                        // get backend name
                        if (root2["backend"].isString()) {
                            QString data = root2["backend"].toString();
                            char *backend = new char[data.size()+1];
                            snprintf(backend,data.size()+1,"%s", qasc(data));
                            Backend = QString(backend);
                            Backend = Backend.replace("\"", "");
                            //qCDebug(archiveSFLog) << "backend name found" << root2["backend"]->toString() << backend;
                            delete[] backend;
                        }
                    }

                    // find data array inside this part of array
                    if (root1["data"].isArray()) {
                        QJsonArray array = root1["data"].toArray();
                        //qCDebug(archiveSFLog) << "\ndata part found as array" << array.size();

                        // scan the data part (big array)
                        if(array.size() < 1) {
                            errorString = tr("no data from %1 : %2").arg(downloadUrl.toString()).arg(Backend);
                            //qCDebug(archiveSFLog) << QTime::currentTime().toString() << this << PV << "finishreply" << errorString;
                            emit requestFinished();
                            return;
                        }

                        // set array size
                        X.resize(array.size());
                        Y.resize(array.size());

                        // binned data
                        if(isBinned) {

                            for (unsigned int i = 0; i < array.size(); i++) {
                                bool valueFound = false;
                                bool timeFound = false;
                                double mean;
                                double archiveTime;

                                // find value part now
                                QJsonObject root1 = array[i].toObject();
                                if (root1["value"].isObject()) {
                                    QJsonObject root2 = root1["value"].toObject();

                                    // look for mean
                                    if (root2["mean"].isDouble()) {
                                        //qCDebug(archiveSFLog) << "mean part found";
                                        mean = root2["mean"].toDouble();
                                        valueFound = true;
                                    }
                                }

                                // look for globalSeconds
                                if (root1["globalSeconds"].isString()) {
                                    //qCDebug(archiveSFLog) << "globalSeconds part found";
                                    if(getDoubleFromString(root1["globalSeconds"].toString(), archiveTime)){
                                        timeFound = true;
                                    } else {
                                        qCDebug(archiveSFLog) << tr("could not decode globalSeconds ????");
                                        break;
                                    }
                                }

                                // fill in our data
                                if(timeFound && valueFound && (seconds - archiveTime) < secndsPast) {
                                    if(!timAxis) X[count] = -(seconds - archiveTime) / 3600.0;
                                    else X[count] = archiveTime * 1000;
                                    Y[count] = mean;
                                    //qCDebug(archiveSFLog) << "binned" << X[count] << Y[count];
                                    count++;
                                }

                            }

                            // non binned data
                        } else {

                            bool valueFound = false;
                            bool timeFound = false;
                            double mean;
                            double archiveTime;
                            for (unsigned int i = 0; i < array.size(); i++) {

                                // simple value
                                QJsonObject root1 = array[i].toObject();
                                if (root1["value"].isDouble()) {
                                    //qCDebug(archiveSFLog) << "value found";
                                    //stat = swscanf(root1["value"].toString(), L "%lf", &mean);
                                    mean=root1["value"].toDouble();
                                    valueFound = true;
                                } else

                                    // an array
                                    if (root1["value"].isArray()) {
                                        //qCDebug(archiveSFLog) << "\nvalue part found as array, not yet supported" << array.size();
                                        errorString = tr("waveforms not supported");
                                        //qCDebug(archiveSFLog)<< QTime::currentTime().toString()  << this << PV << "finishreply" << errorString;
                                        emit requestFinished();
                                        return;
                                    }

                                // look for globalSeconds
                                if (root1["globalSeconds"].isString()) {
                                    //qCDebug(archiveSFLog)<< "globalSeconds part found";
                                    if(getDoubleFromString(root1["globalSeconds"].toString(), archiveTime)){
                                        timeFound = true;
                                        //qCDebug(archiveSFLog) << "time found" << archiveTime;
                                    } else {
                                        qCDebug(archiveSFLog) << tr("could not decode globalSeconds ????");
                                        break;
                                    }
                                }

                                // fill in our data
                                if(timeFound && valueFound && (seconds - archiveTime) < secndsPast) {
                                    if(!timAxis) X[count] = -(seconds - archiveTime) / 3600.0;
                                    else X[count] = archiveTime *1000;
                                    Y[count] = mean;
                                    //qCDebug(archiveSFLog) << "not binned" << X[count] << Y[count];
                                    count++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    totalCount = count;
    qCDebug(archiveSFLog) << QTime::currentTime().toString() << this << PV << "finishreply totalcount =" << count << reply;

#endif

    finished = true;
    emit requestFinished();
}

bool sfRetrieval::getDoubleFromString(QString input, double &value) {
    bool ok;
    value = input.toDouble(&ok);
    if(ok) {
        return true;
    } else {
        return false;
    }
}

bool sfRetrieval::is_Redirected() const
{
    return intern_is_Redirected;
}

QString sfRetrieval::getRedirected_Url() const
{
    return Redirected_Url;
}

int sfRetrieval::getCount()
{
    return totalCount;
}

const QString sfRetrieval::getBackend()
{
    return Backend;
}

void sfRetrieval::getData(QVector<double> &x, QVector<double> &y)
{
    x = X;
    y = Y;
}

const QString sfRetrieval::lastError()
{
    return errorString;
}

const QString sfRetrieval::parseError(QNetworkReply::NetworkError error)
{
    QString errstr = "";
    switch(error)
    {
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
