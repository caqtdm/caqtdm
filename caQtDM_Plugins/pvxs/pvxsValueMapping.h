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
#ifndef PVXSVALUEMAPPING_H
#define PVXSVALUEMAPPING_H

#include <string>
#include <vector>

#include <pvxs/data.h>

#include "knobData.h"

// Maps a pvxs NTScalar/NTScalarArray/NTEnum-shaped Value onto caQtDM's epicsData.
namespace pvxsValueMapping {

// Fills value/alarm fields; false if val has no "value" field.
bool fillValue(const pvxs::Value &val, epicsData &edata);

// Fills units/precision/limits from a "display,control" Get result.
void fillLimitsFromDisplayControl(const pvxs::Value &val, epicsData &edata);

bool isEnumValue(const pvxs::Value &val);

std::vector<std::string> enumChoices(const pvxs::Value &val);

// Resolves a caQtDM put text ("On" or "1") to an enum index; -1 if no match.
int enumIndexFor(const pvxs::Value &val, const std::string &text);

}

#endif // PVXSVALUEMAPPING_H
