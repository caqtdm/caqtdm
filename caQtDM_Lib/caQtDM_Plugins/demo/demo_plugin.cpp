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
#include "demo_plugin.h"

// as defined in knobDefines.h
//caType {caSTRING	= 0, caINT = 1, caFLOAT = 2, caENUM = 3, caCHAR = 4, caLONG = 5, caDOUBLE = 6};

// this demo plugin just gives you an idea how to use a plugin; for more details you should take a look
// at the epics3 plugin

// gives the plugin name back
QString DemoPlugin::pluginName()
{
    return "demo";
}

// constructor
DemoPlugin::DemoPlugin()
{
    qDebug() << "DemoPlugin: Create";
}

// in this demo we update our interface here; normally you should update in from your controlsystem
// take a look how monitors are treated in the epics3 plugin
void DemoPlugin::updateInterface()
{
    double newValue = 0.0;

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
void DemoPlugin::updateValues()
{
    QMutexLocker locker(&mutex);
#ifndef HARDWORK
    QMap<QString, double>::iterator i;
    for (i = listOfDoubles.begin(); i != listOfDoubles.end(); ++i) i.value()++;
#else
    QtConcurrent::run(this, &DemoPlugin::updateHardwork);
#endif
}

#ifdef HARDWORK
void  DemoPlugin::updateHardwork()
{
    qDebug() << "hardwork";
    QMap<QString, double>::iterator i;
    for (i = listOfDoubles.begin(); i != listOfDoubles.end(); ++i) i.value()++;
}
#endif

// initialize our communicationlayer with everything you need
int DemoPlugin::initCommunicationLayer(MutexKnobData *data, MessageWindow *messageWindow, QMap<QString, QString> options)
{
    qDebug() << "DemoPlugin: InitCommunicationLayer with options" << options;

    mutexknobdataP = data;
    messagewindowP = messageWindow;

    initValue = 0.0;

    // we want to update our internal doubles every second
    timerValues = new QTimer(this);
    connect(timerValues, SIGNAL(timeout()), this, SLOT(updateValues()));
    timerValues->start(1000);

    // we want to update the interface every 2 seconds
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateInterface()));
    timer->start(2000);

    return true;
}

// caQtDM_Lib will call this routine for defining a monitor
int DemoPlugin::pvAddMonitor(int index, knobData *kData, int rate, int skip) {
    Q_UNUSED(index);
    Q_UNUSED(rate);
    Q_UNUSED(skip);
    QMutexLocker locker(&mutex);
    QString key = kData->pv;

    qDebug() << "DemoPlugin:pvAddMonitor" << kData->pv << kData->index;
    double value = initValue;
    initValue += 10;

    // append device index to our internal list
    listOfIndexes.append(kData->index);

    // initial values into the doubles list
    if(!listOfDoubles.contains(key)) listOfDoubles.insert(key, value);

    return true;
}

// caQtDM_Lib will call this routine for getting rid of a monitor
int DemoPlugin::pvClearMonitor(knobData *kData) {
    QMutexLocker locker(&mutex);

    qDebug() << "DemoPlugin:pvClearMonitor" << kData->pv << kData->index;
    QString key = kData->pv;
    if(!listOfDoubles.contains(key)) listOfDoubles.remove(key);
    listOfIndexes.removeAll(kData->index);

    return true;
}

int DemoPlugin::pvFreeAllocatedData(knobData *kData)
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
int DemoPlugin::pvSetValue(char *pv, double rdata, int32_t idata, char *sdata, char *object, char *errmess, int forceType) {
    Q_UNUSED(object);
    Q_UNUSED(errmess);
    Q_UNUSED(forceType);
    QMutexLocker locker(&mutex);
    qDebug() << "DemoPlugin:pvSetValue" << pv << rdata << idata << sdata;
    QString key = pv;
    if(listOfDoubles.contains(key)) listOfDoubles.insert(pv, rdata);
    return true;
}

// caQtDM_Lib will call this routine for setting waveforms data (see for more detail the epics3 plugin)
int DemoPlugin::pvSetWave(char *pv, float *fdata, double *ddata, int16_t *data16, int32_t *data32, char *sdata, int nelm, char *object, char *errmess) {
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
    qDebug() << "DemoPlugin:pvSetWave";
    return true;
}

// caQtDM_Lib will call this routine for getting a description of the monitor
int DemoPlugin::pvGetTimeStamp(char *pv, char *timestamp) {
    Q_UNUSED(pv);
    Q_UNUSED(timestamp);
    qDebug() << "DemoPlugin:pvgetTimeStamp";
    strcpy(timestamp, "timestamp in epics format");
    return true;
}

// caQtDM_Lib will call this routine for getting the timestamp for this monitor
int DemoPlugin::pvGetDescription(char *pv, char *description) {
    Q_UNUSED(pv);
    Q_UNUSED(description);
    qDebug() << "DemoPlugin:pvGetDescription";
    strcpy(description, "hello, I am a double");
    return true;
}

// next two routines are used to stop and restart the monitoring (used in case of tabWidgets in the display)
int DemoPlugin::pvClearEvent(void * ptr) {
    Q_UNUSED(ptr);
    qDebug() << "DemoPlugin:pvClearEvent";
    return true;
}

int DemoPlugin::pvAddEvent(void * ptr) {
    Q_UNUSED(ptr);
    qDebug() << "DemoPlugin:pvAddEvent";
    return true;
}

// next two routines are used to connect and disconnect monitors when the application gest suspended and reactivated
int DemoPlugin::pvReconnect(knobData *kData) {
    Q_UNUSED(kData);
    qDebug() << "DemoPlugin:pvReconnect";
    return true;
}

int DemoPlugin::pvDisconnect(knobData *kData) {
    Q_UNUSED(kData);
    qDebug() << "DemoPlugin:pvDisconnect";
    return true;
}

// flush any io is periodically called (1s timer) in order to flush the disconnection and reconnection
// used for pv's that will be hidden and shown in case of tabwidgets
int DemoPlugin::FlushIO() {
    //qDebug() << "DemoPlugin:FlushIO";
    return true;
}

// termination (in case of epics3, this is used to destroy the context when the application gest deactivated
// otherwise probably no meaning; in this demo, we stop the simulation, however it will not be reactivated
// any more (you may do that through pvReconnect)
int DemoPlugin::TerminateIO() {
    //qDebug() << "DemoPlugin:TerminateIO";
    timerValues->stop();
    timer->stop();
    return true;
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#else
    Q_EXPORT_PLUGIN2(DemoPlugin, DemoPlugin)
#endif

