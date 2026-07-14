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
#include <QMetaObject>
#include <QtMath>

#import <Foundation/Foundation.h>
#import <CoreLocation/CoreLocation.h>

#include "beaconscanner_ios.h"

// objective-c delegate forwarding the CoreLocation callbacks into the qt object.
// the callbacks may arrive on any runloop, therefore Qt::QueuedConnection.
@interface BleaconLocationDelegate : NSObject <CLLocationManagerDelegate>
{
@public
    BeaconScannerIos *owner;
}
@end

@implementation BleaconLocationDelegate

- (void)locationManager:(CLLocationManager *)manager
       didRangeBeacons:(NSArray<CLBeacon *> *)beacons
  satisfyingConstraint:(CLBeaconIdentityConstraint *)beaconConstraint
{
    Q_UNUSED(manager);
    Q_UNUSED(beaconConstraint);
    if (owner == Q_NULLPTR) return;

    for (CLBeacon *beacon in beacons) {
        // accuracy < 0 means unknown
        double accuracy = (beacon.accuracy < 0.0) ? qQNaN() : beacon.accuracy;
        QMetaObject::invokeMethod(owner, "onRangedBeacon", Qt::QueuedConnection,
                                  Q_ARG(QString, QString::fromNSString(beacon.UUID.UUIDString)),
                                  Q_ARG(quint16, beacon.major.unsignedShortValue),
                                  Q_ARG(quint16, beacon.minor.unsignedShortValue),
                                  Q_ARG(int, (int) beacon.rssi),
                                  Q_ARG(double, accuracy));
    }
}

- (void)locationManager:(CLLocationManager *)manager
didFailRangingBeaconsForConstraint:(CLBeaconIdentityConstraint *)beaconConstraint
                  error:(NSError *)error
{
    Q_UNUSED(manager);
    Q_UNUSED(beaconConstraint);
    if (owner == Q_NULLPTR) return;
    QString message = QString("bleacon: ranging failed: %1").arg(QString::fromNSString(error.localizedDescription));
    QMetaObject::invokeMethod(owner, "onRangingError", Qt::QueuedConnection, Q_ARG(QString, message));
}

@end

BeaconScannerIos::BeaconScannerIos(QObject *parent) : BeaconScannerBase(parent)
{
    // metatype for the queued invocations from the delegate
    qRegisterMetaType<quint16>("quint16");

    CLLocationManager *manager = [[CLLocationManager alloc] init];
    BleaconLocationDelegate *delegate = [[BleaconLocationDelegate alloc] init];
    delegate->owner = this;
    manager.delegate = delegate;

    locationManager = (void *) manager;
    locationDelegate = (void *) delegate;
}

BeaconScannerIos::~BeaconScannerIos()
{
    stopScan();
    CLLocationManager *manager = (CLLocationManager *) locationManager;
    BleaconLocationDelegate *delegate = (BleaconLocationDelegate *) locationDelegate;
    delegate->owner = Q_NULLPTR;
    manager.delegate = nil;
    [manager release];
    [delegate release];
}

void BeaconScannerIos::startScan(const QList<QUuid> &uuids, const QStringList &eddystoneNamespaces)
{
    Q_UNUSED(eddystoneNamespaces);   // eddystone runs through the parallel Qt Bluetooth scanner

    if (uuids.isEmpty()) {
        emit scannerMessage("bleacon: no CAQTDM_BLEACON_UUIDS configured - "
                            "iBeacon ranging disabled on ios (eddystone still active)", false);
        return;
    }

    CLLocationManager *manager = (CLLocationManager *) locationManager;
    [manager requestWhenInUseAuthorization];

    rangedUuids = uuids;
    foreach (const QUuid &uuid, uuids) {
        QString uuidString = uuid.toString(QUuid::WithoutBraces);
        NSUUID *nsUuid = [[NSUUID alloc] initWithUUIDString:uuidString.toNSString()];
        if (nsUuid == nil) {
            emit scannerMessage(QString("bleacon: invalid uuid %1").arg(uuidString), true);
            continue;
        }
        CLBeaconIdentityConstraint *constraint = [[CLBeaconIdentityConstraint alloc] initWithUUID:nsUuid];
        [manager startRangingBeaconsSatisfyingConstraint:constraint];
        [constraint release];
        [nsUuid release];
    }
}

void BeaconScannerIos::stopScan()
{
    CLLocationManager *manager = (CLLocationManager *) locationManager;
    foreach (const QUuid &uuid, rangedUuids) {
        QString uuidString = uuid.toString(QUuid::WithoutBraces);
        NSUUID *nsUuid = [[NSUUID alloc] initWithUUIDString:uuidString.toNSString()];
        if (nsUuid == nil) continue;
        CLBeaconIdentityConstraint *constraint = [[CLBeaconIdentityConstraint alloc] initWithUUID:nsUuid];
        [manager stopRangingBeaconsSatisfyingConstraint:constraint];
        [constraint release];
        [nsUuid release];
    }
    rangedUuids.clear();
}

void BeaconScannerIos::onRangedBeacon(QString uuid, quint16 major, quint16 minor, int rssi, double accuracyMeters)
{
    // CoreLocation does not expose the advertised txpower (0), the accuracy estimate is used instead
    emit beaconSighting("ibeacon", ibeaconId(major, minor),
                        uuid.toUpper(), rssi, 0, accuracyMeters);
}

void BeaconScannerIos::onRangingError(QString message)
{
    emit scannerMessage(message, true);
}
