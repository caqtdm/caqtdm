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
#include "tst_internal_plugin.h"
#include "qtdefinitions.h"

#include <QMutex>

#include <string.h>

void TestInternalPlugin::initTestCase()
{
    m_plugin = Q_NULLPTR;
    m_mutexKnobData = Q_NULLPTR;
}

void TestInternalPlugin::init()
{
    m_plugin = new InternalPlugin();
    m_mutexKnobData = new MutexKnobData();
    m_widgets.clear();
    m_indexes.clear();

    QCOMPARE(m_plugin->initCommunicationLayer(m_mutexKnobData, Q_NULLPTR, QMap<QString, QString>()), (int) true);
}

void TestInternalPlugin::cleanupTestCase()
{
}

void TestInternalPlugin::cleanup()
{
    foreach(int index, m_indexes) {
        knobData *kData = m_mutexKnobData->GetMutexKnobDataPtr(index);
        if(kData != (knobData *) Q_NULLPTR) {
            m_plugin->pvFreeAllocatedData(kData);
            delete (QMutex *) kData->mutex;
            kData->mutex = (void *) Q_NULLPTR;
        }
    }
    m_indexes.clear();

    delete m_plugin;
    delete m_mutexKnobData;
    qDeleteAll(m_widgets);
    m_widgets.clear();
}

// registers a monitor for the given pv like caqtdm_lib does; the configuration
// is carried by the channelConfigJSON property of the defining widget (as the
// genSoftPV widget does) and read by the plugin through kData->dispW
int TestInternalPlugin::createMonitor(const QString &pv, const QString &configJSON, const QString &widgetName)
{
    QWidget *widget = new QWidget();
    if(!configJSON.isEmpty()) widget->setProperty("channelConfigJSON", configJSON);
    if(!widgetName.isEmpty()) widget->setObjectName(widgetName);
    m_widgets.append(widget);

    int index = m_mutexKnobData->GetMutexKnobDataIndex();

    knobData kData;
    memset(&kData, 0, sizeof(knobData));
    kData.index = index;
    kData.soft = 0;
    kData.thisW = (void *) widget;
    kData.dispW = (void *) widget;
    kData.mutex = (void *) new QMutex();
    qstrncpy(kData.pv, pv.toLatin1().constData(), MAXPVLEN - 1);
    qstrncpy(kData.pluginName, "internal", caqtdm_string_t_length);
    m_mutexKnobData->SetMutexKnobData(index, kData);

    knobData *kPtr = m_mutexKnobData->GetMutexKnobDataPtr(index);
    m_plugin->pvAddMonitor(index, kPtr, 0, 0);
    m_indexes.append(index);
    return index;
}

// executes one base interval of the publish timer without a running event loop
void TestInternalPlugin::pumpTimerOnce()
{
    QMetaObject::invokeMethod(m_plugin, "updateChannels", Qt::DirectConnection);
}

void TestInternalPlugin::pluginNameIsInternal()
{
    QCOMPARE(m_plugin->pluginName(), QString("internal"));
}

void TestInternalPlugin::addMonitorPublishesInitialValue()
{
    int index = createMonitor("READY", R"({"type":"double","val":4.5,"units":"V"})");

    pumpTimerOnce();

    knobData *kData = m_mutexKnobData->GetMutexKnobDataPtr(index);
    QCOMPARE(kData->edata.connected, (int) true);
    QCOMPARE(kData->edata.fieldtype, (short) caDOUBLE);
    QCOMPARE(kData->edata.rvalue, 4.5);
    QCOMPARE(QString(kData->edata.units), QString("V"));
    QVERIFY(kData->edata.monitorCount > 0);
}

