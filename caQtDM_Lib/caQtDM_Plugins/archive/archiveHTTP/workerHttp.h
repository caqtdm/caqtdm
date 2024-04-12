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
#include "httpperformancedata.h"
#include "httpretrieval.h"

class Q_DECL_EXPORT WorkerHTTP : public QObject
{
    Q_OBJECT

public:
    WorkerHTTP();
    ~WorkerHTTP();

public slots:
    void workerFinish();
    HttpRetrieval *getArchive();
    void getFromArchive(QWidget *w,
                        indexes indexNew,
                        QString index_name,
                        MessageWindow *messageWindow,
                        MutexKnobData *mutexKnobDataP,
                        QSharedPointer<HttpPerformanceData> httpPerformanceData);

signals:
    void resultReady(indexes indexNew,
                     int valueCount,
                     QVector<double> XVals,
                     QVector<double> YVals,
                     QVector<double> YMinVals,
                     QVector<double> YMaxVals,
                     QString backend,
                     bool isFinalIteration);

private:
    /* Watch out when working with this element, it might behave entirely different amongst Qt versions.
     * In Qt 5.15 QVector is a distinct Class, whereas since Qt 6.0 it is a mere alias for QList.
     * */
    QVector<double> m_vecX, m_vecY, m_vecMinY, m_vecMaxY;

    HttpRetrieval *m_httpRetrieval;
    bool m_receivedContinueAt;
    QMutex m_globalMutex;
};

#endif // WORKERHTTP_H
