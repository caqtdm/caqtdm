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
#include <QDateTime>
#include <QDebug>

#include "internal_plugin.h"
#include "caQtDM_Plugins_global.h"

// base interval of the publish timer in ms
#define INTERNAL_BASE_INTERVAL 100

Q_LOGGING_CATEGORY(internalLog, "caqtdm.plugins.internal")

QString InternalPlugin::pluginName()
{
    return "internal";
}

InternalPlugin::InternalPlugin()
{
    qCDebug(internalLog) << "InternalPlugin: Create";
    mutexknobdataP = Q_NULLPTR;
    messagewindowP = Q_NULLPTR;
    timer = Q_NULLPTR;
}

InternalPlugin::~InternalPlugin()
{
    qDeleteAll(channels);
    channels.clear();
}

InternalChannel *InternalPlugin::channel(const QString &baseName)
{
    QMutexLocker locker(&mutex);
    return channels.value(baseName, Q_NULLPTR);
}

int InternalPlugin::initCommunicationLayer(MutexKnobData *data, MessageWindow *messageWindow, QMap<QString, QString> options)
{
    Q_UNUSED(options);
    qCDebug(internalLog) << "InternalPlugin: InitCommunicationLayer";

    mutexknobdataP = data;
    messagewindowP = messageWindow;

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateChannels()));
    timer->start(INTERNAL_BASE_INTERVAL);

    return true;
}

// timer slot: advance all counters and publish what has changed
void InternalPlugin::updateChannels()
{
    QMutexLocker locker(&mutex);
    QMapIterator<QString, InternalChannel *> i(channels);
    while(i.hasNext()) {
        i.next();
        InternalChannel *channel = i.value();
        channel->advance(INTERNAL_BASE_INTERVAL);
        if(channel->needsPublish) {
            publishChannel(i.key(), channel);
        }
    }
}

// publish the current value of a channel; field monitors are only triggered
// when their field value really changed
void InternalPlugin::publishChannel(const QString &key, InternalChannel *channel)
{
    channel->needsPublish = false;
    foreach(int index, monitorIndexes.value(key)) {
        InternalChannel::Field field = monitorFields.value(index, InternalChannel::FieldVal);
        if(field != InternalChannel::FieldVal) {
            if(channel->isConfigured()) {
                QVariant current = channel->fieldVariant(field);
                if(lastPublishedField.contains(index) && (lastPublishedField.value(index) == current)) continue;
                lastPublishedField.insert(index, current);
            } else {
                // unconfigured publishes are the disconnected state; without a
                // baseline every field monitor republishes once the
                // configuration (the channel's connect) arrives
                lastPublishedField.remove(index);
            }
        }
        publishIndex(channel, index);
    }
    channel->controlInfoChanged = false;
}

void InternalPlugin::publishIndex(InternalChannel *channel, int index)
{
    if(mutexknobdataP == (MutexKnobData *) Q_NULLPTR) return;
    knobData *kData = mutexknobdataP->GetMutexKnobDataPtr(index);
    if((kData == (knobData *) Q_NULLPTR) || (kData->index == -1)) return;

    bool lockable = (kData->mutex != (void *) Q_NULLPTR);
    if(lockable) mutexknobdataP->DataLock(kData);
    // a changed limit/precision/units re-arms initialize so caqtdm_lib
    // re-evaluates channel-derived widget limits, like a real EPICS
    // DBE_PROPERTY update does (see epics3's displayCallback)
    if(channel->controlInfoChanged) {
        kData->edata.initialize = true;
        qCDebug(internalLog) << "control info changed, forcing initialize=true for index" << index;
    }
    channel->fillKnobDataField(kData, monitorFields.value(index, InternalChannel::FieldVal));
    kData->edata.monitorCount++;
    kData->edata.actTimeMs = QDateTime::currentMSecsSinceEpoch();
    mutexknobdataP->SetMutexKnobData(kData->index, *kData);
    mutexknobdataP->SetMutexKnobDataReceived(kData);
    if(lockable) mutexknobdataP->DataUnlock(kData);
}

// where a channel definition comes from, for the duplicate definition message
static QString definitionOrigin(knobData *kData, QObject *widget)
{
    QString name = (widget != (QObject *) Q_NULLPTR) ? widget->objectName() : QString();
    QString origin = QString("widget '%1'").arg(name.isEmpty() ? QString("genSoftPV") : name);
    if(kData->fileName[0] != '\0') origin += QString(" in %1").arg(QString::fromLatin1(kData->fileName));
    return origin;
}