void TestInternalPlugin::configFromWidgetPropertyWorks()
{
    // full configuration far beyond MAXPVLEN, impossible as a channel name
    QString config = R"({"type":"double","mode":"counter","val":0,"step":0.5,"period":200,
                         "drvl":0,"drvh":100,"loop":true,
                         "low":20,"lolo":10,"high":80,"hihi":90,
                         "units":"V","prec":2})";
    QVERIFY(config.length() > 120);

    int index = createMonitor("RAMP", config);
    pumpTimerOnce();

    knobData *kData = m_mutexKnobData->GetMutexKnobDataPtr(index);
    QCOMPARE(kData->edata.fieldtype, (short) caDOUBLE);
    QCOMPARE(kData->edata.lower_disp_limit, 0.0);
    QCOMPARE(kData->edata.upper_disp_limit, 100.0);
    QCOMPARE(kData->edata.lower_warning_limit, 20.0);
    QCOMPARE(kData->edata.lower_alarm_limit, 10.0);
    QCOMPARE(kData->edata.upper_warning_limit, 80.0);
    QCOMPARE(kData->edata.upper_alarm_limit, 90.0);
    QCOMPARE(QString(kData->edata.units), QString("V"));
    QCOMPARE(kData->edata.precision, (short) 2);

    // the counter starts below lolo: major alarm right away
    QCOMPARE(kData->edata.severity, (short) 2);

    InternalChannel *channel = m_plugin->channel("RAMP");
    QVERIFY(channel != Q_NULLPTR);
    QCOMPARE(channel->isConfigured(), true);
    QCOMPARE(channel->mode, InternalChannel::Counter);
}

void TestInternalPlugin::filterSuffixIsIgnored()
{
    // an epics filter suffix in the channel name is no plugin configuration:
    // it only gets stripped from the channel key
    int index = createMonitor(R"(FILT.{"dbnd":{"abs":0.5}})", R"({"type":"long","val":7})");
    pumpTimerOnce();

    QVERIFY(m_plugin->channel("FILT") != Q_NULLPTR);
    QCOMPARE(m_plugin->channel("FILT")->currentValue(), 7.0);
    QCOMPARE(m_plugin->channel(R"(FILT.{"dbnd":{"abs":0.5}})") == Q_NULLPTR, true);

    knobData *kData = m_mutexKnobData->GetMutexKnobDataPtr(index);
    QCOMPARE(kData->edata.ivalue, 7L);
    QCOMPARE(kData->edata.fieldtype, (short) caLONG);
}

void TestInternalPlugin::channelsAreSharedByBaseName()
{
    int first = createMonitor("SHARED", R"({"type":"long","val":11})");
    int second = createMonitor("SHARED"); // no configuration: attaches to the existing channel

    pumpTimerOnce();

    knobData *kFirst = m_mutexKnobData->GetMutexKnobDataPtr(first);
    knobData *kSecond = m_mutexKnobData->GetMutexKnobDataPtr(second);
    QCOMPARE(kFirst->edata.ivalue, 11L);
    QCOMPARE(kSecond->edata.ivalue, 11L);
    QCOMPARE(kSecond->edata.fieldtype, (short) caLONG);

    // both monitors point to the very same channel object
    QCOMPARE(m_plugin->channel("SHARED") != Q_NULLPTR, true);
    QCOMPARE(m_plugin->channel("SHARED")->currentValue(), 11.0);
}

void TestInternalPlugin::invalidConfigLeavesChannelUnconnected()
{
    int index = createMonitor("BROKEN", R"({"type":"nonsense"})");

    pumpTimerOnce();

    // an unconfigurable channel behaves like a disconnected pv
    knobData *kData = m_mutexKnobData->GetMutexKnobDataPtr(index);
    QCOMPARE(kData->edata.connected, (int) false);

    InternalChannel *channel = m_plugin->channel("BROKEN");
    QVERIFY(channel != Q_NULLPTR);
    QCOMPARE(channel->isConfigured(), false);
}

