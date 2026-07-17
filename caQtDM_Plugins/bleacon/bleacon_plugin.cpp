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
#include <QDebug>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QtMath>
#include <algorithm>

#include "bleacon_plugin.h"
#include "searchfile.h"
#include "fileFunctions.h"
#include "caQtDM_Plugins_global.h"

Q_LOGGING_CATEGORY(bleaconLog, "caqtdm.plugins.bleacon")

// as defined in knobDefines.h
//caType {caSTRING	= 0, caINT = 1, caFLOAT = 2, caENUM = 3, caCHAR = 4, caLONG = 5, caDOUBLE = 6};

#define BLEACON_DEFAULT_CONFIG_FILE "bleacon.config"
// measurement relation: distance = 10^((txpower - rssi) / (10 * n))
// n = path loss exponent: 2.0 free field, 2.5..3.5 indoor (larger n = shorter distance)
#define BLEACON_DEFAULT_PATHLOSS_N 2.2
// minimum seconds without sighting before lost; raised per beacon to
// BLEACON_LOST_INTERVALS x the observed advertising interval (channel .interval),
// so slow advertisers (e.g. 5s interval) survive a few missed frames
#define BLEACON_DEFAULT_TIMEOUT_S 10
#define BLEACON_LOST_INTERVALS 3.0
// smoothed rssi below this [dBm] -> status weak (distance estimate unreliable)
#define BLEACON_DEFAULT_WEAK_RSSI (-90)
// time constant of the rssi/distance smoothing: alpha = 1 - exp(-dt/tau) per sighting
#define BLEACON_DEFAULT_TAU_S 10.0
// 1m calibration (channel .calibrate): collect this many rssi samples with the token
// lying at 1m, then use the median as txpower; abort after the timeout, accept the
// result anyway when at least the minimum number of samples arrived
#define BLEACON_CAL_SAMPLES 8
#define BLEACON_CAL_MIN_SAMPLES 3
#define BLEACON_CAL_TIMEOUT_S 120
// .calibrate enum values
#define CAL_IDLE 0
#define CAL_RUNNING 1
#define CAL_DONE 2
#define CAL_FAILED 3

// n values behind the .pathloss environment scenarios (same order as pathLossStrings)
static const double bleaconPathLossValues[] = {2.0, 2.5, 3.0, 3.5};

// nearest scenario for a given n (config files may carry any n value)
static int bleaconPathLossIndex(double n)
{
    int best = 0;
    double bestDiff = 1.0e9;
    for (size_t i = 0; i < sizeof(bleaconPathLossValues) / sizeof(double); i++) {
        double diff = qAbs(n - bleaconPathLossValues[i]);
        if (diff < bestDiff) { bestDiff = diff; best = (int) i; }
    }
    return best;
}
#define BLEACON_SWEEP_MS 1000
// nearest hysteresis: the challenger has to be closer by max(0.5m, 15%) on 2 consecutive sweeps
#define BLEACON_HYST_MIN_M 0.5
#define BLEACON_HYST_FRACTION 0.15
#define BLEACON_HYST_SWEEPS 2

// gives the plugin name back
QString bleaconPlugin::pluginName()
{
    return "bleacon";
}

// constructor
bleaconPlugin::bleaconPlugin()
{
    qCDebug(bleaconLog) << "bleacon: Create";
    mutexknobdataP = Q_NULLPTR;
    messagewindowP = Q_NULLPTR;
    scanner = Q_NULLPTR;
    sweepTimer = Q_NULLPTR;
    scanning = false;
    scanMode = ScanStop;
    nearestHoldCount = 0;
    pathLossExponent = BLEACON_DEFAULT_PATHLOSS_N;
    staleTimeoutSec = BLEACON_DEFAULT_TIMEOUT_S;
    weakRssiThreshold = BLEACON_DEFAULT_WEAK_RSSI;
    smoothingTauS = BLEACON_DEFAULT_TAU_S;
    configFileName = BLEACON_DEFAULT_CONFIG_FILE;
    statusStrings << "neverseen" << "lost" << "weak" << "present";
    scanStrings << "stop" << "start" << "known";
    calStrings << "idle" << "running" << "done" << "failed";
    // deliberately without umlauts: the enum strings travel through dataB as bytes
    // and Qt6/caQtDM_Lib decode them inconsistently (utf8 vs latin1) - a mangled
    // string breaks the text match of the write path and the menu snaps back
    pathLossStrings << "Freigelaende (n=2.0)"
                    << "Gebaeude Holz (n=2.5)"
                    << "Gebaeude Beton (n=3.0)"
                    << "Bunker (n=3.5)";
}

bleaconPlugin::~bleaconPlugin()
{
}

// canonical, protocol tagged beacon address: "ibeacon:0001:0001", "eddystone:AABBCCDDEE21"
QString bleaconPlugin::beaconKey(const QString &protocol, const QString &beaconId)
{
    return protocol + ":" + beaconId;
}

// canonical ibeacon id: major/minor as 4 hex digits each, uppercase ("1:a" -> "0001:000A")
QString bleaconPlugin::normalizeIBeaconId(const QString &beaconId)
{
    QStringList parts = beaconId.split(":");
    if (parts.size() != 2) return beaconId.toUpper();
    quint16 major = (quint16) parts.at(0).toUInt(Q_NULLPTR, 16);
    quint16 minor = (quint16) parts.at(1).toUInt(Q_NULLPTR, 16);
    return BeaconScannerBase::ibeaconId(major, minor);
}

bleaconPlugin::BeaconState *bleaconPlugin::findOrCreateBeacon(const QString &protocol, const QString &beaconId, const QString &groupId)
{
    QString key = beaconKey(protocol, beaconId);
    QMap<QString, BeaconState>::iterator it = beacons.find(key);
    if (it == beacons.end()) {
        BeaconState b;
        b.protocol = protocol;
        b.id = beaconId;
        b.groupId = groupId;
        b.name = "";
        b.major = 0;
        b.minor = 0;
        if (protocol == "ibeacon") {
            QStringList parts = beaconId.split(":");
            if (parts.size() == 2) {
                b.major = (quint16) parts.at(0).toUInt(Q_NULLPTR, 16);
                b.minor = (quint16) parts.at(1).toUInt(Q_NULLPTR, 16);
            }
        }
        b.ewmaRssi = qQNaN();
        b.distance = qQNaN();
        b.txPower = 0;
        b.txPowerOverride = 0;
        b.pathLossN = qQNaN();
        b.avgIntervalS = qQNaN();
        b.battery = qQNaN();
        b.temperature = qQNaN();
        b.lastSeenMs = 0;
        b.readCounter = 0;
        b.lostCounter = 0;
        b.status = StatusNeverseen;
        b.calibrating = false;
        b.calStartMs = 0;
        it = beacons.insert(key, b);
    }
    return &it.value();
}

