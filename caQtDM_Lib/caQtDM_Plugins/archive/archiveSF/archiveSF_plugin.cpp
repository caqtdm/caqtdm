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
#include "archiveSF_plugin.h"
#include "archiverCommon.h"

#define qasc(x) x.toLatin1().constData()

// this sleep will not block the GUI and QThread::msleep is protected in Qt4.8 (so do not use that)
class Sleep
{
public:
    static void msleep(unsigned long msecs)
    {
#ifndef MOBILE_ANDROID
        QMutex mutex;
        mutex.lock();
        QWaitCondition waitCondition;
        waitCondition.wait(&mutex, msecs);
        mutex.unlock();
#else
        // not nice, but the above does not work on android now (does not wait)
        usleep(msecs * 100);
#endif
    }
};

// gives the plugin name back
QString ArchiveSF_Plugin::pluginName()
{
    return "archiveSF";
}

// constructor
ArchiveSF_Plugin::ArchiveSF_Plugin()
{
    qDebug() << "ArchivePlugin: Create";
    archiverCommon = new ArchiverCommon();
    fromArchive = new sfRetrieval();

    connect(archiverCommon, SIGNAL(Signal_UpdateInterface(QMap<QString, indexes>)), this,SLOT(Callback_UpdateInterface(QMap<QString, indexes>)));
}

// init communication
int ArchiveSF_Plugin::initCommunicationLayer(MutexKnobData *data, MessageWindow *messageWindow, QMap<QString, QString> options)
{
    mutexknobdataP = data;
    return archiverCommon->initCommunicationLayer(data, messageWindow, options);
}

// this routine will be called now every 10 seconds to update the cartesianplot
void ArchiveSF_Plugin::Callback_UpdateInterface( QMap<QString, indexes> listOfIndexes)
{
    QMutexLocker locker(&mutex);

    QMap<QString, indexes>::const_iterator i = listOfIndexes.constBegin();
    while (i != listOfIndexes.constEnd()) {
        QVector<double> TimerN;
        QVector<double> YValsN;

        indexes indexNew = i.value();
        qDebug() << i.key() << ": " << indexNew.indexX << indexNew.indexY << indexNew.pv << endl;

        QString key = indexNew.pv;

        int nbVal = 0;

        // archive sf
        struct timeb now;
        ftime(&now);
        qDebug() << "get from sf archive";
        double endSeconds = (double) now.time + (double) now.millitm / (double)1000;
        double startSeconds = endSeconds - indexNew.secondspast;
        QString response ="'response':{'format':'csv'}";
        QString channels = "'channels': [ '" + key + "' ]";
        QString range = "'range': { 'startSeconds' : '" + QString::number(startSeconds, 'g', 10) + "', 'endSeconds' : '" + QString::number(endSeconds, 'g', 10) + "'}";
        QString fields = "'fields':['channel','iocSeconds','value']}";
        QString agg = "";
        QString total = "{" + response + "," + range + "," + channels + "," + fields + "," + agg + "}";
        total = total.replace("'", "\"");
        QByteArray json_str = total.toUtf8();
        if(fromArchive->requestUrl(QUrl("http://data-api.psi.ch/sf/query"), json_str, indexNew.secondspast)) {
            if((nbVal = fromArchive->getCount()) > 0) {
                TimerN.resize(fromArchive->getCount());
                YValsN.resize(fromArchive->getCount());
                fromArchive->getData(TimerN, YValsN);
            }
        }

        qDebug() << "nbval=" << nbVal;

        archiverCommon->updateCartesian(nbVal, indexNew, TimerN, YValsN);

        ++i;

    }
    qDebug() << "update finished";
}

// define data to be called
int ArchiveSF_Plugin::pvAddMonitor(int index, knobData *kData, int rate, int skip) {
    return archiverCommon->pvAddMonitor(index, kData, rate, skip);
}
// clear routines
int ArchiveSF_Plugin::pvClearMonitor(knobData *kData) {
    return archiverCommon->pvClearMonitor(kData);
}
int ArchiveSF_Plugin::pvFreeAllocatedData(knobData *kData) {
    return archiverCommon->pvFreeAllocatedData(kData);
}
int ArchiveSF_Plugin::TerminateIO() {
    return archiverCommon->TerminateIO();
}

// =======================================================================================================================================================
int ArchiveSF_Plugin::pvSetValue(char *pv, double rdata, int32_t idata, char *sdata, char *object, char *errmess, int forceType) {
    Q_UNUSED(pv); Q_UNUSED(rdata); Q_UNUSED(idata); Q_UNUSED(sdata); Q_UNUSED(object); Q_UNUSED(errmess); Q_UNUSED(forceType);
    return true;
}
int ArchiveSF_Plugin::pvSetWave(char *pv, float *fdata, double *ddata, int16_t *data16, int32_t *data32, char *sdata, int nelm, char *object, char *errmess) {
    Q_UNUSED(pv); Q_UNUSED(fdata); Q_UNUSED(ddata); Q_UNUSED(data16); Q_UNUSED(data32); Q_UNUSED(sdata); Q_UNUSED(nelm); Q_UNUSED(object); Q_UNUSED(errmess);
    return true;
}
int ArchiveSF_Plugin::pvGetTimeStamp(char *pv, char *timestamp) {
    Q_UNUSED(pv); Q_UNUSED(timestamp);
    return true;
}
int ArchiveSF_Plugin::pvGetDescription(char *pv, char *description) {
    Q_UNUSED(pv); Q_UNUSED(description);
    return true;
}
int ArchiveSF_Plugin::pvClearEvent(void * ptr) {
    Q_UNUSED(ptr);
    return true;
}
int ArchiveSF_Plugin::pvAddEvent(void * ptr) {
    Q_UNUSED(ptr);
    return true;
}
int ArchiveSF_Plugin::pvReconnect(knobData *kData) {
    Q_UNUSED(kData);
    return true;
}
int ArchiveSF_Plugin::pvDisconnect(knobData *kData) {
    Q_UNUSED(kData);
    return true;
}
int ArchiveSF_Plugin::FlushIO() {
    return true;
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#else
Q_EXPORT_PLUGIN2(ArchiveSF_Plugin, ArchiveSF_Plugin)
#endif