void TestInternalPlugin::channelConnectsOnlyAfterConfiguration()
{
    // a panel referencing a channel before its defining genSoftPV was loaded
    // (the generator panel scenario): everything stays disconnected
    int viewIndex = createMonitor("LATE");
    int sevrIndex = createMonitor("LATE.SEVR");
    pumpTimerOnce();
    QCOMPARE(m_mutexKnobData->GetMutexKnobDataPtr(viewIndex)->edata.connected, (int) false);
    QCOMPARE(m_mutexKnobData->GetMutexKnobDataPtr(sevrIndex)->edata.connected, (int) false);

    // writes to the unconfigured shell are rejected
    char pv[MAXPVLEN];
    char errmess[SMALL_STRING_LENGTH];
    errmess[0] = '\0';
    qstrncpy(pv, "LATE", MAXPVLEN);
    QCOMPARE(m_plugin->pvSetValue(pv, 1.0, 0, (char *) "", (char *) "tst", errmess, 0), (int) false);
    double waveData[2] = {1.0, 2.0};
    QCOMPARE(m_plugin->pvSetWave(pv, (float *) Q_NULLPTR, waveData, (int16_t *) Q_NULLPTR,
                                 (int32_t *) Q_NULLPTR, (char *) Q_NULLPTR, 2, (char *) "tst", errmess),
             (int) false);

    // the defining panel opens later: all monitors connect, and the first
    // publish carries initialize=true so widgets pick up the limits
    createMonitor("LATE", R"({"type":"double","val":7,"drvl":0,"drvh":10})");
    pumpTimerOnce();
    knobData *kView = m_mutexKnobData->GetMutexKnobDataPtr(viewIndex);
    QCOMPARE(kView->edata.connected, (int) true);
    QCOMPARE(kView->edata.rvalue, 7.0);
    QCOMPARE(kView->edata.initialize, (int) true);
    QCOMPARE(m_mutexKnobData->GetMutexKnobDataPtr(sevrIndex)->edata.connected, (int) true);

    // and writes work now
    QCOMPARE(m_plugin->pvSetValue(pv, 3.0, 0, (char *) "", (char *) "tst", errmess, 0), (int) true);
    QCOMPARE(kView->edata.rvalue, 3.0);
}

void TestInternalPlugin::counterAdvancesWithTimerTicks()
{
    // period 100 ms equals the base interval: every pump is one counter step
    int index = createMonitor("COUNT", R"({"type":"long","mode":"counter","val":0,"step":1,"period":100})");

    pumpTimerOnce(); // publishes and executes the first step
    pumpTimerOnce();
    pumpTimerOnce();

    knobData *kData = m_mutexKnobData->GetMutexKnobDataPtr(index);
    QCOMPARE(kData->edata.ivalue, 3L);
}

void TestInternalPlugin::writeThroughPluginWorks()
{
    int index = createMonitor("SETPOINT", R"({"type":"double","val":1.0})");
    pumpTimerOnce();

    char pv[MAXPVLEN];
    char errmess[SMALL_STRING_LENGTH];
    errmess[0] = '\0';
    qstrncpy(pv, "SETPOINT", MAXPVLEN);

    // writing publishes immediately, without waiting for the timer
    QCOMPARE(m_plugin->pvSetValue(pv, 2.5, 0, (char *) "", (char *) "tst", errmess, 0), (int) true);
    knobData *kData = m_mutexKnobData->GetMutexKnobDataPtr(index);
    QCOMPARE(kData->edata.rvalue, 2.5);

    // writing to an unknown channel fails
    qstrncpy(pv, "DOES_NOT_EXIST", MAXPVLEN);
    QCOMPARE(m_plugin->pvSetValue(pv, 1.0, 0, (char *) "", (char *) "tst", errmess, 0), (int) false);

    // waveform write path
    int waveIndex = createMonitor("WAVE", R"({"type":"double","nelm":3})");
    pumpTimerOnce();
    double waveData[3] = {5.0, 6.0, 7.0};
    qstrncpy(pv, "WAVE", MAXPVLEN);
    QCOMPARE(m_plugin->pvSetWave(pv, (float *) Q_NULLPTR, waveData, (int16_t *) Q_NULLPTR,
                                 (int32_t *) Q_NULLPTR, (char *) Q_NULLPTR, 3, (char *) "tst", errmess),
             (int) true);
    knobData *kWave = m_mutexKnobData->GetMutexKnobDataPtr(waveIndex);
    double *values = (double *) kWave->edata.dataB;
    QVERIFY(values != (double *) Q_NULLPTR);
    QCOMPARE(values[0], 5.0);
    QCOMPARE(values[2], 7.0);

    // a written string is processed by the channel regex (macro-like modification)
    int procIndex = createMonitor("PROC", R"({"type":"string","val":"num=\\1","regex":"[a-z]*([0-9]+).*"})");
    pumpTimerOnce();
    qstrncpy(pv, "PROC", MAXPVLEN);
    QCOMPARE(m_plugin->pvSetValue(pv, 0.0, 0, (char *) "abc123xy", (char *) "tst", errmess, 0), (int) true);
    knobData *kProc = m_mutexKnobData->GetMutexKnobDataPtr(procIndex);
    QCOMPARE(QString((char *) kProc->edata.dataB), QString("num=123"));
}

