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
#include <QCoreApplication>
#include <QMutexLocker>
#include <stdio.h>
#include <time.h>
#include <sys/timeb.h>

#define GCC_VERSION (__GNUC__ * 10000 \
                               + __GNUC_MINOR__ * 100 \
                               + __GNUC_PATCHLEVEL__)

const char* MessageWindow::WINDOW_TITLE = "caQtDM Messages";
MessageWindow* MessageWindow::MsgHandler = NULL;

MessageWindow::MessageWindow(QWidget* parent) : QDockWidget(parent)
{

    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    msgTextEdit.setFont(font);

    setFeatures(QDockWidget::NoDockWidgetFeatures);
    setWindowTitle(tr(WINDOW_TITLE));
    msgTextEdit.setReadOnly(true);
    setWidget(&msgTextEdit);
    MessageWindow::MsgHandler = this;
    setMinimumSize(600, 150);
    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowMinMaxButtonsHint);
    show();

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
    sprintf(prTime, "%02d-%02d-%04d %02d:%02d:%02d ", timess->tm_mday, timess->tm_mon, timess->tm_year+1900,  timess->tm_hour, timess->tm_min, timess->tm_sec);
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
}

void MessageWindow::AppendMsgWrapper(QtMsgType type, char* msg)
{
        static QMutex mutex;
        QMutexLocker locker(&mutex);

        if (MessageWindow::MsgHandler != NULL)
                return MessageWindow::MsgHandler->postMsgEvent(type, msg);
        else
                fprintf(stderr, "%s\n", MessageWindow::QtMsgToQString(type, msg).toLatin1().constData());
}

void MessageWindow::customEvent(QEvent* event)
{
        if (static_cast<MessageWindow::EventType>(event->type()) == MessageWindow::MessageEvent) {
#ifdef __MINGW32__
                msgTextEdit.append(dynamic_cast<typename MessageEvent::MessageEvent* >(event)->msg);
#else
        #ifdef _WIN32
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

void MessageWindow::postMsgEvent(QtMsgType type, char* msg)
{
    QString qmsg = MessageWindow::QtMsgToQString(type, msg);
    switch (type) {
    case QtDebugMsg:
        qmsg.prepend("<FONT color=\"#000000\">");
        qmsg.append("</FONT>");
        break;
    case QtWarningMsg:
        qmsg.prepend("<FONT color=\"#0000FF\">");
        qmsg.append("</FONT>");
        break;
    case QtCriticalMsg:
    case QtFatalMsg:
        qmsg.prepend("<B><FONT color=\"#FF0000\">");
        qmsg.append("</FONT></B>");
        break;
    default:
        qmsg.prepend("<FONT color=\"#0000FF\">");
        qmsg.append("</FONT>");
        break;
    }
    //it's impossible to change GUI directly from thread other than the main thread
    //so post message encapsulated by MessageEvent to the main thread's event queue
#ifdef __MINGW32__
    QCoreApplication::postEvent(this, new typename MessageEvent::MessageEvent(qmsg));
#else
#ifdef _WIN32
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

