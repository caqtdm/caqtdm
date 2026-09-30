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

#include "tst_generalloghandler.h"

#include "consoleloghandler.h"
#include "fileloghandler.h"
#include "generalloghandler.h"
#include "logstashloghandler.h"
#ifdef Q_OS_UNIX
#include "syslogloghandler.h"
#endif

#include <QTest>

#define ENV_LOG_HANDLERS "CAQTDM_LOGGING_HANDLERS"

static void mockMessageHandler(QtMsgType, const QMessageLogContext &, const QString &) {}

class MockLogHandler : public AbstractLogHandler
{
public:
    int handleLogCalls = 0;
    int flushCalls = 0;

    void handleLog(const Log &log) override
    {
        Q_UNUSED(log);
        handleLogCalls++;
    }

    void flush() override { flushCalls++; }
};

void TestGeneralLogHandler::initTestCase()
{
    // code to be executed before the first test function
}

void TestGeneralLogHandler::init()
{
    // code to be executed before each test function

    qunsetenv(ENV_LOG_HANDLERS);
}

void TestGeneralLogHandler::cleanupTestCase()
{
    // code to be executed after the last test function
}

void TestGeneralLogHandler::cleanup()
{
    // code to be executed after each test function

    GeneralLogHandler::shutdown();
    qInstallMessageHandler(nullptr);
}

void TestGeneralLogHandler::injectsMessageHandlerAndReturnsPrevious()
{
    qInstallMessageHandler(mockMessageHandler);

    // This should return the previously injected handler
    QtMessageHandler previousHandler = GeneralLogHandler::initialize();
    QVERIFY(previousHandler == mockMessageHandler);

    // The currently installed handler should be GeneralLogHandler::messageHandler
    QtMessageHandler currentHandler = qInstallMessageHandler(nullptr);
    QVERIFY(currentHandler == GeneralLogHandler::messageHandler);
}

void TestGeneralLogHandler::initializationIsIdempotent()
{
    GeneralLogHandler::initialize();

    // Inject a handler that would be reset in case of complete (faulty) re-initialization
    auto *handler = new MockLogHandler();
    {
        QMutexLocker locker(&GeneralLogHandler::s_mutex);
        GeneralLogHandler::s_logHandlers.append(handler);
    }

    // This should return the already injected message handler
    QtMessageHandler previousHandler = GeneralLogHandler::initialize();
    QVERIFY(previousHandler == GeneralLogHandler::messageHandler);

    // And since the rest should not have been re-initialized, a log should still be received by the mock handler
    QVERIFY(std::any_of(GeneralLogHandler::s_logHandlers.begin(),
                        GeneralLogHandler::s_logHandlers.end(),
                        [handler](AbstractLogHandler *h) {
                            return dynamic_cast<MockLogHandler *>(h) == handler;
                        }));

    int previousCount = handler->handleLogCalls;
    qInfo() << "test";
    int currentCount = handler->handleLogCalls;
    QVERIFY(previousCount != currentCount);
}

void TestGeneralLogHandler::callsHandler()
{
    auto *handler = new MockLogHandler();

    {
        QMutexLocker locker(&GeneralLogHandler::s_mutex);
        GeneralLogHandler::s_logHandlers.append(handler);
    }

    // Should invoke
    GeneralLogHandler::messageHandler(QtInfoMsg, {}, "info");
    QCOMPARE(handler->handleLogCalls, 1);

    // Should also invoke
    GeneralLogHandler::messageHandler(QtWarningMsg, {}, "warning");
    QCOMPARE(handler->handleLogCalls, 2);

    // ... should also invoke
    GeneralLogHandler::messageHandler(QtCriticalMsg, {}, "critical");
    QCOMPARE(handler->handleLogCalls, 3);
}