void TestInternalPlugin::channelDeletedWhenUnreferenced()
{
    int first = createMonitor("TEMP", R"({"type":"long","mode":"counter","val":5,"period":100})");
    int second = createMonitor("TEMP");

    // one of two references removed: the channel stays
    m_plugin->pvClearMonitor(m_mutexKnobData->GetMutexKnobDataPtr(first));
    QVERIFY(m_plugin->channel("TEMP") != Q_NULLPTR);

    // reference count reaches zero: the channel is deleted
    m_plugin->pvClearMonitor(m_mutexKnobData->GetMutexKnobDataPtr(second));
    QVERIFY(m_plugin->channel("TEMP") == Q_NULLPTR);

    // a new definition starts over from its configuration
    createMonitor("TEMP", R"({"type":"long","mode":"counter","val":5,"period":100})");
    QVERIFY(m_plugin->channel("TEMP") != Q_NULLPTR);
    QCOMPARE(m_plugin->channel("TEMP")->currentValue(), 5.0);
}

void TestInternalPlugin::fieldMonitorsAndWritesWork()
{
    int mainIndex = createMonitor("FRAMP", R"({"type":"double","val":50,"low":20,"lolo":10,"high":80,"hihi":90})");
    int lowIndex = createMonitor("FRAMP.LOW");
    int sevrIndex = createMonitor("FRAMP.SEVR");
    pumpTimerOnce();

    // field monitors show the record fields, all attached to the same channel
    QCOMPARE(m_mutexKnobData->GetMutexKnobDataPtr(lowIndex)->edata.rvalue, 20.0);
    QCOMPARE(m_mutexKnobData->GetMutexKnobDataPtr(sevrIndex)->edata.fieldtype, (short) caENUM);
    QCOMPARE(m_mutexKnobData->GetMutexKnobDataPtr(sevrIndex)->edata.ivalue, 0L);
    QCOMPARE(m_plugin->channel("FRAMP.LOW") == Q_NULLPTR, true);

    char pv[MAXPVLEN];
    char errmess[SMALL_STRING_LENGTH];
    errmess[0] = '\0';

    // moving a threshold through a field write raises the alarm on the main monitor
    qstrncpy(pv, "FRAMP.high", MAXPVLEN);
    QCOMPARE(m_plugin->pvSetValue(pv, 40.0, 0, (char *) "", (char *) "tst", errmess, 0), (int) true);
    QCOMPARE(m_mutexKnobData->GetMutexKnobDataPtr(mainIndex)->edata.severity, (short) 1);
    QCOMPARE(m_mutexKnobData->GetMutexKnobDataPtr(mainIndex)->edata.upper_warning_limit, 40.0);
    QCOMPARE(m_mutexKnobData->GetMutexKnobDataPtr(sevrIndex)->edata.ivalue, 1L);

    // writing the severity by name, NOTCONNECTED marks the channel disconnected
    qstrncpy(pv, "FRAMP.SEVR", MAXPVLEN);
    QCOMPARE(m_plugin->pvSetValue(pv, 0.0, 0, (char *) "MAJOR", (char *) "tst", errmess, 0), (int) true);
    QCOMPARE(m_mutexKnobData->GetMutexKnobDataPtr(mainIndex)->edata.severity, (short) 2);
    QCOMPARE(m_plugin->pvSetValue(pv, 0.0, 0, (char *) "NOTCONNECTED", (char *) "tst", errmess, 0), (int) true);
    QCOMPARE(m_mutexKnobData->GetMutexKnobDataPtr(mainIndex)->edata.connected, (int) false);
    QCOMPARE(m_mutexKnobData->GetMutexKnobDataPtr(sevrIndex)->edata.ivalue, 4L);

    // a value write re-evaluates the alarms and reconnects the channel
    qstrncpy(pv, "FRAMP", MAXPVLEN);
    QCOMPARE(m_plugin->pvSetValue(pv, 50.0, 0, (char *) "", (char *) "tst", errmess, 0), (int) true);
    QCOMPARE(m_mutexKnobData->GetMutexKnobDataPtr(mainIndex)->edata.connected, (int) true);
    QCOMPARE(m_mutexKnobData->GetMutexKnobDataPtr(mainIndex)->edata.severity, (short) 1);
    QCOMPARE(m_mutexKnobData->GetMutexKnobDataPtr(sevrIndex)->edata.ivalue, 1L);

    // a field monitor keeps the channel referenced
    m_plugin->pvClearMonitor(m_mutexKnobData->GetMutexKnobDataPtr(mainIndex));
    m_plugin->pvClearMonitor(m_mutexKnobData->GetMutexKnobDataPtr(sevrIndex));
    QVERIFY(m_plugin->channel("FRAMP") != Q_NULLPTR);
    m_plugin->pvClearMonitor(m_mutexKnobData->GetMutexKnobDataPtr(lowIndex));
    QVERIFY(m_plugin->channel("FRAMP") == Q_NULLPTR);
}

