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
#include <QDebug>
#include <QThread>

#include "gps_plugin.h"

// as defined in knobDefines.h
//caType {caSTRING	= 0, caINT = 1, caFLOAT = 2, caENUM = 3, caCHAR = 4, caLONG = 5, caDOUBLE = 6};

// gives the plugin name back
QString gpsPlugin::pluginName()
{
    return "gps";
}

// constructor
gpsPlugin::gpsPlugin()
{
    qDebug() << "gpsPlugin: Create";
    enable_gps_readout=false;
}

// in this gps we update our interface here; normally you should update in from your controlsystem
// take a look how monitors are treated in the epics3 plugin
void gpsPlugin::updateInterface()
{
    double newValue = 0.0;

    QMutexLocker locker(&mutex);

    // go through our devices
    foreach(int index, listOfIndexes) {
        knobData* kData = mutexknobdataP->GetMutexKnobDataPtr(index);
        if((kData != (knobData *) Q_NULLPTR) && (kData->index != -1)) {
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
            kData->edata.accessR = true;
            kData->edata.accessW = false;
            kData->edata.monitorCount++;
            mutexknobdataP->SetMutexKnobData(kData->index, *kData);
            mutexknobdataP->SetMutexKnobDataReceived(kData);
        }
    }
}

void gpsPlugin::positionUpdated(const QGeoPositionInfo &info)
{
     // qDebug() << "Position updated:" << info;
     QMutexLocker locker(&mutex);
     if (!enable_gps_readout) return;
     if (info.isValid()){
         foreach(int index, listOfIndexes) {
             knobData* kData = mutexknobdataP->GetMutexKnobDataPtr(index);
             if((kData != (knobData *) Q_NULLPTR) && (kData->index != -1)) {
                 QString key = kData->pv;
                 if (key.compare("altitude",Qt::CaseInsensitive)==0){
                     kData->edata.monitorCount++;
                     kData->edata.rvalue= info.coordinate().altitude(); }
                 if (key.compare("latitude",Qt::CaseInsensitive)==0){
                     kData->edata.monitorCount++;
                     kData->edata.rvalue= info.coordinate().latitude();}
                 if (key.compare("longitude",Qt::CaseInsensitive)==0){
                     kData->edata.monitorCount++;
                     kData->edata.rvalue= info.coordinate().longitude();}
                 if (key.compare("distanceTo",Qt::CaseInsensitive)==0){
                     kData->edata.monitorCount++;
                     const QGeoCoordinate d=distancedata;
                     kData->edata.rvalue= info.coordinate().distanceTo(d);}
                 if (key.compare("GroundSpeed",Qt::CaseInsensitive)==0)
                     if (info.hasAttribute(QGeoPositionInfo::GroundSpeed))
                     {
                         kData->edata.monitorCount++;
                         kData->edata.rvalue= info.attribute(QGeoPositionInfo::GroundSpeed);
                     }
                 if (key.compare("Direction",Qt::CaseInsensitive)==0)
                     if (info.hasAttribute(QGeoPositionInfo::Direction))
                     {
                         kData->edata.monitorCount++;
                         kData->edata.rvalue=  info.attribute(QGeoPositionInfo::Direction);
                     }
                 if (key.compare("HorizontalAccuracy",Qt::CaseInsensitive)==0)
                     if (info.hasAttribute(QGeoPositionInfo::HorizontalAccuracy))
                     {
                         kData->edata.monitorCount++;
                         kData->edata.rvalue= info.attribute(QGeoPositionInfo::HorizontalAccuracy);
                     }




                 mutexknobdataP->SetMutexKnobData(kData->index, *kData);
                 mutexknobdataP->SetMutexKnobDataReceived(kData);


             }
         }
     }


}

// in this gps we update our values here
void gpsPlugin::updateValues()
{
    QMutexLocker locker(&mutex);
    if (!enable_gps_readout) return;


}


