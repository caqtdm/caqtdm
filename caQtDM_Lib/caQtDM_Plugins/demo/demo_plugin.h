#ifndef EPICS4PLUGIN_H
#define EPICS4PLUGIN_H

#include <QObject>
#include <QMap>
#include <QMutex>
#include <QList>
#include <QTimer>
#include "controlsinterface.h"

class DemoPlugin : public QObject, ControlsInterface
{
    Q_OBJECT
    Q_INTERFACES(ControlsInterface)

public:
    QString pluginName();
    DemoPlugin();

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

 protected:

private slots:
    void updateValues();
    void updateInterface();

private:
    QMutex mutex;
    MutexKnobData* mutexknobdata;
    QMap<QString, double> listOfDoubles;
    QList<int> listOfIndexes;
    double initValue;
    QTimer *timer, *timerValues;
};

#endif