void TestInternalPlugin::unknownExtensionIsRejected()
{
    // unknown extensions create no channel and stay unconnected
    int index = createMonitor("BAD.SUB", R"({"type":"long","val":1})");
    pumpTimerOnce();
    QVERIFY(m_plugin->channel("BAD") == Q_NULLPTR);
    QVERIFY(m_plugin->channel("BAD.SUB") == Q_NULLPTR);
    QCOMPARE(m_mutexKnobData->GetMutexKnobDataPtr(index)->edata.connected, (int) false);

    char pv[MAXPVLEN];
    char errmess[SMALL_STRING_LENGTH];
    errmess[0] = '\0';
    qstrncpy(pv, "BAD.SUB", MAXPVLEN);
    QCOMPARE(m_plugin->pvSetValue(pv, 1.0, 0, (char *) "", (char *) "tst", errmess, 0), (int) false);

    // a configuration on a field monitor does not configure the channel
    createMonitor("FCFG.SEVR", R"({"type":"long","val":7})");
    pumpTimerOnce();
    QVERIFY(m_plugin->channel("FCFG") != Q_NULLPTR);
    QCOMPARE(m_plugin->channel("FCFG")->isConfigured(), false);
}

void TestInternalPlugin::fieldMonitorsTriggerOnlyOnChange()
{
    int mainIndex = createMonitor("TICKY", R"({"type":"long","mode":"counter","val":0,"step":1,"period":100,"low":-5})");
    int lowIndex = createMonitor("TICKY.LOW");
    int sevrIndex = createMonitor("TICKY.SEVR");
    pumpTimerOnce();

    int lowCount = m_mutexKnobData->GetMutexKnobDataPtr(lowIndex)->edata.monitorCount;
    int sevrCount = m_mutexKnobData->GetMutexKnobDataPtr(sevrIndex)->edata.monitorCount;
    int mainCount = m_mutexKnobData->GetMutexKnobDataPtr(mainIndex)->edata.monitorCount;

    // the counter ticks, but the unchanged field monitors are not triggered
    pumpTimerOnce();
    pumpTimerOnce();
    QCOMPARE(m_mutexKnobData->GetMutexKnobDataPtr(lowIndex)->edata.monitorCount, lowCount);
    QCOMPARE(m_mutexKnobData->GetMutexKnobDataPtr(sevrIndex)->edata.monitorCount, sevrCount);
    QVERIFY(m_mutexKnobData->GetMutexKnobDataPtr(mainIndex)->edata.monitorCount > mainCount);

    // a threshold write triggers the LOW monitor exactly once
    char pv[MAXPVLEN];
    char errmess[SMALL_STRING_LENGTH];
    errmess[0] = '\0';
    qstrncpy(pv, "TICKY.low", MAXPVLEN);
    QCOMPARE(m_plugin->pvSetValue(pv, -2.0, 0, (char *) "", (char *) "tst", errmess, 0), (int) true);
    QCOMPARE(m_mutexKnobData->GetMutexKnobDataPtr(lowIndex)->edata.monitorCount, lowCount + 1);
    QCOMPARE(m_mutexKnobData->GetMutexKnobDataPtr(lowIndex)->edata.rvalue, -2.0);
    QCOMPARE(m_mutexKnobData->GetMutexKnobDataPtr(sevrIndex)->edata.monitorCount, sevrCount);

    pumpTimerOnce();
    QCOMPARE(m_mutexKnobData->GetMutexKnobDataPtr(lowIndex)->edata.monitorCount, lowCount + 1);
}

