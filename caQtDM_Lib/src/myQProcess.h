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

#ifndef MYQPROCESS_H
#define MYQPROCESS_H

#include "caQtDM_Lib_global.h"
#include <QObject>
#include <QProcess>
#include <QIODevice>

class CAQTDM_LIBSHARED_EXPORT myQProcess : public QObject
{
    Q_OBJECT
public:

    explicit myQProcess(QObject *parent = 0);
    ~myQProcess() {
     if (process != (QProcess *) 0)
        if (started && process->state() != QProcess::NotRunning)
            process->kill();
    }

public Q_SLOTS:
    void start(QString program, QIODevice::OpenMode mode = QIODevice::ReadWrite);

private:
    QProcess * process;
    bool started;
    QString prog;

private Q_SLOTS:
    void error(QProcess::ProcessError error);
    void finished(int exitCode, QProcess::ExitStatus status);
};

#endif
