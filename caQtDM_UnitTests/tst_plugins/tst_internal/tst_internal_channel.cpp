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
#include "tst_internal_channel.h"
#include "QtTest/qtestcase.h"

#include <string.h>

// fresh zero initialized knobData whose dataB is freed by freeKnobData below
static knobData makeKnobData()
{
    knobData kData;
    memset(&kData, 0, sizeof(knobData));
    return kData;
}

static void freeKnobData(knobData *kData)
{
    if(kData->edata.dataB != (void *) Q_NULLPTR) {
        free(kData->edata.dataB);
        kData->edata.dataB = (void *) Q_NULLPTR;
        kData->edata.dataSize = 0;
    }
}

void TestInternalChannel::baseNameAndJsonPartWork()
{
    QCOMPARE(InternalChannel::baseName("RAMP"), QString("RAMP"));
    QCOMPARE(InternalChannel::jsonPart("RAMP"), QString());

    QCOMPARE(InternalChannel::baseName(R"(RAMP.{"type":"double"})"), QString("RAMP"));
    QCOMPARE(InternalChannel::jsonPart(R"(RAMP.{"type":"double"})"), QString(R"({"type":"double"})"));

    // known record fields are stripped, unknown extensions are invalid
    QCOMPARE(InternalChannel::baseName("DEVICE.VAL"), QString("DEVICE"));
    QCOMPARE(InternalChannel::baseName("DEVICE.SUBNAME"), QString());
}

void TestInternalChannel::fieldSyntaxWorks()
{
    QString base;
    InternalChannel::Field field;

    QCOMPARE(InternalChannel::splitField("RAMP", &base, &field), true);
    QCOMPARE(base, QString("RAMP"));
    QCOMPARE(field, InternalChannel::FieldVal);

    QCOMPARE(InternalChannel::splitField("RAMP.VAL", &base, &field), true);
    QCOMPARE(base, QString("RAMP"));
    QCOMPARE(field, InternalChannel::FieldVal);

    // only the real EPICS field names, case insensitive
    QCOMPARE(InternalChannel::splitField("RAMP.SEVR", &base, &field), true);
    QCOMPARE(field, InternalChannel::FieldSevr);
    QCOMPARE(InternalChannel::splitField("RAMP.sevr", &base, &field), true);
    QCOMPARE(field, InternalChannel::FieldSevr);
    QCOMPARE(InternalChannel::splitField("RAMP.stat", &base, &field), true);
    QCOMPARE(field, InternalChannel::FieldStat);
    QCOMPARE(InternalChannel::splitField("RAMP.hihi", &base, &field), true);
    QCOMPARE(field, InternalChannel::FieldHihi);
    QCOMPARE(InternalChannel::splitField("RAMP.low", &base, &field), true);
    QCOMPARE(field, InternalChannel::FieldLow);
    QCOMPARE(InternalChannel::splitField("RAMP.drvh", &base, &field), true);
    QCOMPARE(field, InternalChannel::FieldDrvh);
    QCOMPARE(InternalChannel::splitField("RAMP.hopr", &base, &field), true);
    QCOMPARE(field, InternalChannel::FieldHopr);
    QCOMPARE(InternalChannel::splitField("RAMP.lopr", &base, &field), true);
    QCOMPARE(field, InternalChannel::FieldLopr);
    QCOMPARE(InternalChannel::splitField("RAMP.egu", &base, &field), true);
    QCOMPARE(field, InternalChannel::FieldEgu);
    QCOMPARE(InternalChannel::splitField("RAMP.NORD", &base, &field), true);
    QCOMPARE(base, QString("RAMP"));
    QCOMPARE(field, InternalChannel::FieldNord);

    // unknown extensions (also the long forms) are not handled at all
    QCOMPARE(InternalChannel::splitField("RAMP.SUB", &base, &field), false);
    QCOMPARE(InternalChannel::splitField("RAMP.severity", &base, &field), false);
    QCOMPARE(InternalChannel::splitField("RAMP.units", &base, &field), false);
    QCOMPARE(InternalChannel::splitField("A.B.SEVR", &base, &field), false);

    // a json suffix is stripped before the field detection
    QCOMPARE(InternalChannel::splitField(R"(RAMP.LOW.{"dbnd":{"abs":1}})", &base, &field), true);
    QCOMPARE(base, QString("RAMP"));
    QCOMPARE(field, InternalChannel::FieldLow);
}

void TestInternalChannel::splitFieldStripsSchemePrefix()
{
    QString base;
    InternalChannel::Field field;

    // CaQtDM_Lib::addMonitor() normally strips the scheme prefix before it
    // ever reaches the plugin, but splitField() must handle it too when the
    // plugin is called directly (e.g. pvSetValue from a test or future code)
    QCOMPARE(InternalChannel::splitField("internal://RAMP", &base, &field), true);
    QCOMPARE(base, QString("RAMP"));
    QCOMPARE(field, InternalChannel::FieldVal);

    QCOMPARE(InternalChannel::splitField("internal://RAMP.LOW", &base, &field), true);
    QCOMPARE(base, QString("RAMP"));
    QCOMPARE(field, InternalChannel::FieldLow);

    // any scheme is stripped, not just "internal" -- future aliases like
    // softPV:// or intern:// that get redirected here work the same way
    QCOMPARE(InternalChannel::splitField("softPV://RAMP", &base, &field), true);
    QCOMPARE(base, QString("RAMP"));
    QCOMPARE(InternalChannel::splitField("intern://RAMP.HOPR", &base, &field), true);
    QCOMPARE(base, QString("RAMP"));
    QCOMPARE(field, InternalChannel::FieldHopr);

    // baseName()/jsonPart() go through the same path (jsonPart() already
    // works regardless of a prefix, kept here as a regression check)
    QCOMPARE(InternalChannel::baseName("internal://RAMP"), QString("RAMP"));
    QCOMPARE(InternalChannel::baseName(R"(internal://RAMP.{"type":"double"})"), QString("RAMP"));
    QCOMPARE(InternalChannel::jsonPart(R"(internal://RAMP.{"type":"double"})"), QString(R"({"type":"double"})"));

    // unprefixed still works exactly as before
    QCOMPARE(InternalChannel::splitField("RAMP.LOW", &base, &field), true);
    QCOMPARE(base, QString("RAMP"));
    QCOMPARE(field, InternalChannel::FieldLow);
}

void TestInternalChannel::counterStopsWhenInvalidOrDisconnected()
{
    QString error;
    InternalChannel channel;
    QVERIFY2(channel.configure(R"({"type":"long","mode":"counter","val":0,"step":1})", &error),
             qPrintable(error));

    channel.tick();
    QCOMPARE(channel.currentValue(), 1.0);

    // an invalid or disconnected channel does not deliver new values
    channel.setFieldValue(InternalChannel::FieldSevr, 0.0, 0, "INVALID");
    channel.tick();
    QCOMPARE(channel.currentValue(), 1.0);

    channel.setFieldValue(InternalChannel::FieldSevr, 0.0, 0, "NOTCONNECTED");
    channel.tick();
    QCOMPARE(channel.currentValue(), 1.0);

    // a value write re-evaluates the alarms and the counter runs again
    channel.setValue(0.0, 5, QString());
    QCOMPARE(channel.severity, (short) 0);
    channel.tick();
    QCOMPARE(channel.currentValue(), 6.0);

    // MINOR/MAJOR alarms do not stop the counter; the next tick re-evaluates
    channel.setFieldValue(InternalChannel::FieldSevr, 0.0, 0, "MAJOR");
    channel.tick();
    QCOMPARE(channel.currentValue(), 7.0);
    QCOMPARE(channel.severity, (short) 0);
}

