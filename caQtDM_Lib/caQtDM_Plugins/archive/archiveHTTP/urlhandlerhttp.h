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
    /*
     * Besides creating an instance of this class, it also sets the api path.
     * If the environment variable "CAQTDM_ARCHIVEHTTP_API_PATH" is set, it uses that value,
     * else is initializes it with the default value "/api/4".
     * By default allowLargeResult is also set to true.
     * */
    UrlHandlerHttp();
    ~UrlHandlerHttp();

    /*
     * Assembles the url using all previously provided and modified parameters.
     * */
    QUrl assembleUrl() const;

    /*
     * Returns the base url in the format example.com
    * */
    QUrl baseUrl() const;
    /*
     * Sets the url and parses & saves all known parameters it can find.
     * The input can be anything from just the base url up to a previously fully assembled url.
     * */
    void setUrl(const QUrl &newBaseUrl);

    bool usesHttps() const;
    void setUsesHttps(const bool &newHttps);

    /*
     * Returns whether or not the url is configured to request binned data.
     * */
    bool binned() const;
    /*
     * Sets whether the url should request binned data.
     * */
    void setBinned(const bool &newBinned);

    int binCount() const;
    void setBinCount(const int &newBinCount);

    QString backend() const;
    void setBackend(const QString &newBackend);

    /*
     * Returns the channel name without the suffix (.X/.Y/.minX/.minY).
     * */
    QString channelName() const;
    /*
     * Sets the channel name with the given name but removes the suffix first (.X/.Y/.minX/.minY)-
     * */
    void setChannelName(const QString &newChannelName);

    QDateTime beginTime() const;
    void setBeginTime(const QDateTime &newBeginTime);

    QDateTime endTime() const;
    void setEndTime(const QDateTime &newEndTime);

    QString apiPath() const;
    /*
     * Sets the api path, input should be like QString("/path/to/endpoint") .
     * */
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

    QString m_apiPath;
    bool m_allowLargeResult;

    QUrl m_baseUrl;
    bool m_usesHttps;
    bool m_binned;
    int m_binCount;
    QString m_backend;
    QString m_channelName;
    QDateTime m_beginTime;
    QDateTime m_endTime;
};

#endif
