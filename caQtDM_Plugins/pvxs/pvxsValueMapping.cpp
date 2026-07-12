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
#include "pvxsValueMapping.h"

#include <cstdlib>
#include <cstring>

#include <db_access.h>

#include <pvxs/sharedArray.h>

using namespace pvxs;

namespace {

short dbfTypeFor(TypeCode code)
{
    switch (code.scalarOf().code) {
    case TypeCode::Int8:
    case TypeCode::UInt8:
        return DBF_CHAR;
    case TypeCode::Float32:
    case TypeCode::Float64:
        return DBF_DOUBLE;
    case TypeCode::String:
        return DBF_STRING;
    default:
        return DBF_LONG;
    }
}

void ensureCapacity(epicsData &edata, int bytes)
{
    if (bytes > edata.dataSize) {
        if (edata.dataB) free(edata.dataB);
        edata.dataB = malloc((size_t) bytes);
        edata.dataSize = bytes;
    }
}

void fillScalar(const Value &valueField, epicsData &edata)
{
    TypeCode code = valueField.type();
    edata.fieldtype = dbfTypeFor(code);
    edata.valueCount = 1;

    if (code.code == TypeCode::String) {
        std::string value;
        valueField.as<std::string>(value);
        int len = int(value.size());
        ensureCapacity(edata, len + 1);
        memcpy(edata.dataB, value.data(), (size_t) len);
        static_cast<char *>(edata.dataB)[len] = '\0';
    } else if (code.kind() == Kind::Real) {
        double value = 0.0;
        valueField.as<double>(value);
        edata.rvalue = value;
        edata.ivalue = (long) value;
    } else {
        int64_t value = 0;
        valueField.as<int64_t>(value);
        edata.ivalue = (long) value;
        edata.rvalue = (double) value;
    }
}

void fillScalarArray(const Value &valueField, epicsData &edata)
{
    TypeCode code = valueField.type();
    edata.fieldtype = dbfTypeFor(code);

    if (code.scalarOf().code == TypeCode::String) {
        shared_array<const void> raw;
        valueField.as<shared_array<const void>>(raw);
        shared_array<const std::string> arr(raw.castTo<const std::string>());
        edata.valueCount = int(arr.size());
        int numBytes = 1;
        for (auto &s : arr) numBytes += int(s.size()) + 1;
        ensureCapacity(edata, numBytes);
        char *ptr = static_cast<char *>(edata.dataB);
        for (auto &s : arr) {
            memcpy(ptr, s.data(), s.size());
            ptr += s.size();
            *ptr++ = '\033';
        }
        if (!arr.empty()) --ptr;
        *ptr = '\0';
        return;
    }

    shared_array<const void> raw;
    valueField.as<shared_array<const void>>(raw);
    edata.valueCount = int(raw.size());

    switch (edata.fieldtype) {
    case DBF_CHAR: {
        shared_array<const int8_t> arr(raw.convertTo<const int8_t>());
        ensureCapacity(edata, int(arr.size()) * int(sizeof(int8_t)));
        memcpy(edata.dataB, arr.data(), arr.size() * sizeof(int8_t));
        break;
    }
    case DBF_DOUBLE: {
        shared_array<const double> arr(raw.convertTo<const double>());
        ensureCapacity(edata, int(arr.size()) * int(sizeof(double)));
        memcpy(edata.dataB, arr.data(), arr.size() * sizeof(double));
        break;
    }
    default: {
        shared_array<const int32_t> arr(raw.convertTo<const int32_t>());
        ensureCapacity(edata, int(arr.size()) * int(sizeof(int32_t)));
        memcpy(edata.dataB, arr.data(), arr.size() * sizeof(int32_t));
        break;
    }
    }
}

void fillEnum(const Value &val, epicsData &edata)
{
    int32_t index = 0;
    val["value.index"].as<int32_t>(index);
    edata.fieldtype = DBF_ENUM;
    edata.ivalue = index;
    edata.rvalue = index;
    edata.valueCount = 1;

    // choices go into dataB, ESC-separated, as caChoice/caMenu expect (like epics4 gotEnum)
    shared_array<const std::string> choices;
    if (!val["value.choices"].as(choices) || choices.empty()) return;
    int numBytes = 1;
    for (auto &s : choices) numBytes += int(s.size()) + 1;
    ensureCapacity(edata, numBytes);
    char *ptr = static_cast<char *>(edata.dataB);
    for (auto &s : choices) {
        memcpy(ptr, s.data(), s.size());
        ptr += s.size();
        *ptr++ = '\033';
    }
    --ptr;
    *ptr = '\0';
    edata.enumCount = int(choices.size());
}

void fillAlarm(const Value &val, epicsData &edata)
{
    Value severityField = val["alarm.severity"];
    if (!severityField.valid()) return;
    int32_t severity = 0;
    severityField.as<int32_t>(severity);
    edata.severity = (short) severity;
    edata.status = (short) (severity == 0 ? 0 : 17);
}

} // namespace

