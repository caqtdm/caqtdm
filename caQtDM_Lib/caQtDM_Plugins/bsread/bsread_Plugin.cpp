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
#include "bsread_Plugin.h"
#include "zmq.h"
#include "bsread_decode.h"
#include "bsread_dispatchercontrol.h"

// as defined in knobDefines.h
//caType {caSTRING	= 0, caINT = 1, caFLOAT = 2, caENUM = 3, caCHAR = 4, caLONG = 5, caDOUBLE = 6};

// this demo plugin just gives you an idea how to use a plugin; for more details you should take a look
// at the epics3 plugin

// gives the plugin name back
QString bsreadPlugin::pluginName()
{
    return "bsread";
}

// constructor
bsreadPlugin::bsreadPlugin()
{
    //qDebug() << "bsreadPlugin: Create";
    DispatcherThread=new QThread(this);
    Dispatcher=new bsread_dispatchercontrol();
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(closeEvent()));

}
bsreadPlugin:: ~bsreadPlugin()
{
    //qDebug() << "bsreadPlugin: Start Destroy";
    //qDebug() << "WaitPre:" <<DispatcherThread->isFinished();
    //qDebug() << "bsreadPlugin: ThreadID" << QThread::currentThreadId();
    Dispatcher->setTerminate();
    //Dispatcher->deleteLater();

    //qDebug() << "bsreadPlugin: DispatcherThread Inter:"<<DispatcherThread->isInterruptionRequested();
    //qDebug() << "bsreadPlugin: DispatcherThread Runni:"<<DispatcherThread->isRunning();
    //qDebug() << "bsreadPlugin: DispatcherThread FINI :"<<DispatcherThread->isFinished();

    delete(DispatcherThread);
    delete(Dispatcher);
/*
    while (!DispatcherThread.isFinished()){
      qDebug() << "Wait:" <<DispatcherThread.isFinished();
       QThread::sleep(5);
    }
*/
    qDebug() << "bsreadPlugin: Destroy";
}

// in this demo we update our interface here; normally you should update in from your controlsystem
// take a look how monitors are treated in the epics3 plugin
void bsreadPlugin::updateInterface()
{
    double newValue;

    QMutexLocker locker(&mutex);

    // go through our devices
    foreach(int index, listOfIndexes) {
        knobData* kData = mutexknobdataP->GetMutexKnobDataPtr(index);
        if((kData != (knobData *) 0) && (kData->index != -1)) {
            QString key = kData->pv;

            // find this pv in our internal double values list (assume for now we are only treating doubles)
            // and increment its value
            QMap<QString, double>::iterator i = listOfDoubles.find(key);
            while (i !=listOfDoubles.end() && i.key() == key) {
                newValue = i.value();
                break;
            }
            // update some data
            kData->edata.rvalue = newValue;
            kData->edata.fieldtype = caDOUBLE;
            kData->edata.connected = true;
            kData->edata.accessR = kData->edata.accessW = true;
            kData->edata.monitorCount++;
            mutexknobdataP->SetMutexKnobData(kData->index, *kData);
            mutexknobdataP->SetMutexKnobDataReceived(kData);
        }
    }
}

// in this demo we update our values here
void bsreadPlugin::updateValues()
{
    QMutexLocker locker(&mutex);
    QMap<QString, double>::iterator i;
    for (i = listOfDoubles.begin(); i != listOfDoubles.end(); ++i) i.value()++;
}

