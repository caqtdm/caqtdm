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
#include <time.h>

#ifndef MOBILE_ANDROID
#include <sys/timeb.h>
#else
#include <androidtimeb.h>
#endif

#include "sfRetrieval.h"
#include <QDebug>
#include <QThread>
#include <QTime>
#include <iostream>
#include <sstream>

#define qasc(x) x.toLatin1().constData()

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
    //qDebug() << QTime::currentTime().toString() << this << "constructor";
    connect(this, SIGNAL(requestFinished()), this, SLOT(downloadFinished()) );
}

void sfRetrieval::timeoutL()
{
    errorString = "http request timeout";
    //qDebug() << QTime::currentTime().toString() << this << PV << "timeout" << errorString;
    cancelDownload();
}

bool sfRetrieval::requestUrl(const QUrl url, const QByteArray &json, int secondsPast, bool binned, bool timeAxis, QString key)
{
    //qDebug() << "sfRetrieval::requestUrl" << json;
    aborted = false;
    finished = false;
    totalCount = 0;
    secndsPast = secondsPast;
    QString out = QString(json);
    //printf("caQtDM -- request from %s with %s\n", qasc(url.toString()), qasc(out));
    downloadUrl = url;
    isBinned = binned;
    timAxis = timeAxis;
    errorString = "";
    PV = key;

    QNetworkRequest *request = new QNetworkRequest(url);

    //for https we need some configuration (with no verify socket)
#ifndef CAQTDM_SSL_IGNORE
#ifndef QT_NO_SSL
    if(url.toString().toUpper().contains("HTTPS")) {
        QSslConfiguration config = request->sslConfiguration();
#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
        config.setProtocol(QSsl::TlsV1);
#endif
        config.setPeerVerifyMode(QSslSocket::VerifyNone);
        request->setSslConfiguration(config);
    }


#endif
#endif

    request->setRawHeader("Content-Type", "application/json");
    request->setRawHeader("Timeout", "86400");

    reply = manager->post(*request, json);

    //qDebug() << "requesturl reply" << reply;

    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(finishReply(QNetworkReply*)));

    finished = false;
    QTimer *timeoutHelper = new QTimer(this);
    timeoutHelper->setInterval(60000);
    timeoutHelper->start();
    connect(timeoutHelper, SIGNAL(timeout()), this, SLOT(timeoutL()));
    //qDebug() << QTime::currentTime().toString() << this << PV << "go on eventloop->exec";
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
        //qDebug() << QTime::currentTime().toString() << this << PV << "!!!!!!!!!!!!!!!!! abort networkreply for";
        reply->abort();
        reply->deleteLater();
        reply = Q_NULLPTR;
    }

    downloadFinished();
    //deleteLater();
}

