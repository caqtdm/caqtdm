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
#ifndef BEACONSCANNER_IOS_H
#define BEACONSCANNER_IOS_H

#include "beaconscanner.h"

// iBeacon ranging through CoreLocation (CLLocationManager / CLBeaconIdentityConstraint).
// ios strips iBeacon frames from CoreBluetooth scans, so Qt Bluetooth cannot be used
// for iBeacon here (eddystone runs through the parallel BeaconScannerQtBle).
// ranging is per proximity uuid: without a uuid list iBeacon stays disabled on ios.
// txPower is not exposed by CoreLocation; the accuracy (meters) is delivered instead.
class BeaconScannerIos : public BeaconScannerBase
{
    Q_OBJECT
public:
    explicit BeaconScannerIos(QObject *parent = Q_NULLPTR);
    ~BeaconScannerIos();

    void startScan(const QList<QUuid> &uuids, const QStringList &eddystoneNamespaces);
    void stopScan();

    // called by the objective-c delegate (queued into the qt thread)
    Q_INVOKABLE void onRangedBeacon(QString uuid, quint16 major, quint16 minor, int rssi, double accuracyMeters);
    Q_INVOKABLE void onRangingError(QString message);

private:
    // opaque pointers to the CLLocationManager and its delegate (objective-c objects)
    void *locationManager;
    void *locationDelegate;
    QList<QUuid> rangedUuids;
};

#endif // BEACONSCANNER_IOS_H