// initialize our communicationlayer with everything you need
int gpsPlugin::initCommunicationLayer(MutexKnobData *data, MessageWindow *messageWindow, QMap<QString, QString> options)
{
    qDebug() << "gpsPlugin: InitCommunicationLayer with options" << options;

    mutexknobdataP = data;
    messagewindowP = messageWindow;

    initValue = 0.0;

    // we want to update our internal doubles every second
    timerValues = new QTimer(this);
    connect(timerValues, SIGNAL(timeout()), this, SLOT(updateValues()));
    timerValues->start(500);

    pos_data_source = QGeoPositionInfoSource::createDefaultSource(this);
      if (pos_data_source) {
          connect(pos_data_source, SIGNAL(positionUpdated(QGeoPositionInfo)),
                  this, SLOT(positionUpdated(QGeoPositionInfo)));

      }
    pos_data_source->setUpdateInterval(500);
    pos_data_source->setPreferredPositioningMethods(QGeoPositionInfoSource::AllPositioningMethods);
    return true;
}

// caQtDM_Lib will call this routine for defining a monitor
int gpsPlugin::pvAddMonitor(int index, knobData *kData, int rate, int skip) {
    Q_UNUSED(index);
    Q_UNUSED(rate);
    Q_UNUSED(skip);
    QMutexLocker locker(&mutex);
    QString key = kData->pv;

    qDebug() << "gpsPlugin:pvAddMonitor" << kData->pv << kData->index;
    bool validkey=false;
    bool writeable=false;

    if (key.compare("altitude",Qt::CaseInsensitive)==0){
       validkey=true; }else
    if (key.compare("latitude",Qt::CaseInsensitive)==0){
        validkey=true;}else
    if (key.compare("longitude",Qt::CaseInsensitive)==0){
       validkey=true;}else
    if (key.compare("distanceTo",Qt::CaseInsensitive)==0){
       validkey=true;}else
    if (key.compare("setAltitude",Qt::CaseInsensitive)==0){
       validkey=true;
       writeable=true;}else
    if (key.compare("setLatitude",Qt::CaseInsensitive)==0){
       validkey=true;
       writeable=true;}else
    if (key.compare("setLongitude",Qt::CaseInsensitive)==0){
       validkey=true;
       writeable=true;}else
    if (key.compare("GroundSpeed",Qt::CaseInsensitive)==0){
        qstrncpy(kData->edata.units,QString("m/s").toLatin1().data(),39);
        validkey=true;}else
    if (key.compare("Direction",Qt::CaseInsensitive)==0){
        validkey=true;}
    if (key.compare("HorizontalAccuracy",Qt::CaseInsensitive)==0){
        qstrncpy(kData->edata.units,QString("m").toLatin1().data(),39);
        validkey=true;}

    if (!validkey) return false;
    if(!enable_gps_readout){
        pos_data_source->startUpdates();
        enable_gps_readout=true;
    }
    // append device index to our internal list
    listOfIndexes.append(kData->index);
    // initial values into the doubles list
    if(!listOfDoubles.contains(key)) listOfDoubles.insert(key, 0.0);

    kData->edata.fieldtype = caDOUBLE;
    kData->edata.precision = 6;
    kData->edata.connected = true;
    kData->edata.accessR = true;
    kData->edata.accessW = writeable;
    kData->edata.monitorCount++;
    mutexknobdataP->SetMutexKnobData(kData->index, *kData);
    mutexknobdataP->SetMutexKnobDataReceived(kData);


    return true;
}

// caQtDM_Lib will call this routine for getting rid of a monitor
int gpsPlugin::pvClearMonitor(knobData *kData) {
    QMutexLocker locker(&mutex);

    qDebug() << "gpsPlugin:pvClearMonitor" << kData->pv << kData->index;
    QString key = kData->pv;
    if(!listOfDoubles.contains(key)) listOfDoubles.remove(key);
    listOfIndexes.removeAll(kData->index);

    return true;
}

