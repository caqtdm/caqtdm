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
 *  Copyright (c) 2010 - 2026
 */

#ifndef SSLVERIFYHELPER_H
#define SSLVERIFYHELPER_H

#include <QNetworkRequest>
#include <QUrl>
#include <QtGlobal>
#ifndef QT_NO_SSL
#include <QSslConfiguration>
#include <QSslSocket>
#endif

// Shared TLS policy for all HTTPS requests of caQtDM:
// the peer certificate is NOT verified unless the runtime
// environment variable CAQTDM_SSL_VERIFY=1 is set.
static inline void caQtDM_applySslPolicy(QNetworkRequest &request)
{
#ifndef QT_NO_SSL
    if(request.url().scheme().compare(QLatin1String("https"), Qt::CaseInsensitive) != 0) return;
    if(qgetenv("CAQTDM_SSL_VERIFY").trimmed() == "1") return;
    QSslConfiguration config = request.sslConfiguration();
    config.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(config);
#else
    Q_UNUSED(request);
#endif
}

#endif // SSLVERIFYHELPER_H
