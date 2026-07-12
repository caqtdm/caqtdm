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
 */
#ifndef PVXSCHANNEL_H
#define PVXSCHANNEL_H

#include <atomic>
#include <memory>
#include <mutex>
#include <string>

#include <pvxs/client.h>

// Per-PV state, owned via kData->edata.info (mirrors epics4's PVAInterfaceGlue).
class PvxsChannel
{
public:
    PvxsChannel(std::string pvName_, int index_)
        : pvName(std::move(pvName_)), index(index_)
    {}

    const std::string pvName;
    const int index;

    // Guards the shared_ptrs below: written from the GUI thread, read from the drain thread.
    std::mutex opMutex;
    std::shared_ptr<pvxs::client::Subscription> subscription;
    std::shared_ptr<pvxs::client::Operation> pendingPutOp;

    std::atomic<short> fieldtype{-1};
    std::atomic<bool> isEnum{false};
};

using PvxsChannelPtr = std::shared_ptr<PvxsChannel>;

#endif // PVXSCHANNEL_H
