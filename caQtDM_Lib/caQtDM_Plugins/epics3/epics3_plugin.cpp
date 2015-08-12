#include <QDebug>
#include "epics3_plugin.h"

MutexKnobData* KnobDataPtr;

QString Epics3Plugin::pluginName()
{
    return "epics3";
}

Epics3Plugin::Epics3Plugin()
{
    qDebug() << "Epics3Plugin: Create";
}

int Epics3Plugin::initCommunicationLayer(MutexKnobData *data)
{
    qDebug() << "Epics3Plugin: InitCommunicationLayer";
    KnobDataPtr = data;
    PrepareDeviceIO();
    return true;
}

int Epics3Plugin::pvAddMonitor(int index, knobData *kData, int rate, int skip) {
    //qDebug() << "Epics3Plugin:pvAddMonitor" << kData->pv;
    CreateAndConnect(index, kData, rate, skip);
    return true;
} 

int Epics3Plugin::pvClearMonitor(knobData *kData) {
    //qDebug() << "Epics3Plugin:pvClearMonitor" << kData->pv;
    ClearMonitor(kData);
    return true;
}

int Epics3Plugin::pvSetValue(char *pv, double rdata, int32_t idata, char *sdata, char *object, char *errmess, int forceType) {

    //qDebug() << "Epics3Plugin:pvSetValue";
    EpicsSetValue(pv, rdata, idata, sdata, object, errmess, forceType);
    return true;
}

int Epics3Plugin::pvSetWave(char *pv, float *fdata, double *ddata, int16_t *data16, int32_t *data32, char *sdata, int nelm, char *object, char *errmess) {
    //qDebug() << "Epics3Plugin:pvSetWave";
    EpicsSetWave(pv, fdata, ddata, data16, data32, sdata, nelm, object, errmess);
    return true;
}

int Epics3Plugin::pvGetTimeStamp(char *pv, char *timestamp) {
    //qDebug() << "Epics3Plugin:pvgetTimeStamp";
    EpicsGetTimeStamp(pv, timestamp);
    return true;
}

int Epics3Plugin::pvGetDescription(char *pv, char *description) {
    //qDebug() << "Epics3Plugin:pvGetDescription";
    EpicsGetDescription(pv, description);
    return true;
}

int Epics3Plugin::pvClearEvent(void * ptr) {
    //qDebug() << "Epics3Plugin:pvClearEvent";
    clearEvent(ptr);
    return true;
}

int Epics3Plugin::pvAddEvent(void * ptr) {
    //qDebug() << "Epics3Plugin:pvAddEvent";
    addEvent(ptr);
    return true;
}

int Epics3Plugin::pvReconnect(knobData *kData) {
    //qDebug() << "Epics3Plugin:pvReconnect";
    EpicsReconnect(kData);
    return true;
}

int Epics3Plugin::pvDisconnect(knobData *kData) {
    //qDebug() << "Epics3Plugin:pvDisconnect";
    EpicsDisconnect(kData);
    return true;
} 

int Epics3Plugin::FlushIO() {
    //qDebug() << "Epics3Plugin:FlushIO";
    EpicsFlushIO();
    return true;
}

int Epics3Plugin::TerminateIO() {
    //qDebug() << "Epics3Plugin:TerminateIO";
    TerminateDeviceIO();
    return true;
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#else
    Q_EXPORT_PLUGIN2(Epics3Plugin, Epics3Plugin)
#endif


