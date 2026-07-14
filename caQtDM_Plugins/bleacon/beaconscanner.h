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
#ifndef BEACONSCANNER_H
#define BEACONSCANNER_H

#include <QObject>
#include <QList>
#include <QStringList>
#include <QUuid>

// backend abstraction for receiving beacon advertisements.
// protocols: "ibeacon"   id = "major:minor" (4 hex digits each, e.g. "0001:001A"), group = proximity uuid
//            "eddystone" id = instance (12 hex),                                   group = namespace (20 hex)
// every backend normalizes txPower to the power at 1m, so the distance
// conversion in the plugin is protocol independent.
// implementations: BeaconScannerQtBle     (Qt Bluetooth: ibeacon + eddystone; on ios only eddystone,
//                                          since ios strips the iBeacon manufacturer frames)
//                  BeaconScannerIos       (CoreLocation ranging: ibeacon, needs the uuid list)
//                  BeaconScannerComposite (ios: CoreLocation + Qt Bluetooth in parallel)
//                  BeaconScannerSim       (synthetic beacons, CAQTDM_BLEACON_SIM=1)
class BeaconScannerBase : public QObject
{
    Q_OBJECT
public:
    explicit BeaconScannerBase(QObject *parent = Q_NULLPTR) : QObject(parent) {}

    // uuids: iBeacon proximity uuids (mandatory on ios); empty = accept all (Qt Bluetooth only)
    // eddystoneNamespaces: eddystone namespace filter (20 hex chars each); empty = accept all
    virtual void startScan(const QList<QUuid> &uuids, const QStringList &eddystoneNamespaces) = 0;
    virtual void stopScan() = 0;

    // selects the platform backend(s), or the simulation when CAQTDM_BLEACON_SIM=1
    static BeaconScannerBase *createScanner(QObject *parent);

    // canonical ibeacon id: major/minor as 4 hex digits each, uppercase
    static QString ibeaconId(quint16 major, quint16 minor)
    {
        return QString("%1:%2").arg(major, 4, 16, QLatin1Char('0')).arg(minor, 4, 16, QLatin1Char('0')).toUpper();
    }

signals:
    // txPowerAt1m = 0 when unknown (CoreLocation); accuracyMeters = backend distance estimate, NaN when not available
    void beaconSighting(QString protocol, QString beaconId, QString groupId, int rssi, int txPowerAt1m, double accuracyMeters);
    // eddystone TLM telemetry; NaN for fields the beacon does not deliver
    void beaconTelemetry(QString protocol, QString beaconId, double batteryVolts, double temperatureC);
    void scannerMessage(QString message, bool isError);
};

// forwards to several backends at once (ios: CoreLocation for ibeacon, Qt Bluetooth for eddystone)
class BeaconScannerComposite : public BeaconScannerBase
{
    Q_OBJECT
public:
    explicit BeaconScannerComposite(QObject *parent = Q_NULLPTR);
    void addScanner(BeaconScannerBase *scanner);

    void startScan(const QList<QUuid> &uuids, const QStringList &eddystoneNamespaces);
    void stopScan();

private:
    QList<BeaconScannerBase *> scanners;
};

#endif // BEACONSCANNER_H
