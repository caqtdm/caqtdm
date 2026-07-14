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
#include <QCoreApplication>
#include <QBluetoothUuid>
#include <QtMath>

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0) && defined(Q_OS_ANDROID)
  #include <QPermissions>
#endif

#include "beaconscanner_qtble.h"

// apple company id carrying the iBeacon frame in the manufacturer specific data
#define APPLE_COMPANY_ID 0x004C
// eddystone service data uuid and frame types
#define EDDYSTONE_SERVICE_UUID 0xFEAA
#define EDDYSTONE_FRAME_UID 0x00
#define EDDYSTONE_FRAME_TLM 0x20
// eddystone txPower is calibrated at 0m; the power at 1m is ~41dB lower
#define EDDYSTONE_0M_TO_1M_DB 41
// scan cycle: restart every 20s (plus 1s pause) so android keeps delivering
// advertisements without hitting the scan throttling of long running scans
#define SCAN_CYCLE_MS 20000
#define SCAN_PAUSE_MS 1000

BeaconScannerQtBle::BeaconScannerQtBle(QObject *parent) : BeaconScannerBase(parent)
{
    scanEnabled = false;

    discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
    discoveryAgent->setLowEnergyDiscoveryTimeout(SCAN_CYCLE_MS);
#endif

    connect(discoveryAgent, SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)),
            this, SLOT(deviceDiscovered(QBluetoothDeviceInfo)));
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    // deviceUpdated carries the rssi/advertisement refreshes of already seen
    // devices and is essential for continuous ranging
    connect(discoveryAgent, SIGNAL(deviceUpdated(QBluetoothDeviceInfo,QBluetoothDeviceInfo::Fields)),
            this, SLOT(deviceUpdated(QBluetoothDeviceInfo,QBluetoothDeviceInfo::Fields)));
#endif
    connect(discoveryAgent, SIGNAL(finished()), this, SLOT(scanFinished()));
    connect(discoveryAgent, SIGNAL(canceled()), this, SLOT(scanFinished()));

    restartTimer = new QTimer(this);
    restartTimer->setSingleShot(true);
    connect(restartTimer, SIGNAL(timeout()), this, SLOT(restartScan()));
}

void BeaconScannerQtBle::startScan(const QList<QUuid> &uuids, const QStringList &eddystoneNamespaces)
{
    uuidFilter = uuids;
    namespaceFilter.clear();
    foreach (const QString &ns, eddystoneNamespaces) namespaceFilter.append(ns.toUpper());
    scanEnabled = true;

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0) && defined(Q_OS_ANDROID)
    // android needs the bluetooth (and on older devices the location) runtime permission
    QBluetoothPermission bluetoothPermission;
    bluetoothPermission.setCommunicationModes(QBluetoothPermission::Access);
    switch (qApp->checkPermission(bluetoothPermission)) {
    case Qt::PermissionStatus::Undetermined:
        qApp->requestPermission(bluetoothPermission, this, [this](const QPermission &permission) {
            if (permission.status() == Qt::PermissionStatus::Granted) startScan(uuidFilter, namespaceFilter);
            else emit scannerMessage("bleacon: bluetooth permission denied", true);
        });
        return;
    case Qt::PermissionStatus::Denied:
        emit scannerMessage("bleacon: bluetooth permission denied", true);
        return;
    case Qt::PermissionStatus::Granted:
        break;
    }
    QLocationPermission locationPermission;
    locationPermission.setAccuracy(QLocationPermission::Precise);
    switch (qApp->checkPermission(locationPermission)) {
    case Qt::PermissionStatus::Undetermined:
        qApp->requestPermission(locationPermission, this, [this](const QPermission &permission) {
            if (permission.status() == Qt::PermissionStatus::Granted) startScan(uuidFilter, namespaceFilter);
            else emit scannerMessage("bleacon: location permission denied, beacon scan results may be empty", true);
        });
        return;
    case Qt::PermissionStatus::Denied:
        emit scannerMessage("bleacon: location permission denied, beacon scan results may be empty", true);
        break;
    case Qt::PermissionStatus::Granted:
        break;
    }
#endif

    reallyStart();
}

void BeaconScannerQtBle::reallyStart()
{
    if (!scanEnabled) return;
    if (discoveryAgent->isActive()) return;
    discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
}

void BeaconScannerQtBle::stopScan()
{
    scanEnabled = false;
    restartTimer->stop();
    if (discoveryAgent->isActive()) discoveryAgent->stop();
}

