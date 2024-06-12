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

/*
 * This file is the successor of archvierCommon.h and used to implement new features in build with Qt 5.15 and upwards,
 * while keeping backwards compatibility until the old archivers are retired.
 */

#ifndef ARCHIVERGENERAL_H
#define ARCHIVERGENERAL_H

#include <QList>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QThread>
#include <QTimer>
#include "MessageWindow.h"
#include "cacartesianplot.h"
#include "mutexKnobData.h"
#include <qwt.h>

struct indexes
{
    QString key;
    int indexX;
    int indexY;
    int secondsPast;
    QString pv;
    float updateSeconds;
    struct timeb lastUpdateTime;
    QWidget *w;
    int nrOfBins;
    QMutex *mutexP;
    bool init;
    QString backend;
    int updateSecondsOrig;
    bool timeAxis;
};
#define CHAR_ARRAY_LENGTH 200

class Q_DECL_EXPORT ArchiverGeneral : public QObject
{
    Q_OBJECT

public:
    ArchiverGeneral();
    ~ArchiverGeneral();

    int initCommunicationLayer(MutexKnobData *data,
                               MessageWindow *messageWindow,
                               QMap<QString, QString> options);
    int pvAddMonitor(int index, knobData *kData, int rate, int skip);
    int pvClearMonitor(knobData *kData);
    int pvFreeAllocatedData(knobData *kData);
    int pvClearEvent(void *ptr);
    int pvAddEvent(void *ptr);
    int TerminateIO() { return true; }
    void updateCartesian(int nbVal,
                         indexes indexNew,
                         QVector<double> XValsN,
                         QVector<double> YValsN,
                         QString backend);
    void updateSecondsPast(indexes indexNew, bool original);
    QTimer *timer;

    QMutex* globalMutex();

protected:
signals:
    void Signal_UpdateInterface(QMap<QString, indexes> listOfIndexes);
    void Signal_AbortOutstandingRequests(QString key);

private slots:
    void updateInterface();
    void stopUpdateInterface();

private:
    typedef struct
    {
        char Dev[40];
    } device;
    QMutex m_globalMutex;
    QMutex *mutexP;
    MutexKnobData *mutexknobdataP;
    MessageWindow *messagewindowP;

    QMap<QString, indexes> listOfIndexes;
    QMap<QString, indexes> alreadyProcessedIndexes; // used to prevent Indexes from being processed twice

    bool timerRunning;
};

#endif