void TestInternalChannel::fieldWritesAndForcingWork()
{
    QString error;
    InternalChannel channel;
    QVERIFY2(channel.configure(R"({"type":"double","val":50,"low":20,"lolo":10,"high":80,"hihi":90})", &error),
             qPrintable(error));
    QCOMPARE(channel.severity, (short) 0);

    // moving a threshold below the value raises the alarm immediately
    channel.setFieldValue(InternalChannel::FieldHigh, 40.0, 0, QString());
    QCOMPARE(channel.high.value, 40.0);
    QCOMPARE(channel.severity, (short) 1);
    channel.setFieldValue(InternalChannel::FieldHihi, 45.0, 0, QString());
    QCOMPARE(channel.severity, (short) 2);

    // a written severity stays until the next value change
    channel.setFieldValue(InternalChannel::FieldSevr, 0.0, 0, "NO_ALARM");
    QCOMPARE(channel.severity, (short) 0);
    channel.setFieldValue(InternalChannel::FieldSevr, 0.0, 3, QString());
    QCOMPARE(channel.severity, (short) 3);

    // NOTCONNECTED (by name, enum index 4 or code 99) marks the channel disconnected
    channel.setFieldValue(InternalChannel::FieldSevr, 0.0, 0, "NOTCONNECTED");
    QCOMPARE(channel.severity, (short) 99);
    knobData kData = makeKnobData();
    channel.fillKnobData(&kData);
    QCOMPARE(kData.edata.connected, (int) false);
    freeKnobData(&kData);

    // a value write re-evaluates the alarms from the limits
    channel.setValue(50.0, 0, QString());
    QCOMPARE(channel.severity, (short) 2); // 50 >= hihi 45

    // a written status stays until the next value change
    channel.setFieldValue(InternalChannel::FieldStat, 0.0, 0, "COMM");
    QCOMPARE(channel.status, (short) 9);
    channel.setValue(50.0, 0, QString());
    QCOMPARE(channel.status, (short) 3); // computed again: HIHI

    // the other writable fields
    channel.setFieldValue(InternalChannel::FieldEgu, 0.0, 0, "mA");
    QCOMPARE(channel.units, QString("mA"));
    channel.setFieldValue(InternalChannel::FieldPrec, 4.0, 0, QString());
    QCOMPARE(channel.precision, (short) 4);
    channel.setFieldValue(InternalChannel::FieldDrvl, -5.0, 0, QString());
    QCOMPARE(channel.drvl.value, -5.0);
    QCOMPARE(channel.drvl.defined, true);
    channel.setFieldValue(InternalChannel::FieldHopr, 40.0, 0, QString());
    QCOMPARE(channel.hopr.value, 40.0);
    QCOMPARE(channel.hopr.defined, true);
    channel.setFieldValue(InternalChannel::FieldLopr, -1.0, 0, QString());
    QCOMPARE(channel.lopr.value, -1.0);
    QCOMPARE(channel.lopr.defined, true);

    // NORD stays limited by NELM, NELM is read only
    InternalChannel wave;
    QVERIFY(wave.configure(R"({"type":"double","nelm":8})", &error));
    wave.setFieldValue(InternalChannel::FieldNord, 0.0, 3, QString());
    QCOMPARE(wave.nord, 3);
    wave.setFieldValue(InternalChannel::FieldNord, 0.0, 99, QString());
    QCOMPARE(wave.nord, 8);
    wave.setFieldValue(InternalChannel::FieldNelm, 0.0, 99, QString());
    QCOMPARE(wave.nelm, 8);
}

void TestInternalChannel::fillKnobDataFieldWorks()
{
    QString error;
    InternalChannel channel;
    QVERIFY2(channel.configure(R"({"type":"double","val":50,"low":20,"lolo":10,"high":80,"hihi":90,
                                   "hopr":95,"lopr":5,"units":"V","prec":2})", &error),
             qPrintable(error));

    // limit fields as plain doubles
    knobData kData = makeKnobData();
    channel.fillKnobDataField(&kData, InternalChannel::FieldLow);
    QCOMPARE(kData.edata.fieldtype, (short) caDOUBLE);
    QCOMPARE(kData.edata.rvalue, 20.0);
    channel.fillKnobDataField(&kData, InternalChannel::FieldHihi);
    QCOMPARE(kData.edata.rvalue, 90.0);
    channel.fillKnobDataField(&kData, InternalChannel::FieldHopr);
    QCOMPARE(kData.edata.rvalue, 95.0);
    channel.fillKnobDataField(&kData, InternalChannel::FieldLopr);
    QCOMPARE(kData.edata.rvalue, 5.0);
    channel.fillKnobDataField(&kData, InternalChannel::FieldPrec);
    QCOMPARE(kData.edata.rvalue, 2.0);

    // SEVR as enum with the severity states
    channel.fillKnobDataField(&kData, InternalChannel::FieldSevr);
    QCOMPARE(kData.edata.fieldtype, (short) caENUM);
    QCOMPARE(kData.edata.enumCount, 5);
    QCOMPARE(kData.edata.ivalue, 0L);
    QCOMPARE(QString((char *) kData.edata.dataB),
             QString("NO_ALARM\033MINOR\033MAJOR\033INVALID\033NOTCONNECTED"));

    // a written NOTCONNECTED maps to the last enum state,
    // a value write re-evaluates the state
    channel.setFieldValue(InternalChannel::FieldSevr, 0.0, 0, "NOTCONNECTED");
    channel.fillKnobDataField(&kData, InternalChannel::FieldSevr);
    QCOMPARE(kData.edata.ivalue, 4L);
    channel.setValue(50.0, 0, QString());
    channel.fillKnobDataField(&kData, InternalChannel::FieldSevr);
    QCOMPARE(kData.edata.ivalue, 0L);

    // STAT as enum with the epicsAlarm status names
    channel.fillKnobDataField(&kData, InternalChannel::FieldStat);
    QCOMPARE(kData.edata.fieldtype, (short) caENUM);
    QCOMPARE(kData.edata.enumCount, 22);
    QCOMPARE(kData.edata.ivalue, 0L);

    // EGU as string, NELM/NORD as long
    channel.fillKnobDataField(&kData, InternalChannel::FieldEgu);
    QCOMPARE(kData.edata.fieldtype, (short) caSTRING);
    QCOMPARE(QString((char *) kData.edata.dataB), QString("V"));
    channel.fillKnobDataField(&kData, InternalChannel::FieldNelm);
    QCOMPARE(kData.edata.fieldtype, (short) caLONG);
    QCOMPARE(kData.edata.ivalue, 1L);
    freeKnobData(&kData);

    // VAL falls through to the normal channel data
    knobData kVal = makeKnobData();
    channel.fillKnobDataField(&kVal, InternalChannel::FieldVal);
    QCOMPARE(kVal.edata.fieldtype, (short) caDOUBLE);
    QCOMPARE(kVal.edata.rvalue, 50.0);
    freeKnobData(&kVal);
}

