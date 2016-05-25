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
 *  Copyright (c) 2010 - 2014
 *
 *  Author:
 *    Anton Mezger
 *  Contact details:
 *    anton.mezger@psi.ch
 */

#ifndef EPICS4SUBS_H
#define EPICS4SUBS_H

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <vector>
#include <string>

#include <pv/clientFactory.h>
#include <pv/logger.h>
#include <pv/lock.h>
#include <pv/rpcService.h>
#include <pv/rpcClient.h>
#include <pv/event.h>
#include <pv/pvAccess.h>

#include "dbrString.h"
#include "knobDefines.h"

#include "knobData.h"
#include "mutexKnobData.h"
#include "mutexKnobDataWrapper.h"

using namespace std;
using namespace std::tr1;
using namespace epics::pvData;
using namespace epics::pvAccess;


#define DEFAULT_TIMEOUT 3.0
#define DEFAULT_REQUEST "field()"

#include <caQtDM_Lib_global.h>

using namespace std;

class CAQTDM_LIBSHARED_EXPORT epics4Subs {

public:
    epics4Subs(MutexKnobData* mutexKnobData);
     ~epics4Subs();
    void CreateAndConnect4(int num, QString pv);
    void ParseScalar(PVScalarPtr const & pv);
    void Epics4SetValue(QString const &pv, QString const & value);
    void ParsePVStructure(PVStructurePtr const & pvStructure, QString const & value);
    void fromString(PVScalarPtr const & pvScalar, QString const & value);

private:

    PVStructure::shared_pointer pvRequestG;
    PVStructure::shared_pointer pvRequestP;

    ChannelProvider::shared_pointer provider;
    MutexKnobData* m_mutexKnobData;
    QVector<Monitor::shared_pointer> monitorArray;
    QVector<Channel::shared_pointer> channelArray;
};

#endif
