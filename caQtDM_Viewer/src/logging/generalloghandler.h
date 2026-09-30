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
 *
 *  Author:
 *    Erik Schwarz
 *  Contact details:
 *    erik.schwarz@psi.ch
 */

#ifndef GENERALLOGHANDLER_H
#define GENERALLOGHANDLER_H

#include "abstractloghandler.h"

#include <QMutex>
#include <QThread>

#define DEFAULT_LOG_LEVEL QtDebugMsg
#define DEFAULT_LOG_HANDLERS QStringLiteral("console")

#include <QLoggingCategory>
Q_DECLARE_LOGGING_CATEGORY(generalLogHandlerLog)

class GeneralLogHandler
{
public:
    GeneralLogHandler() = delete;
    /**
     * @brief Initializes the general handler and loads all specific logging handlers and configurations from environment variables.
     * This is idempotent as long as the QtMsgHandler is not externally modified. This function is thread-safe.
     * @return The previous QtMsgHandler. If it returns GeneralLogHandler::messageHandler, it was already initialized and nothing else was done
     */
    static QtMessageHandler initialize();
    /**
     * @brief Restores the previous Qt message handler and releases all logging resources.
     * This function is idempotent and thread-safe.
     */
    static void shutdown();
    /**
     * @brief The messageHandler called by Qt (after injected) to handle all qDebugs and similar.
     * This function is thread-safe.
     * @param type: The log level
     * @param context: Various metadata, fields may be empty
     * @param message: The log message
     */
    static void messageHandler(QtMsgType type,
                               const QMessageLogContext &context,
                               const QString &message);

#ifdef UNIT_TESTING
public:
#else
private:
#endif
    /**
     * @brief Loads the names for the different logging endpoints to be instantiated, in short form.
     * @param defaultSelection: A QString that holds a valid set of short names, separated by comma and without spaces
     * @return A list with the names, can be an (unordered) selection from: console, file, syslog, logstash
     */
    static QStringList selectedLogHandlersFromEnv(
        const QString &defaultSelection = DEFAULT_LOG_HANDLERS);

    static QMutex s_mutex;
    static QList<AbstractLogHandler *> s_logHandlers;
    static QThread *s_logHandlersThread;
    static QtMessageHandler s_previousMessageHandler;
    static bool s_isInitialized;
    static qint64 s_processId;
};

#endif // GENERALLOGHANDLER_H