void TestInternalChannel::configureParsesAllFields()
{
    InternalChannel channel;
    QString error;
    bool ok = channel.configure(R"({"type":"float","mode":"counter","val":5,"step":2.5,
                                    "period":200,"drvl":1,"drvh":9,"hopr":8,"lopr":2,"overflow":false,"nelm":4,
                                    "low":2,"lolo":1.5,"high":7,"hihi":8.5,
                                    "units":"mA","prec":3})", &error);
    QVERIFY2(ok, qPrintable(error));
    QVERIFY(channel.isConfigured());
    QCOMPARE(channel.fieldtype, (short) caFLOAT);
    QCOMPARE(channel.mode, InternalChannel::Counter);
    QCOMPARE(channel.val, 5.0);
    QCOMPARE(channel.step, 2.5);
    QCOMPARE(channel.periodMs, 200);
    QCOMPARE(channel.drvl.value, 1.0);
    QCOMPARE(channel.drvh.value, 9.0);
    QCOMPARE(channel.drvl.defined, true);
    QCOMPARE(channel.drvh.defined, true);
    QCOMPARE(channel.hopr.value, 8.0);
    QCOMPARE(channel.lopr.value, 2.0);
    QCOMPARE(channel.hopr.defined, true);
    QCOMPARE(channel.lopr.defined, true);
    QCOMPARE(channel.low.value, 2.0);
    QCOMPARE(channel.lolo.value, 1.5);
    QCOMPARE(channel.high.value, 7.0);
    QCOMPARE(channel.hihi.value, 8.5);
    QCOMPARE(channel.low.defined, true);
    QCOMPARE(channel.lolo.defined, true);
    QCOMPARE(channel.high.defined, true);
    QCOMPARE(channel.hihi.defined, true);
    QCOMPARE(channel.overflow, false);
    QCOMPARE(channel.nelm, 4);
    QCOMPARE(channel.units, QString("mA"));
    QCOMPARE(channel.precision, (short) 3);
    QCOMPARE(channel.currentValue(), 5.0);

    // for a string channel "val" carries the text
    InternalChannel stringChannel;
    ok = stringChannel.configure(R"({"type":"string","val":"hello"})", &error);
    QVERIFY2(ok, qPrintable(error));
    QCOMPARE(stringChannel.text, QString("hello"));

    InternalChannel enumChannel;
    ok = enumChannel.configure(R"({"type":"enum","enums":["A","B","C"]})", &error);
    QVERIFY2(ok, qPrintable(error));
    QCOMPARE(enumChannel.fieldtype, (short) caENUM);
    QCOMPARE(enumChannel.enums, QStringList() << "A" << "B" << "C");
}

void TestInternalChannel::configureUsesDefaults()
{
    InternalChannel channel;
    QString error;
    QVERIFY2(channel.configure("{}", &error), qPrintable(error));
    QCOMPARE(channel.fieldtype, (short) caDOUBLE);
    QCOMPARE(channel.mode, InternalChannel::Constant);
    QCOMPARE(channel.val, 0.0);
    QCOMPARE(channel.step, 1.0);
    QCOMPARE(channel.periodMs, 1000);
    QCOMPARE(channel.drvl.defined, false);
    QCOMPARE(channel.drvh.defined, false);
    QCOMPARE(channel.hopr.defined, false);
    QCOMPARE(channel.lopr.defined, false);
    QCOMPARE(channel.low.defined, false);
    QCOMPARE(channel.lolo.defined, false);
    QCOMPARE(channel.high.defined, false);
    QCOMPARE(channel.hihi.defined, false);
    QCOMPARE(channel.overflow, true);
    QCOMPARE(channel.nelm, 1);
    QCOMPARE(channel.precision, (short) 2);

    // an enum without explicit states gets a default state list
    InternalChannel enumChannel;
    QVERIFY(enumChannel.configure(R"({"type":"enum"})", &error));
    QCOMPARE(enumChannel.enums, QStringList() << "OFF" << "ON");

    // integer types default to precision 0
    InternalChannel intChannel;
    QVERIFY(intChannel.configure(R"({"type":"int"})", &error));
    QCOMPARE(intChannel.precision, (short) 0);
}

void TestInternalChannel::configureRejectsInvalidInput()
{
    InternalChannel channel;
    QString error;

    QCOMPARE(channel.configure("{kaputt}", &error), false);
    QVERIFY(!error.isEmpty());
    QCOMPARE(channel.isConfigured(), false);

    QCOMPARE(channel.configure(R"({"type":"quaternion"})", &error), false);
    QCOMPARE(channel.isConfigured(), false);

    QCOMPARE(channel.configure(R"({"mode":"randomwalk"})", &error), false);
    QCOMPARE(channel.isConfigured(), false);

    QCOMPARE(channel.configure(R"([1,2,3])", &error), false);
    QCOMPARE(channel.isConfigured(), false);
}

void TestInternalChannel::counterTicksAndWraps()
{
    InternalChannel channel;
    QString error;
    QVERIFY2(channel.configure(R"({"mode":"counter","val":8,"step":1,"drvl":0,"drvh":10,"overflow":true})", &error),
             qPrintable(error));

    channel.tick();
    QCOMPARE(channel.currentValue(), 9.0);
    channel.tick();
    QCOMPARE(channel.currentValue(), 10.0);
    channel.tick(); // beyond drvh -> wraps to drvl
    QCOMPARE(channel.currentValue(), 0.0);

    // without overflow the counter saturates at drvh
    InternalChannel saturating;
    QVERIFY(saturating.configure(R"({"mode":"counter","val":9,"step":2,"drvl":0,"drvh":10,"overflow":false})", &error));
    saturating.tick();
    QCOMPARE(saturating.currentValue(), 10.0);
    saturating.tick();
    QCOMPARE(saturating.currentValue(), 10.0);

    // negative step wraps at the lower drive limit
    InternalChannel backwards;
    QVERIFY(backwards.configure(R"({"mode":"counter","val":1,"step":-1,"drvl":0,"drvh":5,"overflow":true})", &error));
    backwards.tick();
    QCOMPARE(backwards.currentValue(), 0.0);
    backwards.tick();
    QCOMPARE(backwards.currentValue(), 5.0);

    // an enum cycles through its states without explicit limits
    InternalChannel enumChannel;
    QVERIFY(enumChannel.configure(R"({"type":"enum","mode":"counter","enums":["A","B","C"]})", &error));
    QCOMPARE(enumChannel.currentValue(), 0.0);
    enumChannel.tick();
    enumChannel.tick();
    QCOMPARE(enumChannel.currentValue(), 2.0);
    enumChannel.tick();
    QCOMPARE(enumChannel.currentValue(), 0.0);

    // constant mode never changes the value
    InternalChannel constant;
    QVERIFY(constant.configure(R"({"val":7})", &error));
    constant.tick();
    QCOMPARE(constant.currentValue(), 7.0);
}

void TestInternalChannel::nativeTypesWrapLikeEpics()
{
    QString error;

    // a caINT counter is stored as dbr_short_t and wraps at the int16 range
    InternalChannel int16Channel;
    QVERIFY(int16Channel.configure(R"({"type":"int","mode":"counter","val":32767,"step":1})", &error));
    QCOMPARE(int16Channel.native.int16Value, (qint16) 32767);
    int16Channel.tick();
    QCOMPARE(int16Channel.native.int16Value, (qint16) -32768);
    QCOMPARE(int16Channel.currentValue(), -32768.0);

    // a caCHAR counter is stored as dbr_char_t (unsigned) and wraps at 255
    InternalChannel charChannel;
    QVERIFY(charChannel.configure(R"({"type":"char","mode":"counter","val":255,"step":1})", &error));
    charChannel.tick();
    QCOMPARE(charChannel.native.charValue, (quint8) 0);

    // a caENUM index is stored as dbr_enum_t (unsigned 16 bit)
    InternalChannel enumChannel;
    QVERIFY(enumChannel.configure(R"({"type":"enum","enums":["A","B"]})", &error));
    enumChannel.setValue(0.0, 1, QString());
    QCOMPARE(enumChannel.native.enumValue, (quint16) 1);

    // a caFLOAT value is stored as dbr_float_t and loses double precision
    InternalChannel floatChannel;
    QVERIFY(floatChannel.configure(R"({"type":"float"})", &error));
    floatChannel.setValue(0.1, 0, QString());
    QCOMPARE(floatChannel.native.floatValue, 0.1f);
    QVERIFY(floatChannel.currentValue() != 0.1); // float(0.1) as double is not 0.1

    // a caLONG value is stored as dbr_long_t (int32)
    InternalChannel longChannel;
    QVERIFY(longChannel.configure(R"({"type":"long","val":2147483647})", &error));
    QCOMPARE(longChannel.native.int32Value, (qint32) 2147483647);
}

