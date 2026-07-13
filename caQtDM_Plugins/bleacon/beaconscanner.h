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
#include <QUuid>

// backend abstraction for receiving iBeacon advertisements.
// implementations: BeaconScannerQtBle (android/linux/macx via Qt Bluetooth),
//                  BeaconScannerIos   (CoreLocation ranging, needs the uuid list),
//                  BeaconScannerSim   (synthetic beacons, CAQTDM_BLEACON_SIM=1)
class BeaconScannerBase : public QObject
{
    Q_OBJECT
public:
    explicit BeaconScannerBase(QObject *parent = Q_NULLPTR) : QObject(parent) {}

    // uuids: iBeacon proximity uuids to range; empty list = accept all (not possible on ios)
    virtual void startScan(const QList<QUuid> &uuids) = 0;
    virtual void stopScan() = 0;

    // selects the platform backend, or the simulation when CAQTDM_BLEACON_SIM=1
    static BeaconScannerBase *createScanner(QObject *parent);

signals:
    // accuracyMeters is a backend distance estimate (ios CLBeacon.accuracy), NaN when not available
    void beaconSighting(QUuid uuid, quint16 major, quint16 minor, int rssi, int txPower, double accuracyMeters);
    void scannerMessage(QString message, bool isError);
};

#endif // BEACONSCANNER_H