// accepts an alias, a protocol tagged address or the <major>:<minor> shorthand
// and returns the canonical beacon key ("" when unresolvable)
QString bleaconPlugin::resolveBeaconKey(const QString &addressOrAlias)
{
    if (keyByAlias.contains(addressOrAlias)) return keyByAlias.value(addressOrAlias);
    if (addressOrAlias.startsWith("ibeacon:")) return beaconKey("ibeacon", normalizeIBeaconId(addressOrAlias.mid(8)));
    if (addressOrAlias.startsWith("eddystone:")) return beaconKey("eddystone", addressOrAlias.mid(10).toUpper());
    if (addressOrAlias.contains(':')) return beaconKey("ibeacon", normalizeIBeaconId(addressOrAlias));
    return QString();
}

// 1m calibration finished: the median of the collected raw rssi samples is the
// measured power at 1m; persist it so the token stays calibrated
void bleaconPlugin::finishCalibration(const QString &key, BeaconState &beacon)
{
    QVector<int> samples = beacon.calSamples;
    std::sort(samples.begin(), samples.end());
    int median = samples.at(samples.size() / 2);

    beacon.calibrating = false;
    beacon.txPowerOverride = median;
    beacon.txPower = median;

    updateChannelLong(key + ".calprogress", 100);
    updateChannelEnum(key + ".calibrate", CAL_DONE, calStrings);
    updateChannelLong(key + ".txpower", beacon.txPower);

    QString msg = QString("bleacon: calibrated %1 : txpower=%2 dBm (1m, %3 samples)")
                  .arg(beacon.name.isEmpty() ? key : beacon.name).arg(median).arg(samples.size());
    if (messagewindowP != Q_NULLPTR) messagewindowP->postMsgEvent(QtDebugMsg, (char *) msg.toLatin1().constData());
    qCDebug(bleaconLog) << msg;

    writeConfigFile();
}

// ------------------------------------------------------------------ configuration

// optional calibration parameters on an address line, comma separated:
// txpower=<measured rssi at 1m dBm>, n=<path loss exponent>
void bleaconPlugin::applyBeaconParams(BeaconState *beacon, const QStringList &params)
{
    bool ok = false;
    foreach (const QString &param, params) {
        QString trimmed = param.trimmed();
        if (trimmed.isEmpty()) continue;
        if (trimmed.startsWith("txpower=", Qt::CaseInsensitive)) {
            int tx = trimmed.mid(8).toInt(&ok);
            if (ok && tx < 0) {
                beacon->txPowerOverride = tx;
                beacon->txPower = tx;
                continue;
            }
        } else if (trimmed.startsWith("n=", Qt::CaseInsensitive)) {
            double n = trimmed.mid(2).toDouble(&ok);
            if (ok && n > 0.5 && n < 6.0) {
                beacon->pathLossN = n;
                continue;
            }
        }
        QString msg = "bleacon: invalid beacon parameter ignored (expect txpower=<dBm@1m> or n=<exp>) : " + trimmed;
        if (messagewindowP != Q_NULLPTR) messagewindowP->postMsgEvent(QtWarningMsg, (char *) msg.toLatin1().constData());
    }
}

// config entries are auto-detected by format: "Name=Adresse" defines a human readable
// alias (like the modbus/opcua translation), a beacon address (ibeacon:/eddystone:)
// pre-creates the beacon in the discovery list, a parsable uuid is an iBeacon
// proximity uuid, 20 hex characters are an eddystone namespace; address lines may
// carry calibration parameters (",txpower=-59,n=2.5")
void bleaconPlugin::addConfigEntry(const QString &entry)
{
    QString trimmed = entry.trimmed();
    if (trimmed.isEmpty()) return;

    static const QRegularExpression addressRegex("^(?:(ibeacon):([0-9A-Fa-f]{1,4}:[0-9A-Fa-f]{1,4})|(eddystone):([0-9A-Fa-f]{12}))$");

    // alias definition: Name=Adresse[,param,...]
    int eq = trimmed.indexOf('=');
    if (eq > 0 && !trimmed.left(eq).contains(',')) {
        QString alias = trimmed.left(eq).trimmed();
        QStringList parts = trimmed.mid(eq + 1).split(',');
        QString address = parts.takeFirst().trimmed();
        QRegularExpressionMatch aliasAddressMatch = addressRegex.match(address);
        if (!aliasAddressMatch.hasMatch()) {
            QString msg = "bleacon: invalid alias entry ignored (expect Name=ibeacon:.../eddystone:...) : " + trimmed;
            if (messagewindowP != Q_NULLPTR) messagewindowP->postMsgEvent(QtWarningMsg, (char *) msg.toLatin1().constData());
            return;
        }
        BeaconState *beacon;
        if (!aliasAddressMatch.captured(1).isEmpty()) beacon = findOrCreateBeacon("ibeacon", normalizeIBeaconId(aliasAddressMatch.captured(2)), QString());
        else beacon = findOrCreateBeacon("eddystone", aliasAddressMatch.captured(4).toUpper(), QString());
        beacon->name = alias;
        keyByAlias.insert(alias, beaconKey(beacon->protocol, beacon->id));
        applyBeaconParams(beacon, parts);
        return;
    }

    QStringList parts = trimmed.split(',');
    QString address = parts.takeFirst().trimmed();
    QRegularExpressionMatch addressMatch = addressRegex.match(address);
    if (addressMatch.hasMatch()) {
        BeaconState *beacon;
        if (!addressMatch.captured(1).isEmpty()) beacon = findOrCreateBeacon("ibeacon", normalizeIBeaconId(addressMatch.captured(2)), QString());
        else beacon = findOrCreateBeacon("eddystone", addressMatch.captured(4).toUpper(), QString());
        applyBeaconParams(beacon, parts);
        return;
    }

    QUuid uuid(trimmed);
    if (!uuid.isNull()) {
        if (!uuidList.contains(uuid)) uuidList.append(uuid);
        return;
    }

    static const QRegularExpression namespaceRegex("^[0-9A-Fa-f]{20}$");
    if (namespaceRegex.match(trimmed).hasMatch()) {
        QString ns = trimmed.toUpper();
        if (!namespaceList.contains(ns)) namespaceList.append(ns);
        return;
    }

    QString msg = "bleacon: invalid config entry ignored (expect beacon address, iBeacon uuid or eddystone namespace) : " + trimmed;
    if (messagewindowP != Q_NULLPTR) messagewindowP->postMsgEvent(QtWarningMsg, (char *) msg.toLatin1().constData());
}