int gpsPlugin::pvFreeAllocatedData(knobData *kData)
{
    QMutexLocker locker(&mutex);
    //qDebug() << "gpsPlugin:pvFreeAllocatedData";
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
int gpsPlugin::pvSetValue(char *pv, double rdata, int32_t idata, char *sdata, char *object, char *errmess, int forceType) {
    Q_UNUSED(object)
    Q_UNUSED(errmess)
    Q_UNUSED(forceType)
    Q_UNUSED(idata)
    Q_UNUSED(sdata)

     QString key = pv;
     if (key.compare("setAltitude",Qt::CaseInsensitive)){
        distancedata.setAltitude(rdata); }else
     if (key.compare("setLatitude",Qt::CaseInsensitive)){
         distancedata.setLatitude(rdata);}else
     if (key.compare("setLongitude",Qt::CaseInsensitive)){
         distancedata.setLongitude(rdata);}

    return true;
}

// caQtDM_Lib will call this routine for setting waveforms data (see for more detail the epics3 plugin)
int gpsPlugin::pvSetWave(char *pv, float *fdata, double *ddata, int16_t *data16, int32_t *data32, char *sdata, int nelm, char *object, char *errmess) {
    Q_UNUSED(pv);
    Q_UNUSED(fdata);
    Q_UNUSED(ddata);
    Q_UNUSED(data16);
    Q_UNUSED(data32);
    Q_UNUSED(sdata);
    Q_UNUSED(nelm);
    Q_UNUSED(object);
    Q_UNUSED(errmess);
    return true;
}

// caQtDM_Lib will call this routine for getting a description of the monitor
int gpsPlugin::pvGetTimeStamp(char *pv, char *timestamp) {
    Q_UNUSED(pv);
    Q_UNUSED(timestamp);
    qDebug() << "gpsPlugin:pvgetTimeStamp";
    strcpy(timestamp, "timestamp in epics format");
    return true;
}

// caQtDM_Lib will call this routine for getting the timestamp for this monitor
int gpsPlugin::pvGetDescription(char *pv, char *description) {
    Q_UNUSED(pv);
    Q_UNUSED(description);
    qDebug() << "gpsPlugin:pvGetDescription";

    QString data=pos_data_source->sourceName();
    data="Datasource: "+data;
    strcpy(description,  data.toLatin1().data());
    return true;
}

// next two routines are used to stop and restart the monitoring (used in case of tabWidgets in the display)
int gpsPlugin::pvClearEvent(void * ptr) {
    Q_UNUSED(ptr);
    qDebug() << "gpsPlugin:pvClearEvent";
    return true;
}

int gpsPlugin::pvAddEvent(void * ptr) {
    Q_UNUSED(ptr);
    qDebug() << "gpsPlugin:pvAddEvent";
    return true;
}

// next two routines are used to connect and disconnect monitors when the application gest suspended and reactivated
int gpsPlugin::pvReconnect(knobData *kData) {
    Q_UNUSED(kData);
    qDebug() << "gpsPlugin:pvReconnect";
    if(!enable_gps_readout){
        qDebug() << "gpsPlugin:startUpdates";
        pos_data_source->startUpdates();
        enable_gps_readout=true;
    }

    return true;
}

int gpsPlugin::pvDisconnect(knobData *kData) {
    Q_UNUSED(kData);
    qDebug() << "gpsPlugin:pvDisconnect";
    return true;
}

// flush any io is periodically called (1s timer) in order to flush the disconnection and reconnection
// used for pv's that will be hidden and shown in case of tabwidgets
int gpsPlugin::FlushIO() {
    //qDebug() << "gpsPlugin:FlushIO";
    return true;
}

// termination (in case of epics3, this is used to destroy the context when the application gest deactivated
// otherwise probably no meaning; in this gps, we stop the simulation, however it will not be reactivated
// any more (you may do that through pvReconnect)
int gpsPlugin::TerminateIO() {
    qDebug() << "gpsPlugin:TerminateIO";

    if(enable_gps_readout){
        qDebug() << "gpsPlugin:stopUpdates";
        pos_data_source->stopUpdates();
        enable_gps_readout=false;
    }

    //timerValues->stop();
    //timer->stop();
    return true;
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#else
    Q_EXPORT_PLUGIN2(gpsPlugin, gpsPlugin)
#endif

