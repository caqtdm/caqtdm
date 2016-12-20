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
#include <QDebug>
#include <QThread>
#include "archivePRO_plugin.h"
#include "archiverCommon.h"

#define qasc(x) x.toLatin1().constData()

// gives the plugin name back
QString ArchivePRO_Plugin::pluginName()
{
    return "archivePRO";
}

// constructor
ArchivePRO_Plugin::ArchivePRO_Plugin()
{
    qDebug() << "ArchivePro_Plugin: Create (logging retrieval)";
    archiverCommon = new ArchiverCommon();

    connect(archiverCommon, SIGNAL(Signal_UpdateInterface(QMap<QString, indexes>)), this,SLOT(Callback_UpdateInterface(QMap<QString, indexes>)));
}

// init communication
int ArchivePRO_Plugin::initCommunicationLayer(MutexKnobData *data, MessageWindow *messageWindow, QMap<QString, QString> options)
{
    mutexknobdataP = data;
    return archiverCommon->initCommunicationLayer(data, messageWindow, options);
}

// this routine will be called now every 10 seconds to update the cartesianplot
void ArchivePRO_Plugin::Callback_UpdateInterface( QMap<QString, indexes> listOfIndexes)
{
    QMutexLocker locker(&mutex);

    //qDebug() << "-------------------- ArchivePRO_Plugin::Callback_UpdateInterface";

    QMap<QString, indexes>::const_iterator i = listOfIndexes.constBegin();
    while (i != listOfIndexes.constEnd()) {
        QVector<double> TimerN;
        QVector<double> YValsN;

        indexes indexNew = i.value();
        //qDebug() << i.key() << ": " << indexNew.indexX << indexNew.indexY << indexNew.pv << endl;

        QString key = indexNew.pv;

        int nbVal = 0;

        char dev[40];
        float *Timer, *YVals;

        QString loggingServer = (QString)  qgetenv("LOGGINGSERVER");
        loggingServer = loggingServer.toUpper();
        if(loggingServer.isEmpty() || !loggingServer.contains("PRO")) setenv("LOGGINGSERVER", "proscan-lgexp.psi.ch", 1);

        //qDebug() << "get from archive at " << "proscan-lgexp.psi.ch";
        int startHours = indexNew.secondsPast / 3600;
        int day =  startHours/24 + 1;
        int arraySize =  3600/5 * 24 * (day+1);

        Timer = (float*) malloc(arraySize * sizeof(float));
        YVals = (float*) malloc(arraySize * sizeof(float));

        strcpy(dev, qasc(key));
        GetLogShift(indexNew.secondsPast, dev, &nbVal, Timer, YVals);

        // resize arrays
        TimerN.resize(nbVal);
        YValsN.resize(nbVal);
        int k=0;
        for(int j=0; j<nbVal; j++) {
            if(Timer[j] >= -startHours) {
                TimerN[k] = Timer[j];
                YValsN[k] = YVals[j];
                ++k;
            }
        }
        nbVal = k;
        free(Timer);
        free(YVals);

        //qDebug() << "nbval=" << nbVal;

        archiverCommon->updateCartesian(nbVal, indexNew, TimerN, YValsN);

        ++i;
    }
    //qDebug() << "ArchivePRO_Plugin::update finished";
}

// define data to be called
int ArchivePRO_Plugin::pvAddMonitor(int index, knobData *kData, int rate, int skip) {
    return archiverCommon->pvAddMonitor(index, kData, rate, skip);
}
// clear routines
int ArchivePRO_Plugin::pvClearMonitor(knobData *kData) {
    return archiverCommon->pvClearMonitor(kData);
}
int ArchivePRO_Plugin::pvFreeAllocatedData(knobData *kData) {
    return archiverCommon->pvFreeAllocatedData(kData);
}
int ArchivePRO_Plugin::TerminateIO() {
    return archiverCommon->TerminateIO();
}

// =======================================================================================================================================================
int ArchivePRO_Plugin::pvSetValue(char *pv, double rdata, int32_t idata, char *sdata, char *object, char *errmess, int forceType) {
    Q_UNUSED(pv); Q_UNUSED(rdata); Q_UNUSED(idata); Q_UNUSED(sdata); Q_UNUSED(object); Q_UNUSED(errmess); Q_UNUSED(forceType);
    return true;
}
int ArchivePRO_Plugin::pvSetWave(char *pv, float *fdata, double *ddata, int16_t *data16, int32_t *data32, char *sdata, int nelm, char *object, char *errmess) {
    Q_UNUSED(pv); Q_UNUSED(fdata); Q_UNUSED(ddata); Q_UNUSED(data16); Q_UNUSED(data32); Q_UNUSED(sdata); Q_UNUSED(nelm); Q_UNUSED(object); Q_UNUSED(errmess);
    return true;
}
int ArchivePRO_Plugin::pvGetTimeStamp(char *pv, char *timestamp) {
    Q_UNUSED(pv); Q_UNUSED(timestamp);
    return true;
}
int ArchivePRO_Plugin::pvGetDescription(char *pv, char *description) {
    Q_UNUSED(pv); Q_UNUSED(description);
    return true;
}
int ArchivePRO_Plugin::pvClearEvent(void * ptr) {
    Q_UNUSED(ptr);
    return true;
}
int ArchivePRO_Plugin::pvAddEvent(void * ptr) {
    Q_UNUSED(ptr);
    return true;
}
int ArchivePRO_Plugin::pvReconnect(knobData *kData) {
    Q_UNUSED(kData);
    return true;
}
int ArchivePRO_Plugin::pvDisconnect(knobData *kData) {
    Q_UNUSED(kData);
    return true;
}
int ArchivePRO_Plugin::FlushIO() {
    return true;
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#else
Q_EXPORT_PLUGIN2(ArchivePRO_Plugin, ArchivePRO_Plugin)
#endif

