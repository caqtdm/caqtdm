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
#include "bsread_plugin.h"
#include "zmq.h"
#include "bsread_decode.h"

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
    qDebug() << "bsreadPlugin: Create";
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
int bsreadPlugin::initCommunicationLayer(MutexKnobData *data, MessageWindow *messageWindow)
{
    int i;
    int rc;

    qDebug() << "bsreadPlugin: InitCommunicationLayer" << data;
    mutexknobdataP = data;
    messagewindowP = messageWindow;

    initValue = 0.0;

    // INIT ZMQ Layer
    zmqcontex = zmq_init (1);
    QString ZMQ_ADDR_LIST = (QString)  qgetenv("BSREAD_ZMQ_ADDR_LIST");
#ifdef _MSC_VER
    QStringList BSREAD_ZMQ_ADDRS = ZMQ_ADDR_LIST.split(";");
#else
    QStringList BSREAD_ZMQ_ADDRS = ZMQ_ADDR_LIST.split(" ");
#endif
    for (i=0;i<BSREAD_ZMQ_ADDRS.count();i++){
        bsreadconnections.append(new bsread_Decode(zmqcontex,BSREAD_ZMQ_ADDRS.at(i)));
        bsreadconnections.last()->setKnobData(data);
        bsreadconnections.last()->start();
    }




    return true;
}

// caQtDM_Lib will call this routine for defining a monitor
int bsreadPlugin::pvAddMonitor(int index, knobData *kData, int rate, int skip) {
    bool result;
    int i;
    QMutexLocker locker(&mutex);

    qDebug() << "bsreadPlugin:pvAddMonitor" << kData->pv << kData->index;
    i=0;
    result=false;
    while ((i<bsreadconnections.size())&&(!result)){
     result=bsreadconnections.at(i)->bsread_DataMonitorConnection(kData);
     i++;
    }



    return result;
}

// caQtDM_Lib will call this routine for getting rid of a monitor
int bsreadPlugin::pvClearMonitor(knobData *kData) {
    int i;

    QMutexLocker locker(&mutex);

    qDebug() << "bsreadPlugin:pvClearMonitor" << kData << kData->pv << kData->index;
    while (i<bsreadconnections.size()){
     bsreadconnections.at(i)->bsread_DataMonitorUnConnect(kData);
     i++;
    }
    return true;
}

// caQtDM_Lib will call this routine for setting data (see for more detail the epics3 plugin)
int bsreadPlugin::pvSetValue(char *pv, double rdata, int32_t idata, char *sdata, char *object, char *errmess, int forceType) {
    QMutexLocker locker(&mutex);
    qDebug() << "bsreadPlugin:pvSetValue" << pv << rdata << idata << sdata;
    QString key = pv;
    if(listOfDoubles.contains(key)) listOfDoubles.insert(pv, rdata);
    return true;
}

// caQtDM_Lib will call this routine for setting waveforms data (see for more detail the epics3 plugin)
int bsreadPlugin::pvSetWave(char *pv, float *fdata, double *ddata, int16_t *data16, int32_t *data32, char *sdata, int nelm, char *object, char *errmess) {
    QMutexLocker locker(&mutex);
    qDebug() << "bsreadPlugin:pvSetWave";
    return true;
}

// caQtDM_Lib will call this routine for getting a description of the monitor
int bsreadPlugin::pvGetTimeStamp(char *pv, char *timestamp) {
    qDebug() << "bsreadPlugin:pvgetTimeStamp";
    strcpy(timestamp, "timestamp in epics format");
    return true;
}

// caQtDM_Lib will call this routine for getting the timestamp for this monitor
int bsreadPlugin::pvGetDescription(char *pv, char *description) {
    qDebug() << "bsreadPlugin:pvGetDescription";
    strcpy(description, "hello, I am a double");
    return true;
}

// next routines are used to stop and restart the dataacquisition (used in case of tabWidgets in the display)
int bsreadPlugin::pvClearEvent(void * ptr) {
    qDebug() << "bsreadPlugin:pvClearEvent";
    return true;
}

int bsreadPlugin::pvAddEvent(void * ptr) {
    qDebug() << "bsreadPlugin:pvAddEvent";
    return true;
}

// next routines are used to Connect and disconnect monitors
int bsreadPlugin::pvReconnect(knobData *kData) {
    qDebug() << "bsreadPlugin:pvReconnect";
    return true;
}

int bsreadPlugin::pvDisconnect(knobData *kData) {
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

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#else
Q_EXPORT_PLUGIN2(bsreadPlugin, bsreadPlugin)
#endif