void TestInternalPlugin::controlInfoWriteForcesReinitialize()
{
    // guards the mechanism that makes a live HOPR/LOPR/DRVL/DRVH/PREC/EGU
    // change reach already-connected widgets: InternalChannel::setFieldValue()
    // marks controlInfoChanged, and InternalPlugin::publishIndex() turns that
    // into edata.initialize=true on the next publish, exactly like an EPICS
    // DBE_PROPERTY event does for the epics3 plugin (see displayCallback in
    // epicsSubs.c). Without this, ComputeNumericMaxMinPrec() and the other
    // "channel limits" widget logic silently ignore the new values.
    int index = createMonitor("REINIT", R"({"type":"double","val":50,"drvl":0,"drvh":100})");
    pumpTimerOnce();
    // the configuration itself is the channel's connect and forces
    // initialize=true; caqtdm_lib consumes the flag when it updates the
    // widget - simulated here
    m_mutexKnobData->GetMutexKnobDataPtr(index)->edata.initialize = false;

    char pv[MAXPVLEN];
    char errmess[SMALL_STRING_LENGTH];
    errmess[0] = '\0';

    // an ordinary value write leaves initialize untouched
    qstrncpy(pv, "REINIT", MAXPVLEN);
    QCOMPARE(m_plugin->pvSetValue(pv, 60.0, 0, (char *) "", (char *) "tst", errmess, 0), (int) true);
    QCOMPARE(m_mutexKnobData->GetMutexKnobDataPtr(index)->edata.initialize, (int) false);

    // writing HOPR is a control-info change: the resulting publish must
    // carry initialize=true
    qstrncpy(pv, "REINIT.HOPR", MAXPVLEN);
    QCOMPARE(m_plugin->pvSetValue(pv, 90.0, 0, (char *) "", (char *) "tst", errmess, 0), (int) true);
    knobData *kData = m_mutexKnobData->GetMutexKnobDataPtr(index);
    QCOMPARE(kData->edata.initialize, (int) true);
    QCOMPARE(kData->edata.upper_disp_limit, 90.0);

    // the flag is consumed by that publish: a further ordinary write does
    // not keep forcing initialize=true
    qstrncpy(pv, "REINIT", MAXPVLEN);
    QCOMPARE(m_plugin->pvSetValue(pv, 65.0, 0, (char *) "", (char *) "tst", errmess, 0), (int) true);
    QCOMPARE(m_mutexKnobData->GetMutexKnobDataPtr(index)->edata.initialize, (int) true);
}

void TestInternalPlugin::drvlDrvhClampThroughPlugin()
{
    // DRVL/DRVH are hard write limits (InternalChannel::clampToDriveLimits),
    // enforced both for the initial VAL (from channelConfigJSON) and for
    // every subsequent pvSetValue() write, through the real plugin path
    int index = createMonitor("CLAMPED", R"({"type":"double","val":150,"drvl":0,"drvh":100})");
    pumpTimerOnce();
    QCOMPARE(m_mutexKnobData->GetMutexKnobDataPtr(index)->edata.rvalue, 100.0);

    char pv[MAXPVLEN];
    char errmess[SMALL_STRING_LENGTH];
    errmess[0] = '\0';
    qstrncpy(pv, "CLAMPED", MAXPVLEN);

    QCOMPARE(m_plugin->pvSetValue(pv, -50.0, 0, (char *) "", (char *) "tst", errmess, 0), (int) true);
    QCOMPARE(m_mutexKnobData->GetMutexKnobDataPtr(index)->edata.rvalue, 0.0);

    QCOMPARE(m_plugin->pvSetValue(pv, 42.0, 0, (char *) "", (char *) "tst", errmess, 0), (int) true);
    QCOMPARE(m_mutexKnobData->GetMutexKnobDataPtr(index)->edata.rvalue, 42.0);
}

