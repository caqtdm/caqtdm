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
    connect(process, SIGNAL(error(QProcess::ProcessError)), SLOT(error(QProcess::ProcessError)));
    connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(finished(int, QProcess::ExitStatus)));
}

void myQProcess::start(QString program, QIODevice::OpenMode mode)
{
    started = true;
    prog = program;
    process->start(program, mode);
    //qDebug() << process->readAllStandardOutput();
}

void myQProcess::error(QProcess::ProcessError err)
{
    switch(err)
    {
    case QProcess::FailedToStart:
        started = false;
        QMessageBox::information(0,"FailedToStart","FailedToStart <" + prog + ">");
        break;
    case QProcess::Crashed:
        started = false;
        QMessageBox::information(0,"Crashed","Crashed <" + prog + ">");
        break;
    case QProcess::Timedout:
        QMessageBox::information(0,"FailedToStart","FailedToStart <" + prog + ">");
        break;
    case QProcess::WriteError:
        QMessageBox::information(0,"Timedout","Timedout <" + prog + ">");
        break;
    case QProcess::ReadError:
        QMessageBox::information(0,"ReadError","ReadError <" + prog + ">");
        break;
    case QProcess::UnknownError:
        QMessageBox::information(0,"UnknownError","UnknownError <" + prog + ">");
        break;
    default:
        QMessageBox::information(0,"default","default <" + prog + ">");
        break;
    }
    process->deleteLater();
}

void myQProcess::finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    started = false;
    if(exitStatus == QProcess::CrashExit) {
        QMessageBox::information(0,"Crashed","Crashed <" + prog + ">");
    } else if(exitCode != 0) {
        QMessageBox::information(0,"Some failure","failure on <" + prog + ">");
    }
    process->deleteLater();
}

