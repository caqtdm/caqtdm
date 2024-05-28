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
#include "archiveCA_plugin.h"
#include "archiverCommon.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
// Because QtInfoMsg doesnt exist there.
#define QtInfoMsg QtWarningMsg
#endif


#define qasc(x) x.toLatin1().constData()

// gives the plugin name back
QString ArchiveCA_Plugin::pluginName()
{
    return "archiveCA";
}

// constructor
ArchiveCA_Plugin::ArchiveCA_Plugin()
{
    qRegisterMetaType<indexes>("indexes");
    qRegisterMetaType<stdString>("stdString");
    qRegisterMetaType<QVector<double> >("QVector<double>");

    qDebug() << "ArchiveCA_Plugin: Create (epics channel archiver retrieval)";
    archiverCommon = new ArchiverCommon();

    connect(archiverCommon, SIGNAL(Signal_UpdateInterface(QMap<QString, indexes>)), this,SLOT(Callback_UpdateInterface(QMap<QString, indexes>)));
    connect(this, SIGNAL(Signal_StopUpdateInterface()), archiverCommon,SLOT(stopUpdateInterface()));
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(closeEvent()));
}

void ArchiveCA_Plugin::closeEvent(){
   qDebug() << "ArchiveCA_Plugin::closeEvent ";
   emit Signal_StopUpdateInterface();
}

// init communication
int ArchiveCA_Plugin::initCommunicationLayer(MutexKnobData *data, MessageWindow *messageWindow, QMap<QString, QString> options)
{
    mutexknobdataP = data;
    messagewindowP = messageWindow;
    return archiverCommon->initCommunicationLayer(data, messageWindow, options);
}

// this routine will be called now every 10 seconds to update the cartesianplot
// however when many data it may take much longer, then  suppress any new request
void ArchiveCA_Plugin::Callback_UpdateInterface( QMap<QString, indexes> listOfIndexes)
{
    int nbVal;

    precision = 5;

    // Index name
    stdString index_name =  "/gfa/archiver-data/archive_PRO_ST/index";

    //qDebug() << "====================== ArchiveCA_Plugin::Callback_UpdateInterface";

    QMap<QString, indexes>::const_iterator i = listOfIndexes.constBegin();

    while (i != listOfIndexes.constEnd()) {
        QThread *tmpThread = (QThread *) Q_NULLPTR;
        indexes indexNew = i.value();
        //qDebug() << i.key() << ": " << indexNew.indexX << indexNew.indexY << indexNew.pv << indexNew.w << endl;

        nbVal = 0;

        QMap<QString, QThread *>::iterator j = listOfThreads.find(indexNew.key);
        while (j !=listOfThreads.end() && j.key() == indexNew.key) {
            tmpThread = (QThread *) j.value();
            ++j;
        }

        //qDebug() << "tmpThread" << tmpThread;

        if((tmpThread != (QThread *) Q_NULLPTR) && tmpThread->isRunning()) {
            //qDebug() << "workerthread is running" << tmpThread->isRunning();
        } else {

            // Get Index name if specified for this widget
            if(caCartesianPlot* w = qobject_cast<caCartesianPlot *>((QWidget*) indexNew.w)) {
                QVariant var = w->property("archiverIndex");
                if(!var.isNull()) {
                    QString indexName = var.toString();
                    index_name = qasc(indexName);
                } else if(indexNew.init){
                    QString mess("ArchiveCA plugin -- no archiverIndex defined as dynamic property in widget "  + w->objectName() + ", defaulting to /gfa/archiver-data/archive_PRO_ST/index");
                    if(messagewindowP != (MessageWindow *) Q_NULLPTR) messagewindowP->postMsgEvent(QtInfoMsg, (char*) qasc(mess));
                }
            }

            WorkerCA *worker = new WorkerCA;
            QThread *tmpThread = new QThread;
            listOfThreads.insert(i.key(), tmpThread);
            worker->moveToThread(tmpThread);
            connect(tmpThread, SIGNAL(finished()), worker, SLOT(workerFinish()));
            connect(tmpThread, SIGNAL(finished()), tmpThread, SLOT(deleteLater()) );
            connect(this, SIGNAL(operate( QWidget *, indexes, const stdString)), worker,
                    SLOT(getFromArchive(QWidget *, indexes, stdString)));
            connect(worker, SIGNAL(resultReady(indexes, int, QVector<double>, QVector<double>, QString)), this,
                    SLOT(handleResults(indexes, int, QVector<double>, QVector<double>, QString)));
            tmpThread->start();

            //qDebug() << "CA emit operate";
            emit operate((QWidget *) messagewindowP ,indexNew, index_name);
        }
        ++i;
    }
}

