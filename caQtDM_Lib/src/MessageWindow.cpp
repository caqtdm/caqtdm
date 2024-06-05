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


#include "MessageWindow.h"
#include "messageWindowWrapper.h"
#include "qdatetime.h"
#include <QCoreApplication>
#include <QMutexLocker>
#include <stdio.h>
#include <time.h>
#include <QFile>
#include <QDebug>
#include <QTextStream>
#ifndef MOBILE_ANDROID
#include <sys/timeb.h>
#else
#include <androidtimeb.h>
#endif
#include "qtdefinitions.h"

#define GCC_VERSION (__GNUC__ * 10000 \
                               + __GNUC_MINOR__ * 100 \
                               + __GNUC_PATCHLEVEL__)

const char* MessageWindow::WINDOW_TITLE = "caQtDM Messages";
MessageWindow* MessageWindow::MsgHandler = Q_NULLPTR;

MessageWindow::MessageWindow(QWidget* parent) : QDockWidget(parent)
{

    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    msgTextEdit.setFont(font);

    setFeatures(QDockWidget::NoDockWidgetFeatures);
    setWindowTitle(tr(WINDOW_TITLE));
    msgTextEdit.setReadOnly(true);
    msgTextEdit.document()->setMaximumBlockCount(400);
    setWidget(&msgTextEdit);
    MessageWindow::MsgHandler = this;
    setMinimumSize(600, 150);
    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowMinMaxButtonsHint);
    setContextMenuPolicy(Qt::CustomContextMenu);
    show();

    QString createLogFile = qgetenv("CAQTDM_CREATE_LOGFILE");
    if (createLogFile.toLower() == "true") {
        QDateTime currentTime = QDateTime::currentDateTime();
        QString logFileName = QString("caQtDM_Logfile_%1.txt").arg(currentTime.toLocalTime().toString("yyyy-dd-M--HH-mm-ss-zzz"));
        QString logFilePath = qgetenv("CAQTDM_LOGFILE_PATH");
        if (!logFilePath.isEmpty()) {
            logFilePath += "/" + logFileName;
            m_logFilePath = logFilePath;
        } else {
            m_logFilePath = logFileName;
        }
    }

    move(x(), 0);
}

QString MessageWindow::QtMsgToQString(QtMsgType type, const char *msg)
{
    time_t          time_val;
    struct tm       *timess;
    struct timeb    timeA;
    char            prTime[200];

    ftime(&timeA);
    time_val = timeA.time;
    timess = localtime(&time_val);
    if(timess != Q_NULLPTR) {
        sprintf(prTime, "%02d-%02d-%04d %02d:%02d:%02d ", timess->tm_mday, timess->tm_mon+1, timess->tm_year+1900,  timess->tm_hour, timess->tm_min, timess->tm_sec);
        switch (type) {
                case QtDebugMsg:
                        return QString(prTime) + QString(msg);
                case QtWarningMsg:
                        return QString(prTime) + QString(msg);
                case QtCriticalMsg:
                        return QString(prTime) + QString(msg);
                case QtFatalMsg:
                        return QString(prTime) + QString(msg);
                default:
                        return QString(prTime) + QString(msg);
        }
     } else {
        return QString(msg);
     }
}

void MessageWindow::AppendMsgWrapper(QtMsgType type, char* msg)
{
        static QMutex mutex;
        QMutexLocker locker(&mutex);

        if (MessageWindow::MsgHandler != Q_NULLPTR)
                return MessageWindow::MsgHandler->postMsgEvent(type, msg);
        else
                fprintf(stderr, "%s\n", qasc(MessageWindow::QtMsgToQString(type, msg)));
}

void MessageWindow::customEvent(QEvent* event)
{
        if (static_cast<MessageWindow::EventType>(event->type()) == MessageWindow::MessageEvent) {
#ifdef __MINGW32__
                msgTextEdit.append(dynamic_cast<typename MessageEvent::MessageEvent* >(event)->msg);
#else
        #if defined(_WIN32)  || defined(__clang__)
                msgTextEdit.append(dynamic_cast<::MessageEvent* >(event)->msg);
        #else
               #if GCC_VERSION > 40407
                   msgTextEdit.append(dynamic_cast<typename MessageEvent::MessageEvent* >(event)->msg);
               #else
                   msgTextEdit.append(dynamic_cast<MessageEvent::MessageEvent* >(event)->msg);
               #endif
        #endif
#endif
        }
}

void MessageWindow::clearText()
{
    msgTextEdit.setPlainText("");
}

QString MessageWindow::getMessageBoxContents() {
    return msgTextEdit.toPlainText();
}

QString MessageWindow::getLogFilePath()
{
    return m_logFilePath;
}

void MessageWindow::postMsgEvent(QtMsgType type, char* msg)
{
    QString qmsg = MessageWindow::QtMsgToQString(type, msg);

    // Also write the message to a temporary logfile that gets permanent if the progam crashes.
    if (!m_logFilePath.isEmpty()) {
            QFile logFile(m_logFilePath);
        if (logFile.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream textStream(&logFile);
            textStream << qmsg.append("\n");
            logFile.close();
        } else {
            qWarning() << "Failed to write to logfile";
        }
    }

    switch (type) {
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
    case QtInfoMsg:
        qmsg.prepend("<FONT color=\"#000000\">");
        qmsg.append("</FONT>");
        break;
#endif
    case QtDebugMsg:
        qmsg.prepend("<FONT color=\"#0000FF\">");
        qmsg.append("</FONT>");
        break;
    case QtWarningMsg:
        qmsg.prepend("<FONT color=\"#FF8C00\">");
        qmsg.append("</FONT>");
        break;
    case QtCriticalMsg:
    case QtFatalMsg:
        qmsg.prepend("<B><FONT color=\"#FF0000\">");
        qmsg.append("</FONT></B>");
        break;
    default:
        qmsg.prepend("<FONT color=\"#000000\">");
        qmsg.append("</FONT>");
        break;
    }
    //it's impossible to change GUI directly from thread other than the main thread
    //so post message encapsulated by MessageEvent to the main thread's event queue
#ifdef __MINGW32__
    QCoreApplication::postEvent(this, new typename MessageEvent::MessageEvent(qmsg));
#else
#if defined(_WIN32)  || defined(__clang__)
    QCoreApplication::postEvent(this, new ::MessageEvent(qmsg));
#else
#if GCC_VERSION > 40407
    QCoreApplication::postEvent(this, new typename MessageEvent::MessageEvent(qmsg));
#else
    QCoreApplication::postEvent(this, new MessageEvent::MessageEvent(qmsg));
#endif
#endif
#endif
}

extern "C" MessageWindow* C_postMsgEvent(MessageWindow* p, int type, char* msg)
{
    if(p == 0) return p;
    if(type == 0) p->postMsgEvent(QtDebugMsg, msg);
    else if(type == 1) p->postMsgEvent(QtWarningMsg, msg);
    else if(type == 2) p->postMsgEvent(QtCriticalMsg, msg);
    else if(type == 3) p->postMsgEvent(QtCriticalMsg, msg);
    return p;
}

void MessageWindow::closeEvent(QCloseEvent* ce)
{
    Q_UNUSED(ce);
}

MessageEvent::MessageEvent(QString & msg):
        QEvent(static_cast<QEvent::Type>(MessageWindow::MessageEvent))
{
        this->msg = msg;
}