void TestInternalChannel::advanceRespectsPeriod()
{
    InternalChannel channel;
    QString error;
    QVERIFY2(channel.configure(R"({"mode":"counter","val":0,"step":1,"period":250})", &error),
             qPrintable(error));

    QCOMPARE(channel.advance(100), false);
    QCOMPARE(channel.advance(100), false);
    QCOMPARE(channel.advance(100), true); // 300 ms accumulated -> one tick
    QCOMPARE(channel.currentValue(), 1.0);

    // remaining 50 ms are kept, a big step executes several ticks
    QCOMPARE(channel.advance(700), true); // 750 ms -> three ticks
    QCOMPARE(channel.currentValue(), 4.0);

    // constant mode never reports a change
    InternalChannel constant;
    QVERIFY(constant.configure("{}", &error));
    QCOMPARE(constant.advance(10000), false);
}

void TestInternalChannel::regexGeneratorWorks()
{
    QString error;

    // character class with quantifier: STATE-00 .. STATE-99, wrapping around
    InternalChannel channel;
    QVERIFY2(channel.configure(R"({"type":"string","mode":"counter","regex":"STATE-[0-9]{2}"})", &error),
             qPrintable(error));
    QCOMPARE(channel.combinations(), Q_INT64_C(100));
    QCOMPARE(channel.generatedString(0), QString("STATE-00"));
    QCOMPARE(channel.generatedString(7), QString("STATE-07"));
    QCOMPARE(channel.generatedString(42), QString("STATE-42"));
    QCOMPARE(channel.generatedString(99), QString("STATE-99"));
    QCOMPARE(channel.generatedString(100), QString("STATE-00")); // wraps

    // the counter advances the generated string
    knobData kData = makeKnobData();
    channel.fillKnobData(&kData);
    QCOMPARE(QString((char *) kData.edata.dataB), QString("STATE-00"));
    channel.tick();
    channel.fillKnobData(&kData);
    QCOMPARE(QString((char *) kData.edata.dataB), QString("STATE-01"));
    freeKnobData(&kData);

    // starting index via val, wrap at the end of the combinations
    InternalChannel startAt;
    QVERIFY(startAt.configure(R"({"type":"string","mode":"counter","val":99,"regex":"STATE-[0-9]{2}"})", &error));
    QCOMPARE(startAt.generatedString((qint64) startAt.currentValue()), QString("STATE-99"));
    startAt.tick();
    QCOMPARE(startAt.generatedString((qint64) startAt.currentValue()), QString("STATE-00"));

    // alternation group and character ranges, last segment changes fastest
    InternalChannel combined;
    QVERIFY(combined.configure(R"({"type":"string","mode":"counter","regex":"(ON|OFF)-[a-c]"})", &error));
    QCOMPARE(combined.combinations(), Q_INT64_C(6));
    QCOMPARE(combined.generatedString(0), QString("ON-a"));
    QCOMPARE(combined.generatedString(2), QString("ON-c"));
    QCOMPARE(combined.generatedString(3), QString("OFF-a"));
    QCOMPARE(combined.generatedString(5), QString("OFF-c"));

    // escaped characters stay literal
    InternalChannel escaped;
    QVERIFY(escaped.configure(R"({"type":"string","mode":"counter","regex":"V\\[[0-1]\\]"})", &error));
    QCOMPARE(escaped.combinations(), Q_INT64_C(2));
    QCOMPARE(escaped.generatedString(1), QString("V[1]"));

    // writing positions a regex channel by index
    InternalChannel writable;
    QVERIFY(writable.configure(R"({"type":"string","mode":"counter","regex":"MSG-[0-9]"})", &error));
    writable.setValue(0.0, 7, QString());
    QCOMPARE(writable.generatedString((qint64) writable.currentValue()), QString("MSG-7"));

    // invalid patterns are rejected
    InternalChannel broken;
    QCOMPARE(broken.configure(R"({"type":"string","mode":"counter","regex":"[0-9"})", &error), false);
    QVERIFY(!error.isEmpty());
    QCOMPARE(broken.configure(R"({"type":"string","mode":"counter","regex":"[9-0]"})", &error), false);
    QCOMPARE(broken.configure(R"({"type":"string","mode":"counter","regex":"A{x}"})", &error), true); // quantifier only after class/group -> literal
}

void TestInternalChannel::regexProcessesWrittenStrings()
{
    QString error;

    // like the macro modification: the written string is matched by the regex
    // and replaced by val (capture groups allowed), the result becomes VAL
    InternalChannel channel;
    QVERIFY2(channel.configure(R"lim({"type":"string","val":"A\\1","regex":"([1,2]+)([3])"})lim", &error),
             qPrintable(error));
    channel.setValue(0.0, 0, "12345");
    QCOMPARE(channel.text, QString("A1245"));

    // full replacement
    InternalChannel full;
    QVERIFY(full.configure(R"({"type":"string","val":"HUI","regex":"\\S+"})", &error));
    full.setValue(0.0, 0, "abc");
    QCOMPARE(full.text, QString("HUI"));

    // no match keeps the written string
    InternalChannel numbers;
    QVERIFY(numbers.configure(R"({"type":"string","val":"N","regex":"^[0-9]+$"})", &error));
    numbers.setValue(0.0, 0, "xyz");
    QCOMPARE(numbers.text, QString("xyz"));

    // macro constant syntax as input (parser stress with the treatMacro examples)
    InternalChannel stress;
    QVERIFY2(stress.configure(R"lim({"type":"string","val":"<\\1>","regex":"\\$\\(([A-Z0-9]+)=[^)]*\\)"})lim", &error),
             qPrintable(error));
    stress.setValue(0.0, 0, "$(TEST5=This)");
    QCOMPARE(stress.text, QString("<TEST5>"));

    // nested constants, json-like macros and $$ pass through untouched when the pattern does not match
    InternalChannel untouched;
    QVERIFY(untouched.configure(R"({"type":"string","val":"X","regex":"NEVERMATCHES"})", &error));
    untouched.setValue(0.0, 0, R"lim($(TESTLEER=1$(TEST=2$(TEST3=3$(TEST4)3)2)1))lim");
    QCOMPARE(untouched.text, QString(R"lim($(TESTLEER=1$(TEST=2$(TEST3=3$(TEST4)3)2)1))lim"));
    untouched.setValue(0.0, 0, R"lim($(TEST{"regex":"\S+","value":"X"}))lim");
    QCOMPARE(untouched.text, QString(R"lim($(TEST{"regex":"\S+","value":"X"}))lim"));
    untouched.setValue(0.0, 0, "xyz$$876_A1245______");
    QCOMPARE(untouched.text, QString("xyz$$876_A1245______"));

    // invalid full-syntax pattern is rejected in constant mode
    InternalChannel broken;
    QCOMPARE(broken.configure(R"({"type":"string","val":"X","regex":"([1,2]+"})", &error), false);
    QVERIFY(!error.isEmpty());
}

void TestInternalChannel::alarmLimitsWork()
{
    QString error;
    InternalChannel channel;
    QVERIFY2(channel.configure(R"({"type":"double","mode":"counter","val":50,"step":10,
                                   "drvl":0,"drvh":100,
                                   "low":20,"lolo":10,"high":80,"hihi":90})", &error),
             qPrintable(error));

    knobData kData = makeKnobData();

    // alarm limits are propagated like an EPICS record
    channel.fillKnobData(&kData);
    QCOMPARE(kData.edata.lower_warning_limit, 20.0);
    QCOMPARE(kData.edata.lower_alarm_limit, 10.0);
    QCOMPARE(kData.edata.upper_warning_limit, 80.0);
    QCOMPARE(kData.edata.upper_alarm_limit, 90.0);

    // value 50: no alarm
    QCOMPARE(kData.edata.severity, (short) 0);
    QCOMPARE(kData.edata.status, (short) 0);

    // value 80: HIGH -> minor alarm (status 4, epicsAlarm.h)
    channel.setValue(80.0, 0, QString());
    channel.fillKnobData(&kData);
    QCOMPARE(kData.edata.severity, (short) 1);
    QCOMPARE(kData.edata.status, (short) 4);

    // value 90: HIHI -> major alarm (status 3)
    channel.setValue(90.0, 0, QString());
    channel.fillKnobData(&kData);
    QCOMPARE(kData.edata.severity, (short) 2);
    QCOMPARE(kData.edata.status, (short) 3);

    // value 20: LOW -> minor alarm (status 6)
    channel.setValue(20.0, 0, QString());
    channel.fillKnobData(&kData);
    QCOMPARE(kData.edata.severity, (short) 1);
    QCOMPARE(kData.edata.status, (short) 6);

    // value 10: LOLO -> major alarm (status 5)
    channel.setValue(10.0, 0, QString());
    channel.fillKnobData(&kData);
    QCOMPARE(kData.edata.severity, (short) 2);
    QCOMPARE(kData.edata.status, (short) 5);

    // the counter walks back into the healthy band
    channel.setValue(45.0, 0, QString());
    channel.tick(); // 55
    channel.fillKnobData(&kData);
    QCOMPARE(kData.edata.severity, (short) 0);
    freeKnobData(&kData);

    // without configured alarm limits everything stays at NO_ALARM
    InternalChannel plain;
    QVERIFY(plain.configure(R"({"type":"double","val":1000})", &error));
    knobData kPlain = makeKnobData();
    plain.fillKnobData(&kPlain);
    QCOMPARE(kPlain.edata.severity, (short) 0);
    QCOMPARE(kPlain.edata.status, (short) 0);
    freeKnobData(&kPlain);
}

