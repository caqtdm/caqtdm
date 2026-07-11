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
#include "tst_pvxsValueMapping.h"

#include <cstdlib>
#include <cstring>

#include <db_access.h>

#include <pvxs/nt.h>

#include "pvxsValueMapping.h"

using namespace pvxs;

namespace {

epicsData freshEdata()
{
    epicsData edata;
    memset(&edata, 0, sizeof(edata));
    return edata;
}

void freeEdata(epicsData &edata)
{
    if (edata.dataB) {
        free(edata.dataB);
        edata.dataB = nullptr;
    }
}

}

void TestPvxsValueMapping::scalarDouble()
{
    Value val = nt::NTScalar{TypeCode::Float64}.create();
    val["value"] = 3.14;
    val["alarm.severity"] = 0;

    epicsData edata = freshEdata();
    QVERIFY(pvxsValueMapping::fillValue(val, edata));

    QCOMPARE(edata.fieldtype, (short) DBF_DOUBLE);
    QCOMPARE(edata.rvalue, 3.14);
    QCOMPARE(edata.valueCount, 1);
    QCOMPARE(edata.severity, (short) 0);
    QCOMPARE(edata.status, (short) 0);

    freeEdata(edata);
}

void TestPvxsValueMapping::scalarBool()
{
    Value val = nt::NTScalar{TypeCode::Bool}.create();
    val["value"] = true;

    epicsData edata = freshEdata();
    QVERIFY(pvxsValueMapping::fillValue(val, edata));

    QCOMPARE(edata.fieldtype, (short) DBF_LONG);
    QCOMPARE(edata.ivalue, 1L);

    freeEdata(edata);
}

void TestPvxsValueMapping::scalarString()
{
    Value val = nt::NTScalar{TypeCode::String}.create();
    val["value"] = std::string("hello world");

    epicsData edata = freshEdata();
    QVERIFY(pvxsValueMapping::fillValue(val, edata));

    QCOMPARE(edata.fieldtype, (short) DBF_STRING);
    QCOMPARE(edata.valueCount, 1);
    QCOMPARE(QString::fromLatin1(static_cast<char *>(edata.dataB)), QString("hello world"));

    freeEdata(edata);
}

void TestPvxsValueMapping::scalarArrayDouble()
{
    Value val = nt::NTScalar{TypeCode::Float64A}.create();
    val["value"] = shared_array<double>({1.0, 2.0, 3.0, 4.0}).freeze();

    epicsData edata = freshEdata();
    QVERIFY(pvxsValueMapping::fillValue(val, edata));

    QCOMPARE(edata.fieldtype, (short) DBF_DOUBLE);
    QCOMPARE(edata.valueCount, 4);
    double *values = static_cast<double *>(edata.dataB);
    QCOMPARE(values[0], 1.0);
    QCOMPARE(values[3], 4.0);

    freeEdata(edata);
}

void TestPvxsValueMapping::scalarArrayInt()
{
    Value val = nt::NTScalar{TypeCode::Int32A}.create();
    val["value"] = shared_array<int32_t>({10, 20, 30}).freeze();

    epicsData edata = freshEdata();
    QVERIFY(pvxsValueMapping::fillValue(val, edata));

    QCOMPARE(edata.fieldtype, (short) DBF_LONG);
    QCOMPARE(edata.valueCount, 3);
    int32_t *values = static_cast<int32_t *>(edata.dataB);
    QCOMPARE(values[0], 10);
    QCOMPARE(values[2], 30);

    freeEdata(edata);
}

void TestPvxsValueMapping::enumValue()
{
    Value val = nt::NTEnum{}.create();
    val["value.index"] = 1;
    val["value.choices"] = shared_array<std::string>({"Off", "On"}).freeze();

    QVERIFY(pvxsValueMapping::isEnumValue(val));

    epicsData edata = freshEdata();
    QVERIFY(pvxsValueMapping::fillValue(val, edata));

    QCOMPARE(edata.fieldtype, (short) DBF_ENUM);
    QCOMPARE(edata.ivalue, 1L);

    // choices must be packed into dataB, ESC-separated, for caChoice/caMenu
    QCOMPARE(edata.enumCount, 2);
    QVERIFY(edata.dataB != nullptr);
    QCOMPARE(QString::fromLatin1(static_cast<char *>(edata.dataB)), QString("Off\033On"));

    std::vector<std::string> choices = pvxsValueMapping::enumChoices(val);
    QCOMPARE((int) choices.size(), 2);
    QCOMPARE(QString::fromStdString(choices[0]), QString("Off"));
    QCOMPARE(QString::fromStdString(choices[1]), QString("On"));

    freeEdata(edata);
}

void TestPvxsValueMapping::enumIndexLookup()
{
    Value val = nt::NTEnum{}.create();
    val["value.choices"] = shared_array<std::string>({"Off", "On"}).freeze();

    QCOMPARE(pvxsValueMapping::enumIndexFor(val, "Off"), 0);
    QCOMPARE(pvxsValueMapping::enumIndexFor(val, "On"), 1);
    QCOMPARE(pvxsValueMapping::enumIndexFor(val, "1"), 1);      // numeric fallback
    QCOMPARE(pvxsValueMapping::enumIndexFor(val, "banana"), -1);
    QCOMPARE(pvxsValueMapping::enumIndexFor(val, "7"), -1);     // out of range
    QCOMPARE(pvxsValueMapping::enumIndexFor(val, ""), -1);
}

void TestPvxsValueMapping::alarmSeverity()
{
    Value val = nt::NTScalar{TypeCode::Float64}.create();
    val["value"] = 1.0;
    val["alarm.severity"] = 2; // MAJOR

    epicsData edata = freshEdata();
    QVERIFY(pvxsValueMapping::fillValue(val, edata));

    QCOMPARE(edata.severity, (short) 2);
    QVERIFY(edata.status != 0);

    freeEdata(edata);
}

void TestPvxsValueMapping::displayControlLimits()
{
    Value val = nt::NTScalar{TypeCode::Float64, true, true, false, true}.create();
    val["display.limitLow"] = -10.0;
    val["display.limitHigh"] = 10.0;
    val["display.precision"] = 3;
    val["display.units"] = std::string("mA");
    val["control.limitLow"] = -5.0;
    val["control.limitHigh"] = 5.0;

    epicsData edata = freshEdata();
    pvxsValueMapping::fillLimitsFromDisplayControl(val, edata);

    QCOMPARE(edata.lower_disp_limit, -10.0);
    QCOMPARE(edata.upper_disp_limit, 10.0);
    QCOMPARE(edata.precision, (short) 3);
    QCOMPARE(QString::fromLatin1(edata.units), QString("mA"));
    QCOMPARE(edata.lower_ctrl_limit, -5.0);
    QCOMPARE(edata.upper_ctrl_limit, 5.0);
    QCOMPARE(edata.accessR, 1);
    QCOMPARE(edata.accessW, 1);

    freeEdata(edata);
}

void TestPvxsValueMapping::missingValueField()
{
    Value val; // default-constructed, invalid, has no "value" field at all
    epicsData edata = freshEdata();
    QVERIFY(!pvxsValueMapping::fillValue(val, edata));
    freeEdata(edata);
}