int sfRetrieval::downloadFinished()
{
    //qDebug() << QTime::currentTime().toString() << this << PV << "download finished";
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
    //qDebug() << QTime::currentTime().toString() << this << PV << "reply received";
    int count = 0;
    struct timeb now;
    int valueIndex = 2;
    int expected = 3;
    double seconds;
    if(isBinned) {
        valueIndex = 3;
        expected = 5;
    }

    QVariant status =  reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    if(status.toInt() == 301||status.toInt() == 302||status.toInt() == 303||status.toInt() == 307||status.toInt() == 308) {
        errorString = tr("Temporary Redirect status code %1 [%2] from %3").arg(status.toInt()).arg(reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString()).arg(downloadUrl.toString());
        //qDebug() << QTime::currentTime().toString() << this << PV << "finishreply" << errorString;
        QByteArray header = reply->rawHeader("location");
        qDebug() << "location" << header;
        finished = true;
        intern_is_Redirected=true;
        Redirected_Url=header;

        emit requestFinished();
        reply->deleteLater();

        return;
    }

    if(status.toInt() != 200) {
        errorString = tr("unexpected http status code %1 [%2] from %3").arg(status.toInt()).arg(reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString()).arg(downloadUrl.toString());
        //qDebug() << QTime::currentTime().toString() << this << PV << "finishreply" << errorString;
        emit requestFinished();
        reply->deleteLater();
        return;
    }

    if(reply->error()) {
        errorString = tr("%1: %2").arg(parseError(reply->error())).arg(downloadUrl.toString());
        //qDebug() << QTime::currentTime().toString() << this << PV << "finishreply" << errorString;
        emit requestFinished();
        reply->deleteLater();
        return;
    }

    QString out = QString(reply->readAll());
    //qDebug() << "received Data in archiveSF";
    reply->deleteLater();

    errorString = "";
    ftime(&now);
    seconds = (double) now.time + (double) now.millitm / (double)1000;


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
            //qDebug() << "i=" << i <<  "linecount" << line.count() << line[1];
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

    JSONValue *value = JSON::Parse(qasc(out));

    //printf("\n\nout: %s\n\n", qasc(out));

    // Did it go wrong?
    if (value == Q_NULLPTR) {
        errorString = tr("could not parse json string left=%1 right=%2").arg(out.left(20)).arg(out.right(20));
        //qDebug() << QTime::currentTime().toString() << this << PV << "finishreply" << errorString;
        emit requestFinished();
        return;
    } else {

        if(!value->IsArray()) {
            qDebug() << QTime::currentTime().toString() << "finishreply the json root element is not an array, something has changed";
        }

        if(value->IsArray()) {
            JSONArray array = value->AsArray();

            for (unsigned int i = 0; i < array.size(); i++) {
                JSONValue *value1 = JSON::Parse(array[i]->Stringify().c_str());

                if(value1->IsObject()) {

                    JSONObject root;
                    root = value1->AsObject();

                    // find channel data inside this part of array
                    if (root.find(L"channel") != root.end() && root[L"channel"]->IsObject()) {
                        //qDebug() << "\nchannel part found as object";
                        JSONValue *value2 = JSON::Parse(root[L"channel"]->Stringify().c_str());
                        JSONObject root0 = value2->AsObject();

                        // get channel name
                        if (root0.find(L"name") != root0.end() && root0[L"name"]->IsString()) {
                            std::wstring data=root0[L"name"]->Stringify();
                            char *channel = new char[data.size()+1];
                            sprintf(channel,"%ls", data.c_str());
                            //qDebug()<< "channel name found" << root0[L"name"]->AsString().c_str() << channel;
                            delete[] channel;
                        }

                        // get backend name
                        if (root0.find(L"backend") != root0.end() && root0[L"backend"]->IsString()) {
                            //char backend[800];
                            std::wstring data=root0[L"backend"]->Stringify();
                            char *backend = new char[data.size()+1];
                            sprintf(backend,"%ls", data.c_str());
                            Backend = QString(backend);
                            Backend = Backend.replace("\"", "");
                            //qDebug()<< "backend name found" << root0[L"backend"]->AsString().c_str() << backend;
                            delete[] backend;
                        }
                        delete value2;
                    }

                    // find data array inside this part of array
                    if (root.find(L"data") != root.end() && root[L"data"]->IsArray()) {
                        JSONArray array = root[L"data"]->AsArray();
                        //qDebug() << "\ndata part found as array" << array.size();

                        // scan the data part (big array)
                        if(array.size() < 1) {
                            errorString = tr("no data from %1 : %2").arg(downloadUrl.toString()).arg(Backend);
                            //qDebug() << QTime::currentTime().toString() << this << PV << "finishreply" << errorString;
                            emit requestFinished();
                            delete value;
                            delete value1;
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
                                JSONObject root1 = array[i]->AsObject();
                                if (root1.find(L"value") != root1.end() && root1[L"value"]->IsObject()) {
                                    JSONValue *value2 = JSON::Parse(root1[L"value"]->Stringify().c_str());
                                    JSONObject root2 = value2->AsObject();

                                    // look for mean
                                    if (root2.find(L"mean") != root2.end() && root2[L"mean"]->IsNumber()) {
                                        //qDebug() << "mean part found";
                                        //stat = swscanf(root2[L"mean"]->Stringify().c_str(), L"%lf", &mean);
                                        mean=root2[L"mean"]->AsNumber();
                                        valueFound = true;
                                    }
                                    delete value2;
                                }

                                // look for globalSeconds
                                if (root1.find(L"globalSeconds") != root1.end() && root1[L"globalSeconds"]->IsString()) {
                                    //qDebug()<< "globalSeconds part found";
                                    if(getDoubleFromString(QString::fromWCharArray(root1[L"globalSeconds"]->AsString().c_str()), archiveTime)){
                                        timeFound = true;
                                    } else {
                                        qDebug() << tr("could not decode globalSeconds ????");
                                        break;
                                    }
                                }

                                // fill in our data
                                if(timeFound && valueFound && (seconds - archiveTime) < secndsPast) {
                                    if(!timAxis) X[count] = -(seconds - archiveTime) / 3600.0;
                                    else X[count] = archiveTime * 1000;
                                    Y[count] = mean;
                                    //qDebug() << "binned" << X[count] << Y[count];
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
                                JSONObject root1 = array[i]->AsObject();
                                if (root1.find(L"value") != root1.end() && root1[L"value"]->IsNumber()) {
                                    //qDebug() << "value found";
                                    //stat = swscanf(root1[L"value"]->Stringify().c_str(), L"%lf", &mean);
                                    mean=root1[L"value"]->AsNumber();
                                    valueFound = true;
                                } else

                                    // an array
                                    if (root1.find(L"value") != root1.end() && root1[L"value"]->IsArray()) {
                                        //qDebug() << "\nvalue part found as array, not yet supported" << array.size();
                                        errorString = tr("waveforms not supported");
                                        //qDebug()<< QTime::currentTime().toString()  << this << PV << "finishreply" << errorString;
                                        emit requestFinished();
                                        return;
                                    }

                                // look for globalSeconds
                                if (root1.find(L"globalSeconds") != root1.end() && root1[L"globalSeconds"]->IsString()) {
                                    //qDebug()<< "globalSeconds part found";
                                    if(getDoubleFromString(QString::fromWCharArray(root1[L"globalSeconds"]->AsString().c_str()), archiveTime)){
                                        timeFound = true;
                                        //qDebug() << "time found" << archiveTime;
                                    } else {
                                        qDebug() << tr("could not decode globalSeconds ????");
                                        break;
                                    }
                                }

                                // fill in our data
                                if(timeFound && valueFound && (seconds - archiveTime) < secndsPast) {
                                    if(!timAxis) X[count] = -(seconds - archiveTime) / 3600.0;
                                    else X[count] = archiveTime *1000;
                                    Y[count] = mean;
                                    //qDebug() << "not binned" << X[count] << Y[count];
                                    count++;
                                }
                            }
                        }
                    }
                }
                delete value1;
            }
        }
        delete value;
    }

    totalCount = count;
    //qDebug() << QTime::currentTime().toString() << this << PV << "finishreply totalcount =" << count << reply;

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
