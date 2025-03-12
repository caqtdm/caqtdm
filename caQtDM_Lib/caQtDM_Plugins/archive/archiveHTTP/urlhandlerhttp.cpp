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

#include "urlhandlerhttp.h"
#include "qurlquery.h"

UrlHandlerHttp::UrlHandlerHttp()
{
    // Set some values using environmnent variables if they are set, and otherwise initialize them with a default value.

    // Set the api path for binned data.
    QString customApiPathBinned = (QString) qgetenv("CAQTDM_ARCHIVEHTTP_APIPATH_BINNED");
    if (!customApiPathBinned.isEmpty() && !customApiPathBinned.isNull()) {
        m_apiPathBinned = customApiPathBinned;
    } else {
        m_apiPathBinned = "/api/4/binned";
    }

    // Set the api path for raw (unbinned) data.
    QString customApiPathRaw = (QString) qgetenv("CAQTDM_ARCHIVEHTTP_APIPATH_RAW");
    if (!customApiPathRaw.isEmpty() && !customApiPathRaw.isNull()) {
        m_apiPathRaw = customApiPathRaw;
    } else {
        m_apiPathRaw = "/api/4/events";
    }

    QString customApiPathList = (QString) qgetenv("CAQTDM_ARCHIVEHTTP_APIPATH_LIST");

    if (!customApiPathList.isEmpty() && !customApiPathList.isNull()) {
        m_apiPathList = customApiPathList;
    } else {
        m_apiPathList = "/api/4/backend/list";
    }

    // Set this to true by default as we are not a browser and can handle large results.
    m_allowLargeResult = true;

    // Set this to false by default as we don't need https for our archiver data, if needed the archiver can redirect us.
    m_usesHttps = false;
    // the list is not the standard url that is generated, and it is only needed at startup
    m_backendList = false;

    // Initialize memory
    m_hostName = "data-api.psi.ch";
    m_binned = true;
    m_binCount = 100;
    m_backend = "sf-archiver";
    m_channelName  = "";
    m_beginTime = QDateTime::fromSecsSinceEpoch(0);
    m_endTime = QDateTime::currentDateTime();

}

UrlHandlerHttp::~UrlHandlerHttp()
{
}

QUrl UrlHandlerHttp::assembleUrl() const
{
    QUrl assembledUrl;

    // Set first part of the url, including http or https.
    if (m_usesHttps) {
        assembledUrl = QUrl(QString(QString("https://") + m_hostName.toString()));
    } else {
        assembledUrl = QUrl(QString(QString("http://") + m_hostName.toString()));
    }
    if (m_backendList){
        // the list is a simple command and dosen't need any query
        assembledUrl.setPath(m_apiPathList);
        return assembledUrl;
    }

    // Add the needed api path.
    if (m_binned) {
        assembledUrl.setPath(m_apiPathBinned);
    } else {
        assembledUrl.setPath(m_apiPathRaw);
    }

    // Construct a QUrlQuery to generate a GET query.
    QUrlQuery query;
    query.addQueryItem(m_backendKey, m_backend);
    query.addQueryItem(m_channelNameKey, m_channelName);
    query.addQueryItem(m_beginTimeKey, m_beginTime.toTimeSpec(Qt::OffsetFromUTC).toString(Qt::ISODate));
    query.addQueryItem(m_endTimeKey, m_endTime.toTimeSpec(Qt::OffsetFromUTC).toString(Qt::ISODate));

    if (m_binned) {
        query.addQueryItem(m_binCountKey, QString::number(m_binCount));
    }

    if (m_allowLargeResult) {
        query.addQueryItem(m_allowLargeResultKey, "true");
    }

    // Lastly, append this query to the url.
    assembledUrl.setQuery(query);

    return assembledUrl;
}

