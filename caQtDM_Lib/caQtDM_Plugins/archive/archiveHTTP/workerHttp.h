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

#ifndef WORKERHTTP_H
#define WORKERHTTP_H

#include "archiverCommon.h"
#include "httpretrieval.h"

class Q_DECL_EXPORT WorkerHTTP : public QObject
{
    Q_OBJECT

public:
    WorkerHTTP();
    ~WorkerHTTP();

private:
    QVector<double> m_vecX, m_vecY;

public slots:
    void workerFinish();
    HttpRetrieval *getArchive();
    void getFromArchive(QWidget *w,
                        indexes indexNew,
                        QString index_name,
                        MessageWindow *messageWindow);

signals:
    void resultReady(indexes indexNew,
                     int nbVal,
                     QVector<double> TimerN,
                     QVector<double> YValsN,
                     QString backend);

private:
    HttpRetrieval *m_httpRetrieval;
};

#endif // WORKERHTTP_H