void TestInternalChannel::fillKnobDataScalarTypesWork()
{
    QString error;

    {
        InternalChannel channel;
        QVERIFY(channel.configure(R"({"type":"double","val":3.5,"drvl":-10,"drvh":10,"units":"V","prec":4})", &error));
        knobData kData = makeKnobData();
        channel.fillKnobData(&kData);
        QCOMPARE(kData.edata.fieldtype, (short) caDOUBLE);
        QCOMPARE(kData.edata.rvalue, 3.5);
        QCOMPARE(kData.edata.ivalue, 3L);
        QCOMPARE(kData.edata.connected, (int) true);
        QCOMPARE(kData.edata.accessW, (int) true);
        QCOMPARE(kData.edata.precision, (short) 4);
        QCOMPARE(QString(kData.edata.units), QString("V"));
        QCOMPARE(kData.edata.lower_disp_limit, -10.0);
        QCOMPARE(kData.edata.upper_disp_limit, 10.0);
        QCOMPARE(kData.edata.lower_ctrl_limit, -10.0);
        QCOMPARE(kData.edata.upper_ctrl_limit, 10.0);
        QCOMPARE(kData.edata.valueCount, 1);
        freeKnobData(&kData);
    }

    {
        InternalChannel channel;
        QVERIFY(channel.configure(R"({"type":"long","val":42})", &error));
        knobData kData = makeKnobData();
        channel.fillKnobData(&kData);
        QCOMPARE(kData.edata.fieldtype, (short) caLONG);
        QCOMPARE(kData.edata.ivalue, 42L);
        QCOMPARE(kData.edata.rvalue, 42.0);
        freeKnobData(&kData);
    }

    {
        InternalChannel channel;
        QVERIFY(channel.configure(R"({"type":"enum","val":1,"enums":["OFF","ON","ERROR"]})", &error));
        knobData kData = makeKnobData();
        channel.fillKnobData(&kData);
        QCOMPARE(kData.edata.fieldtype, (short) caENUM);
        QCOMPARE(kData.edata.ivalue, 1L);
        QCOMPARE(kData.edata.enumCount, 3);
        QCOMPARE(QString((char *) kData.edata.dataB), QString("OFF\033ON\033ERROR"));
        freeKnobData(&kData);
    }

    {
        InternalChannel channel;
        QVERIFY(channel.configure(R"({"type":"string","val":"hello world"})", &error));
        knobData kData = makeKnobData();
        channel.fillKnobData(&kData);
        QCOMPARE(kData.edata.fieldtype, (short) caSTRING);
        QCOMPARE(QString((char *) kData.edata.dataB), QString("hello world"));
        freeKnobData(&kData);
    }

    {
        InternalChannel channel;
        QVERIFY(channel.configure(R"({"type":"char","val":65})", &error));
        knobData kData = makeKnobData();
        channel.fillKnobData(&kData);
        QCOMPARE(kData.edata.fieldtype, (short) caCHAR);
        QCOMPARE(kData.edata.ivalue, 65L);
        QCOMPARE(((char *) kData.edata.dataB)[0], 'A');
        freeKnobData(&kData);
    }
}

void TestInternalChannel::fillKnobDataArraysWork()
{
    QString error;

    {
        InternalChannel channel;
        QVERIFY(channel.configure(R"({"type":"double","mode":"counter","val":10,"step":2,"nelm":5})", &error));
        knobData kData = makeKnobData();
        channel.fillKnobData(&kData);
        QCOMPARE(kData.edata.valueCount, 5);
        QCOMPARE(kData.edata.nelm, 5);
        QCOMPARE(kData.edata.dataSize, (int) (5 * sizeof(double)));
        double *values = (double *) kData.edata.dataB;
        for(int i = 0; i < 5; i++) QCOMPARE(values[i], 10.0 + 2.0 * i); // deterministic ramp
        freeKnobData(&kData);
    }

    {
        InternalChannel channel;
        QVERIFY(channel.configure(R"({"type":"int","val":1,"nelm":3})", &error));
        knobData kData = makeKnobData();
        channel.fillKnobData(&kData);
        QCOMPARE(kData.edata.dataSize, (int) (3 * sizeof(int16_t)));
        int16_t *values = (int16_t *) kData.edata.dataB;
        QCOMPARE(values[0], (int16_t) 1);
        QCOMPARE(values[2], (int16_t) 3);
        freeKnobData(&kData);
    }

    {
        InternalChannel channel;
        QVERIFY(channel.configure(R"({"type":"long","val":100,"nelm":3})", &error));
        knobData kData = makeKnobData();
        channel.fillKnobData(&kData);
        QCOMPARE(kData.edata.dataSize, (int) (3 * sizeof(int32_t)));
        int32_t *values = (int32_t *) kData.edata.dataB;
        QCOMPARE(values[2], (int32_t) 102);
        freeKnobData(&kData);
    }

    {
        InternalChannel channel;
        QVERIFY(channel.configure(R"({"type":"float","val":0.5,"step":0.5,"nelm":4})", &error));
        knobData kData = makeKnobData();
        channel.fillKnobData(&kData);
        QCOMPARE(kData.edata.dataSize, (int) (4 * sizeof(float)));
        float *values = (float *) kData.edata.dataB;
        QCOMPARE(values[3], 2.0f);
        freeKnobData(&kData);
    }

    {
        InternalChannel channel;
        QVERIFY(channel.configure(R"({"type":"char","val":65,"nelm":3})", &error));
        knobData kData = makeKnobData();
        channel.fillKnobData(&kData);
        char *values = (char *) kData.edata.dataB;
        QCOMPARE(values[0], 'A');
        QCOMPARE(values[1], 'B');
        QCOMPARE(values[2], 'C');
        freeKnobData(&kData);
    }
}