void bleaconPlugin::loadConfiguration(QMap<QString, QString> options)
{
    bool ok = false;

    QString value = (QString) qgetenv("CAQTDM_BLEACON_PATHLOSS_N");
    if (!options.value("bleacon_pathloss_n", "").isEmpty()) value = options.value("bleacon_pathloss_n");
    double n = value.toDouble(&ok);
    if (ok && n > 0.5 && n < 6.0) pathLossExponent = n;

    value = (QString) qgetenv("CAQTDM_BLEACON_TIMEOUT");
    if (!options.value("bleacon_timeout", "").isEmpty()) value = options.value("bleacon_timeout");
    int t = value.toInt(&ok);
    if (ok && t > 0) staleTimeoutSec = t;

    value = (QString) qgetenv("CAQTDM_BLEACON_WEAK_RSSI");
    if (!options.value("bleacon_weak_rssi", "").isEmpty()) value = options.value("bleacon_weak_rssi");
    int w = value.toInt(&ok);
    if (ok && w < 0) weakRssiThreshold = w;

    value = (QString) qgetenv("CAQTDM_BLEACON_TAU");
    if (!options.value("bleacon_tau", "").isEmpty()) value = options.value("bleacon_tau");
    double tau = value.toDouble(&ok);
    if (ok && tau > 0.1 && tau < 300.0) smoothingTauS = tau;

    // uuids/namespaces from the environment / options, ';' separated
    QStringList entries;
    value = (QString) qgetenv("CAQTDM_BLEACON_UUIDS");
    if (!value.isEmpty()) entries.append(value.split(";"));
    if (!options.value("bleacon_uuids", "").isEmpty()) entries.append(options.value("bleacon_uuids").split(";"));

    // entries from the config file, searched and downloaded like the modbus database file
    value = (QString) qgetenv("CAQTDM_BLEACON_CONFIG");
    if (!options.value("bleacon_config", "").isEmpty()) value = options.value("bleacon_config");
    if (!value.isEmpty()) configFileName = value;

    // target directory for beacons.config (writable at runtime through beacons.writepath)
    value = (QString) qgetenv("CAQTDM_BLEACON_WRITEPATH");
    if (!options.value("bleacon_writepath", "").isEmpty()) value = options.value("bleacon_writepath");
    if (!value.isEmpty()) configWritePath = value;

    QString url = (QString) qgetenv("CAQTDM_URL_DISPLAY_PATH");
    if (!url.isEmpty()) {
        fileFunctions filefunction;
        filefunction.checkFileAndDownload(configFileName, url);
    }
    searchFile *s = new searchFile(configFileName);
    QString fileNameFound = s->findFile();
    delete s;
    if (!fileNameFound.isEmpty()) {
        QFile file(fileNameFound);
        if (file.open(QIODevice::ReadOnly)) {
            QString msg = "bleacon configuration found : ";
            msg.append(fileNameFound);
            if (messagewindowP != Q_NULLPTR) messagewindowP->postMsgEvent(QtDebugMsg, (char *) msg.toLatin1().constData());
            QTextStream in(&file);
            while (!in.atEnd()) {
                QString line = in.readLine().trimmed();
                if (line.isEmpty() || line.startsWith("#")) continue;
                entries.append(line);
            }
            file.close();
        }
    }

    foreach (const QString &entry, entries) addConfigEntry(entry);

    qCDebug(bleaconLog) << "bleacon: configuration" << uuidList << namespaceList
                        << pathLossExponent << staleTimeoutSec << weakRssiThreshold;
}

// write the (minimal) config file: groups (uuids/namespaces, used as scan filter) and
// the discovered beacon addresses, one per line - lets the plugin create its own
// configuration during development (bleacon://beacons.config)
void bleaconPlugin::writeConfigFile()
{
    QList<QUuid> allUuids = uuidList;
    QStringList allNamespaces = namespaceList;
    QMap<QString, BeaconState>::const_iterator it;
    for (it = beacons.constBegin(); it != beacons.constEnd(); ++it) {
        const BeaconState &beacon = it.value();
        if (beacon.groupId.isEmpty()) continue;
        if (beacon.protocol == "ibeacon") {
            QUuid uuid(beacon.groupId);
            if (!uuid.isNull() && !allUuids.contains(uuid)) allUuids.append(uuid);
        } else if (beacon.protocol == "eddystone") {
            if (!allNamespaces.contains(beacon.groupId)) allNamespaces.append(beacon.groupId);
        }
    }

    // target: beacons.writepath / CAQTDM_BLEACON_WRITEPATH, otherwise overwrite an
    // existing config file, otherwise create it in the current directory
    QString fileName;
    if (!configWritePath.isEmpty()) {
        QString dir = configWritePath;
        while (dir.endsWith("/")) dir.chop(1);
        fileName = dir + "/" + configFileName;
    } else {
        searchFile *s = new searchFile(configFileName);
        fileName = s->findFile();
        delete s;
        if (fileName.isEmpty()) fileName = QDir::currentPath() + "/" + configFileName;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QString msg = "bleacon: could not write config file : " + fileName;
        if (messagewindowP != Q_NULLPTR) messagewindowP->postMsgEvent(QtCriticalMsg, (char *) msg.toLatin1().constData());
        return;
    }
    QTextStream out(&file);
    out << "# bleacon configuration, one entry per line, auto-detected by format:\n";
    out << "# iBeacon proximity uuid | eddystone namespace (20 hex) | beacon address\n";
    foreach (const QUuid &uuid, allUuids) {
        out << uuid.toString(QUuid::WithoutBraces).toUpper() << "\n";
    }
    foreach (const QString &ns, allNamespaces) {
        out << ns << "\n";
    }
    for (it = beacons.constBegin(); it != beacons.constEnd(); ++it) {
        const BeaconState &beacon = it.value();
        if (!beacon.name.isEmpty()) out << beacon.name << "=" << it.key();
        else out << it.key();
        // keep the calibration parameters across a write/load cycle
        if (beacon.txPowerOverride != 0) out << ",txpower=" << beacon.txPowerOverride;
        if (!qIsNaN(beacon.pathLossN)) out << ",n=" << beacon.pathLossN;
        out << "\n";
    }
    file.close();

    QString msg = "bleacon: config file written : " + fileName;
    if (messagewindowP != Q_NULLPTR) messagewindowP->postMsgEvent(QtDebugMsg, (char *) msg.toLatin1().constData());
}

// ------------------------------------------------------------------ data push helpers

void bleaconPlugin::pushKnobData(knobData *kData)
{
    kData->edata.connected = true;
    kData->edata.accessR = true;
    kData->edata.monitorCount++;
    mutexknobdataP->SetMutexKnobData(kData->index, *kData);
    mutexknobdataP->SetMutexKnobDataReceived(kData);
}

void bleaconPlugin::updateChannelDouble(const QString &pv, double value)
{
    foreach (int index, monitorsByPv.values(pv)) {
        knobData *kData = mutexknobdataP->GetMutexKnobDataPtr(index);
        if ((kData == (knobData *) Q_NULLPTR) || (kData->index == -1)) continue;
        kData->edata.fieldtype = caDOUBLE;
        kData->edata.rvalue = value;
        kData->edata.ivalue = (long) value;
        pushKnobData(kData);
    }
}

void bleaconPlugin::updateChannelLong(const QString &pv, qint64 value)
{
    foreach (int index, monitorsByPv.values(pv)) {
        knobData *kData = mutexknobdataP->GetMutexKnobDataPtr(index);
        if ((kData == (knobData *) Q_NULLPTR) || (kData->index == -1)) continue;
        kData->edata.fieldtype = caLONG;
        kData->edata.ivalue = (long) value;
        kData->edata.rvalue = (double) value;
        pushKnobData(kData);
    }
}

void bleaconPlugin::fillStringData(knobData *kData, const QString &value)
{
    QByteArray bytes = value.toLatin1();
    int needed = bytes.size() + 1;
    if (kData->edata.dataSize < needed) {
        if (kData->edata.dataB != (void *) Q_NULLPTR) free(kData->edata.dataB);
        kData->edata.dataB = (void *) malloc((size_t) needed);
        kData->edata.dataSize = needed;
    }
    memcpy((char *) kData->edata.dataB, bytes.constData(), (size_t) needed);
    kData->edata.fieldtype = caSTRING;
    kData->edata.nelm = 1;
    kData->edata.valueCount = 1;
}