void TestInternalPlugin::schemePrefixInDirectCallsIsStripped()
{
    // reproduces the bug found in tst_caqtdm_lib.cpp's writeInternalField():
    // calling the plugin directly (bypassing CaQtDM_Lib::addMonitor(), which
    // normally strips the scheme prefix) with a pv that still carries
    // "internal://" must resolve to the same channel as the bare name
    int index = createMonitor("internal://PREFIXED", R"({"type":"double","val":1})");
    pumpTimerOnce();

    QVERIFY(m_plugin->channel("PREFIXED") != Q_NULLPTR);
    QCOMPARE(m_mutexKnobData->GetMutexKnobDataPtr(index)->edata.rvalue, 1.0);

    char pv[MAXPVLEN];
    char errmess[SMALL_STRING_LENGTH];
    errmess[0] = '\0';

    // a direct value write, still carrying the prefix, resolves to the same channel
    qstrncpy(pv, "internal://PREFIXED", MAXPVLEN);
    QCOMPARE(m_plugin->pvSetValue(pv, 5.0, 0, (char *) "", (char *) "tst", errmess, 0), (int) true);
    QCOMPARE(m_mutexKnobData->GetMutexKnobDataPtr(index)->edata.rvalue, 5.0);

    // a field write with the prefix still attached resolves too
    qstrncpy(pv, "internal://PREFIXED.HOPR", MAXPVLEN);
    QCOMPARE(m_plugin->pvSetValue(pv, 90.0, 0, (char *) "", (char *) "tst", errmess, 0), (int) true);
    QCOMPARE(m_mutexKnobData->GetMutexKnobDataPtr(index)->edata.upper_disp_limit, 90.0);
}

void TestInternalPlugin::matrixChannelThroughPluginWorks()
{
    // a 4x4 double matrix through the real plugin path: the dim metadata
    // travels with every publish and wave writes keep the shape
    int matIndex = createMonitor("POSE", R"({"type":"double","dim":[4,4]})");
    int nordIndex = createMonitor("POSE.NORD");
    pumpTimerOnce();

    knobData *kMat = m_mutexKnobData->GetMutexKnobDataPtr(matIndex);
    QCOMPARE(kMat->edata.fieldtype, (short) caDOUBLE);
    QCOMPARE(kMat->edata.ntType, (int) NT_MATRIX);
    QCOMPARE(kMat->edata.dimCount, 2);
    QCOMPARE(kMat->edata.dim[0], 4);
    QCOMPARE(kMat->edata.dim[1], 4);
    QCOMPARE(kMat->edata.valueCount, 16);
    double *values = (double *) kMat->edata.dataB;
    QVERIFY(values != (double *) Q_NULLPTR);
    QCOMPARE(values[0], 1.0); // identity default
    QCOMPARE(values[9], 0.0);
    QCOMPARE(m_mutexKnobData->GetMutexKnobDataPtr(nordIndex)->edata.ivalue, (long) 16);

    char pv[MAXPVLEN];
    char errmess[SMALL_STRING_LENGTH];
    errmess[0] = '\0';

    // a whole matrix write through the double wave path, row-major
    double matrix[16];
    for(int i = 0; i < 16; i++) matrix[i] = (double) i;
    qstrncpy(pv, "POSE", MAXPVLEN);
    QCOMPARE(m_plugin->pvSetWave(pv, (float *) Q_NULLPTR, matrix, (int16_t *) Q_NULLPTR,
                                 (int32_t *) Q_NULLPTR, (char *) Q_NULLPTR, 16, (char *) "tst", errmess),
             (int) true);
    values = (double *) kMat->edata.dataB;
    QCOMPARE(values[1 * 4 + 2], 6.0);
    QCOMPARE(kMat->edata.dimCount, 2);

    // a partial write keeps the remaining elements and the count
    double part[4] = {100.0, 101.0, 102.0, 103.0};
    QCOMPARE(m_plugin->pvSetWave(pv, (float *) Q_NULLPTR, part, (int16_t *) Q_NULLPTR,
                                 (int32_t *) Q_NULLPTR, (char *) Q_NULLPTR, 4, (char *) "tst", errmess),
             (int) true);
    values = (double *) kMat->edata.dataB;
    QCOMPARE(values[3], 103.0);
    QCOMPARE(values[4], 4.0);
    QCOMPARE(kMat->edata.valueCount, 16);
    QCOMPARE(kMat->edata.dim[1], 4);
    QCOMPARE(m_mutexKnobData->GetMutexKnobDataPtr(nordIndex)->edata.ivalue, (long) 16);

    // a scalar write lands on element 0
    QCOMPARE(m_plugin->pvSetValue(pv, 0.5, 0, (char *) "", (char *) "tst", errmess, 0), (int) true);
    values = (double *) kMat->edata.dataB;
    QCOMPARE(values[0], 0.5);
    QCOMPARE(values[1], 101.0);
    QCOMPARE(kMat->edata.rvalue, 0.5);

    // a plain waveform next to it carries no dim
    int waveIndex = createMonitor("WAVE", R"({"type":"double","nelm":4})");
    pumpTimerOnce();
    QCOMPARE(m_mutexKnobData->GetMutexKnobDataPtr(waveIndex)->edata.ntType, (int) NT_NONE);
    QCOMPARE(m_mutexKnobData->GetMutexKnobDataPtr(waveIndex)->edata.dimCount, 0);
}