void BeaconScannerQtBle::scanFinished()
{
    // cycle the scan while enabled, with a short pause between the cycles
    if (scanEnabled) restartTimer->start(SCAN_PAUSE_MS);
}

void BeaconScannerQtBle::restartScan()
{
    reallyStart();
}

void BeaconScannerQtBle::deviceDiscovered(const QBluetoothDeviceInfo &info)
{
    handleDeviceInfo(info);
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
void BeaconScannerQtBle::deviceUpdated(const QBluetoothDeviceInfo &info, QBluetoothDeviceInfo::Fields updatedFields)
{
    Q_UNUSED(updatedFields);
    handleDeviceInfo(info);
}
#endif

void BeaconScannerQtBle::handleDeviceInfo(const QBluetoothDeviceInfo &info)
{
    handleIBeacon(info);
    handleEddystone(info);
}

// on ios/macx CoreBluetooth hides the device address, the per session device uuid is used instead
QString BeaconScannerQtBle::deviceKey(const QBluetoothDeviceInfo &info)
{
    QString key = info.address().toString();
    if (key.isEmpty() || key == "00:00:00:00:00:00") key = info.deviceUuid().toString();
    return key;
}

void BeaconScannerQtBle::handleIBeacon(const QBluetoothDeviceInfo &info)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    // iBeacon frame: 0x02 0x15, 16 byte proximity uuid, major, minor, int8 txpower (at 1m)
    const QList<QByteArray> frames = info.manufacturerData().values(APPLE_COMPANY_ID);
    foreach (const QByteArray &frame, frames) {
        if (frame.size() < 23) continue;
        if ((quint8) frame.at(0) != 0x02 || (quint8) frame.at(1) != 0x15) continue;

        QUuid uuid = QUuid::fromRfc4122(frame.mid(2, 16));
        quint16 major = (quint16) (((quint8) frame.at(18) << 8) | (quint8) frame.at(19));
        quint16 minor = (quint16) (((quint8) frame.at(20) << 8) | (quint8) frame.at(21));
        int txPower = (qint8) frame.at(22);

        if (!uuidFilter.isEmpty() && !uuidFilter.contains(uuid)) continue;

        emit beaconSighting("ibeacon", ibeaconId(major, minor),
                            uuid.toString(QUuid::WithoutBraces).toUpper(),
                            info.rssi(), txPower, qQNaN());
    }
#else
    Q_UNUSED(info);
#endif
}

void BeaconScannerQtBle::handleEddystone(const QBluetoothDeviceInfo &info)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    const QList<QByteArray> frames = info.serviceData().values(QBluetoothUuid((quint16) EDDYSTONE_SERVICE_UUID));
    foreach (const QByteArray &frame, frames) {
        if (frame.isEmpty()) continue;
        quint8 frameType = (quint8) frame.at(0);

        if (frameType == EDDYSTONE_FRAME_UID && frame.size() >= 18) {
            // UID frame: int8 txpower (at 0m!), 10 byte namespace, 6 byte instance
            int txPowerAt1m = ((qint8) frame.at(1)) - EDDYSTONE_0M_TO_1M_DB;
            QString ns = QString(frame.mid(2, 10).toHex()).toUpper();
            QString instance = QString(frame.mid(12, 6).toHex()).toUpper();

            if (!namespaceFilter.isEmpty() && !namespaceFilter.contains(ns)) continue;

            instanceByDevice.insert(deviceKey(info), instance);
            emit beaconSighting("eddystone", instance, ns, info.rssi(), txPowerAt1m, qQNaN());
        } else if (frameType == EDDYSTONE_FRAME_TLM && frame.size() >= 14) {
            // plain TLM frame (version 0): uint16 battery [mV], int16 temperature [8.8 fixed]
            if ((quint8) frame.at(1) != 0x00) continue;

            // TLM carries no beacon id: associate through the UID frame of the same device
            QString instance = instanceByDevice.value(deviceKey(info));
            if (instance.isEmpty()) continue;

            quint16 batteryMv = (quint16) (((quint8) frame.at(2) << 8) | (quint8) frame.at(3));
            qint16 temperatureRaw = (qint16) (((quint8) frame.at(4) << 8) | (quint8) frame.at(5));

            double battery = (batteryMv == 0) ? qQNaN() : batteryMv / 1000.0;
            double temperature = (temperatureRaw == (qint16) 0x8000) ? qQNaN() : temperatureRaw / 256.0;

            emit beaconTelemetry("eddystone", instance, battery, temperature);
        }
    }
#else
    Q_UNUSED(info);
#endif
}
