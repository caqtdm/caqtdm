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
     * Besides creating an instance of this class, it also sets the api path and a few default values.
     * If they exist, the environment variables "CAQTDM_ARCHIVEHTTP_APIPATH_BINNED" and "CAQTDM_ARCHIVEHTTP_APIPATH_RAW"
     * are used to set the according paths, otherwise they have the default value "/api/4/binned" and "/api/4/events".
     * Also, allowLargeResult is set to the default value <true> and usesHttps is set to the default value <false>.
     * */
    UrlHandlerHttp();
    ~UrlHandlerHttp();

    /*
     * Assembles the url using all previously provided and modified parameters.
     * */
    QUrl assembleUrl() const;

    /*
     * Sets the url by parsing & saving all known parameters it can find, including https, domain name, path and GET parameters.
     * The input can be anything from just the domain name up to a previously fully assembled url.
     * */
    void setUrl(const QUrl &newUrl);

    QString apiPathBinned() const;
    /*
     * Sets the api path for binned data, input should be like QString("/path/to/binned").
     * */
    void setApiPathBinned(const QString &newApiPathBinned);

    QString apiPathRaw() const;
    /*
     * Sets the api path for raw (non binned) data, input should be like QString("/path/to/raw").
     * */
    void setApiPathRaw(const QString &newApiPathRaw);

    bool allowLargeResult() const;
    void setAllowLargeResult(const bool &newAllowLargeResults);

    /*
     * Returns the domain name in the format example.com
     * */
    QUrl domainName() const;
    /*
     * Sets the domain name. Input must be in the format example.com.
     * */
    void setDomainName(const QUrl &newBaseUrl);

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
     * Returns the channel name without the suffix (.X/.Y/.minY/.maxY).
     * */
    QString channelName() const;
    /*
     * Sets the channel name with the given name but removes the suffix first (.X/.Y/.minY/.maxY).
     * */
    void setChannelName(const QString &newChannelName);

    QDateTime beginTime() const;
    void setBeginTime(const QDateTime &newBeginTime);

    QDateTime endTime() const;
    void setEndTime(const QDateTime &newEndTime);

private:
    const QString m_binCountKey = "binCount";
    const QString m_backendKey = "backend";
    const QString m_channelNameKey = "channelName";
    const QString m_beginTimeKey = "begDate";
    const QString m_endTimeKey = "endDate";
    const QString m_allowLargeResultKey = "allowLargeResult";

    QString m_apiPathBinned;
    QString m_apiPathRaw;
    bool m_allowLargeResult;
    QUrl m_domainName;
    bool m_usesHttps;
    bool m_binned;
    int m_binCount;
    QString m_backend;
    QString m_channelName;
    QDateTime m_beginTime;
    QDateTime m_endTime;
};

#endif
