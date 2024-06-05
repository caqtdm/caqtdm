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
 *  Copyright (c) 2010 - 2020
 *
 *  Author:
 *    Helge Brands
 *  Contact details:
 *    helge.brands@psi.ch
 */
#include <QtCore>
#include <QDebug>
#include <QString>
#include <QApplication>
#include "environment_plugin.h"

// as defined in knobDefines.h
//caType {caSTRING	= 0, caINT = 1, caFLOAT = 2, caENUM = 3, caCHAR = 4, caLONG = 5, caDOUBLE = 6};
#define qasc(x) x.toLatin1().constData()

// gives the plugin name back
QString environmentPlugin::pluginName()
{
    return "environment";
}

// constructor
environmentPlugin::environmentPlugin()
{
    mutexknobdataP = Q_NULLPTR;
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(closeEvent()));
}
environmentPlugin:: ~environmentPlugin()
{
}

// in this demo we update our interface here; normally you should update in from your controlsystem
// take a look how monitors are treated in the epics3 plugin
void environmentPlugin::updateInterface()
{
    QString newValue;

    QMutexLocker locker(&mutex);

    // go through our devices
    foreach(int index, listOfIndexes) {
        knobData* kData = mutexknobdataP->GetMutexKnobDataPtr(index);
        if((kData != (knobData *) Q_NULLPTR) && (kData->index != -1)) {
            QString key = kData->pv;

            // find this pv in our internal double values list (assume for now we are only treating doubles)
            // and increment its value
            QMap<QString, QString>::iterator i = listOfStrings.find(key);
            while (i !=listOfStrings.end() && i.key() == key) {
                newValue = i.value();
                break;
            }
        }
    }
}

// in this demo we update our values here
void environmentPlugin::updateValues()
{
    QMutexLocker locker(&mutex);
    QMap<QString, QString>::iterator i;
    //for (i = listOfStrings.begin(); i != listOfStrings.end(); ++i) i.value()++;
}

// initialize our communicationlayer with everything you need
int environmentPlugin::initCommunicationLayer(MutexKnobData *data, MessageWindow *messageWindow,QMap<QString, QString> options)
{
    qDebug() << "environmentPlugin: InitCommunicationLayer with options" << options;

    mutexknobdataP = data;
    messagewindowP = messageWindow;

    return true;
}

// caQtDM_Lib will call this routine for defining a monitor
int environmentPlugin::pvAddMonitor(int index, knobData *kData, int rate, int skip) {
    Q_UNUSED(index)
    Q_UNUSED(rate)
    Q_UNUSED(skip)

    int dataSize;
    QMutexLocker locker(&mutex);

    //remove EPICS addjustment parameter
    QString datapv=kData->pv;
    int pos = datapv.indexOf(".{");
    if(pos != -1) {
     datapv.truncate(pos);
    }
    listOfIndexes.append(kData->index);
    listOfRequestedChannels.append(datapv);
    QString value = (QString)  qgetenv(qasc(datapv));
    if (!value.isEmpty()){
        dataSize= value.length();
        //qDebug() <<"PV:" << datapv << value;
        if(dataSize != kData->edata.dataSize) {
           if(kData->edata.dataB != (void*) Q_NULLPTR) free(kData->edata.dataB);
            kData->edata.dataB = (void*) malloc((size_t) dataSize*sizeof(char)+5);
            kData->edata.dataSize = dataSize;
        }

        qstrncpy((char*)kData->edata.dataB, (char*)qasc(value),(size_t)kData->edata.dataSize);

        kData->edata.fieldtype = caSTRING;
        kData->edata.connected = true;
        kData->edata.accessR = kData->edata.accessW = true;
        kData->edata.monitorCount++;
        mutexknobdataP->SetMutexKnobData(kData->index, *kData);
        mutexknobdataP->SetMutexKnobDataReceived(kData);

    }

    return 0;
}

// caQtDM_Lib will call this routine for getting rid of a monitor
int environmentPlugin::pvClearMonitor(knobData *kData) {

    QMutexLocker locker(&mutex);
    QString datapv=kData->pv;
    int pos = datapv.indexOf(".{");
    if(pos != -1) {
     datapv.truncate(pos);
    }

    listOfIndexes.removeAll(kData->index);
    listOfRequestedChannels.removeAll(datapv);

    return true;
}
int environmentPlugin::pvFreeAllocatedData(knobData *kData)
{
    //qDebug() << "DemoPlugin:pvFreeAllocatedData";
    if (kData->edata.info != (void *) Q_NULLPTR) {
        free(kData->edata.info);
        kData->edata.info = (void*) Q_NULLPTR;
    }
    if(kData->edata.dataB != (void*) Q_NULLPTR) {
        free(kData->edata.dataB);
        kData->edata.dataB = (void*) Q_NULLPTR;
    }

    return true;
}

// caQtDM_Lib will call this routine for setting data (see for more detail the epics3 plugin)
int environmentPlugin::pvSetValue(char *pv, double rdata, int32_t idata, char *sdata, char *object, char *errmess, int forceType) {
    Q_UNUSED(idata)
    Q_UNUSED(sdata)
    Q_UNUSED(rdata)
    Q_UNUSED(pv)
    Q_UNUSED(forceType)
    Q_UNUSED(errmess)
    Q_UNUSED(object)
    QMutexLocker locker(&mutex);
    //qDebug() << "environmentPlugin:pvSetValue" << pv << rdata << idata << sdata;
    return 0;
}

// caQtDM_Lib will call this routine for setting waveforms data (see for more detail the epics3 plugin)
int environmentPlugin::pvSetWave(char *pv, float *fdata, double *ddata, int16_t *data16, int32_t *data32, char *sdata, int nelm, char *object, char *errmess) {
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
    return false;
}

// caQtDM_Lib will call this routine for getting a description of the monitor
int environmentPlugin::pvGetTimeStamp(char *pv, char *timestamp) {
    Q_UNUSED(pv);
    strcpy(timestamp, "timestamp in epics format");
    return true;
}

// caQtDM_Lib will call this routine for getting the timestamp for this monitor
int environmentPlugin::pvGetDescription(char *pv, char *description) {
    Q_UNUSED(pv);
    strcpy(description, "no Description available Environment data transfer");
    return true;
}

// next routines are used to stop and restart the dataacquisition (used in case of tabWidgets in the display)
int environmentPlugin::pvClearEvent(void * ptr) {
    Q_UNUSED(ptr);
    return true;
}

int environmentPlugin::pvAddEvent(void * ptr) {
    Q_UNUSED(ptr);
    return true;
}

// next routines are used to Connect and disconnect monitors
int environmentPlugin::pvReconnect(knobData *kData) {
     Q_UNUSED(kData);
    return true;
}

int environmentPlugin::pvDisconnect(knobData *kData) {
    Q_UNUSED(kData);
    return true;
}

// flush any io
int environmentPlugin::FlushIO() {
    //qDebug() << "environmentPlugin:FlushIO";
    return true;
}

// termination
int environmentPlugin::TerminateIO() {
    //qDebug() << "environmentPlugin:TerminateIO";
    //timerValues->stop();
    //timer->stop();
    return true;
}

void environmentPlugin::closeEvent(){
    //qDebug() << "environmentPlugin:closeEvent ";
    emit closeSignal();

}


#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#else
Q_EXPORT_PLUGIN2(environmentPlugin, environmentPlugin)
#endif