// initialize our communicationlayer with everything you need
int bsreadPlugin::initCommunicationLayer(MutexKnobData *data, MessageWindow *messageWindow,QMap<QString, QString> options)
{
    int i;
    Q_UNUSED(options);
    qDebug() << "bsreadPlugin: InitCommunicationLayer" << data;
    mutexknobdataP = data;
    messagewindowP = messageWindow;
    // INIT ZMQ Layer
    zmqcontex = zmq_init (1);

    initValue = 0.0;
    QString DispacherConfig = (QString)  qgetenv("BSREAD_DISPATCHER");
    if (DispacherConfig.length()>0){



        Dispatcher->set_Dispatcher(&DispacherConfig);
        Dispatcher->setMessagewindow(messagewindowP);
        Dispatcher->setZmqcontex(zmqcontex);
        Dispatcher->setMutexknobdataP(data);
        Dispatcher->moveToThread(DispatcherThread);
        connect(DispatcherThread, SIGNAL(started()), Dispatcher, SLOT(process()));
        connect(Dispatcher, SIGNAL(finished()), DispatcherThread, SLOT(quit()));
        DispatcherThread->start();

        //qDebug() << "Start Status:" <<DispatcherThread->isFinished();

    }else{
        QString msg="Using Manual BSREAD Connection";
        if(messagewindowP != (MessageWindow *) 0) messagewindowP->postMsgEvent(QtDebugMsg,(char*) msg.toLatin1().constData());
        QString ZMQ_ADDR_LIST = (QString)  qgetenv("BSREAD_ZMQ_ADDR_LIST");

        if (!ZMQ_ADDR_LIST.isEmpty()){
    #ifdef _MSC_VER
            QStringList BSREAD_ZMQ_ADDRS = ZMQ_ADDR_LIST.split(";");
    #else
            QStringList BSREAD_ZMQ_ADDRS = ZMQ_ADDR_LIST.split(" ");
    #endif
            for (i=0;i<BSREAD_ZMQ_ADDRS.count();i++){

                bsreadconnections.append(new bsread_Decode(zmqcontex,BSREAD_ZMQ_ADDRS.at(i)));
                bsreadThreads.append(new QThread(this));
                bsreadconnections.last()->setKnobData(mutexknobdataP);
                bsreadconnections.last()->moveToThread(bsreadThreads.last());
                connect(bsreadThreads.last(), SIGNAL(started()), bsreadconnections.last(), SLOT(process()));
                connect(bsreadconnections.last(), SIGNAL(finished()), bsreadThreads.last(), SLOT(quit()));
                connect(bsreadThreads.last(), SIGNAL(finished()), bsreadThreads.last(), SLOT(deleteLater()));
                connect(bsreadconnections.last(), SIGNAL(finished()), bsreadconnections.last(), SLOT(deleteLater()));
                bsreadThreads.last()->start();
                msg="Connection started: ";
                disconnect(bsreadconnections.last());
                msg.append(BSREAD_ZMQ_ADDRS.at(i));
                if(messagewindowP != (MessageWindow *) 0) messagewindowP->postMsgEvent(QtDebugMsg,(char*) msg.toLatin1().constData());
            }
        }else{
            QString msg="no BSREAD Connection";
            if(messagewindowP != (MessageWindow *) 0) messagewindowP->postMsgEvent(QtDebugMsg,(char*) msg.toLatin1().constData());

        }
    }


    return true;
}

// caQtDM_Lib will call this routine for defining a monitor
int bsreadPlugin::pvAddMonitor(int index, knobData *kData, int rate, int skip) {
    Q_UNUSED(index);
    Q_UNUSED(rate);
    Q_UNUSED(skip);

    int i;
    QMutexLocker locker(&mutex);
    Dispatcher->add_Channel(kData->pv,kData->index);
    //qDebug() << "bsreadPlugin:pvAddMonitor" << kData->pv << kData->index << kData;
    i=0;

    while ((i<bsreadconnections.size())){
     bsreadconnections.at(i)->bsread_DataMonitorConnection(kData);
     i++;
    }



    return 0;
}

