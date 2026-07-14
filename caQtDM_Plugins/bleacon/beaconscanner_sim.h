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
#ifndef BEACONSCANNER_SIM_H
#define BEACONSCANNER_SIM_H

#include <QTimer>
#include <QVector>
#include "beaconscanner.h"

// synthetic beacons for display development without hardware (CAQTDM_BLEACON_SIM=1):
// ibeacon:0001:0001 near (~2m), ibeacon:0001:0002 far (~8m),
// eddystone:AABBCCDDEE21 (~4m, with TLM telemetry) disappears every 15s to exercise lost/present
class BeaconScannerSim : public BeaconScannerBase
{
    Q_OBJECT
public:
    explicit BeaconScannerSim(QObject *parent = Q_NULLPTR);

    void startScan(const QList<QUuid> &uuids, const QStringList &eddystoneNamespaces);
    void stopScan();

private slots:
    void emitSightings();

private:
    struct SimBeacon {
        QString protocol;     // "ibeacon" or "eddystone"
        QString id;           // "major:minor" or instance hex
        QString group;        // proximity uuid or namespace hex
        double distance;      // simulated true distance in m
        bool intermittent;    // toggles visibility every 15s
        bool telemetry;       // emits TLM battery/temperature
    };

    QVector<SimBeacon> simBeacons;
    QTimer *timer;
    int tickCounter;
};

#endif // BEACONSCANNER_SIM_H
