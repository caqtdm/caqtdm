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


#include "myQProcess.h"
#include <QCoreApplication>
#include <QMessageBox>
#include <QtDebug>

myQProcess::myQProcess(QObject *parent) : QObject(parent), started(false)
{
    process = new QProcess(this);
    process->setWorkingDirectory(".");
    process->setProcessChannelMode(QProcess::MergedChannels);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    connect(process, SIGNAL(error(QProcess::ProcessError)), SLOT(error(QProcess::ProcessError)));
#else
    connect(process, SIGNAL(errorOccurred(QProcess::ProcessError)), SLOT(error(QProcess::ProcessError)));
#endif

    connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(finished(int, QProcess::ExitStatus)));
}

void myQProcess::start(QString program, QIODevice::OpenMode mode)
{
    started = true;
    prog = program;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    process->start(program, mode);
#else
     QStringList arguments=QStringList();
    process->start(program,arguments, mode);
#endif

    //qDebug() << process->readAllStandardOutput();
}

void myQProcess::error(QProcess::ProcessError err)
{
    switch(err)
    {
    case QProcess::FailedToStart:
        started = false;
        QMessageBox::information(Q_NULLPTR,"FailedToStart","FailedToStart <" + prog + ">");
        break;
    case QProcess::Crashed:
        started = false;
        QMessageBox::information(Q_NULLPTR,"Crashed","Crashed <" + prog + ">");
        break;
    case QProcess::Timedout:
        QMessageBox::information(Q_NULLPTR,"FailedToStart","FailedToStart <" + prog + ">");
        break;
    case QProcess::WriteError:
        QMessageBox::information(Q_NULLPTR,"Timedout","Timedout <" + prog + ">");
        break;
    case QProcess::ReadError:
        QMessageBox::information(Q_NULLPTR,"ReadError","ReadError <" + prog + ">");
        break;
    case QProcess::UnknownError:
        QMessageBox::information(Q_NULLPTR,"UnknownError","UnknownError <" + prog + ">");
        break;
    default:
        QMessageBox::information(Q_NULLPTR,"default","default <" + prog + ">");
        break;
    }
    process->deleteLater();
}

void myQProcess::finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    started = false;
    if(exitStatus == QProcess::CrashExit) {
        QMessageBox::information(Q_NULLPTR,"Crashed","Crashed <" + prog + ">");
    } else if(exitCode != 0) {
        QMessageBox::information(Q_NULLPTR,"Some failure","failure on <" + prog + ">");
    }
    process->deleteLater();
}