void TestInternalPlugin::duplicateDefinitionIsReportedAndIgnored()
{
    // the first genSoftPV definition wins; a different later one is ignored
    // (and reported to the message window, which the test does not have)
    int first = createMonitor("DUP", R"({"type":"long","val":11})", "gensoftpv_a");
    int second = createMonitor("DUP", R"({"type":"double","val":5,"mode":"counter"})", "gensoftpv_b");
    pumpTimerOnce();

    InternalChannel *channel = m_plugin->channel("DUP");
    QVERIFY(channel != Q_NULLPTR);
    QCOMPARE(channel->fieldtype, (short) caLONG);
    QCOMPARE(channel->mode, InternalChannel::Constant);
    QCOMPARE(channel->currentValue(), 11.0);
    QVERIFY(channel->definedBy.contains("gensoftpv_a"));
    QCOMPARE(channel->configJson, InternalChannel::normalizedJson(R"({"type":"long","val":11})"));
    QCOMPARE(m_mutexKnobData->GetMutexKnobDataPtr(first)->edata.ivalue, 11L);
    QCOMPARE(m_mutexKnobData->GetMutexKnobDataPtr(second)->edata.ivalue, 11L);
    QCOMPARE(m_mutexKnobData->GetMutexKnobDataPtr(second)->edata.fieldtype, (short) caLONG);

    // the same definition written differently changes nothing
    createMonitor("DUP", R"({ "val": 11,  "type": "long" })", "gensoftpv_c");
    pumpTimerOnce();
    QCOMPARE(channel->currentValue(), 11.0);
    QVERIFY(channel->definedBy.contains("gensoftpv_a"));
}

void TestInternalPlugin::persistentChannelKeepsRunning()
{
    int index = createMonitor("PCOUNT", R"({"type":"long","mode":"counter","val":0,"step":1,"period":100,"persistent":true})");
    pumpTimerOnce(); // publishes and counts to 1

    // removing the last reference keeps a persistent channel alive
    m_plugin->pvClearMonitor(m_mutexKnobData->GetMutexKnobDataPtr(index));
    QVERIFY(m_plugin->channel("PCOUNT") != Q_NULLPTR);

    // and it keeps counting without any monitor
    pumpTimerOnce();
    pumpTimerOnce();
    QCOMPARE(m_plugin->channel("PCOUNT")->currentValue(), 3.0);

    // a display re-attaching (no configuration) continues with the live value
    int again = createMonitor("PCOUNT");
    pumpTimerOnce();
    knobData *kData = m_mutexKnobData->GetMutexKnobDataPtr(again);
    QCOMPARE(kData->edata.ivalue, 4L);
}
