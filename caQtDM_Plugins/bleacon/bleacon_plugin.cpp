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
    nearestHoldCount = 0;
    pathLossExponent = BLEACON_DEFAULT_PATHLOSS_N;
    staleTimeoutSec = BLEACON_DEFAULT_TIMEOUT_S;
    weakRssiThreshold = BLEACON_DEFAULT_WEAK_RSSI;
    configFileName = BLEACON_DEFAULT_CONFIG_FILE;
    statusStrings << "neverseen" << "lost" << "weak" << "present";
    scanStrings << "stop" << "start";
}

bleaconPlugin::~bleaconPlugin()
{
}

QString bleaconPlugin::beaconKey(quint16 major, quint16 minor)
{
    return QString("%1:%2").arg(major).arg(minor);
}

bleaconPlugin::BeaconState *bleaconPlugin::findOrCreateBeacon(quint16 major, quint16 minor, const QUuid &uuid)
{
    QString key = beaconKey(major, minor);
    QMap<QString, BeaconState>::iterator it = beacons.find(key);
    if (it == beacons.end()) {
        BeaconState b;
        b.major = major;
        b.minor = minor;
        b.uuid = uuid;
        b.ewmaRssi = qQNaN();
        b.distance = qQNaN();
        b.txPower = 0;
        b.lastSeenMs = 0;
        b.readCounter = 0;
        b.lostCounter = 0;
        b.status = StatusNeverseen;
        it = beacons.insert(key, b);
    }
    return &it.value();
}

// ------------------------------------------------------------------ configuration

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

    // uuids from the environment / options, ';' separated
    QStringList uuidStrings;
    value = (QString) qgetenv("CAQTDM_BLEACON_UUIDS");
    if (!value.isEmpty()) uuidStrings.append(value.split(";"));
    if (!options.value("bleacon_uuids", "").isEmpty()) uuidStrings.append(options.value("bleacon_uuids").split(";"));

    // uuids from the config file, searched and downloaded like the modbus database file
    value = (QString) qgetenv("CAQTDM_BLEACON_CONFIG");
    if (!options.value("bleacon_config", "").isEmpty()) value = options.value("bleacon_config");
    if (!value.isEmpty()) configFileName = value;

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
                uuidStrings.append(line);
            }
            file.close();
        }
    }

    foreach (const QString &uuidString, uuidStrings) {
        QUuid uuid(uuidString.trimmed());
        if (uuid.isNull()) {
            if (!uuidString.trimmed().isEmpty()) {
                QString msg = "bleacon: invalid uuid ignored : " + uuidString.trimmed();
                if (messagewindowP != Q_NULLPTR) messagewindowP->postMsgEvent(QtWarningMsg, (char *) msg.toLatin1().constData());
            }
            continue;
        }
        if (!uuidList.contains(uuid)) uuidList.append(uuid);
    }

    qCDebug(bleaconLog) << "bleacon: configuration" << uuidList << pathLossExponent << staleTimeoutSec << weakRssiThreshold;
}

// write the (minimal) config file: one proximity uuid per line, generated from the
// configured and the sighted uuids - lets the plugin create its own configuration
// during development (bleacon://beacons.config)
void bleaconPlugin::writeConfigFile()
{
    QList<QUuid> allUuids = uuidList;
    QMap<QString, BeaconState>::const_iterator it;
    for (it = beacons.constBegin(); it != beacons.constEnd(); ++it) {
        if (!it.value().uuid.isNull() && !allUuids.contains(it.value().uuid)) allUuids.append(it.value().uuid);
    }

    // overwrite an existing config file, otherwise create it in the current directory
    searchFile *s = new searchFile(configFileName);
    QString fileName = s->findFile();
    delete s;
    if (fileName.isEmpty()) fileName = QDir::currentPath() + "/" + configFileName;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QString msg = "bleacon: could not write config file : " + fileName;
        if (messagewindowP != Q_NULLPTR) messagewindowP->postMsgEvent(QtCriticalMsg, (char *) msg.toLatin1().constData());
        return;
    }
    QTextStream out(&file);
    out << "# bleacon proximity uuid list, one uuid per line\n";
    foreach (const QUuid &uuid, allUuids) {
        out << uuid.toString(QUuid::WithoutBraces).toUpper() << "\n";
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
}

// discovery channel: one channel base per line, ready to be used in a display
void bleaconPlugin::pushBeaconList()
{
    QStringList list;
    QMap<QString, BeaconState>::const_iterator it;
    for (it = beacons.constBegin(); it != beacons.constEnd(); ++it) {
        list.append("bleacon://" + it.key());
    }
    updateChannelString("beacons.list", list.join("\n"));
}

// ------------------------------------------------------------------ scanning

void bleaconPlugin::startScanning()
{
    if (scanning) return;
    if (scanner == Q_NULLPTR) return;
    qCDebug(bleaconLog) << "bleacon: start scanning";
    scanning = true;
    scanner->startScan(uuidList);
    sweepTimer->start(BLEACON_SWEEP_MS);
    updateChannelEnum("scan.enable", 1, scanStrings);
}

void bleaconPlugin::stopScanning()
{
    if (!scanning) return;
    qCDebug(bleaconLog) << "bleacon: stop scanning";
    scanning = false;
    scanner->stopScan();
    sweepTimer->stop();
    updateChannelEnum("scan.enable", 0, scanStrings);
}