void TestInternalChannel::hoprLoprWork()
{
    QString error;

    // HOPR/LOPR within the drive range become the display/input limits,
    // decoupled from the drive (control) limits
    {
        InternalChannel channel;
        QVERIFY(channel.configure(R"({"type":"double","val":50,"drvl":0,"drvh":100,"hopr":80,"lopr":20})", &error));
        knobData kData = makeKnobData();
        channel.fillKnobData(&kData);
        QCOMPARE(kData.edata.lower_disp_limit, 20.0);
        QCOMPARE(kData.edata.upper_disp_limit, 80.0);
        QCOMPARE(kData.edata.lower_ctrl_limit, 0.0);
        QCOMPARE(kData.edata.upper_ctrl_limit, 100.0);
        freeKnobData(&kData);
    }

    // HOPR/LOPR outside the drive range are clamped into it
    {
        InternalChannel channel;
        QVERIFY(channel.configure(R"({"type":"double","val":50,"drvl":0,"drvh":100,"hopr":150,"lopr":-50})", &error));
        knobData kData = makeKnobData();
        channel.fillKnobData(&kData);
        QCOMPARE(kData.edata.lower_disp_limit, 0.0);
        QCOMPARE(kData.edata.upper_disp_limit, 100.0);
        QCOMPARE(kData.edata.lower_ctrl_limit, 0.0);
        QCOMPARE(kData.edata.upper_ctrl_limit, 100.0);
        freeKnobData(&kData);
    }

    // without HOPR/LOPR the display limits fall back to the drive limits
    // (regression check: identical to the pre-HOPR/LOPR behaviour)
    {
        InternalChannel channel;
        QVERIFY(channel.configure(R"({"type":"double","val":50,"drvl":0,"drvh":100})", &error));
        knobData kData = makeKnobData();
        channel.fillKnobData(&kData);
        QCOMPARE(kData.edata.lower_disp_limit, 0.0);
        QCOMPARE(kData.edata.upper_disp_limit, 100.0);
        freeKnobData(&kData);
    }

    // HOPR/LOPR without any drive limits are used as given, unclamped
    {
        InternalChannel channel;
        QVERIFY(channel.configure(R"({"type":"double","val":50,"hopr":80,"lopr":20})", &error));
        knobData kData = makeKnobData();
        channel.fillKnobData(&kData);
        QCOMPARE(kData.edata.lower_disp_limit, 20.0);
        QCOMPARE(kData.edata.upper_disp_limit, 80.0);
        QCOMPARE(kData.edata.lower_ctrl_limit, 0.0);
        QCOMPARE(kData.edata.upper_ctrl_limit, 0.0);
        freeKnobData(&kData);
    }

    // resetting HOPR/LOPR to the same value (0) at runtime, as a caput to
    // NAME.HOPR/NAME.LOPR would, is recognized as "not configured" again and
    // falls back to DRVL/DRVH, exactly like they were never set
    {
        InternalChannel channel;
        QVERIFY(channel.configure(R"({"type":"double","val":50,"drvl":0,"drvh":100,"hopr":80,"lopr":20})", &error));
        knobData kData = makeKnobData();
        channel.fillKnobData(&kData);
        QCOMPARE(kData.edata.lower_disp_limit, 20.0);
        QCOMPARE(kData.edata.upper_disp_limit, 80.0);

        channel.setFieldValue(InternalChannel::FieldHopr, 0.0, 0, QString());
        channel.setFieldValue(InternalChannel::FieldLopr, 0.0, 0, QString());
        channel.fillKnobData(&kData);
        QCOMPARE(kData.edata.lower_disp_limit, 0.0);
        QCOMPARE(kData.edata.upper_disp_limit, 100.0);
        freeKnobData(&kData);
    }
}

void TestInternalChannel::drvlDrvhClampTheValue()
{
    QString error;

    // configure(): an out-of-range VAL is clamped to the nearest drive limit
    {
        InternalChannel channel;
        QVERIFY(channel.configure(R"({"type":"double","val":150,"drvl":0,"drvh":100})", &error));
        QCOMPARE(channel.currentValue(), 100.0);

        InternalChannel below;
        QVERIFY(below.configure(R"({"type":"double","val":-50,"drvl":0,"drvh":100})", &error));
        QCOMPARE(below.currentValue(), 0.0);
    }

    // setValue(): a write outside the drive range is clamped the same way
    {
        InternalChannel channel;
        QVERIFY(channel.configure(R"({"type":"double","val":50,"drvl":0,"drvh":100})", &error));
        channel.setValue(500.0, 0, QString());
        QCOMPARE(channel.currentValue(), 100.0);
        channel.setValue(-500.0, 0, QString());
        QCOMPARE(channel.currentValue(), 0.0);
        channel.setValue(42.0, 0, QString());
        QCOMPARE(channel.currentValue(), 42.0);
    }

    // DRVL==DRVH==0 (or both unset) is "no drive limits configured": no clamping
    {
        InternalChannel channel;
        QVERIFY(channel.configure(R"({"type":"double","val":12345,"drvl":0,"drvh":0})", &error));
        QCOMPARE(channel.currentValue(), 12345.0);

        InternalChannel unset;
        QVERIFY(unset.configure(R"({"type":"double","val":12345})", &error));
        QCOMPARE(unset.currentValue(), 12345.0);
        unset.setValue(-98765.0, 0, QString());
        QCOMPARE(unset.currentValue(), -98765.0);
    }

    // only one side configured: only that side clamps
    {
        InternalChannel channel;
        QVERIFY(channel.configure(R"({"type":"double","val":50,"drvh":100})", &error));
        channel.setValue(500.0, 0, QString());
        QCOMPARE(channel.currentValue(), 100.0);
        channel.setValue(-500.0, 0, QString());
        QCOMPARE(channel.currentValue(), -500.0);
    }
}

void TestInternalChannel::nordAndNelmWorkLikeEpics()
{
    QString error;

    // NORD defaults to NELM (full array)
    InternalChannel full;
    QVERIFY(full.configure(R"({"type":"double","nelm":4})", &error));
    QCOMPARE(full.nord, 4);

    // NORD limits the used elements, NELM stays the maximum size
    InternalChannel partial;
    QVERIFY(partial.configure(R"({"type":"double","mode":"counter","val":10,"step":1,"nelm":8,"nord":3})", &error));
    knobData kData = makeKnobData();
    partial.fillKnobData(&kData);
    QCOMPARE(kData.edata.nelm, 8);
    QCOMPARE(kData.edata.valueCount, 3);
    QCOMPARE(kData.edata.dataSize, (int) (3 * sizeof(double)));
    double *values = (double *) kData.edata.dataB;
    QCOMPARE(values[0], 10.0);
    QCOMPARE(values[2], 12.0);

    // NORD can never exceed NELM
    InternalChannel clamped;
    QVERIFY(clamped.configure(R"({"type":"double","nelm":4,"nord":99})", &error));
    QCOMPARE(clamped.nord, 4);

    // a waveform write updates NORD like an EPICS waveform record
    QVector<double> wave;
    wave << 5.0 << 6.0;
    partial.setWave(wave);
    QCOMPARE(partial.nord, 2);
    partial.fillKnobData(&kData);
    QCOMPARE(kData.edata.nelm, 8);
    QCOMPARE(kData.edata.valueCount, 2);
    values = (double *) kData.edata.dataB;
    QCOMPARE(values[0], 5.0);
    QCOMPARE(values[1], 6.0);
    freeKnobData(&kData);

    // "val" given as array initializes the waveform content and NORD
    InternalChannel initialized;
    QVERIFY(initialized.configure(R"({"type":"double","nelm":8,"val":[1.5,2.5,3.5]})", &error));
    QCOMPARE(initialized.nord, 3);
    QCOMPARE(initialized.currentValue(), 1.5);
    knobData kInit = makeKnobData();
    initialized.fillKnobData(&kInit);
    QCOMPARE(kInit.edata.nelm, 8);
    QCOMPARE(kInit.edata.valueCount, 3);
    values = (double *) kInit.edata.dataB;
    QCOMPARE(values[0], 1.5);
    QCOMPARE(values[1], 2.5);
    QCOMPARE(values[2], 3.5);
    freeKnobData(&kInit);

    // an explicitly given NORD wins over the array length
    InternalChannel explicitNord;
    QVERIFY(explicitNord.configure(R"({"type":"double","nelm":8,"nord":2,"val":[1,2,3]})", &error));
    QCOMPARE(explicitNord.nord, 2);

    // a counter waveform starts from the val array and every element counts on
    InternalChannel counting;
    QVERIFY(counting.configure(R"({"type":"double","mode":"counter","val":[1,2,3],"nelm":8,
                                   "step":1,"period":100,"drvl":0,"drvh":4,"overflow":true})", &error));
    counting.tick();
    knobData kCounting = makeKnobData();
    counting.fillKnobData(&kCounting);
    QCOMPARE(kCounting.edata.valueCount, 3);
    values = (double *) kCounting.edata.dataB;
    QCOMPARE(values[0], 2.0);
    QCOMPARE(values[1], 3.0);
    QCOMPARE(values[2], 4.0);
    counting.tick(); // element 3 exceeds drvh 4 and wraps to drvl 0
    counting.fillKnobData(&kCounting);
    values = (double *) kCounting.edata.dataB;
    QCOMPARE(values[0], 3.0);
    QCOMPARE(values[1], 4.0);
    QCOMPARE(values[2], 0.0);
    freeKnobData(&kCounting);

    // a string waveform is initialized with an array of strings
    InternalChannel texts;
    QVERIFY(texts.configure(R"({"type":"string","nelm":4,"val":["one","two"]})", &error));
    QCOMPARE(texts.nord, 2);
    knobData kTexts = makeKnobData();
    texts.fillKnobData(&kTexts);
    QCOMPARE(kTexts.edata.valueCount, 2);
    QCOMPARE(QString((char *) kTexts.edata.dataB), QString("one\033two"));
    freeKnobData(&kTexts);
}

