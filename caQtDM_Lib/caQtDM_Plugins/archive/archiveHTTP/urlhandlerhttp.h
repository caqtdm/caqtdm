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

#ifndef URLHANDLERHTTP_H
#define URLHANDLERHTTP_H

#include "qdatetime.h"
#include "qurl.h"
#include <QObject>

class Q_DECL_EXPORT UrlHandlerHttp : public QObject
{
    Q_OBJECT
public:
    UrlHandlerHttp();
    ~UrlHandlerHttp();

    QUrl assembleUrl() const;

    QUrl baseUrl() const;
    void setUrl(const QUrl &newBaseUrl);

    bool https() const;
    void setHttps(const bool &newHttps);

    bool binned() const;
    void setBinned(const bool &newBinned);

    int binCount() const;
    void setBinCount(const int &newBinCount);

    QString backend() const;
    void setBackend(const QString &newBackend);

    QString channelName() const;
    void setChannelName(const QString &newChannelName);

    QDateTime beginTime() const;
    void setBeginTime(const QDateTime &newBeginTime);

    QDateTime endTime() const;
    void setEndTime(const QDateTime &newEndTime);

    QString apiPath() const;
    void setApiPath(const QString &newApiPath);

    bool allowLargeResult() const;
    void setAllowLargeResult(const bool &newAllowLargeResults);

private:
    const QString m_binCountKey = "binCount";
    const QString m_backendKey = "backend";
    const QString m_channelNameKey = "channelName";
    const QString m_beginTimeKey = "begDate";
    const QString m_endTimeKey = "endDate";
    const QString m_allowLargeResultKey = "allowLargeResult";

    const QString m_endpointBinned = "/binned";
    const QString m_endpointRaw = "/events";

    QString m_apiPath = "/api/4";
    bool m_allowLargeResult = true;

    QUrl m_baseUrl;
    bool m_https;
    bool m_binned;
    int m_binCount;
    QString m_backend;
    QString m_channelName;
    QDateTime m_beginTime;
    QDateTime m_endTime;
};

#endif