int InternalPlugin::pvAddMonitor(int index, knobData *kData, int rate, int skip)
{
    Q_UNUSED(index);
    Q_UNUSED(rate);
    Q_UNUSED(skip);
    QMutexLocker locker(&mutex);

    QString pv = QString::fromLatin1(kData->pv);
    QString key;
    InternalChannel::Field field;

    qCDebug(internalLog) << "pvAddMonitor" << pv << kData->index;

    // unknown extensions are not handled at all
    if(!InternalChannel::splitField(pv, &key, &field)) {
        char asc[MAX_STRING_LENGTH];
        snprintf(asc, MAX_STRING_LENGTH, "internal plugin: unknown field in %s, channel not handled", qasc(pv));
        if(messagewindowP != (MessageWindow *) Q_NULLPTR) messagewindowP->postMsgEvent(QtWarningMsg, asc);
        qCWarning(internalLog) << asc;
        return false;
    }

    // a JSON suffix in the channel name (epics filters, caqtdm_monitor) is no configuration here
    if(!InternalChannel::jsonPart(pv).isEmpty()) {
        qCDebug(internalLog) << "json suffix of" << pv
                             << "ignored, internal channels are configured through a genSoftPV widget";
    }

    InternalChannel *channel = channels.value(key, Q_NULLPTR);
    if(channel == Q_NULLPTR) {
        channel = new InternalChannel();
        channels.insert(key, channel);
    }

    // the configuration comes from the channelConfigJSON property of the defining
    // widget (genSoftPV); field monitors never configure the channel
    if(field == InternalChannel::FieldVal) {
        QObject *object = (QObject *) kData->dispW;
        QString config = (object != (QObject *) Q_NULLPTR) ? object->property("channelConfigJSON").toString() : QString();
        if(!config.isEmpty()) {
            QString origin = definitionOrigin(kData, object);
            char asc[MAX_STRING_LENGTH];
            if(!channel->isConfigured()) {
                QString error;
                if(channel->configure(config, &error)) {
                    channel->definedBy = origin;
                } else {
                    snprintf(asc, MAX_STRING_LENGTH, "internal plugin: invalid configuration for %s by %s (%s), channel stays unconnected",
                             qasc(key), qasc(origin), qasc(error));
                    if(messagewindowP != (MessageWindow *) Q_NULLPTR) messagewindowP->postMsgEvent(QtWarningMsg, asc);
                    qCWarning(internalLog) << asc;
                }
            } else if(InternalChannel::normalizedJson(config) != channel->configJson) {
                // first definition wins; a different later one is only reported
                snprintf(asc, MAX_STRING_LENGTH, "internal plugin: channel %s is already defined by %s, the different definition of %s is ignored",
                         qasc(key), qasc(channel->definedBy), qasc(origin));
                if(messagewindowP != (MessageWindow *) Q_NULLPTR) messagewindowP->postMsgEvent(QtInfoMsg, asc);
                qCDebug(internalLog) << asc;
            } else {
                qCDebug(internalLog) << "channel" << key << "same definition by" << origin << "ignored";
            }
        }
    }

    monitorIndexes[key].append(kData->index);
    monitorFields.insert(kData->index, field);
    channel->needsPublish = true;

    return true;
}

int InternalPlugin::pvClearMonitor(knobData *kData)
{
    QMutexLocker locker(&mutex);
    QString key = InternalChannel::baseName(QString::fromLatin1(kData->pv));
    if(key.isEmpty()) return true;

    qCDebug(internalLog) << "pvClearMonitor" << kData->pv << kData->index;

    monitorFields.remove(kData->index);
    lastPublishedField.remove(kData->index);
    QMap<QString, QList<int> >::iterator i = monitorIndexes.find(key);
    if(i != monitorIndexes.end()) {
        i.value().removeAll(kData->index);
        if(i.value().isEmpty()) {
            monitorIndexes.erase(i);
            // reference count dropped to zero: delete the channel, unless it is persistent
            InternalChannel *channel = channels.value(key, Q_NULLPTR);
            if((channel != Q_NULLPTR) && !channel->persistent) {
                channels.remove(key);
                delete channel;
                qCDebug(internalLog) << "channel deleted (unreferenced)" << key;
            }
        }
    }
    return true;
}

int InternalPlugin::pvFreeAllocatedData(knobData *kData)
{
    qCDebug(internalLog) << "pvFreeAllocatedData";
    if(kData->edata.info != (void *) Q_NULLPTR) {
        free(kData->edata.info);
        kData->edata.info = (void *) Q_NULLPTR;
    }
    if(kData->edata.dataB != (void *) Q_NULLPTR) {
        free(kData->edata.dataB);
        kData->edata.dataB = (void *) Q_NULLPTR;
        kData->edata.dataSize = 0;
    }
    return true;
}