void TestInternalChannel::setValueWorks()
{
    QString error;

    {
        InternalChannel channel;
        QVERIFY(channel.configure(R"({"type":"double","mode":"counter","val":0,"step":1})", &error));
        channel.setValue(12.5, 0, QString());
        QCOMPARE(channel.currentValue(), 12.5);
        channel.tick(); // the counter continues from the written value
        QCOMPARE(channel.currentValue(), 13.5);
    }

    {
        InternalChannel channel;
        QVERIFY(channel.configure(R"({"type":"long"})", &error));
        channel.setValue(0.0, 77, QString());
        QCOMPARE(channel.currentValue(), 77.0);
    }

    {
        InternalChannel channel;
        QVERIFY(channel.configure(R"({"type":"enum","enums":["OFF","ON","ERROR"]})", &error));
        channel.setValue(0.0, 0, "ERROR"); // writing the state name selects its index
        QCOMPARE(channel.currentValue(), 2.0);
        channel.setValue(0.0, 1, "unknown state");
        QCOMPARE(channel.currentValue(), 1.0);
    }

    {
        InternalChannel channel;
        QVERIFY(channel.configure(R"({"type":"string","val":"before"})", &error));
        channel.setValue(0.0, 0, "after");
        QCOMPARE(channel.text, QString("after"));
    }
}

void TestInternalChannel::setWaveWorks()
{
    QString error;
    InternalChannel channel;
    QVERIFY(channel.configure(R"({"type":"double","nelm":3,"val":0})", &error));

    QVector<double> wave;
    wave << 7.0 << 8.0 << 9.0;
    channel.setWave(wave);
    QCOMPARE(channel.currentValue(), 7.0);

    knobData kData = makeKnobData();
    channel.fillKnobData(&kData);
    double *values = (double *) kData.edata.dataB;
    QCOMPARE(values[0], 7.0);
    QCOMPARE(values[1], 8.0);
    QCOMPARE(values[2], 9.0);
    freeKnobData(&kData);
}

void TestInternalChannel::unconfiguredChannelIsNotConnected()
{
    InternalChannel channel;
    knobData kData = makeKnobData();
    kData.edata.connected = true; // stale state from a previous use

    // before any configuration the channel presents itself disconnected,
    // for the value monitor and for every field monitor
    channel.fillKnobData(&kData);
    QCOMPARE(kData.edata.connected, 0);
    QCOMPARE(kData.edata.valueCount, 0);
    kData.edata.connected = true;
    channel.fillKnobDataField(&kData, InternalChannel::FieldSevr);
    QCOMPARE(kData.edata.connected, 0);

    // the successful configuration is the connect
    QString error;
    QVERIFY2(channel.configure(R"({"type":"double","val":5})", &error), qPrintable(error));
    QVERIFY(channel.controlInfoChanged); // forces initialize on the next publish
    channel.fillKnobData(&kData);
    QCOMPARE(kData.edata.connected, 1);
    QCOMPARE(kData.edata.rvalue, 5.0);
    freeKnobData(&kData);
}

void TestInternalChannel::configureMatrixWorks()
{
    QString error;

    // dim fixes the element count, a square matrix starts as the identity
    {
        InternalChannel channel;
        QVERIFY2(channel.configure(R"({"type":"double","dim":[3,3]})", &error), qPrintable(error));
        QVERIFY(channel.isMatrix());
        QCOMPARE(channel.dimRows, 3);
        QCOMPARE(channel.dimCols, 3);
        QCOMPARE(channel.fieldtype, (short) caDOUBLE);
        QCOMPARE(channel.nelm, 9);
        QCOMPARE(channel.nord, 9);
        QCOMPARE(channel.currentValue(), 1.0); // element 0 of the identity
        knobData kData = makeKnobData();
        channel.fillKnobData(&kData);
        double *values = (double *) kData.edata.dataB;
        for(int i = 0; i < 9; i++) QCOMPARE(values[i], (i % 4 == 0) ? 1.0 : 0.0);
        freeKnobData(&kData);
    }

    // a non square matrix starts with zeros, any numeric element type works
    {
        InternalChannel channel;
        QVERIFY2(channel.configure(R"({"type":"float","dim":[2,3]})", &error), qPrintable(error));
        QCOMPARE(channel.nelm, 6);
        knobData kData = makeKnobData();
        channel.fillKnobData(&kData);
        float *values = (float *) kData.edata.dataB;
        for(int i = 0; i < 6; i++) QCOMPARE(values[i], 0.0f);
        freeKnobData(&kData);
    }

    // an array initialization is taken row-major and padded with zeros
    {
        InternalChannel channel;
        QVERIFY(channel.configure(R"({"type":"double","dim":[2,2],"val":[1.5,0.1,7]})", &error));
        knobData kData = makeKnobData();
        channel.fillKnobData(&kData);
        double *values = (double *) kData.edata.dataB;
        QCOMPARE(values[0], 1.5);
        QCOMPARE(values[1], 0.1);
        QCOMPARE(values[2], 7.0);
        QCOMPARE(values[3], 0.0);
        QCOMPARE(channel.currentValue(), 1.5); // scalar mirror is element 0
        freeKnobData(&kData);
    }

    // a scalar initialization targets element 0, no identity
    {
        InternalChannel channel;
        QVERIFY(channel.configure(R"({"type":"double","dim":[4,4],"val":9})", &error));
        knobData kData = makeKnobData();
        channel.fillKnobData(&kData);
        double *values = (double *) kData.edata.dataB;
        QCOMPARE(values[0], 9.0);
        QCOMPARE(values[5], 0.0);
        freeKnobData(&kData);
    }

    // configuration errors
    {
        InternalChannel channel;
        QVERIFY(!channel.configure(R"({"type":"enum","dim":[2,2]})", &error));
        QVERIFY(!channel.configure(R"({"type":"string","dim":[2,2]})", &error));
        QVERIFY(!channel.configure(R"({"type":"double","dim":[0,4]})", &error));
        QVERIFY(!channel.configure(R"({"type":"double","dim":[4]})", &error));
        QVERIFY(!channel.configure(R"({"type":"double","dim":[2,2],"nelm":5})", &error));
        QVERIFY(!channel.configure(R"({"type":"double","dim":[2,2],"nord":2})", &error));
        QVERIFY(!channel.configure(R"({"type":"double","dim":[4,4],"val":[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17]})", &error));
    }

    // matching nelm/nord are accepted, a reconfiguration without dim drops the shape
    {
        InternalChannel channel;
        QVERIFY2(channel.configure(R"({"type":"double","dim":[4,4],"nelm":16,"nord":16})", &error), qPrintable(error));
        QCOMPARE(channel.nelm, 16);
        QVERIFY(channel.configure(R"({"type":"double","val":3})", &error));
        QVERIFY(!channel.isMatrix());
    }
}

