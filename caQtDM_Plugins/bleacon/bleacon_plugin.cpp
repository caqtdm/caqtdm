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

#include "bleacon_plugin.h"
#include "searchfile.h"
#include "fileFunctions.h"
#include "caQtDM_Plugins_global.h"

Q_LOGGING_CATEGORY(bleaconLog, "caqtdm.plugins.bleacon")

// as defined in knobDefines.h
//caType {caSTRING	= 0, caINT = 1, caFLOAT = 2, caENUM = 3, caCHAR = 4, caLONG = 5, caDOUBLE = 6};

#define BLEACON_DEFAULT_CONFIG_FILE "bleacon.config"
#define BLEACON_DEFAULT_PATHLOSS_N 2.2
#define BLEACON_DEFAULT_TIMEOUT_S 10
#define BLEACON_DEFAULT_WEAK_RSSI (-90)
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
    configFileName = BLEACON_DEFAULT_CONFIG_FILE;
    statusStrings << "neverseen" << "lost" << "weak" << "present";
    scanStrings << "stop" << "start" << "known";
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
        b.battery = qQNaN();
        b.temperature = qQNaN();
        b.lastSeenMs = 0;
        b.readCounter = 0;
        b.lostCounter = 0;
        b.status = StatusNeverseen;
        it = beacons.insert(key, b);
    }
    return &it.value();
}

// ------------------------------------------------------------------ configuration

// config entries are auto-detected by format: "Name=Adresse" defines a human readable
// alias (like the modbus/opcua translation), a beacon address (ibeacon:/eddystone:)
// pre-creates the beacon in the discovery list, a parsable uuid is an iBeacon
// proximity uuid, 20 hex characters are an eddystone namespace
void bleaconPlugin::addConfigEntry(const QString &entry)
{
    QString trimmed = entry.trimmed();
    if (trimmed.isEmpty()) return;

    static const QRegularExpression addressRegex("^(?:(ibeacon):([0-9A-Fa-f]{1,4}:[0-9A-Fa-f]{1,4})|(eddystone):([0-9A-Fa-f]{12}))$");

    // alias definition: Name=Adresse
    int eq = trimmed.indexOf('=');
    if (eq > 0) {
        QString alias = trimmed.left(eq).trimmed();
        QString address = trimmed.mid(eq + 1).trimmed();
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
        return;
    }

    QRegularExpressionMatch addressMatch = addressRegex.match(trimmed);
    if (addressMatch.hasMatch()) {
        if (!addressMatch.captured(1).isEmpty()) findOrCreateBeacon("ibeacon", normalizeIBeaconId(addressMatch.captured(2)), QString());
        else findOrCreateBeacon("eddystone", addressMatch.captured(4).toUpper(), QString());
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
        if (!it.value().name.isEmpty()) out << it.value().name << "=" << it.key() << "\n";
        else out << it.key() << "\n";
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
    double age = (beacon.lastSeenMs > 0) ? (now - beacon.lastSeenMs) / 1000.0 : -1.0;

    if (!qIsNaN(beacon.ewmaRssi)) updateChannelDouble(key + ".rssi", beacon.ewmaRssi);
    if (!qIsNaN(beacon.distance)) updateChannelDouble(key + ".distance", beacon.distance);
    updateChannelLong(key + ".txpower", beacon.txPower);
    updateChannelDouble(key + ".age", age);
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
    if (txPowerAt1m != 0) beacon->txPower = txPowerAt1m;

    // exponentially weighted moving average against the +-6dB advertisement noise
    if (qIsNaN(beacon->ewmaRssi)) beacon->ewmaRssi = rssi;
    else beacon->ewmaRssi = 0.3 * rssi + 0.7 * beacon->ewmaRssi;

    // distance: the ios backend delivers a ready estimate (CLBeacon.accuracy), otherwise
    // log-distance path loss model from the smoothed rssi (txPower already at 1m,
    // the protocol specific conversion happened in the backend)
    if (!qIsNaN(accuracyMeters)) {
        if (qIsNaN(beacon->distance)) beacon->distance = accuracyMeters;
        else beacon->distance = 0.3 * accuracyMeters + 0.7 * beacon->distance;
    } else if (beacon->txPower != 0) {
        beacon->distance = pow(10.0, (beacon->txPower - beacon->ewmaRssi) / (10.0 * pathLossExponent));
    }

    beacon->lastSeenMs = QDateTime::currentMSecsSinceEpoch();
    beacon->readCounter++;
    beacon->status = (beacon->ewmaRssi < weakRssiThreshold) ? StatusWeak : StatusPresent;

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

        double age = (now - beacon.lastSeenMs) / 1000.0;
        updateChannelDouble(it.key() + ".age", age);

        if ((beacon.status == StatusPresent || beacon.status == StatusWeak) && age > staleTimeoutSec) {
            beacon.status = StatusLost;
            beacon.lostCounter++;
            qCDebug(bleaconLog) << "bleacon: lost" << it.key();
            updateChannelEnum(it.key() + ".status", beacon.status, statusStrings);
            updateChannelLong(it.key() + ".lostcounter", beacon.lostCounter);
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
                                                "\\.(rssi|distance|txpower|age|status|readcounter|lostcounter|battery|temperature|name)$");
    static const QRegularExpression nearestRegex("^nearest\\.(id|name|major|minor|distance|rssi|valid)$");
    static const QRegularExpression beaconsRegex("^beacons\\.(list|reset|config|writepath)$");
    static const QRegularExpression aliasRegex("^([A-Za-z_][A-Za-z0-9_-]*)"
                                               "\\.(rssi|distance|txpower|age|status|readcounter|lostcounter|battery|temperature|name)$");

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
            kData->edata.fieldtype = caLONG;
            qstrncpy(kData->edata.units, "dBm", 39);
            kData->edata.ivalue = beacon->txPower;
            kData->edata.rvalue = (double) beacon->txPower;
        } else if (field == "age") {
            kData->edata.fieldtype = caDOUBLE;
            kData->edata.precision = 1;
            qstrncpy(kData->edata.units, "s", 39);
            kData->edata.rvalue = (beacon->lastSeenMs > 0) ? (now - beacon->lastSeenMs) / 1000.0 : -1.0;
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
    QString text = QString(sdata).trimmed();

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
