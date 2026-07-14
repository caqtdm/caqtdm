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
#ifndef BEACONSCANNER_QTBLE_H
#define BEACONSCANNER_QTBLE_H

#include <QTimer>
#include <QHash>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include "beaconscanner.h"

// beacon reception through Qt Bluetooth (android, linux, ios, macx):
//   ibeacon   - apple manufacturer data 0x004C (not delivered on ios and newer macx,
//               there CoreLocation/the simulation take over)
//   eddystone - service data 0xFEAA: UID frames (identity) and TLM frames (telemetry)
class BeaconScannerQtBle : public BeaconScannerBase
{
    Q_OBJECT
public:
    explicit BeaconScannerQtBle(QObject *parent = Q_NULLPTR);

    void startScan(const QList<QUuid> &uuids, const QStringList &eddystoneNamespaces);
    void stopScan();

private slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &info);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    void deviceUpdated(const QBluetoothDeviceInfo &info, QBluetoothDeviceInfo::Fields updatedFields);
#endif
    void scanFinished();
    void restartScan();

private:
    void reallyStart();
    void handleDeviceInfo(const QBluetoothDeviceInfo &info);
    void handleIBeacon(const QBluetoothDeviceInfo &info);
    void handleEddystone(const QBluetoothDeviceInfo &info);
    static QString deviceKey(const QBluetoothDeviceInfo &info);

    QBluetoothDeviceDiscoveryAgent *discoveryAgent;
    QTimer *restartTimer;
    QList<QUuid> uuidFilter;
    QStringList namespaceFilter;
    // TLM frames carry no beacon id: remember the last UID instance per device
    QHash<QString, QString> instanceByDevice;
    bool scanEnabled;
};

#endif // BEACONSCANNER_QTBLE_H