void TestInternalChannel::matrixWritesKeepShape()
{
    QString error;
    InternalChannel channel;
    QVERIFY(channel.configure(R"({"type":"double","dim":[2,2],"val":[1,2,3,4]})", &error));
    knobData kData = makeKnobData();

    // a scalar VAL write targets element 0, the other elements stay
    channel.setValue(10.0, 0, QString());
    QCOMPARE(channel.currentValue(), 10.0);
    channel.fillKnobData(&kData);
    double *values = (double *) kData.edata.dataB;
    QCOMPARE(values[0], 10.0);
    QCOMPARE(values[3], 4.0);
    QCOMPARE(channel.nord, 4);

    // a full wave write replaces everything, double values read back exactly
    QVector<double> wave;
    wave << 0.1 << 6.0 << 7.0 << 8.0;
    channel.setWave(wave);
    channel.fillKnobData(&kData);
    values = (double *) kData.edata.dataB;
    QCOMPARE(values[0], 0.1);
    QCOMPARE(channel.currentValue(), 0.1);
    QCOMPARE(values[3], 8.0);

    // a partial write keeps the remaining elements and never shrinks the count
    QVector<double> partial;
    partial << 50.0 << 60.0;
    channel.setWave(partial);
    channel.fillKnobData(&kData);
    values = (double *) kData.edata.dataB;
    QCOMPARE(values[0], 50.0);
    QCOMPARE(values[1], 60.0);
    QCOMPARE(values[2], 7.0);
    QCOMPARE(kData.edata.valueCount, 4);
    QCOMPARE(channel.nord, 4);

    // NORD stays fixed on a matrix
    channel.setFieldValue(InternalChannel::FieldNord, 0.0, 2, QString());
    QCOMPARE(channel.nord, 4);
    freeKnobData(&kData);

    // the scalar write is clamped to the drive limits
    {
        InternalChannel limited;
        QVERIFY(limited.configure(R"({"type":"double","dim":[1,2],"drvl":-1,"drvh":1})", &error));
        limited.setValue(5.0, 0, QString());
        QCOMPARE(limited.currentValue(), 1.0);
    }

    // an int16 matrix truncates and wraps like the scalar type and takes
    // idata for scalar writes
    {
        InternalChannel intChannel;
        QVERIFY(intChannel.configure(R"({"type":"int","dim":[1,3]})", &error));
        QVector<double> ints;
        ints << 1.7 << 2.2 << 70000.0;
        intChannel.setWave(ints);
        knobData k = makeKnobData();
        intChannel.fillKnobData(&k);
        qint16 *iv = (qint16 *) k.edata.dataB;
        QCOMPARE(iv[0], (qint16) 1);
        QCOMPARE(iv[1], (qint16) 2);
        QCOMPARE(iv[2], (qint16) 70000);
        QCOMPARE(k.edata.rvalue, 1.0);
        intChannel.setValue(0.0, 5, QString());
        intChannel.fillKnobData(&k);
        iv = (qint16 *) k.edata.dataB;
        QCOMPARE(iv[0], (qint16) 5);
        QCOMPARE(iv[1], (qint16) 2);
        freeKnobData(&k);
    }
}

void TestInternalChannel::matrixCounterTicks()
{
    QString error;
    InternalChannel channel;
    QVERIFY(channel.configure(R"({"type":"double","dim":[1,3],"mode":"counter","val":[0,10,20],"step":1})", &error));

    channel.tick();
    knobData kData = makeKnobData();
    channel.fillKnobData(&kData);
    double *values = (double *) kData.edata.dataB;
    QCOMPARE(values[0], 1.0);
    QCOMPARE(values[1], 11.0);
    QCOMPARE(values[2], 21.0);
    QCOMPARE(channel.currentValue(), 1.0); // scalar mirror follows element 0
    QCOMPARE(kData.edata.dimCount, 2);     // dim travels with every publish
    freeKnobData(&kData);
}

void TestInternalChannel::fillKnobDataMatrixWorks()
{
    QString error;
    InternalChannel channel;
    QVERIFY(channel.configure(R"({"type":"double","dim":[4,4],"val":[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16]})", &error));

    knobData kData = makeKnobData();
    channel.fillKnobData(&kData);
    QCOMPARE(kData.edata.fieldtype, (short) caDOUBLE);
    QCOMPARE(kData.edata.ntType, (int) NT_MATRIX);
    QCOMPARE(kData.edata.dimCount, 2);
    QCOMPARE(kData.edata.dim[0], 4);
    QCOMPARE(kData.edata.dim[1], 4);
    QCOMPARE(kData.edata.nelm, 16);
    QCOMPARE(kData.edata.valueCount, 16);
    QCOMPARE(kData.edata.dataSize, (int) (16 * sizeof(double)));
    QCOMPARE(kData.edata.rvalue, 1.0); // element 0
    double *values = (double *) kData.edata.dataB;
    for(int i = 0; i < 16; i++) QCOMPARE(values[i], (double) (i + 1));
    QCOMPARE(values[1 * 4 + 2], 7.0);  // row 1, column 2 in row-major order
    freeKnobData(&kData);

    // an int16 matrix carries the same metadata with its native element type
    {
        InternalChannel intChannel;
        QVERIFY(intChannel.configure(R"({"type":"int","dim":[2,3]})", &error));
        knobData k = makeKnobData();
        intChannel.fillKnobData(&k);
        QCOMPARE(k.edata.fieldtype, (short) caINT);
        QCOMPARE(k.edata.ntType, (int) NT_MATRIX);
        QCOMPARE(k.edata.dim[0], 2);
        QCOMPARE(k.edata.dim[1], 3);
        QCOMPARE(k.edata.dataSize, (int) (6 * sizeof(qint16)));
        freeKnobData(&k);
    }

    // a plain waveform carries no dim, stale metadata is cleared
    {
        InternalChannel wave;
        QVERIFY(wave.configure(R"({"type":"double","nelm":16})", &error));
        knobData k = makeKnobData();
        k.edata.dimCount = 2;
        k.edata.dim[0] = 4;
        wave.fillKnobData(&k);
        QCOMPARE(k.edata.ntType, (int) NT_NONE);
        QCOMPARE(k.edata.dimCount, 0);
        QCOMPARE(k.edata.dim[0], 0);
        freeKnobData(&k);
    }

    // the NELM/NORD field monitors report the fixed count and no dim
    knobData fieldData = makeKnobData();
    fieldData.edata.dimCount = 2;
    channel.fillKnobDataField(&fieldData, InternalChannel::FieldNelm);
    QCOMPARE(fieldData.edata.ivalue, (long) 16);
    QCOMPARE(fieldData.edata.dimCount, 0);
    channel.fillKnobDataField(&fieldData, InternalChannel::FieldNord);
    QCOMPARE(fieldData.edata.ivalue, (long) 16);
    freeKnobData(&fieldData);

    // an unconfigured channel reports no dim either
    {
        InternalChannel none;
        knobData k = makeKnobData();
        k.edata.dimCount = 2;
        none.fillKnobData(&k);
        QCOMPARE(k.edata.dimCount, 0);
        freeKnobData(&k);
    }
}

void TestInternalChannel::configJsonIsStoredNormalized()
{
    QString error;
    InternalChannel channel;
    QVERIFY(channel.configure(R"({ "val": 3,  "type": "long" })", &error));
    QCOMPARE(channel.configJson, QString(R"({"type":"long","val":3})"));
    QVERIFY(channel.definedBy.isEmpty()); // the plugin sets the origin

    // whitespace and key order do not matter, invalid text stays as written
    QCOMPARE(InternalChannel::normalizedJson(R"({"type":"long","val":3})"), channel.configJson);
    QCOMPARE(InternalChannel::normalizedJson(R"({"type":"long","val":4})") == channel.configJson, false);
    QCOMPARE(InternalChannel::normalizedJson("  not json "), QString("not json"));
}