namespace pvxsValueMapping {

bool isEnumValue(const Value &val)
{
    Value valueField = val["value"];
    return valueField.valid() && valueField["index"].valid() && valueField["choices"].valid();
}

std::vector<std::string> enumChoices(const Value &val)
{
    std::vector<std::string> result;
    Value choicesField = val["value.choices"];
    if (!choicesField.valid()) return result;
    shared_array<const std::string> choices;
    if (!choicesField.as<shared_array<const std::string>>(choices)) return result;
    result.assign(choices.begin(), choices.end());
    return result;
}

int enumIndexFor(const Value &val, const std::string &text)
{
    shared_array<const std::string> choices;
    val["value.choices"].as(choices);
    for (size_t i = 0; i < choices.size(); i++) {
        if (text == choices[i]) return int(i);
    }
    // numeric fallback like epics4, but only for pure digit strings ("On" would atoi to 0)
    if (!text.empty() && text.find_first_not_of("0123456789") == std::string::npos) {
        int index = atoi(text.c_str());
        if (index >= 0 && (size_t) index < choices.size()) return index;
    }
    return -1;
}

bool fillValue(const Value &val, epicsData &edata)
{
    Value valueField = val["value"];
    if (!valueField.valid()) return false;

    if (isEnumValue(val)) {
        fillEnum(val, edata);
    } else if (valueField.type().isarray()) {
        fillScalarArray(valueField, edata);
    } else {
        fillScalar(valueField, edata);
    }

    fillAlarm(val, edata);
    edata.monitorCount++;
    return true;
}

void fillLimitsFromDisplayControl(const Value &val, epicsData &edata)
{
    double limitLow = 0.0, limitHigh = 0.0, controlLow = 0.0, controlHigh = 0.0;
    double alarmLow = 0.0, alarmHigh = 0.0, warnLow = 0.0, warnHigh = 0.0;
    int32_t precision = 0;
    std::string units;

    val["display.limitLow"].as<double>(limitLow);
    val["display.limitHigh"].as<double>(limitHigh);
    val["display.precision"].as<int32_t>(precision);
    val["display.units"].as<std::string>(units);
    val["control.limitLow"].as<double>(controlLow);
    val["control.limitHigh"].as<double>(controlHigh);
    // QSRV maps LOLO/LOW/HIGH/HIHI onto valueAlarm (see pva2pva pvif.cpp)
    val["valueAlarm.lowAlarmLimit"].as<double>(alarmLow);
    val["valueAlarm.lowWarningLimit"].as<double>(warnLow);
    val["valueAlarm.highWarningLimit"].as<double>(warnHigh);
    val["valueAlarm.highAlarmLimit"].as<double>(alarmHigh);

    edata.lower_disp_limit = limitLow;
    edata.upper_disp_limit = limitHigh;
    edata.lower_ctrl_limit = controlLow;
    edata.upper_ctrl_limit = controlHigh;
    edata.lower_alarm_limit = alarmLow;
    edata.lower_warning_limit = warnLow;
    edata.upper_warning_limit = warnHigh;
    edata.upper_alarm_limit = alarmHigh;
    edata.precision = (short) precision;

    int len = int(units.size());
    if (len > caqtdm_string_t_length - 1) len = caqtdm_string_t_length - 1;
    memcpy(edata.units, units.data(), (size_t) len);
    edata.units[len] = '\0';

    edata.accessR = 1;
    edata.accessW = 1;
}

} // namespace pvxsValueMapping