void ArchiveCA_Plugin::handleResults(indexes indexNew, int nbVal, QVector<double> TimerN, QVector<double> YValsN, QString backend)
{
    //qDebug() << "in CA handle results" << nbVal << TimerN.count();

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
        //qDebug() << tmpThread << "ca quit";
        ++j;
    }

    for(int i=0; i< removeKeys.count(); i++) {
        listOfThreads.remove(removeKeys.at(i));
    }
}

// define data to be called
int ArchiveCA_Plugin::pvAddMonitor(int index, knobData *kData, int rate, int skip) {
    return archiverCommon->pvAddMonitor(index, kData, rate, skip);
}
// clear routines
int ArchiveCA_Plugin::pvClearMonitor(knobData *kData) {
    return archiverCommon->pvClearMonitor(kData);
}
int ArchiveCA_Plugin::pvFreeAllocatedData(knobData *kData) {
    return archiverCommon->pvFreeAllocatedData(kData);
}
int ArchiveCA_Plugin::TerminateIO() {
    return archiverCommon->TerminateIO();
}

// =======================================================================================================================================================
int ArchiveCA_Plugin::pvSetValue(char *pv, double rdata, int32_t idata, char *sdata, char *object, char *errmess, int forceType) {
    Q_UNUSED(pv); Q_UNUSED(rdata); Q_UNUSED(idata); Q_UNUSED(sdata); Q_UNUSED(object); Q_UNUSED(errmess); Q_UNUSED(forceType);
    return true;
}
int ArchiveCA_Plugin::pvSetWave(char *pv, float *fdata, double *ddata, int16_t *data16, int32_t *data32, char *sdata, int nelm, char *object, char *errmess) {
    Q_UNUSED(pv); Q_UNUSED(fdata); Q_UNUSED(ddata); Q_UNUSED(data16); Q_UNUSED(data32); Q_UNUSED(sdata); Q_UNUSED(nelm); Q_UNUSED(object); Q_UNUSED(errmess);
    return true;
}
int ArchiveCA_Plugin::pvGetTimeStamp(char *pv, char *timestamp) {
    Q_UNUSED(pv); Q_UNUSED(timestamp);
    return true;
}
int ArchiveCA_Plugin::pvGetDescription(char *pv, char *description) {
    Q_UNUSED(pv); Q_UNUSED(description);
    return true;
}
int ArchiveCA_Plugin::pvClearEvent(void * ptr) {
    Q_UNUSED(ptr);
    return true;
}
int ArchiveCA_Plugin::pvAddEvent(void * ptr) {
    Q_UNUSED(ptr);
    return true;
}
int ArchiveCA_Plugin::pvReconnect(knobData *kData) {
    Q_UNUSED(kData);
    return true;
}
int ArchiveCA_Plugin::pvDisconnect(knobData *kData) {
    Q_UNUSED(kData);
    return true;
}
int ArchiveCA_Plugin::FlushIO() {
    return true;
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#else
Q_EXPORT_PLUGIN2(ArchiveCA_Plugin, ArchiveCA_Plugin)
#endif