void UrlHandlerHttp::setUrl(const QUrl &newUrl)
{
    QUrlQuery query(newUrl);

    // Check for all GET parameters and set member variables
    if (query.hasQueryItem(m_backendKey)){
        m_backend = query.queryItemValue(m_backendKey);
    }
    if (query.hasQueryItem(m_channelNameKey)){
        m_channelName = query.queryItemValue(m_channelNameKey);
    }
    if (query.hasQueryItem(m_beginTimeKey)){
        m_beginTime = QDateTime::fromString(query.queryItemValue(m_beginTimeKey), Qt::ISODate);
    }
    if (query.hasQueryItem(m_endTimeKey)){
        m_endTime = QDateTime::fromString(query.queryItemValue(m_endTimeKey), Qt::ISODate);
    }
    if (query.hasQueryItem(m_binCountKey)){
        m_binCount = query.queryItemValue(m_binCountKey).toInt();
        m_binned = true;
        // In this case we request binned data so set the binned path.
        // Because this parameter is specified, the path (coming before it) must be specified too.
        m_apiPathBinned = newUrl.path();
    } else {
        m_binned = false;
        // In this case we request raw data so set the raw path, but only if it is not empty or there are other query items.
        // Because if there are other query items, then the path has to be specified, but if not it could also only be the hostname,
        // in which case we must watch out not to overwrite the default api path with an empty value.
        if (!query.queryItems().isEmpty() || !newUrl.path().isEmpty()) {
            m_apiPathRaw = newUrl.path();
        }
    }
    if (newUrl.toString().toLower().startsWith("https")) {
        m_usesHttps = true;
    } else {
        m_usesHttps = false;
    }

    // Split URL into '/' separated parts so we can save the base url
    QStringList urlParts = newUrl.toString().split("/");

    // Check if url starts with http (e.g. http(s)://example.com), or not (e.g. example.com).
    if (newUrl.toString().toLower().startsWith("http")) {
        // If it starts with http, the domain name is the third part.
        m_hostName = urlParts[2];
    } else {
        // Otherwise the domain name is the first part.
        m_hostName = urlParts[0];
    }
}

QString UrlHandlerHttp::apiPathBinned() const
{
    return m_apiPathBinned;
}

void UrlHandlerHttp::setApiPathBinned(const QString &newApiPathBinned)
{
    m_apiPathBinned = newApiPathBinned;
}

QString UrlHandlerHttp::apiPathRaw() const
{
    return m_apiPathRaw;
}

void UrlHandlerHttp::setApiPathRaw(const QString &newApiPathRaw)
{
    m_apiPathRaw = newApiPathRaw;
}

bool UrlHandlerHttp::allowLargeResult() const
{
    return m_allowLargeResult;
}

void UrlHandlerHttp::setAllowLargeResult(const bool &newAllowLargeResults)
{
    m_allowLargeResult = newAllowLargeResults;
}

void UrlHandlerHttp::setHostName(const QUrl &newHostName)
{
    m_hostName = newHostName;
}

QUrl UrlHandlerHttp::hostName() const
{
    return m_hostName;
}

bool UrlHandlerHttp::usesHttps() const
{
    return m_usesHttps;
}

void UrlHandlerHttp::setUsesHttps(const bool &newHttps)
{
    m_usesHttps = newHttps;
}

bool UrlHandlerHttp::BackendListRequest() const
{
    return m_backendList;
}

void UrlHandlerHttp::setBackendListRequest(const bool &newBackendList)
{
    m_backendList=newBackendList;
}

bool UrlHandlerHttp::binned() const
{
    return m_binned;
}

void UrlHandlerHttp::setBinned(const bool &newBinned)
{
    m_binned = newBinned;
}

int UrlHandlerHttp::binCount() const
{
    return m_binCount;
}

void UrlHandlerHttp::setBinCount(const int &newBinCount)
{
    m_binCount = newBinCount;
}

QString UrlHandlerHttp::backend() const
{
    return m_backend;
}

void UrlHandlerHttp::setBackend(const QString &newBackend)
{
    m_backend = newBackend;
}

QString UrlHandlerHttp::channelName() const
{
    return m_channelName;
}

void UrlHandlerHttp::setChannelName(const QString &newChannelName)
{
    m_channelName = newChannelName;

    // Remove possible suffixes because the api only takes the channel name itself.
    QStringList suffixList = {".X",".Y",".minY",".maxY"};
    for (const QString &suffix : suffixList) {
        if (m_channelName.endsWith(suffix)) {
            m_channelName.chop(suffix.length());
            break;
        }
    }
}

QDateTime UrlHandlerHttp::beginTime() const
{
    return m_beginTime;
}

void UrlHandlerHttp::setBeginTime(const QDateTime &newBeginTime)
{
    m_beginTime = newBeginTime;
}

QDateTime UrlHandlerHttp::endTime() const
{
    return m_endTime;
}

void UrlHandlerHttp::setEndTime(const QDateTime &newEndTime)
{
    m_endTime = newEndTime;
}
