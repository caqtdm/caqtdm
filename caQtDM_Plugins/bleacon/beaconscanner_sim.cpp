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
#include <QRandomGenerator>
#include <QtMath>

#include "beaconscanner_sim.h"

// calibrated power at 1m used by the simulation and the matching path loss exponent
#define SIM_TXPOWER (-59)
#define SIM_PATHLOSS_N 2.2

BeaconScannerSim::BeaconScannerSim(QObject *parent) : BeaconScannerBase(parent)
{
    SimBeacon b1 = {"ibeacon", ibeaconId(1, 1), "FDA50693A4E24FB1AFCFC6EB07647825", 2.0, false, false};
    SimBeacon b2 = {"ibeacon", ibeaconId(1, 2), "FDA50693A4E24FB1AFCFC6EB07647825", 8.0, false, false};
    SimBeacon b3 = {"eddystone", "AABBCCDDEE21", "0102030405060708090A", 4.0, true, true};
    simBeacons << b1 << b2 << b3;

    tickCounter = 0;
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(emitSightings()));
}

void BeaconScannerSim::startScan(const QList<QUuid> &uuids, const QStringList &eddystoneNamespaces)
{
    Q_UNUSED(uuids);
    Q_UNUSED(eddystoneNamespaces);
    emit scannerMessage("bleacon: simulation backend active (CAQTDM_BLEACON_SIM=1)", false);
    timer->start(500);
}

void BeaconScannerSim::stopScan()
{
    timer->stop();
}

void BeaconScannerSim::emitSightings()
{
    tickCounter++;

    for (int i = 0; i < simBeacons.size(); i++) {
        SimBeacon &b = simBeacons[i];

        // intermittent beacon is gone for 15s out of every 30s
        if (b.intermittent && ((tickCounter / 30) % 2 == 1)) continue;

        // random walk of the simulated distance, kept in a sane range
        double step = (QRandomGenerator::global()->generateDouble() - 0.5) * 0.4;
        b.distance = qBound(0.5, b.distance + step, 20.0);

        // rssi from the log-distance path loss model plus measurement noise
        double noise = (QRandomGenerator::global()->generateDouble() - 0.5) * 6.0;
        int rssi = qRound(SIM_TXPOWER - 10.0 * SIM_PATHLOSS_N * log10(b.distance) + noise);

        emit beaconSighting(b.protocol, b.id, b.group, rssi, SIM_TXPOWER, qQNaN());

        // TLM telemetry every 5s: slowly draining battery, wandering temperature,
        // advertisement counter and uptime like a real token
        if (b.telemetry && (tickCounter % 10 == 0)) {
            double battery = 3.05 - 0.00005 * tickCounter;
            double temperature = 21.5 + (QRandomGenerator::global()->generateDouble() - 0.5);
            emit beaconTelemetry(b.protocol, b.id, battery, temperature,
                                 (qint64) tickCounter, tickCounter * 0.5);
        }
    }
}