void bleaconPlugin::updateChannelString(const QString &pv, const QString &value)
{
    foreach (int index, monitorsByPv.values(pv)) {
        knobData *kData = mutexknobdataP->GetMutexKnobDataPtr(index);
        if ((kData == (knobData *) Q_NULLPTR) || (kData->index == -1)) continue;
        fillStringData(kData, value);
        pushKnobData(kData);
    }
}

// enum: value index in ivalue/rvalue, the state strings ESC (\033) separated in dataB
// (same transport as the epics3 plugin, see epicsSubs.c DBF_ENUM)
void bleaconPlugin::fillEnumData(knobData *kData, int index, const QStringList &strings)
{
    QByteArray joined = strings.join(QChar(27)).toLatin1();
    int needed = joined.size() + 1;
    if (kData->edata.dataSize < needed) {
        if (kData->edata.dataB != (void *) Q_NULLPTR) free(kData->edata.dataB);
        kData->edata.dataB = (void *) malloc((size_t) needed);
        kData->edata.dataSize = needed;
    }
    memcpy((char *) kData->edata.dataB, joined.constData(), (size_t) needed);
    kData->edata.fieldtype = caENUM;
    kData->edata.enumCount = strings.size();
    kData->edata.ivalue = index;
    kData->edata.rvalue = (double) index;
    kData->edata.precision = 0;
    kData->edata.nelm = 1;
    kData->edata.valueCount = 1;
}

void bleaconPlugin::updateChannelEnum(const QString &pv, int index, const QStringList &strings)
{
    foreach (int monitorIndex, monitorsByPv.values(pv)) {
        knobData *kData = mutexknobdataP->GetMutexKnobDataPtr(monitorIndex);
        if ((kData == (knobData *) Q_NULLPTR) || (kData->index == -1)) continue;
        fillEnumData(kData, index, strings);
        pushKnobData(kData);
    }
}

void bleaconPlugin::pushBeaconState(const QString &key, const BeaconState &beacon)
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    // clamped: the wall clock may step backwards (ntp sync after a standby)
    double age = (beacon.lastSeenMs > 0) ? qMax(0.0, (now - beacon.lastSeenMs) / 1000.0) : -1.0;

    if (!qIsNaN(beacon.ewmaRssi)) updateChannelDouble(key + ".rssi", beacon.ewmaRssi);
    if (!qIsNaN(beacon.distance)) updateChannelDouble(key + ".distance", beacon.distance);
    updateChannelLong(key + ".txpower", beacon.txPower);
    updateChannelEnum(key + ".pathloss", bleaconPathLossIndex(qIsNaN(beacon.pathLossN) ? pathLossExponent : beacon.pathLossN), pathLossStrings);
    updateChannelDouble(key + ".age", age);
    if (!qIsNaN(beacon.avgIntervalS)) updateChannelDouble(key + ".interval", beacon.avgIntervalS);
    updateChannelEnum(key + ".status", beacon.status, statusStrings);
    updateChannelLong(key + ".readcounter", beacon.readCounter);
    updateChannelLong(key + ".lostcounter", beacon.lostCounter);
    if (!qIsNaN(beacon.battery)) updateChannelDouble(key + ".battery", beacon.battery);
    if (!qIsNaN(beacon.temperature)) updateChannelDouble(key + ".temperature", beacon.temperature);
}

// discovery channel: one address per line, ready to be used in a display;
// aliased beacons show the readable name (the mapping lives in the config file)
void bleaconPlugin::pushBeaconList()
{
    QStringList list;
    QMap<QString, BeaconState>::const_iterator it;
    for (it = beacons.constBegin(); it != beacons.constEnd(); ++it) {
        if (!it.value().name.isEmpty()) list.append("bleacon://" + it.value().name);
        else list.append("bleacon://" + it.key());
    }
    updateChannelString("beacons.list", list.join("\n"));
}

// ------------------------------------------------------------------ scanning

// scan.enable: stop (backend off), start (process everything),
// known (backend on, but only already known beacons are processed)
void bleaconPlugin::applyScanMode(int mode)
{
    if (scanner == Q_NULLPTR) return;
    scanMode = mode;
    if (mode == ScanStop) {
        if (scanning) {
            qCDebug(bleaconLog) << "bleacon: stop scanning";
            scanning = false;
            scanner->stopScan();
            sweepTimer->stop();
        }
    } else {
        if (!scanning) {
            qCDebug(bleaconLog) << "bleacon: start scanning, mode" << scanStrings.value(mode);
            scanning = true;
            scanner->startScan(uuidList, namespaceList);
            sweepTimer->start(BLEACON_SWEEP_MS);
        }
    }
    updateChannelEnum("scan.enable", scanMode, scanStrings);
}

void bleaconPlugin::scannerMessage(QString message, bool isError)
{
    qCDebug(bleaconLog) << "bleacon:" << message;
    if (messagewindowP != Q_NULLPTR) {
        messagewindowP->postMsgEvent(isError ? QtCriticalMsg : QtDebugMsg, (char *) message.toLatin1().constData());
    }
}

void bleaconPlugin::beaconSighting(QString protocol, QString beaconId, QString groupId, int rssi, int txPowerAt1m, double accuracyMeters)
{
    QMutexLocker locker(&mutex);
    if (!scanning) return;

    QString key = beaconKey(protocol, beaconId);
    bool isNew = !beacons.contains(key);
    // known mode: ignore tags that are not in the discovery list (config/monitors)
    if (isNew && scanMode == ScanKnown) return;
    BeaconState *beacon = findOrCreateBeacon(protocol, beaconId, groupId);
    if (beacon->groupId.isEmpty()) beacon->groupId = groupId;
    // calibration from the config file wins over the advertised ranging byte
    // (vendor tools disagree whether their value is at 0m or 1m)
    if (beacon->txPowerOverride != 0) beacon->txPower = beacon->txPowerOverride;
    else if (txPowerAt1m != 0) beacon->txPower = txPowerAt1m;

    qint64 now = QDateTime::currentMSecsSinceEpoch();
    double dt = (beacon->lastSeenMs > 0) ? (now - beacon->lastSeenMs) / 1000.0 : qQNaN();

    // observed advertising interval (channel .interval); gaps across lost don't count
    if (!qIsNaN(dt) && dt > 0.0 && dt < 120.0
            && (beacon->status == StatusPresent || beacon->status == StatusWeak)) {
        beacon->avgIntervalS = qIsNaN(beacon->avgIntervalS) ? dt : 0.3 * dt + 0.7 * beacon->avgIntervalS;
    }

    // time based smoothing against the +-6dB advertisement noise: alpha follows the
    // real time between sightings, so slow advertisers (5s interval) react as fast
    // as quick ones instead of needing many samples
    double alpha = 1.0;
    if (!qIsNaN(dt) && dt >= 0.0) alpha = 1.0 - exp(-dt / smoothingTauS);
    if (qIsNaN(beacon->ewmaRssi)) beacon->ewmaRssi = rssi;
    else beacon->ewmaRssi = alpha * rssi + (1.0 - alpha) * beacon->ewmaRssi;

    // distance: the ios backend delivers a ready estimate (CLBeacon.accuracy), otherwise
    // log-distance path loss model from the smoothed rssi (txPower already at 1m,
    // the protocol specific conversion happened in the backend)
    if (!qIsNaN(accuracyMeters)) {
        if (qIsNaN(beacon->distance)) beacon->distance = accuracyMeters;
        else beacon->distance = alpha * accuracyMeters + (1.0 - alpha) * beacon->distance;
    } else if (beacon->txPower != 0) {
        double n = qIsNaN(beacon->pathLossN) ? pathLossExponent : beacon->pathLossN;
        beacon->distance = pow(10.0, (beacon->txPower - beacon->ewmaRssi) / (10.0 * n));
    }

    beacon->lastSeenMs = now;
    beacon->readCounter++;
    beacon->status = (beacon->ewmaRssi < weakRssiThreshold) ? StatusWeak : StatusPresent;

    // 1m calibration: collect the raw rssi (not the smoothed one) of every sighting
    if (beacon->calibrating) {
        beacon->calSamples.append(rssi);
        updateChannelLong(key + ".calprogress", qMin(99, beacon->calSamples.size() * 100 / BLEACON_CAL_SAMPLES));
        if (beacon->calSamples.size() >= BLEACON_CAL_SAMPLES) finishCalibration(key, *beacon);
    }

    pushBeaconState(key, *beacon);
    if (isNew) {
        qCDebug(bleaconLog) << "bleacon: new beacon" << key << groupId;
        pushBeaconList();
    }
}