void TestGeneralLogHandler::fatalMessageFlushesHandler()
{
    auto *handler = new MockLogHandler();

    {
        QMutexLocker locker(&GeneralLogHandler::s_mutex);
        GeneralLogHandler::s_logHandlers.append(handler);
    }

    GeneralLogHandler::messageHandler(QtFatalMsg, {}, "fatal");

    QCOMPARE(handler->flushCalls, 1);
}

void TestGeneralLogHandler::shutdownRestoresPreviousHandlerAndIsIdempotent()
{
    qInstallMessageHandler(mockMessageHandler);
    GeneralLogHandler::initialize();

    GeneralLogHandler::shutdown();

    QtMessageHandler currentHandler = qInstallMessageHandler(nullptr);
    QVERIFY(currentHandler == mockMessageHandler);
    QVERIFY(GeneralLogHandler::s_logHandlers.isEmpty());
    QVERIFY(GeneralLogHandler::s_logHandlersThread == Q_NULLPTR);
    QVERIFY(!GeneralLogHandler::s_isInitialized);

    GeneralLogHandler::shutdown();
    QVERIFY(GeneralLogHandler::s_logHandlers.isEmpty());
    QVERIFY(GeneralLogHandler::s_logHandlersThread == Q_NULLPTR);
}

void TestGeneralLogHandler::logHandlersAreInitializedFromEnv()
{
    // Only console
    qputenv(ENV_LOG_HANDLERS, "console");
    GeneralLogHandler::initialize();
    QVERIFY(std::any_of(GeneralLogHandler::s_logHandlers.begin(),
                        GeneralLogHandler::s_logHandlers.end(),
                        [](AbstractLogHandler *h) { return dynamic_cast<ConsoleLogHandler *>(h); }));

    cleanup();
    init();

    // Only file
    qputenv(ENV_LOG_HANDLERS, "fileloghandler");
    GeneralLogHandler::initialize();
    QVERIFY(std::any_of(GeneralLogHandler::s_logHandlers.begin(),
                        GeneralLogHandler::s_logHandlers.end(),
                        [](AbstractLogHandler *h) { return dynamic_cast<FileLogHandler *>(h); }));

    cleanup();
    init();

    // Only logstash
    qputenv(ENV_LOG_HANDLERS, "logstash");
    GeneralLogHandler::initialize();
    QVERIFY(
        std::any_of(GeneralLogHandler::s_logHandlers.begin(),
                    GeneralLogHandler::s_logHandlers.end(),
                    [](AbstractLogHandler *h) { return dynamic_cast<LogstashLogHandler *>(h); }));

    cleanup();
    init();

    // Console + file + logstash
    qputenv(ENV_LOG_HANDLERS, "logstashloghandler, consoleloghandler,file");
    GeneralLogHandler::initialize();
    QVERIFY(std::any_of(GeneralLogHandler::s_logHandlers.begin(),
                        GeneralLogHandler::s_logHandlers.end(),
                        [](AbstractLogHandler *h) { return dynamic_cast<ConsoleLogHandler *>(h); }));
    QVERIFY(std::any_of(GeneralLogHandler::s_logHandlers.begin(),
                        GeneralLogHandler::s_logHandlers.end(),
                        [](AbstractLogHandler *h) { return dynamic_cast<FileLogHandler *>(h); }));
    QVERIFY(
        std::any_of(GeneralLogHandler::s_logHandlers.begin(),
                    GeneralLogHandler::s_logHandlers.end(),
                    [](AbstractLogHandler *h) { return dynamic_cast<LogstashLogHandler *>(h); }));

    cleanup();
    init();

    // for unix also check syslog (here: in combination with file and extra ',')
#ifdef Q_OS_UNIX
    qputenv(ENV_LOG_HANDLERS, "file,syslog,");
    GeneralLogHandler::initialize();
    QVERIFY(std::any_of(GeneralLogHandler::s_logHandlers.begin(),
                        GeneralLogHandler::s_logHandlers.end(),
                        [](AbstractLogHandler *h) { return dynamic_cast<SyslogLogHandler *>(h); }));
#endif
}
