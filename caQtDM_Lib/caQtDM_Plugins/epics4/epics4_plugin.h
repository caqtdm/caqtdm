#ifndef EPICS4PLUGIN_H
#define EPICS4PLUGIN_H

#include <QObject>
#include "controlsinterface.h"
#ifdef EPICS4
  #include "epics4Subs.h"
#endif

class Epics4Plugin : public QObject, ControlsInterface
{
    Q_OBJECT
    Q_INTERFACES(ControlsInterface)
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.epics4controls")
#endif

public:
    QString pluginName();
    Epics4Plugin();

    int initCommunicationLayer(MutexKnobData *data);
    int pvAddMonitor(int index, knobData *kData, int rate, int skip);
    int pvClearMonitor(knobData *kData);
    int pvSetValue(char *pv, double rdata, int32_t idata, char *sdata, char *object, char *errmess, int forceType);
    int pvSetWave(char *pv, float *fdata, double *ddata, int16_t *data16, int32_t *data32, char *sdata, int nelm, char *object, char *errmess);
    int pvGetTimeStamp(char *pv, char *timestamp);
    int pvGetDescription(char *pv, char *description);
    int pvClearEvent(void * ptr);
    int pvAddEvent(void * ptr);
    int pvReconnect(knobData *kData);
    int pvDisconnect(knobData *kData);
    int FlushIO();
    int TerminateIO();       

  private:
    MutexKnobData* _mutexknobdata;
#ifdef EPICS4
    epics4Subs *Epics4;
#endif
};

#endif