// eddystone TLM telemetry
void bleaconPlugin::beaconTelemetry(QString protocol, QString beaconId, double batteryVolts, double temperatureC)
{
    QMutexLocker locker(&mutex);
    if (!scanning) return;

    QString key = beaconKey(protocol, beaconId);
    if (!beacons.contains(key)) return;
    BeaconState &beacon = beacons[key];

    if (!qIsNaN(batteryVolts)) {
        beacon.battery = batteryVolts;
        updateChannelDouble(key + ".battery", beacon.battery);
    }
    if (!qIsNaN(temperatureC)) {
        beacon.temperature = temperatureC;
        updateChannelDouble(key + ".temperature", beacon.temperature);
    }
}

// 1s housekeeping: ages, present->lost transitions and the nearest evaluation
void bleaconPlugin::sweep()
{
    QMutexLocker locker(&mutex);
    qint64 now = QDateTime::currentMSecsSinceEpoch();

    QMap<QString, BeaconState>::iterator it;
    for (it = beacons.begin(); it != beacons.end(); ++it) {
        BeaconState &beacon = it.value();
        if (beacon.status == StatusNeverseen) continue;

        double age = qMax(0.0, (now - beacon.lastSeenMs) / 1000.0);
        updateChannelDouble(it.key() + ".age", age);

        // adaptive: a beacon advertising every 5s must survive a few missed frames -
        // lost only after the configured timeout or 3 x the observed interval
        double effectiveTimeout = staleTimeoutSec;
        if (!qIsNaN(beacon.avgIntervalS)) effectiveTimeout = qMax(effectiveTimeout, BLEACON_LOST_INTERVALS * beacon.avgIntervalS);

        if ((beacon.status == StatusPresent || beacon.status == StatusWeak) && age > effectiveTimeout) {
            beacon.status = StatusLost;
            beacon.lostCounter++;
            qCDebug(bleaconLog) << "bleacon: lost" << it.key();
            updateChannelEnum(it.key() + ".status", beacon.status, statusStrings);
            updateChannelLong(it.key() + ".lostcounter", beacon.lostCounter);
        }

        // calibration watchdog: accept a short measurement, otherwise report failed
        if (beacon.calibrating && (now - beacon.calStartMs) / 1000.0 > BLEACON_CAL_TIMEOUT_S) {
            if (beacon.calSamples.size() >= BLEACON_CAL_MIN_SAMPLES) {
                finishCalibration(it.key(), beacon);
            } else {
                beacon.calibrating = false;
                updateChannelEnum(it.key() + ".calibrate", CAL_FAILED, calStrings);
                QString msg = "bleacon: calibration failed (no signal) : " + it.key();
                if (messagewindowP != Q_NULLPTR) messagewindowP->postMsgEvent(QtCriticalMsg, (char *) msg.toLatin1().constData());
            }
        }
    }

    updateNearest();
}

void bleaconPlugin::updateNearest()
{
    // candidate: smallest distance among the currently visible beacons
    QString bestKey;
    double bestDistance = 0.0;
    QMap<QString, BeaconState>::const_iterator it;
    for (it = beacons.constBegin(); it != beacons.constEnd(); ++it) {
        const BeaconState &beacon = it.value();
        if (beacon.status != StatusPresent && beacon.status != StatusWeak) continue;
        if (qIsNaN(beacon.distance)) continue;
        if (bestKey.isEmpty() || beacon.distance < bestDistance) {
            bestKey = it.key();
            bestDistance = beacon.distance;
        }
    }

    if (bestKey.isEmpty()) {
        if (!nearestKey.isEmpty()) {
            nearestKey.clear();
            nearestHoldCount = 0;
            updateChannelString("nearest.id", "");
            updateChannelString("nearest.name", "");
            updateChannelLong("nearest.valid", 0);
        }
        return;
    }

    bool changed = false;
    bool holderAlive = beacons.contains(nearestKey)
            && (beacons.value(nearestKey).status == StatusPresent || beacons.value(nearestKey).status == StatusWeak);

    if (nearestKey.isEmpty() || !holderAlive) {
        nearestKey = bestKey;
        nearestHoldCount = 0;
        changed = true;
    } else if (bestKey != nearestKey) {
        // hysteresis so the nearest does not flap between two beacons at similar distance
        double holderDistance = beacons.value(nearestKey).distance;
        if (qIsNaN(holderDistance)
                || bestDistance < holderDistance - qMax(BLEACON_HYST_MIN_M, BLEACON_HYST_FRACTION * holderDistance)) {
            nearestHoldCount++;
            if (nearestHoldCount >= BLEACON_HYST_SWEEPS) {
                nearestKey = bestKey;
                nearestHoldCount = 0;
                changed = true;
            }
        } else {
            nearestHoldCount = 0;
        }
    } else {
        nearestHoldCount = 0;
    }

    const BeaconState holder = beacons.value(nearestKey);
    if (changed) {
        qCDebug(bleaconLog) << "bleacon: nearest" << nearestKey;
        updateChannelString("nearest.id", nearestKey);
        updateChannelString("nearest.name", holder.name.isEmpty() ? nearestKey : holder.name);
        updateChannelLong("nearest.major", holder.major);
        updateChannelLong("nearest.minor", holder.minor);
    }
    if (!qIsNaN(holder.distance)) updateChannelDouble("nearest.distance", holder.distance);
    if (!qIsNaN(holder.ewmaRssi)) updateChannelDouble("nearest.rssi", holder.ewmaRssi);
    updateChannelLong("nearest.valid", 1);
}

// ------------------------------------------------------------------ plugin interface

