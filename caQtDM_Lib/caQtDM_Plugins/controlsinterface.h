#ifndef CONTROLSINTERFACE_H
#define CONTROLSINTERFACE_H
#include "mutexKnobData.h"
#include <QString>
#include <QtPlugin>

class ControlsInterface

{
public:
    virtual ~ControlsInterface() {}
    virtual QString pluginName() = 0;
    virtual int initCommunicationLayer(MutexKnobData *data) = 0;
    virtual int pvAddMonitor(int index, knobData *kData, int rate, int skip) = 0;
    virtual int pvClearMonitor(knobData *kData) = 0;
    virtual int pvSetValue(char *pv, double rdata, int32_t idata, char *sdata, char *object, char *errmess, int forceType) = 0;
    virtual int pvSetWave(char *pv, float *fdata, double *ddata, int16_t *data16, int32_t *data32, char *sdata, int nelm, char *object, char *errmess) = 0;
    virtual int pvGetTimeStamp(char *pv, char *timestamp) = 0;
    virtual int pvGetDescription(char *pv, char *description) = 0;
    virtual int pvClearEvent(void * ptr) = 0;
    virtual int pvAddEvent(void * ptr) = 0;
    virtual int pvReconnect(knobData *kData) = 0;
    virtual int pvDisconnect(knobData *kData) = 0;
    virtual int FlushIO() = 0;
    virtual int TerminateIO() = 0;

    
    
};

QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(ControlsInterface, "Plugin.ControlsInterface/1.0")
QT_END_NAMESPACE

#endif