int InternalPlugin::setValueForPv(const QString &pv, double rdata, int32_t idata, char *sdata)
{
    QMutexLocker locker(&mutex);
    QString key;
    InternalChannel::Field field;
    if(!InternalChannel::splitField(pv, &key, &field)) return false;
    InternalChannel *channel = channels.value(key, Q_NULLPTR);
    if(channel == Q_NULLPTR) return false;
    // an unconfigured channel is a disconnected shell, writes are rejected
    if(!channel->isConfigured()) return false;

    channel->setFieldValue(field, rdata, idata,
                           (sdata != (char *) Q_NULLPTR) ? QString::fromLatin1(sdata) : QString());
    publishChannel(key, channel);
    return true;
}

int InternalPlugin::pvSetValue(char *pv, double rdata, int32_t idata, char *sdata, char *object, char *errmess, int forceType)
{
    Q_UNUSED(object);
    Q_UNUSED(errmess);
    Q_UNUSED(forceType);
    qCDebug(internalLog) << "pvSetValue" << pv << rdata << idata << sdata;
    return setValueForPv(QString::fromLatin1(pv), rdata, idata, sdata);
}

bool InternalPlugin::pvSetValue(knobData *kData, double rdata, int32_t idata, char *sdata, char *object, char *errmess, int forceType)
{
    Q_UNUSED(object);
    Q_UNUSED(errmess);
    Q_UNUSED(forceType);
    return setValueForPv(QString::fromLatin1(kData->pv), rdata, idata, sdata) != 0;
}

int InternalPlugin::setWaveForPv(const QString &pv, float *fdata, double *ddata, int16_t *data16, int32_t *data32, int nelm)
{
    QMutexLocker locker(&mutex);
    QString key;
    InternalChannel::Field field;
    if(!InternalChannel::splitField(pv, &key, &field)) return false;
    if(field != InternalChannel::FieldVal) return false;
    InternalChannel *channel = channels.value(key, Q_NULLPTR);
    if(channel == Q_NULLPTR) return false;
    if(!channel->isConfigured()) return false;

    QVector<double> values;
    values.reserve(nelm);
    for(int i = 0; i < nelm; i++) {
        if(ddata != (double *) Q_NULLPTR)       values.append(ddata[i]);
        else if(fdata != (float *) Q_NULLPTR)   values.append((double) fdata[i]);
        else if(data32 != (int32_t *) Q_NULLPTR) values.append((double) data32[i]);
        else if(data16 != (int16_t *) Q_NULLPTR) values.append((double) data16[i]);
    }
    channel->setWave(values);
    publishChannel(key, channel);
    return true;
}

int InternalPlugin::pvSetWave(char *pv, float *fdata, double *ddata, int16_t *data16, int32_t *data32, char *sdata, int nelm, char *object, char *errmess)
{
    Q_UNUSED(sdata);
    Q_UNUSED(object);
    Q_UNUSED(errmess);
    qCDebug(internalLog) << "pvSetWave" << pv << nelm;
    return setWaveForPv(QString::fromLatin1(pv), fdata, ddata, data16, data32, nelm);
}

bool InternalPlugin::pvSetWave(knobData *kData, float *fdata, double *ddata, int16_t *data16, int32_t *data32, char *sdata, int nelm, char *object, char *errmess)
{
    Q_UNUSED(sdata);
    Q_UNUSED(object);
    Q_UNUSED(errmess);
    return setWaveForPv(QString::fromLatin1(kData->pv), fdata, ddata, data16, data32, nelm) != 0;
}

int InternalPlugin::pvGetTimeStamp(char *pv, char *timestamp)
{
    Q_UNUSED(pv);
    QString now = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    qstrncpy(timestamp, qasc(now), TIMESTAMP_STRING_LENGTH);
    return true;
}

int InternalPlugin::pvGetDescription(char *pv, char *description)
{
    InternalChannel *chan = channel(InternalChannel::baseName(QString::fromLatin1(pv)));
    if(chan != Q_NULLPTR && chan->isMatrix()) {
        snprintf(description, MAX_STRING_LENGTH, "internal test/simulation channel (matrix %dx%d, row-major)",
                 chan->dimRows, chan->dimCols);
        return true;
    }
    qstrncpy(description, "internal test/simulation channel", MAX_STRING_LENGTH);
    return true;
}

int InternalPlugin::pvClearEvent(void *ptr)
{
    Q_UNUSED(ptr);
    return true;
}

int InternalPlugin::pvAddEvent(void *ptr)
{
    Q_UNUSED(ptr);
    return true;
}

int InternalPlugin::pvReconnect(knobData *kData)
{
    Q_UNUSED(kData);
    return true;
}

int InternalPlugin::pvDisconnect(knobData *kData)
{
    Q_UNUSED(kData);
    return true;
}

int InternalPlugin::FlushIO()
{
    return true;
}

int InternalPlugin::TerminateIO()
{
    qCDebug(internalLog) << "InternalPlugin: TerminateIO";
    if(timer) timer->stop();
    return true;
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    Q_EXPORT_PLUGIN2(InternalPlugin, InternalPlugin)
#endif