// initialize our communicationlayer with everything you need
int bleaconPlugin::initCommunicationLayer(MutexKnobData *data, MessageWindow *messageWindow, QMap<QString, QString> options)
{
    qCDebug(bleaconLog) << "bleaconPlugin: InitCommunicationLayer with options" << options;

    mutexknobdataP = data;
    messagewindowP = messageWindow;

    loadConfiguration(options);

    scanner = BeaconScannerBase::createScanner(this);
    connect(scanner, SIGNAL(beaconSighting(QString,QString,QString,int,int,double)),
            this, SLOT(beaconSighting(QString,QString,QString,int,int,double)));
    connect(scanner, SIGNAL(beaconTelemetry(QString,QString,double,double)),
            this, SLOT(beaconTelemetry(QString,QString,double,double)));
    connect(scanner, SIGNAL(scannerMessage(QString,bool)),
            this, SLOT(scannerMessage(QString,bool)));

    sweepTimer = new QTimer(this);
    connect(sweepTimer, SIGNAL(timeout()), this, SLOT(sweep()));

    return true;
}

// caQtDM_Lib will call this routine for defining a monitor
int bleaconPlugin::pvAddMonitor(int index, knobData *kData, int rate, int skip)
{
    Q_UNUSED(index);
    Q_UNUSED(rate);
    Q_UNUSED(skip);
    QMutexLocker locker(&mutex);

    // remove EPICS adjustment parameter
    QString pv = kData->pv;
    int pos = pv.indexOf(".{");
    if (pos != -1) pv.truncate(pos);

    qCDebug(bleaconLog) << "bleaconPlugin:pvAddMonitor" << pv << kData->index;

    // protocol tagged beacon address (ids hexadecimal); plain <major>:<minor> is an ibeacon shorthand
    static const QRegularExpression beaconRegex("^(?:(ibeacon):([0-9A-Fa-f]{1,4}:[0-9A-Fa-f]{1,4})|(eddystone):([0-9A-Fa-f]{12})|([0-9A-Fa-f]{1,4}:[0-9A-Fa-f]{1,4}))"
                                                "\\.(rssi|distance|txpower|pathloss|age|interval|status|readcounter|lostcounter|battery|temperature|name|calibrate|calprogress)$");
    static const QRegularExpression nearestRegex("^nearest\\.(id|name|major|minor|distance|rssi|valid)$");
    static const QRegularExpression beaconsRegex("^beacons\\.(list|reset|config|writepath)$");
    static const QRegularExpression aliasRegex("^([A-Za-z_][A-Za-z0-9_-]*)"
                                               "\\.(rssi|distance|txpower|pathloss|age|interval|status|readcounter|lostcounter|battery|temperature|name|calibrate|calprogress)$");

    // resolve a human readable alias from the config file to its canonical address
    QRegularExpressionMatch aliasMatch = aliasRegex.match(pv);
    if (aliasMatch.hasMatch() && keyByAlias.contains(aliasMatch.captured(1))) {
        pv = keyByAlias.value(aliasMatch.captured(1)) + "." + aliasMatch.captured(2);
    }

    kData->edata.precision = 0;
    kData->edata.connected = true;
    kData->edata.accessR = true;
    kData->edata.accessW = false;

    QRegularExpressionMatch match = beaconRegex.match(pv);
    if (match.hasMatch()) {
        QString protocol, beaconId;
        if (!match.captured(1).isEmpty()) {          // ibeacon:<major>:<minor>
            protocol = "ibeacon";
            beaconId = normalizeIBeaconId(match.captured(2));
        } else if (!match.captured(3).isEmpty()) {   // eddystone:<instance>
            protocol = "eddystone";
            beaconId = match.captured(4).toUpper();
        } else {                                     // <major>:<minor> shorthand
            protocol = "ibeacon";
            beaconId = normalizeIBeaconId(match.captured(5));
        }
        QString field = match.captured(6);
        // canonical pv for the monitor list, so shorthand and tagged form update together
        pv = beaconKey(protocol, beaconId) + "." + field;

        BeaconState *beacon = findOrCreateBeacon(protocol, beaconId, QString());
        qint64 now = QDateTime::currentMSecsSinceEpoch();

        if (field == "rssi") {
            kData->edata.fieldtype = caDOUBLE;
            kData->edata.precision = 1;
            qstrncpy(kData->edata.units, "dBm", 39);
            kData->edata.rvalue = qIsNaN(beacon->ewmaRssi) ? 0.0 : beacon->ewmaRssi;
        } else if (field == "distance") {
            kData->edata.fieldtype = caDOUBLE;
            kData->edata.precision = 2;
            qstrncpy(kData->edata.units, "m", 39);
            kData->edata.rvalue = qIsNaN(beacon->distance) ? 0.0 : beacon->distance;
        } else if (field == "txpower") {
            // writable: manual 1m calibration value, persisted into the config file
            kData->edata.fieldtype = caLONG;
            qstrncpy(kData->edata.units, "dBm", 39);
            kData->edata.ivalue = beacon->txPower;
            kData->edata.rvalue = (double) beacon->txPower;
            kData->edata.accessW = true;
        } else if (field == "pathloss") {
            // writable environment scenario; the n of the distance formula is in the string
            double n = qIsNaN(beacon->pathLossN) ? pathLossExponent : beacon->pathLossN;
            fillEnumData(kData, bleaconPathLossIndex(n), pathLossStrings);
            kData->edata.accessW = true;
        } else if (field == "calibrate") {
            fillEnumData(kData, beacon->calibrating ? CAL_RUNNING : CAL_IDLE, calStrings);
            kData->edata.accessW = true;
        } else if (field == "calprogress") {
            kData->edata.fieldtype = caLONG;
            qstrncpy(kData->edata.units, "%", 39);
            kData->edata.ivalue = 0;
            kData->edata.rvalue = 0.0;
        } else if (field == "age") {
            kData->edata.fieldtype = caDOUBLE;
            kData->edata.precision = 1;
            qstrncpy(kData->edata.units, "s", 39);
            kData->edata.rvalue = (beacon->lastSeenMs > 0) ? qMax(0.0, (now - beacon->lastSeenMs) / 1000.0) : -1.0;
        } else if (field == "interval") {
            kData->edata.fieldtype = caDOUBLE;
            kData->edata.precision = 1;
            qstrncpy(kData->edata.units, "s", 39);
            kData->edata.rvalue = qIsNaN(beacon->avgIntervalS) ? 0.0 : beacon->avgIntervalS;
        } else if (field == "status") {
            fillEnumData(kData, beacon->status, statusStrings);
        } else if (field == "readcounter") {
            kData->edata.fieldtype = caLONG;
            kData->edata.ivalue = (long) beacon->readCounter;
            kData->edata.rvalue = (double) beacon->readCounter;
        } else if (field == "lostcounter") {
            kData->edata.fieldtype = caLONG;
            kData->edata.ivalue = (long) beacon->lostCounter;
            kData->edata.rvalue = (double) beacon->lostCounter;
        } else if (field == "battery") {
            kData->edata.fieldtype = caDOUBLE;
            kData->edata.precision = 2;
            qstrncpy(kData->edata.units, "V", 39);
            kData->edata.rvalue = qIsNaN(beacon->battery) ? 0.0 : beacon->battery;
        } else if (field == "temperature") {
            kData->edata.fieldtype = caDOUBLE;
            kData->edata.precision = 1;
            qstrncpy(kData->edata.units, "C", 39);
            kData->edata.rvalue = qIsNaN(beacon->temperature) ? 0.0 : beacon->temperature;
        } else if (field == "name") {
            fillStringData(kData, beacon->name);
        }
    } else if (nearestRegex.match(pv).hasMatch()) {
        QString field = nearestRegex.match(pv).captured(1);
        if (field == "id") {
            fillStringData(kData, nearestKey);
        } else if (field == "name") {
            QString name = nearestKey;
            if (beacons.contains(nearestKey) && !beacons.value(nearestKey).name.isEmpty()) name = beacons.value(nearestKey).name;
            fillStringData(kData, name);
        } else if (field == "distance") {
            kData->edata.fieldtype = caDOUBLE;
            kData->edata.precision = 2;
            qstrncpy(kData->edata.units, "m", 39);
        } else if (field == "rssi") {
            kData->edata.fieldtype = caDOUBLE;
            kData->edata.precision = 1;
            qstrncpy(kData->edata.units, "dBm", 39);
        } else {
            // major, minor, valid
            kData->edata.fieldtype = caLONG;
        }
    } else if (beaconsRegex.match(pv).hasMatch()) {
        QString field = beaconsRegex.match(pv).captured(1);
        if (field == "list") {
            QStringList list;
            QMap<QString, BeaconState>::const_iterator it;
            for (it = beacons.constBegin(); it != beacons.constEnd(); ++it) list.append("bleacon://" + it.key());
            fillStringData(kData, list.join("\n"));
        } else if (field == "writepath") {
            // target directory for beacons.config (desktop use)
            fillStringData(kData, configWritePath);
            kData->edata.accessW = true;
        } else {
            // reset, config: write channels
            kData->edata.fieldtype = caLONG;
            kData->edata.accessW = true;
        }
    } else if (pv == "scan.enable") {
        fillEnumData(kData, scanMode, scanStrings);
        kData->edata.accessW = true;
    } else {
        QString msg = "bleacon: invalid channel : " + pv;
        if (messagewindowP != Q_NULLPTR) messagewindowP->postMsgEvent(QtCriticalMsg, (char *) msg.toLatin1().constData());
        return false;
    }

    monitorsByPv.insert(pv, kData->index);
    pushKnobData(kData);

    if (!scanning) applyScanMode((scanMode == ScanStop) ? ScanStart : scanMode);

    return true;
}

