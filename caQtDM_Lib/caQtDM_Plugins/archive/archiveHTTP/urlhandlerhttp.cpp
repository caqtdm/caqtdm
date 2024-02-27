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

// =======================================================================================================================================================
//  public:

UrlHandlerHttp::UrlHandlerHttp()
{
    QString customApiPath = (QString) qgetenv("CAQTDM_ARCHIVEHTTP_API_PATH");
    if (!customApiPath.isEmpty() && !customApiPath.isNull()) {
        m_apiPath = customApiPath;
    }
}

UrlHandlerHttp::~UrlHandlerHttp()
{
}

QUrl UrlHandlerHttp::assembleUrl() const
{
    QUrl assembledUrl = m_baseUrl;

    if (m_binned) {
        assembledUrl.setPath("/api/4/binned");
    } else {
        assembledUrl.setPath("/api/4/events");
    }

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

    assembledUrl.setQuery(query);

    return assembledUrl;
}

QUrl UrlHandlerHttp::baseUrl() const
{
    return m_baseUrl;
}

void UrlHandlerHttp::setUrl(const QUrl &newUrl)
{
    QUrlQuery query(newUrl);

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
    }
    // remove path and query parameters to save base url
    QStringList urlParts = newUrl.toString().split("/");
    m_baseUrl = urlParts[0] + "//" + urlParts[2];
}

QString UrlHandlerHttp::apiPath() const
{
    return m_apiPath;
}

void UrlHandlerHttp::setApiPath(const QString &newApiPath)
{
    m_apiPath = newApiPath;
}

bool UrlHandlerHttp::allowLargeResult() const
{
    return m_allowLargeResult;
}

void UrlHandlerHttp::setAllowLargeResult(bool newAllowLargeResults)
{
    m_allowLargeResult = newAllowLargeResults;
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
    m_channelName.remove(".minY", Qt::CaseInsensitive);
    m_channelName.remove(".maxY", Qt::CaseInsensitive);
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