void bleaconPlugin::scannerMessage(QString message, bool isError)
{
    qCDebug(bleaconLog) << "bleacon:" << message;
    if (messagewindowP != Q_NULLPTR) {
        messagewindowP->postMsgEvent(isError ? QtCriticalMsg : QtDebugMsg, (char *) message.toLatin1().constData());
    }
}

void bleaconPlugin::beaconSighting(QUuid uuid, quint16 major, quint16 minor, int rssi, int txPower, double accuracyMeters)
{
    QMutexLocker locker(&mutex);
    if (!scanning) return;

    QString key = beaconKey(major, minor);
    bool isNew = !beacons.contains(key);
    BeaconState *beacon = findOrCreateBeacon(major, minor, uuid);
    if (beacon->uuid.isNull()) beacon->uuid = uuid;
    if (txPower != 0) beacon->txPower = txPower;

    // exponentially weighted moving average against the +-6dB advertisement noise
    if (qIsNaN(beacon->ewmaRssi)) beacon->ewmaRssi = rssi;
    else beacon->ewmaRssi = 0.3 * rssi + 0.7 * beacon->ewmaRssi;

    // distance: the ios backend delivers a ready estimate (CLBeacon.accuracy),
    // otherwise log-distance path loss model from the smoothed rssi
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
        qCDebug(bleaconLog) << "bleacon: new beacon" << key << uuid;
        pushBeaconList();
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
    connect(scanner, SIGNAL(beaconSighting(QUuid,quint16,quint16,int,int,double)),
            this, SLOT(beaconSighting(QUuid,quint16,quint16,int,int,double)));
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

    static const QRegularExpression beaconRegex("^(\\d{1,5}):(\\d{1,5})\\.(rssi|distance|txpower|age|status|readcounter|lostcounter)$");
    static const QRegularExpression nearestRegex("^nearest\\.(id|major|minor|distance|rssi|valid)$");
    static const QRegularExpression beaconsRegex("^beacons\\.(list|reset|config)$");

    kData->edata.precision = 0;
    kData->edata.connected = true;
    kData->edata.accessR = true;
    kData->edata.accessW = false;

    QRegularExpressionMatch match = beaconRegex.match(pv);
    if (match.hasMatch()) {
        quint16 major = (quint16) match.captured(1).toUInt();
        quint16 minor = (quint16) match.captured(2).toUInt();
        QString field = match.captured(3);
        BeaconState *beacon = findOrCreateBeacon(major, minor, QUuid());
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
        }
    } else if (nearestRegex.match(pv).hasMatch()) {
        QString field = nearestRegex.match(pv).captured(1);
        if (field == "id") {
            fillStringData(kData, nearestKey);
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
        } else {
            // reset, config: write channels
            kData->edata.fieldtype = caLONG;
            kData->edata.accessW = true;
        }
    } else if (pv == "scan.enable") {
        fillEnumData(kData, scanning ? 1 : 0, scanStrings);
        kData->edata.accessW = true;
    } else {
        QString msg = "bleacon: invalid channel : " + pv;
        if (messagewindowP != Q_NULLPTR) messagewindowP->postMsgEvent(QtCriticalMsg, (char *) msg.toLatin1().constData());
        return false;
    }

    monitorsByPv.insert(pv, kData->index);
    pushKnobData(kData);

    if (!scanning) startScanning();

    return true;
}

// caQtDM_Lib will call this routine for getting rid of a monitor
int bleaconPlugin::pvClearMonitor(knobData *kData)
{
    QMutexLocker locker(&mutex);
    qCDebug(bleaconLog) << "bleaconPlugin:pvClearMonitor" << kData->pv << kData->index;

    QString pv = kData->pv;
    int pos = pv.indexOf(".{");
    if (pos != -1) pv.truncate(pos);

    monitorsByPv.remove(pv, kData->index);
    if (monitorsByPv.isEmpty()) stopScanning();

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
        bool enable;
        if (!text.isEmpty()) enable = (text.compare("start", Qt::CaseInsensitive) == 0) || (text == "1");
        else enable = (idata != 0) || (rdata != 0.0);
        if (enable) startScanning();
        else stopScanning();
    } else if (key.compare("beacons.reset", Qt::CaseInsensitive) == 0) {
        // clear the discovery list: drop unmonitored beacons, reset the monitored ones
        QMutableMapIterator<QString, BeaconState> it(beacons);
        while (it.hasNext()) {
            it.next();
            bool monitored = false;
            foreach (const QString &monitorPv, monitorsByPv.uniqueKeys()) {
                if (monitorPv.startsWith(it.key() + ".")) { monitored = true; break; }
            }
            if (!monitored) {
                it.remove();
            } else {
                BeaconState &beacon = it.value();
                beacon.ewmaRssi = qQNaN();
                beacon.distance = qQNaN();
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
        updateChannelLong("nearest.valid", 0);
        pushBeaconList();
    } else if (key.compare("beacons.config", Qt::CaseInsensitive) == 0) {
        writeConfigFile();
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
    QString data = "bleacon iBeacon channel: " + QString(pv);
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
    if (!scanning && !monitorsByPv.isEmpty()) startScanning();
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
    stopScanning();
    return true;
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#else
    Q_EXPORT_PLUGIN2(bleaconPlugin, bleaconPlugin)
#endif
