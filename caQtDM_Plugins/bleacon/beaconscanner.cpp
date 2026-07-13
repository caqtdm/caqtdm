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
#include <QByteArray>

#include "beaconscanner.h"
#include "beaconscanner_sim.h"

#ifdef Q_OS_IOS
  #include "beaconscanner_ios.h"
#else
  #include "beaconscanner_qtble.h"
#endif

BeaconScannerBase *BeaconScannerBase::createScanner(QObject *parent)
{
    if (qgetenv("CAQTDM_BLEACON_SIM") == "1") {
        return new BeaconScannerSim(parent);
    }
#ifdef Q_OS_IOS
    return new BeaconScannerIos(parent);
#else
    return new BeaconScannerQtBle(parent);
#endif
}
