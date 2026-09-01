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

#include "generalloghandler.h"

#include "consoleloghandler.h"
#include "fileloghandler.h"
#include "logstashloghandler.h"
#ifdef Q_OS_UNIX
#include "syslogloghandler.h"
#endif

#include <chrono>
#include <cstdio>
#include <ctime>

#include <QCoreApplication>

#define ENV_LOG_HANDLERS "CAQTDM_LOGGING_HANDLERS"

QMutex GeneralLogHandler::s_mutex;
QList<AbstractLogHandler *> GeneralLogHandler::s_logHandlers;
QThread *GeneralLogHandler::s_logHandlersThread = Q_NULLPTR;
QtMessageHandler GeneralLogHandler::s_previousMessageHandler = Q_NULLPTR;
bool GeneralLogHandler::s_isInitialized = false;
qint64 GeneralLogHandler::s_processId = -1;

Q_LOGGING_CATEGORY(generalLogHandlerLog, "caqtdm.logging.general")

QtMessageHandler GeneralLogHandler::initialize()
{
    QMutexLocker locker(&s_mutex);

    QtMessageHandler previousHandler = qInstallMessageHandler(GeneralLogHandler::messageHandler);
    if (previousHandler == GeneralLogHandler::messageHandler) {
        // Was already initialized
        return previousHandler;
    }

    s_previousMessageHandler = previousHandler;

    // Re-install previous handler for initialization logs
    qInstallMessageHandler(previousHandler);

    // Unless overwritten via env, disable all QtDebugMsg Logs
    QLoggingCategory::setFilterRules("*.debug=false");
    qCDebug(generalLogHandlerLog) << "If you see this message, the env QT_LOGGING_CONF or QT_LOGGING_RULES is defined and allows debug messages explicitely.";
    qCInfo(generalLogHandlerLog) << "If not overwritten by env QT_LOGGING_CONF or QT_LOGGING_RULES caQtDM will disable all debug messages. You will still see info, warning, critical and fatal messages. This is an info message.";

    // This shouldn't change
    s_processId = QCoreApplication::applicationPid();

    // Clean up any previous handlers
    for (auto existingLogHandler : s_logHandlers) {
        delete existingLogHandler;
    }
    s_logHandlers.clear();

    if (!s_logHandlersThread) {
        s_logHandlersThread = new QThread();
        s_logHandlersThread->setObjectName(QSL("LogHandlersThread"));
        s_logHandlersThread->start();
    }

    const QStringList selectedLogHandlers = selectedLogHandlersFromEnv();
    if (selectedLogHandlers.contains(QSL("console"))) {
        qCInfo(generalLogHandlerLog) << QSL("adding console log handler");
        auto *consoleHandler = new ConsoleLogHandler();
        consoleHandler->moveToThread(s_logHandlersThread);
        s_logHandlers.append(consoleHandler);
        QObject::connect(QCoreApplication::instance(),
                         &QCoreApplication::aboutToQuit,
                         consoleHandler,
                         &ConsoleLogHandler::flush,
                         Qt::QueuedConnection);
    }

    if (selectedLogHandlers.contains(QSL("file"))) {
        qCInfo(generalLogHandlerLog) << QSL("adding file log handler");
        auto *fileHandler = new FileLogHandler();
        fileHandler->moveToThread(s_logHandlersThread);
        s_logHandlers.append(fileHandler);
        QObject::connect(QCoreApplication::instance(),
                         &QCoreApplication::aboutToQuit,
                         fileHandler,
                         &FileLogHandler::flush,
                         Qt::QueuedConnection);
    }

    if (selectedLogHandlers.contains(QSL("logstash"))) {
        qCInfo(generalLogHandlerLog) << QSL("adding logstash log handler");
        auto *logstashHandler = new LogstashLogHandler();
        logstashHandler->moveToThread(s_logHandlersThread);
        s_logHandlers.append(logstashHandler);
        QObject::connect(QCoreApplication::instance(),
                         &QCoreApplication::aboutToQuit,
                         logstashHandler,
                         &LogstashLogHandler::flush,
                         Qt::QueuedConnection);
    }

#ifdef Q_OS_UNIX
    if (selectedLogHandlers.contains(QSL("syslog"))) {
        qCInfo(generalLogHandlerLog) << QSL("adding syslog log handler");
        auto *syslogHandler = new SyslogLogHandler();
        // Not a QObject, also no async operations, so not moved to separate thread.
        s_logHandlers.append(syslogHandler);
    }
#endif

    // Now the custom handler is ready to accept logs, so install it again
    qInstallMessageHandler(GeneralLogHandler::messageHandler);
    s_isInitialized = true;

    return previousHandler;
}

