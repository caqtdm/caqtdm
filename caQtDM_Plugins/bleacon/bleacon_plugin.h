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

// BLE beacon data plugin (iBeacon and Eddystone).
//
// beacon addresses carry the protocol, so displays and the discovery list say
// what to listen to and the backends know how to convert the parameters.
// all ids are hexadecimal:
//   ibeacon:<major>:<minor>      major/minor 4 hex digits, e.g. bleacon://ibeacon:0001:001A.distance
//   eddystone:<instance 12 hex>  e.g. bleacon://eddystone:AABBCCDDEE21.distance
//   <major>:<minor>              shorthand, normalized to ibeacon:<major>:<minor>
//   <alias>                      human readable name from the config file (like the
//                                modbus/opcua translation): Quelle1=eddystone:7CD9F4099CA4
//                                -> bleacon://Quelle1.distance
// unpadded/lowercase input is accepted and normalized (1:a -> ibeacon:0001:000A)
//
// channels (prefix bleacon://, <addr> = protocol tagged address):
//   <addr>.rssi          smoothed rssi [dBm]                    (caDOUBLE)
//   <addr>.distance      estimated distance [m]                 (caDOUBLE)
//   <addr>.txpower       power at 1m [dBm], eddystone converted (caLONG, 0 on ios ibeacon)
//   <addr>.age           seconds since last sighting            (caDOUBLE)
//   <addr>.status        neverseen/lost/weak/present            (caENUM)
//   <addr>.readcounter   number of received sightings           (caLONG)
//   <addr>.lostcounter   number of present->lost transitions    (caLONG)
//   <addr>.battery       eddystone TLM battery [V]              (caDOUBLE)
//   <addr>.temperature   eddystone TLM temperature [degC]       (caDOUBLE)
//   <addr>.name          alias from the config file, "" if none (caSTRING)
//   nearest.id           address of the nearest beacon          (caSTRING)
//   nearest.name         alias of the nearest beacon (address if none) (caSTRING)
//   nearest.major/.minor numeric id (ibeacon only, else 0)      (caLONG)
//   nearest.distance/.rssi values of the nearest beacon         (caDOUBLE)
//   nearest.valid        1 = a beacon is currently present      (caLONG)
//   beacons.list         discovered beacons, one address per line (caSTRING)
//   beacons.reset        write 1 to clear the discovery list    (caLONG, write)
//   beacons.config       write 1 to generate the config file    (caLONG, write)
//   beacons.writepath    directory the config file is written to (caSTRING, write; desktop use)
//   scan.enable          stop/start/known                       (caENUM, write)
//                        known = keep scanning but process only beacons already
//                        known from the config file or from monitors; unknown
//                        tags are ignored completely
//
// the config file stores the groups (uuids/namespaces, used as scan filter) AND the
// discovered beacon addresses, optionally with a human readable alias (Name=Adresse);
// on load the addresses are pre-created (status neverseen), so beacons.list is
// populated right after the start.
//
// configuration (environment, mobile .config file sets these too):
//   CAQTDM_BLEACON_UUIDS      ';' separated: iBeacon proximity uuids (mandatory for ios iBeacon)
//                             and/or eddystone namespaces (20 hex chars), auto-detected by format
//   CAQTDM_BLEACON_CONFIG     config file name (default bleacon.config; uuids, namespaces
//                             and beacon addresses, one per line, auto-detected by format)
//   CAQTDM_BLEACON_WRITEPATH  directory beacons.config writes to (default: found config
//                             file location, else current directory)
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
    void beaconSighting(QString protocol, QString beaconId, QString groupId, int rssi, int txPowerAt1m, double accuracyMeters);
    void beaconTelemetry(QString protocol, QString beaconId, double batteryVolts, double temperatureC);
    void scannerMessage(QString message, bool isError);
    void sweep();

private:
    enum BeaconStatus { StatusNeverseen = 0, StatusLost, StatusWeak, StatusPresent };
    enum ScanMode { ScanStop = 0, ScanStart, ScanKnown };

    struct BeaconState {
        QString protocol;     // "ibeacon" or "eddystone"
        QString id;           // "major:minor" or instance hex
        QString groupId;      // proximity uuid or namespace hex
        QString name;         // human readable alias from the config file, "" if none
        quint16 major;        // ibeacon only, 0 otherwise
        quint16 minor;
        double ewmaRssi;      // NaN until the first sighting
        double distance;      // NaN until the first sighting
        int txPower;          // at 1m, 0 = unknown
        double battery;       // NaN until TLM received
        double temperature;   // NaN until TLM received
        qint64 lastSeenMs;
        qint64 readCounter;
        qint64 lostCounter;
        int status;
    };

    static QString beaconKey(const QString &protocol, const QString &beaconId);
    static QString normalizeIBeaconId(const QString &beaconId);
    BeaconState *findOrCreateBeacon(const QString &protocol, const QString &beaconId, const QString &groupId);

    void loadConfiguration(QMap<QString, QString> options);
    void writeConfigFile();
    void addConfigEntry(const QString &entry);

    void applyScanMode(int mode);

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
    bool scanning;            // backend active
    int scanMode;             // ScanStop / ScanStart / ScanKnown

    QMap<QString, BeaconState> beacons;
    QMap<QString, QString> keyByAlias;   // alias -> canonical beacon key
    QMultiMap<QString, int> monitorsByPv;

    QString nearestKey;
    int nearestHoldCount;

    // configuration
    QList<QUuid> uuidList;
    QStringList namespaceList;
    QString configFileName;
    QString configWritePath;
    double pathLossExponent;
    int staleTimeoutSec;
    int weakRssiThreshold;

    QStringList statusStrings;
    QStringList scanStrings;
};

#endif // BLEACONPLUGIN_H
