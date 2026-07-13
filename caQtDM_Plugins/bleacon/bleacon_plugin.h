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
 *  Copyright (c) 2010 - 2026
 *
 *  Author:
 *    Helge Brands
 *  Contact details:
 *    helge.brands@psi.ch
 */
#ifndef BLEACONPLUGIN_H
#define BLEACONPLUGIN_H

#include <QObject>
#include <QMap>
#include <QMultiMap>
#include <QMutex>
#include <QList>
#include <QStringList>
#include <QTimer>
#include <QUuid>
#include "controlsinterface.h"
#include "beaconscanner.h"

// iBeacon data plugin.
//
// channels (prefix bleacon://):
//   <major>:<minor>.rssi         smoothed rssi [dBm]                    (caDOUBLE)
//   <major>:<minor>.distance     estimated distance [m]                 (caDOUBLE)
//   <major>:<minor>.txpower      advertised power at 1m [dBm]           (caLONG, 0 on ios)
//   <major>:<minor>.age          seconds since last sighting            (caDOUBLE)
//   <major>:<minor>.status       neverseen/lost/weak/present            (caENUM)
//   <major>:<minor>.readcounter  number of received sightings           (caLONG)
//   <major>:<minor>.lostcounter  number of present->lost transitions    (caLONG)
//   nearest.id                   "major:minor" of the nearest beacon    (caSTRING)
//   nearest.major/.minor         numeric id of the nearest beacon       (caLONG)
//   nearest.distance/.rssi       values of the nearest beacon           (caDOUBLE)
//   nearest.valid                1 = a beacon is currently present      (caLONG)
//   beacons.list                 discovered beacons, one channel base per line (caSTRING)
//   beacons.reset                write 1 to clear the discovery list    (caLONG, write)
//   beacons.config               write 1 to generate the config file    (caLONG, write)
//   scan.enable                  stop/start scanning                    (caENUM, write)
//
// configuration (environment, mobile .config file sets these too):
//   CAQTDM_BLEACON_UUIDS       proximity uuids, ';' separated (mandatory on ios)
//   CAQTDM_BLEACON_CONFIG     config file name (default bleacon.config, one uuid per line)
//   CAQTDM_BLEACON_PATHLOSS_N path loss exponent for the distance estimate (default 2.2)
//   CAQTDM_BLEACON_TIMEOUT    seconds without sighting until a beacon is lost (default 10)
//   CAQTDM_BLEACON_WEAK_RSSI  rssi threshold for status weak (default -90)
//   CAQTDM_BLEACON_SIM        1 = simulated beacons instead of real hardware
class Q_DECL_EXPORT bleaconPlugin : public QObject, ControlsInterface
{
    Q_OBJECT
    Q_INTERFACES(ControlsInterface)
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "ch.psi.caqtdm.Plugin.ControlsInterface/1.0.bleaconcontrols")
#endif

public:
    QString pluginName();
    bleaconPlugin();
    ~bleaconPlugin();

    int initCommunicationLayer(MutexKnobData *data, MessageWindow *messageWindow, QMap<QString, QString> options);
    int pvAddMonitor(int index, knobData *kData, int rate, int skip);
    int pvClearMonitor(knobData *kData);
    int pvFreeAllocatedData(knobData *kData);
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
    void beaconSighting(QUuid uuid, quint16 major, quint16 minor, int rssi, int txPower, double accuracyMeters);
    void scannerMessage(QString message, bool isError);
    void sweep();

private:
    enum BeaconStatus { StatusNeverseen = 0, StatusLost, StatusWeak, StatusPresent };

    struct BeaconState {
        quint16 major;
        quint16 minor;
        QUuid uuid;
        double ewmaRssi;      // NaN until the first sighting
        double distance;      // NaN until the first sighting
        int txPower;
        qint64 lastSeenMs;
        qint64 readCounter;
        qint64 lostCounter;
        int status;
    };

    static QString beaconKey(quint16 major, quint16 minor);
    BeaconState *findOrCreateBeacon(quint16 major, quint16 minor, const QUuid &uuid);

    void loadConfiguration(QMap<QString, QString> options);
    void writeConfigFile();

    void startScanning();
    void stopScanning();

    void updateChannelDouble(const QString &pv, double value);
    void updateChannelLong(const QString &pv, qint64 value);
    void updateChannelString(const QString &pv, const QString &value);
    void updateChannelEnum(const QString &pv, int index, const QStringList &strings);
    void fillEnumData(knobData *kData, int index, const QStringList &strings);
    void fillStringData(knobData *kData, const QString &value);
    void pushKnobData(knobData *kData);

    void pushBeaconState(const QString &key, const BeaconState &beacon);
    void pushBeaconList();
    void updateNearest();

    QMutex mutex;
    MutexKnobData *mutexknobdataP;
    MessageWindow *messagewindowP;

    BeaconScannerBase *scanner;
    QTimer *sweepTimer;
    bool scanning;

    QMap<QString, BeaconState> beacons;
    QMultiMap<QString, int> monitorsByPv;

    QString nearestKey;
    int nearestHoldCount;

    // configuration
    QList<QUuid> uuidList;
    QString configFileName;
    double pathLossExponent;
    int staleTimeoutSec;
    int weakRssiThreshold;

    QStringList statusStrings;
    QStringList scanStrings;
};

#endif // BLEACONPLUGIN_H
