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
 */
#ifndef PVXSPLUGIN_H
#define PVXSPLUGIN_H

#include <atomic>
#include <thread>

#include <pvxs/client.h>

#include <QObject>

#include "controlsinterface.h"
#include "pvxsChannel.h"

// caQtDM control-system plugin, pv name prefix "pvxs://".
class Q_DECL_EXPORT PvxsPlugin : public QObject, ControlsInterface
{
    Q_OBJECT
    Q_INTERFACES(ControlsInterface)
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "ch.psi.caqtdm.Plugin.ControlsInterface/1.0.pvxscontrols")
#endif

public:
    QString pluginName();
    PvxsPlugin();
    ~PvxsPlugin();

    int initCommunicationLayer(MutexKnobData *data, MessageWindow *messageWindow, QMap<QString, QString> options);
    int pvAddMonitor(int index, knobData *kData, int rate, int skip);
    int pvClearMonitor(knobData *kData);
    int pvFreeAllocatedData(knobData *kData);

    int pvSetValue(char *pv, double rdata, int32_t idata, char *sdata, char *object, char *errmess, int forceType)
    {
        Q_UNUSED(pv); Q_UNUSED(rdata); Q_UNUSED(idata); Q_UNUSED(sdata); Q_UNUSED(object); Q_UNUSED(errmess); Q_UNUSED(forceType);
        return -1;
    }
    bool pvSetValue(knobData *kData, double rdata, int32_t idata, char *sdata, char *object, char *errmess, int forceType);

    int pvSetWave(char *pv, float *fdata, double *ddata, int16_t *data16, int32_t *data32, char *sdata, int nelm, char *object, char *errmess)
    {
        Q_UNUSED(pv); Q_UNUSED(fdata); Q_UNUSED(ddata); Q_UNUSED(data16); Q_UNUSED(data32);
        Q_UNUSED(sdata); Q_UNUSED(nelm); Q_UNUSED(object); Q_UNUSED(errmess);
        return -1;
    }
    bool pvSetWave(knobData *kData, float *fdata, double *ddata, int16_t *data16, int32_t *data32, char *sdata, int nelm, char *object, char *errmess);

    int pvGetTimeStamp(char *pv, char *timestamp)
    {
        Q_UNUSED(pv); Q_UNUSED(timestamp);
        return -1;
    }
    int pvGetDescription(char *pv, char *description)
    {
        Q_UNUSED(pv); Q_UNUSED(description);
        return -1;
    }

    int pvClearEvent(void *ptr);
    int pvAddEvent(void *ptr);
    int pvReconnect(knobData *kData);
    int pvDisconnect(knobData *kData);
    int FlushIO();
    int TerminateIO();

private:
    MutexKnobData *mutexKnobData = nullptr;
    MessageWindow *messageWindow = nullptr;
    pvxs::client::Context ctxt;

    // unbounded: push() must never block (TerminateIO() pushes the shutdown sentinel from the GUI thread)
    pvxs::MPMCFIFO<PvxsChannelPtr> workQueue;
    std::thread drainThread;
    std::atomic<bool> stopDrain{false};

    void drainLoop();
    void handleMonitorValue(const PvxsChannelPtr &ch, const pvxs::Value &val);
    void handleMonitorConnected(const PvxsChannelPtr &ch);
    void handleMonitorDisconnected(const PvxsChannelPtr &ch);
    void handleDisplayControlResult(const PvxsChannelPtr &ch, pvxs::client::Result &&res);

    void startMonitor(const PvxsChannelPtr &ch);
    void stopMonitor(const PvxsChannelPtr &ch);

    void logError(const QString &msg);

    static PvxsChannelPtr channelFor(knobData *kData);
};

#endif // PVXSPLUGIN_H