void GeneralLogHandler::shutdown()
{
    QMutexLocker locker(&s_mutex);

    if (!s_isInitialized) {
        return;
    }

    // Remove this handler before destroying objects which may emit Qt messages.
    qInstallMessageHandler(s_previousMessageHandler);

    for (auto logHandler : s_logHandlers) {
        if (logHandler) {
            logHandler->flush();
            delete logHandler;
        }
    }
    s_logHandlers.clear();

    if (s_logHandlersThread) {
        s_logHandlersThread->quit();
        s_logHandlersThread->wait();
        delete s_logHandlersThread;
        s_logHandlersThread = Q_NULLPTR;
    }

    s_previousMessageHandler = Q_NULLPTR;
    s_isInitialized = false;
    s_processId = -1;
}

QStringList GeneralLogHandler::selectedLogHandlersFromEnv(const QString &defaultConfig)
{
    if (!qEnvironmentVariableIsSet(ENV_LOG_HANDLERS)) {
        return defaultConfig.split(',');
    }

    QStringList selectedLogHandlers;
    const QString config = qgetenv(ENV_LOG_HANDLERS).toLower().replace(" ", "");
    for (const auto &handler : config.split(',')) {
        if (handler == QSL("console") || handler == QSL("consoleloghandler")) {
            selectedLogHandlers.append(QSL("console"));
        } else if (handler == QSL("file") || handler == QSL("fileloghandler")) {
            selectedLogHandlers.append(QSL("file"));
        } else if (handler == QSL("logstash") || handler == "logstashloghandler") {
            selectedLogHandlers.append(QSL("logstash"));
        } else if (handler == QSL("syslog") || handler == QSL("syslogloghandler")) {
#ifndef Q_OS_UNIX
            qCCritical(generalLogHandlerLog)
                << ENV_LOG_HANDLERS
                << QSL("specified syslog log handler, but this is invalid as system is not unix");
#else
            selectedLogHandlers.append("syslog");
#endif
        }
    }

    return selectedLogHandlers;
}

void GeneralLogHandler::messageHandler(QtMsgType type,
                                       const QMessageLogContext &context,
                                       const QString &message)
{
    // Loglevel is filtered via QT_LOGGING_RULES environment variable.

    const long long msSinceEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(
                                       std::chrono::system_clock::now().time_since_epoch())
                                       .count();

    const std::time_t seconds = msSinceEpoch / 1000;
    const int milliseconds = msSinceEpoch % 1000;
    std::tm tm;
#if defined(_WIN32)
    gmtime_s(&tm, &seconds);
#else
    gmtime_r(&seconds, &tm);
#endif
    char timestampUtc[sizeof("yyyy-MM-ddTHH:mm:ss.zzzZ")];
    std::snprintf(timestampUtc,
                  sizeof(timestampUtc),
                  "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
                  tm.tm_year + 1900,
                  tm.tm_mon + 1,
                  tm.tm_mday,
                  tm.tm_hour,
                  tm.tm_min,
                  tm.tm_sec,
                  milliseconds);

    QString logLevelString;
    switch (type) {
    case QtDebugMsg:
        logLevelString = QSL("QtDebugMsg");
        break;
    case QtInfoMsg:
        logLevelString = QSL("QtInfoMsg");
        break;
    case QtWarningMsg:
        logLevelString = QSL("QtWarningMsg");

        break;
    case QtCriticalMsg:
        logLevelString = QSL("QtCriticalMsg");
        break;
    case QtFatalMsg:
        logLevelString = QSL("QtFatalMsg");
        break;
    default:
        logLevelString = QSL("unkown QtMsgType");
    }

    const QString locationString = QString(context.file) + QSL(":") + context.function + QSL(":")
                                   + QString::number(context.line);

    QString truncatedMessage = message;
    // No need for trailing newlines, log handlers should receive message without them.
    if (message.endsWith('\n')) {
        truncatedMessage.remove(truncatedMessage.size() - 1, 1);
    }

    const Log log = {msSinceEpoch,
                     timestampUtc,
                     type,
                     logLevelString,
                     truncatedMessage,
                     locationString,
                     context.file,
                     context.function,
                     context.line,
                     context.category,
                     s_processId};

    QMutexLocker locker(&s_mutex);
// if check for the logHandler is done because of a race condition on mac
    for (const auto logHandler : s_logHandlers) {
        if (logHandler) logHandler->handleLog(log);
    }

    // Qt will exit faulty after this returns, so make sure to flush
    if (type == QtFatalMsg) {
        for (const auto logHandler : s_logHandlers) {
            if (logHandler) logHandler->flush();
        }
    }
}