// caQtDM_Lib will call this routine for getting rid of a monitor
int bleaconPlugin::pvClearMonitor(knobData *kData)
{
    QMutexLocker locker(&mutex);
    qCDebug(bleaconLog) << "bleaconPlugin:pvClearMonitor" << kData->pv << kData->index;

    // the monitor list is keyed by the canonical pv: remove by index
    QMultiMap<QString, int>::iterator it = monitorsByPv.begin();
    while (it != monitorsByPv.end()) {
        if (it.value() == kData->index) it = monitorsByPv.erase(it);
        else ++it;
    }
    if (monitorsByPv.isEmpty()) applyScanMode(ScanStop);

    return true;
}

int bleaconPlugin::pvFreeAllocatedData(knobData *kData)
{
    QMutexLocker locker(&mutex);
    qCDebug(bleaconLog) << "bleaconPlugin:pvFreeAllocatedData";
    if (kData->edata.info != (void *) Q_NULLPTR) {
        free(kData->edata.info);
        kData->edata.info = (void *) Q_NULLPTR;
    }
    if (kData->edata.dataB != (void *) Q_NULLPTR) {
        free(kData->edata.dataB);
        kData->edata.dataB = (void *) Q_NULLPTR;
    }

    return true;
}

// caQtDM_Lib will call this routine for setting data
int bleaconPlugin::pvSetValue(char *pv, double rdata, int32_t idata, char *sdata, char *object, char *errmess, int forceType)
{
    Q_UNUSED(object)
    Q_UNUSED(errmess)
    Q_UNUSED(forceType)
    QMutexLocker locker(&mutex);

    QString key = pv;
    int pos = key.indexOf(".{");
    if (pos != -1) key.truncate(pos);
    // caQtDM delivers widget texts latin1 encoded (qasc); Qt6 would decode char* as
    // utf8 and break the umlauts of the .pathloss enum strings
    QString text = QString::fromLatin1(sdata).trimmed();

    qCDebug(bleaconLog) << "bleaconPlugin:pvSetValue" << key << rdata << idata << text;

    if (key.compare("scan.enable", Qt::CaseInsensitive) == 0) {
        int mode;
        if (!text.isEmpty()) {
            // enum text from the display, or a numeric string
            int textIndex = -1;
            for (int i = 0; i < scanStrings.size(); i++) {
                if (text.compare(scanStrings.at(i), Qt::CaseInsensitive) == 0) { textIndex = i; break; }
            }
            if (textIndex >= 0) mode = textIndex;
            else mode = qBound(0, text.toInt(), scanStrings.size() - 1);
        } else {
            mode = qBound(0, (int) idata, scanStrings.size() - 1);
        }
        applyScanMode(mode);
    } else if (key.compare("beacons.reset", Qt::CaseInsensitive) == 0) {
        // clear the discovery list: drop unmonitored beacons (named ones from the
        // config file stay), reset the remaining ones
        QMutableMapIterator<QString, BeaconState> it(beacons);
        while (it.hasNext()) {
            it.next();
            bool monitored = !it.value().name.isEmpty();
            foreach (const QString &monitorPv, monitorsByPv.uniqueKeys()) {
                if (monitored) break;
                if (monitorPv.startsWith(it.key() + ".")) { monitored = true; break; }
            }
            if (!monitored) {
                it.remove();
            } else {
                BeaconState &beacon = it.value();
                beacon.ewmaRssi = qQNaN();
                beacon.distance = qQNaN();
                beacon.battery = qQNaN();
                beacon.temperature = qQNaN();
                beacon.lastSeenMs = 0;
                beacon.readCounter = 0;
                beacon.lostCounter = 0;
                beacon.status = StatusNeverseen;
                pushBeaconState(it.key(), beacon);
            }
        }
        nearestKey.clear();
        nearestHoldCount = 0;
        updateChannelString("nearest.id", "");
        updateChannelString("nearest.name", "");
        updateChannelLong("nearest.valid", 0);
        pushBeaconList();
    } else if (key.compare("beacons.config", Qt::CaseInsensitive) == 0) {
        writeConfigFile();
    } else if (key.compare("beacons.writepath", Qt::CaseInsensitive) == 0) {
        configWritePath = text;
        updateChannelString("beacons.writepath", configWritePath);
        qCDebug(bleaconLog) << "bleacon: config write path" << configWritePath;
    } else {
        // per beacon writes: <addr|alias>.calibrate / .txpower / .pathloss
        static const QRegularExpression setRegex("^(.+)\\.(calibrate|txpower|pathloss)$");
        QRegularExpressionMatch setMatch = setRegex.match(key);
        if (setMatch.hasMatch()) {
            QString beaconKeyResolved = resolveBeaconKey(setMatch.captured(1));
            if (!beaconKeyResolved.isEmpty() && beacons.contains(beaconKeyResolved)) {
                BeaconState &beacon = beacons[beaconKeyResolved];
                QString field = setMatch.captured(2);

                if (field == "calibrate") {
                    bool start = !text.isEmpty() ? !(text == "0" || text.compare("idle", Qt::CaseInsensitive) == 0)
                                                 : ((idata != 0) || (rdata != 0.0));
                    if (start && !beacon.calibrating) {
                        beacon.calibrating = true;
                        beacon.calStartMs = QDateTime::currentMSecsSinceEpoch();
                        beacon.calSamples.clear();
                        updateChannelLong(beaconKeyResolved + ".calprogress", 0);
                        updateChannelEnum(beaconKeyResolved + ".calibrate", CAL_RUNNING, calStrings);
                        qCDebug(bleaconLog) << "bleacon: calibration started" << beaconKeyResolved;
                    } else if (!start && beacon.calibrating) {
                        beacon.calibrating = false;
                        updateChannelEnum(beaconKeyResolved + ".calibrate", CAL_IDLE, calStrings);
                    }
                } else if (field == "txpower") {
                    int tx = !text.isEmpty() ? text.toInt() : (int) idata;
                    if (tx < 0) {
                        beacon.txPowerOverride = tx;
                        beacon.txPower = tx;
                        updateChannelLong(beaconKeyResolved + ".txpower", beacon.txPower);
                        writeConfigFile();
                    }
                } else if (field == "pathloss") {
                    // environment scenario: enum text from a caMenu, an index, or a raw n value
                    int scenarioIndex = -1;
                    if (!text.isEmpty()) {
                        for (int i = 0; i < pathLossStrings.size(); i++) {
                            if (text.compare(pathLossStrings.at(i), Qt::CaseInsensitive) == 0) { scenarioIndex = i; break; }
                        }
                        if (scenarioIndex < 0) {
                            // robust against any string mangling on the way: the ascii
                            // "n=<value>" inside the enum text survives every encoding
                            static const QRegularExpression nRegex("n=([0-9]+\\.?[0-9]*)");
                            QRegularExpressionMatch nMatch = nRegex.match(text);
                            bool okN = false;
                            double n = nMatch.hasMatch() ? nMatch.captured(1).toDouble(&okN) : text.toDouble(&okN);
                            if (okN && n > 0.5 && n < 6.0) {
                                beacon.pathLossN = n;
                                scenarioIndex = bleaconPathLossIndex(n);
                            }
                        } else {
                            beacon.pathLossN = bleaconPathLossValues[scenarioIndex];
                        }
                    } else {
                        scenarioIndex = qBound(0, (int) idata, pathLossStrings.size() - 1);
                        beacon.pathLossN = bleaconPathLossValues[scenarioIndex];
                    }
                    if (scenarioIndex >= 0) {
                        updateChannelEnum(beaconKeyResolved + ".pathloss", scenarioIndex, pathLossStrings);
                        writeConfigFile();
                    }
                }
            }
        }
    }

    return true;
}

