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
#include <QApplication>
#include <QThread>
#include "archiveHIPA_plugin.h"
#include "archiverCommon.h"

#define qasc(x) x.toLatin1().constData()

// gives the plugin name back
QString ArchiveHIPA_Plugin::pluginName()
{
    return "archiveHIPA";
}

// constructor
ArchiveHIPA_Plugin::ArchiveHIPA_Plugin()
{
    qRegisterMetaType<indexes>("indexes");
    qRegisterMetaType<QVector<float> >("QVector<float>");

    qDebug() << "ArchiveHIPA_Plugin: Create (logging retrieval)";
    archiverCommon = new ArchiverCommon();

    connect(archiverCommon, SIGNAL(Signal_UpdateInterface(QMap<QString, indexes>)), this,SLOT(Callback_UpdateInterface(QMap<QString, indexes>)));
    connect(this, SIGNAL(Signal_StopUpdateInterface()), archiverCommon,SLOT(stopUpdateInterface()));
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(closeEvent()));
}

void ArchiveHIPA_Plugin::closeEvent(){
   qDebug() << "ArchiveHIPA_Plugin::closeEvent ";
   emit Signal_StopUpdateInterface();
}

// init communication
int ArchiveHIPA_Plugin::initCommunicationLayer(MutexKnobData *data, MessageWindow *messageWindow, QMap<QString, QString> options)
{
    mutexknobdataP = data;
    return archiverCommon->initCommunicationLayer(data, messageWindow, options);
}


// this routine will be called now every 10 seconds to update the cartesianplot
// however when many data it may take much longer, then  suppress any new request
void ArchiveHIPA_Plugin::Callback_UpdateInterface( QMap<QString, indexes> listOfIndexes)
{
    //qDebug() << "-------------------- ArchiveHIPA_Plugin::Callback_UpdateInterface";

    QMap<QString, indexes>::const_iterator i = listOfIndexes.constBegin();

    while (i != listOfIndexes.constEnd()) {
        QThread *tmpThread = (QThread *) 0;
        indexes indexNew = i.value();
        //qDebug() <<" -------------" << i.key() << ": " << indexNew.indexX << indexNew.indexY << indexNew.pv << indexNew.w;

        QMap<QString, QThread *>::iterator j = listOfThreads.find(indexNew.key);
        while (j !=listOfThreads.end() && j.key() == indexNew.key) {
            tmpThread = (QThread *) j.value();
            ++j;
        }

        //qDebug() << "tmpThread" << tmpThread;

        if((tmpThread != (QThread *) 0) && tmpThread->isRunning()) {
            //qDebug() << "workerthread is running" << tmpThread->isRunning();
        } else {
            WorkerHIPA *worker = new WorkerHIPA;
            QThread *tmpThread = new QThread;
            listOfThreads.insert(i.key(), tmpThread);
            worker->moveToThread(tmpThread);
            connect(tmpThread, SIGNAL(finished()), worker, SLOT(workerFinish()));
            connect(tmpThread, SIGNAL(finished()), tmpThread, SLOT(deleteLater()) );
            connect(this, SIGNAL(operate( QWidget *, indexes)), worker,
                          SLOT(getFromArchive(QWidget *, indexes)));
            connect(worker, SIGNAL(resultReady(indexes, int, QVector<double>, QVector<double>, QString)), this,
                            SLOT(handleResults(indexes, int, QVector<double>, QVector<double>, QString)));
            tmpThread->start();

            //qDebug() << "HIPA emit operate";
            emit operate((QWidget *) messagewindowP ,indexNew);
            disconnect(worker);
        }

        ++i;
    }
   // qDebug() << "-------------------- ArchiveHIPA_Plugin::Callback_UpdateInterface finish";
}

void ArchiveHIPA_Plugin::handleResults(indexes indexNew, int nbVal, QVector<double> TimerN, QVector<double> YValsN,  QString backend)
{
    //qDebug() << "in HIPA handle results" << nbVal << TimerN.count();

    if(nbVal > 0) {
        TimerN.resize(nbVal);
        YValsN.resize(nbVal);
        archiverCommon->updateCartesian(nbVal, indexNew, TimerN, YValsN, backend);
        TimerN.resize(0);
        YValsN.resize(0);
    }

    QList<QString> removeKeys;
    removeKeys.clear();

    QMap<QString, QThread *>::iterator j = listOfThreads.find(indexNew.key);
    while (j !=listOfThreads.end() && j.key() == indexNew.key) {
        QThread *tmpThread = (QThread*) j.value();
        tmpThread->quit();
        removeKeys.append(indexNew.key);
        //qDebug() << tmpThread << "hipa quit";
        ++j;
    }

    for(int i=0; i< removeKeys.count(); i++) {
        listOfThreads.remove(removeKeys.at(i));
    }
}

// define data to be called
int ArchiveHIPA_Plugin::pvAddMonitor(int index, knobData *kData, int rate, int skip) {
    return archiverCommon->pvAddMonitor(index, kData, rate, skip);
}
// clear routines
int ArchiveHIPA_Plugin::pvClearMonitor(knobData *kData) {
    return archiverCommon->pvClearMonitor(kData);
}
int ArchiveHIPA_Plugin::pvFreeAllocatedData(knobData *kData) {
    return archiverCommon->pvFreeAllocatedData(kData);
}
int ArchiveHIPA_Plugin::TerminateIO() {
    return archiverCommon->TerminateIO();
}

// =======================================================================================================================================================
int ArchiveHIPA_Plugin::pvSetValue(char *pv, double rdata, int32_t idata, char *sdata, char *object, char *errmess, int forceType) {
    Q_UNUSED(pv); Q_UNUSED(rdata); Q_UNUSED(idata); Q_UNUSED(sdata); Q_UNUSED(object); Q_UNUSED(errmess); Q_UNUSED(forceType);
    return true;
}
int ArchiveHIPA_Plugin::pvSetWave(char *pv, float *fdata, double *ddata, int16_t *data16, int32_t *data32, char *sdata, int nelm, char *object, char *errmess) {
    Q_UNUSED(pv); Q_UNUSED(fdata); Q_UNUSED(ddata); Q_UNUSED(data16); Q_UNUSED(data32); Q_UNUSED(sdata); Q_UNUSED(nelm); Q_UNUSED(object); Q_UNUSED(errmess);
    return true;
}
int ArchiveHIPA_Plugin::pvGetTimeStamp(char *pv, char *timestamp) {
    Q_UNUSED(pv); Q_UNUSED(timestamp);
    return true;
}
int ArchiveHIPA_Plugin::pvGetDescription(char *pv, char *description) {
    Q_UNUSED(pv); Q_UNUSED(description);
    return true;
}
int ArchiveHIPA_Plugin::pvClearEvent(void * ptr) {
    Q_UNUSED(ptr);
    return true;
}
int ArchiveHIPA_Plugin::pvAddEvent(void * ptr) {
    Q_UNUSED(ptr);
    return true;
}
int ArchiveHIPA_Plugin::pvReconnect(knobData *kData) {
    Q_UNUSED(kData);
    return true;
}
int ArchiveHIPA_Plugin::pvDisconnect(knobData *kData) {
    Q_UNUSED(kData);
    return true;
}
int ArchiveHIPA_Plugin::FlushIO() {
    return true;
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#else
Q_EXPORT_PLUGIN2(ArchiveHIPA_Plugin, ArchiveHIPA_Plugin)
#endif