// caQtDM_Lib will call this routine for getting rid of a monitor
int bsreadPlugin::pvClearMonitor(knobData *kData) {
    int i=0;

    QMutexLocker locker(&mutex);
    Dispatcher->rem_Channel(QString(kData->pv),kData->index);
    //qDebug() << "bsreadPlugin:pvClearMonitor" << kData << kData->pv << kData->index <<bsreadconnections.size();
    while (i<bsreadconnections.size()){
        bsreadconnections.at(i)->bsread_DataMonitorUnConnect(kData);
		i++;
    }
    return true;
}
int bsreadPlugin::pvFreeAllocatedData(knobData *kData)
{
    //qDebug() << "DemoPlugin:pvFreeAllocatedData";
    if (kData->edata.info != (void *) 0) {
        free(kData->edata.info);
        kData->edata.info = (void*) 0;
    }
    if(kData->edata.dataB != (void*) 0) {
        free(kData->edata.dataB);
        kData->edata.dataB = (void*) 0;
    }

    return true;
}

// caQtDM_Lib will call this routine for setting data (see for more detail the epics3 plugin)
int bsreadPlugin::pvSetValue(char *pv, double rdata, int32_t idata, char *sdata, char *object, char *errmess, int forceType) {
    Q_UNUSED(forceType);
    Q_UNUSED(errmess);
    Q_UNUSED(object);

    QMutexLocker locker(&mutex);
    qDebug() << "bsreadPlugin:pvSetValue" << pv << rdata << idata << sdata;
    return false;
}

// caQtDM_Lib will call this routine for setting waveforms data (see for more detail the epics3 plugin)
int bsreadPlugin::pvSetWave(char *pv, float *fdata, double *ddata, int16_t *data16, int32_t *data32, char *sdata, int nelm, char *object, char *errmess) {
    Q_UNUSED(pv);
    Q_UNUSED(fdata);
    Q_UNUSED(ddata);
    Q_UNUSED(data16);
    Q_UNUSED(data32);
    Q_UNUSED(sdata);
    Q_UNUSED(nelm);
    Q_UNUSED(object);
    Q_UNUSED(errmess);

    QMutexLocker locker(&mutex);
    qDebug() << "bsreadPlugin:pvSetWave";
    return false;
}

// caQtDM_Lib will call this routine for getting a description of the monitor
int bsreadPlugin::pvGetTimeStamp(char *pv, char *timestamp) {
    Q_UNUSED(pv);
    qDebug() << "bsreadPlugin:pvgetTimeStamp";
    strcpy(timestamp, "timestamp in epics format");
    return true;
}

// caQtDM_Lib will call this routine for getting the timestamp for this monitor
int bsreadPlugin::pvGetDescription(char *pv, char *description) {
    Q_UNUSED(pv);
    qDebug() << "bsreadPlugin:pvGetDescription";
    strcpy(description, "no Description available BSREAD data transfer");
    return true;
}

// next routines are used to stop and restart the dataacquisition (used in case of tabWidgets in the display)
int bsreadPlugin::pvClearEvent(void * ptr) {
    Q_UNUSED(ptr);
    qDebug() << "bsreadPlugin:pvClearEvent";
    return true;
}

int bsreadPlugin::pvAddEvent(void * ptr) {
    Q_UNUSED(ptr);
    qDebug() << "bsreadPlugin:pvAddEvent";
    return true;
}

// next routines are used to Connect and disconnect monitors
int bsreadPlugin::pvReconnect(knobData *kData) {
     Q_UNUSED(kData);
    qDebug() << "bsreadPlugin:pvReconnect";
    return true;
}

int bsreadPlugin::pvDisconnect(knobData *kData) {
    Q_UNUSED(kData);
    qDebug() << "bsreadPlugin:pvDisconnect";
    return true;
}

// flush any io
int bsreadPlugin::FlushIO() {
    //qDebug() << "bsreadPlugin:FlushIO";
    return true;
}

// termination
int bsreadPlugin::TerminateIO() {
    //qDebug() << "bsreadPlugin:TerminateIO";
    timerValues->stop();
    timer->stop();
    return true;
}

void bsreadPlugin::closeEvent(){
   //qDebug() << "bsreadPlugin:closeEvent ";
   emit closeSignal();
   Dispatcher->setTerminate();
   DispatcherThread->quit();
   DispatcherThread->wait();
}


#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#else
Q_EXPORT_PLUGIN2(bsreadPlugin, bsreadPlugin)
#endif