// caQtDM_Lib will call this routine for setting waveforms data
int bleaconPlugin::pvSetWave(char *pv, float *fdata, double *ddata, int16_t *data16, int32_t *data32, char *sdata, int nelm, char *object, char *errmess)
{
    Q_UNUSED(pv);
    Q_UNUSED(fdata);
    Q_UNUSED(ddata);
    Q_UNUSED(data16);
    Q_UNUSED(data32);
    Q_UNUSED(sdata);
    Q_UNUSED(nelm);
    Q_UNUSED(object);
    Q_UNUSED(errmess);
    return true;
}

// caQtDM_Lib will call this routine for getting the timestamp for this monitor
int bleaconPlugin::pvGetTimeStamp(char *pv, char *timestamp)
{
    QMutexLocker locker(&mutex);
    QString key = QString(pv);
    int pos = key.indexOf('.');
    if (pos != -1) key.truncate(pos);
    // alias or plain <major>:<minor> shorthand (hex), normalize to the canonical key
    if (keyByAlias.contains(key)) {
        key = keyByAlias.value(key);
    } else if (!key.startsWith("ibeacon:") && !key.startsWith("eddystone:") && key.contains(':')) {
        key = beaconKey("ibeacon", normalizeIBeaconId(key));
    } else if (key.startsWith("ibeacon:")) {
        key = beaconKey("ibeacon", normalizeIBeaconId(key.mid(8)));
    }

    QString stamp = "";
    if (beacons.contains(key) && beacons.value(key).lastSeenMs > 0) {
        stamp = QDateTime::fromMSecsSinceEpoch(beacons.value(key).lastSeenMs).toString("hh:mm:ss.zzz");
        stamp = "last sighting at " + stamp;
    }
    qstrncpy(timestamp, stamp.toLatin1().constData(), TIMESTAMP_STRING_LENGTH);
    return true;
}

// caQtDM_Lib will call this routine for getting a description of the monitor
int bleaconPlugin::pvGetDescription(char *pv, char *description)
{
    QString data = "bleacon beacon channel (ibeacon/eddystone): " + QString(pv);
    qstrncpy(description, data.toLatin1().constData(), MAX_STRING_LENGTH);
    return true;
}

// next two routines are used to stop and restart the monitoring (used in case of tabWidgets in the display)
int bleaconPlugin::pvClearEvent(void *ptr)
{
    Q_UNUSED(ptr);
    qCDebug(bleaconLog) << "bleaconPlugin:pvClearEvent";
    return true;
}

int bleaconPlugin::pvAddEvent(void *ptr)
{
    Q_UNUSED(ptr);
    qCDebug(bleaconLog) << "bleaconPlugin:pvAddEvent";
    return true;
}

// next two routines are used to connect and disconnect monitors when the application gets suspended and reactivated
int bleaconPlugin::pvReconnect(knobData *kData)
{
    Q_UNUSED(kData);
    qCDebug(bleaconLog) << "bleaconPlugin:pvReconnect";
    QMutexLocker locker(&mutex);
    // resume with the mode active before the suspend (known survives a suspend/resume)
    if (!scanning && !monitorsByPv.isEmpty()) applyScanMode((scanMode == ScanStop) ? ScanStart : scanMode);
    return true;
}

int bleaconPlugin::pvDisconnect(knobData *kData)
{
    Q_UNUSED(kData);
    qCDebug(bleaconLog) << "bleaconPlugin:pvDisconnect";
    return true;
}

// flush any io is periodically called (1s timer)
int bleaconPlugin::FlushIO()
{
    return true;
}

// termination, called when the application gets deactivated (reactivation through pvReconnect)
int bleaconPlugin::TerminateIO()
{
    qCDebug(bleaconLog) << "bleaconPlugin:TerminateIO";
    QMutexLocker locker(&mutex);
    // suspend the backend but keep scanMode, so pvReconnect can restore it
    if (scanning) {
        scanning = false;
        scanner->stopScan();
        sweepTimer->stop();
    }
    return true;
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#else
    Q_EXPORT_PLUGIN2(bleaconPlugin, bleaconPlugin)
#endif
