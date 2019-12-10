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
#include "modbus_Plugin.h"

// as defined in knobDefines.h
//caType {caSTRING	= 0, caINT = 1, caFLOAT = 2, caENUM = 3, caCHAR = 4, caLONG = 5, caDOUBLE = 6};


// gives the plugin name back
QString modbusPlugin::pluginName()
{
    return "bsread";
}

// constructor
modbusPlugin::modbusPlugin()
{
    qDebug() << "modbusPlugin: Create";

    mutexknobdataP = Q_NULLPTR;
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(closeEvent()));

}
modbusPlugin:: ~modbusPlugin()
{


}

// in this demo we update our interface here; normally you should update in from your controlsystem
// take a look how monitors are treated in the epics3 plugin
void modbusPlugin::updateInterface()
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
void modbusPlugin::updateValues()
{
    QMutexLocker locker(&mutex);
    QMap<QString, double>::iterator i;
    for (i = listOfDoubles.begin(); i != listOfDoubles.end(); ++i) i.value()++;
}

// initialize our communicationlayer with everything you need
int modbusPlugin::initCommunicationLayer(MutexKnobData *data, MessageWindow *messageWindow,QMap<QString, QString> options)
{
    int i;
    //Q_UNUSED(options);
    //qDebug() << "modbusPlugin: InitCommunicationLayer" << data;
    qDebug() << "modbusPlugin: InitCommunicationLayer with options" << options;

    mutexknobdataP = data;
    messagewindowP = messageWindow;


    initValue = 0.0;




    return true;
}

// caQtDM_Lib will call this routine for defining a monitor
int modbusPlugin::pvAddMonitor(int index, knobData *kData, int rate, int skip) {
    Q_UNUSED(index);
    Q_UNUSED(rate);
    Q_UNUSED(skip);

    int i;
    QMutexLocker locker(&mutex);
    //qDebug() << "modbusPlugin:pvAddMonitor" << kData->pv << kData->index << kData;

    //remove EPICS addjustment parameter
    QString datapv=kData->pv;
    int pos = datapv.indexOf(".{");
    if(pos != -1) {
     datapv.truncate(pos);
    }



    return 0;
}

// caQtDM_Lib will call this routine for getting rid of a monitor
int modbusPlugin::pvClearMonitor(knobData *kData) {
    int i=0;

    QMutexLocker locker(&mutex);

    return true;
}
int modbusPlugin::pvFreeAllocatedData(knobData *kData)
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
int modbusPlugin::pvSetValue(char *pv, double rdata, int32_t idata, char *sdata, char *object, char *errmess, int forceType) {
    Q_UNUSED(forceType);
    Q_UNUSED(errmess);
    Q_UNUSED(object);
    QMutexLocker locker(&mutex);
    //qDebug() << "modbusPlugin:pvSetValue" << pv << rdata << idata << sdata;
    return 0;
}

// caQtDM_Lib will call this routine for setting waveforms data (see for more detail the epics3 plugin)
int modbusPlugin::pvSetWave(char *pv, float *fdata, double *ddata, int16_t *data16, int32_t *data32, char *sdata, int nelm, char *object, char *errmess) {
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
    qDebug() << "modbusPlugin:pvSetWave";
    return false;
}

// caQtDM_Lib will call this routine for getting a description of the monitor
int modbusPlugin::pvGetTimeStamp(char *pv, char *timestamp) {
    Q_UNUSED(pv);
    qDebug() << "modbusPlugin:pvgetTimeStamp";
    strcpy(timestamp, "timestamp in epics format");
    return true;
}

// caQtDM_Lib will call this routine for getting the timestamp for this monitor
int modbusPlugin::pvGetDescription(char *pv, char *description) {
    Q_UNUSED(pv);
    qDebug() << "modbusPlugin:pvGetDescription";
    strcpy(description, "no Description available BSREAD data transfer");
    return true;
}

// next routines are used to stop and restart the dataacquisition (used in case of tabWidgets in the display)
int modbusPlugin::pvClearEvent(void * ptr) {
    Q_UNUSED(ptr);
    qDebug() << "modbusPlugin:pvClearEvent";
    return true;
}

int modbusPlugin::pvAddEvent(void * ptr) {
    Q_UNUSED(ptr);
    qDebug() << "modbusPlugin:pvAddEvent";
    return true;
}

// next routines are used to Connect and disconnect monitors
int modbusPlugin::pvReconnect(knobData *kData) {
     Q_UNUSED(kData);
    qDebug() << "modbusPlugin:pvReconnect";
    return true;
}

int modbusPlugin::pvDisconnect(knobData *kData) {
    Q_UNUSED(kData);
    qDebug() << "modbusPlugin:pvDisconnect";
    return true;
}

// flush any io
int modbusPlugin::FlushIO() {
    //qDebug() << "modbusPlugin:FlushIO";
    return true;
}

// termination
int modbusPlugin::TerminateIO() {
    //qDebug() << "modbusPlugin:TerminateIO";
    timerValues->stop();
    timer->stop();
    return true;
}

void modbusPlugin::closeEvent(){
    //qDebug() << "modbusPlugin:closeEvent ";
    emit closeSignal();

}


#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#else
Q_EXPORT_PLUGIN2(modbusPlugin, modbusPlugin)
#endif

