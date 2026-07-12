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
#include "pvxs_plugin.h"

#include <cstdlib>
#include <cstring>

#include <db_access.h>

#include <pvxs/version.h>

#include <QDebug>

#include "caQtDM_Plugins_global.h"
#include "pvxsValueMapping.h"

using namespace pvxs;

Q_LOGGING_CATEGORY(pvxsLog, "caqtdm.plugins.pvxs")

QString PvxsPlugin::pluginName()
{
    return "pvxs";
}

PvxsPlugin::PvxsPlugin()
{
    qCDebug(pvxsLog) << "PvxsPlugin: create";
}

PvxsPlugin::~PvxsPlugin()
{
    TerminateIO();
}

void PvxsPlugin::logError(const QString &msg)
{
    qCWarning(pvxsLog) << msg;
    if (messageWindow) {
        QByteArray bytes = msg.toLatin1();
        messageWindow->postMsgEvent(QtWarningMsg, bytes.data());
    }
}

PvxsChannelPtr PvxsPlugin::channelFor(knobData *kData)
{
    if (!kData || !kData->edata.info) return nullptr;
    return *static_cast<PvxsChannelPtr *>(kData->edata.info);
}

int PvxsPlugin::initCommunicationLayer(MutexKnobData *data, MessageWindow *messageWindowP, QMap<QString, QString> options)
{
    Q_UNUSED(options);
    mutexKnobData = data;
    messageWindow = messageWindowP;

    QString msg = QString("PvxsPlugin: %1").arg(version_str());
    qCInfo(pvxsLog) << msg;
    if (messageWindow) {
        QByteArray bytes = msg.toLatin1();
        messageWindow->postMsgEvent(QtDebugMsg, bytes.data());
    }

    ctxt = client::Context::fromEnv();
    stopDrain = false;
    drainThread = std::thread(&PvxsPlugin::drainLoop, this);
    qCDebug(pvxsLog) << "initCommunicationLayer: context created, drain thread started";
    return true;
}

void PvxsPlugin::startMonitor(const PvxsChannelPtr &ch)
{
    qCDebug(pvxsLog) << "startMonitor" << QString::fromStdString(ch->pvName) << "index" << ch->index;

    // weak capture: the channel holds the Subscription which stores the
    // lambda - a strong capture would create a reference cycle
    std::weak_ptr<PvxsChannel> wch(ch);

    // monitor the full structure (no pvRequest restriction): display/control/valueAlarm
    // limit changes then arrive as PROPERTY deltas, like DBE_PROPERTY with epics3
    auto sub = ctxt.monitor(ch->pvName)
                   .maskConnected(false)
                   .maskDisconnected(false)
                   .event([this, wch](client::Subscription &) {
                       if (PvxsChannelPtr c = wch.lock()) workQueue.push(c);
                   })
                   .exec();

    std::lock_guard<std::mutex> lk(ch->opMutex);
    ch->subscription = sub;
}

void PvxsPlugin::stopMonitor(const PvxsChannelPtr &ch)
{
    qCDebug(pvxsLog) << "stopMonitor" << QString::fromStdString(ch->pvName);

    std::shared_ptr<client::Subscription> sub;
    std::shared_ptr<client::Operation> putOp;
    std::shared_ptr<client::Operation> nelmOp;
    {
        std::lock_guard<std::mutex> lk(ch->opMutex);
        sub = ch->subscription;
        ch->subscription.reset();
        putOp = ch->pendingPutOp;
        ch->pendingPutOp.reset();
        nelmOp = ch->pendingNelmGetOp;
        ch->pendingNelmGetOp.reset();
    }
    if (sub) sub->cancel();
    if (putOp) putOp->cancel();
    if (nelmOp) nelmOp->cancel();
}

int PvxsPlugin::pvAddMonitor(int index, knobData *kData, int rate, int skip)
{
    Q_UNUSED(rate);
    Q_UNUSED(skip);
    qCDebug(pvxsLog) << "pvAddMonitor" << kData->pv << kData->index;

    if (kData->edata.info != nullptr) {
        qCWarning(pvxsLog) << "pvAddMonitor: already added" << kData->pv;
        return true;
    }

    PvxsChannelPtr ch = std::make_shared<PvxsChannel>(std::string(kData->pv), index);
    kData->edata.info = new PvxsChannelPtr(ch);
    // propagate edata.info into the shared array slot, not just this local kData
    mutexKnobData->SetMutexKnobData(index, *kData);

    startMonitor(ch);
    return true;
}

int PvxsPlugin::pvClearMonitor(knobData *kData)
{
    qCDebug(pvxsLog) << "pvClearMonitor" << kData->pv;
    PvxsChannelPtr ch = channelFor(kData);
    if (!ch) return true;
    stopMonitor(ch);
    return true;
}

int PvxsPlugin::pvFreeAllocatedData(knobData *kData)
{
    qCDebug(pvxsLog) << "pvFreeAllocatedData" << kData->pv;
    auto *chPtr = static_cast<PvxsChannelPtr *>(kData->edata.info);
    if (chPtr) {
        stopMonitor(*chPtr);
        delete chPtr;
        kData->edata.info = nullptr;
    }
    if (kData->edata.dataB != nullptr) {
        free(kData->edata.dataB);
        kData->edata.dataB = nullptr;
    }
    return true;
}

bool PvxsPlugin::pvSetValue(knobData *kData, double rdata, int32_t idata, char *sdata,
                            char *object, char *errmess, int forceType)
{
    Q_UNUSED(object);
    Q_UNUSED(errmess);

    qCDebug(pvxsLog) << "pvSetValue" << kData->pv << "rdata" << rdata << "idata" << idata
                     << "sdata" << sdata << "forceType" << forceType;

    PvxsChannelPtr ch = channelFor(kData);
    if (!ch) {
        qCWarning(pvxsLog) << "pvSetValue: no channel for" << kData->pv;
        return false;
    }

    std::weak_ptr<PvxsChannel> wch(ch);
    auto resultCb = [this, wch](client::Result &&res) {
        PvxsChannelPtr c = wch.lock();
        try {
            res();
            if (c) qCDebug(pvxsLog) << "put ok" << QString::fromStdString(c->pvName);
        } catch (std::exception &e) {
            if (c) logError(QString("pvxs put %1: %2").arg(QString::fromStdString(c->pvName), QString(e.what())));
        }
    };

    std::shared_ptr<client::Operation> op;
    if (ch->isEnum) {
        // caMenu/caChoice deliver the choice label in sdata (idata is always 0);
        // resolve it against the fetched choices list like epics4 does.
        // cloneEmpty is essential: returning the fetched struct as-is would write
        // choices/alarm/timeStamp back and drive the record into an undefined state.
        std::string choice(sdata ? sdata : "");
        qCDebug(pvxsLog) << "pvSetValue: enum put, resolving choice" << choice.c_str();
        op = ctxt.put(ch->pvName)
                 .fetchPresent(true)
                 .build([choice](Value &&current) -> Value {
                     int index = pvxsValueMapping::enumIndexFor(current, choice);
                     if (index < 0)
                         throw std::runtime_error("no matching enum choice '" + choice + "'");
                     Value toput(current.cloneEmpty());
                     toput["value.index"] = index;
                     return toput;
                 })
                 .result(resultCb)
                 .exec();
    } else if (ch->fieldtype == DBF_STRING) {
        qCDebug(pvxsLog) << "pvSetValue: string put";
        op = ctxt.put(ch->pvName).set("value", std::string(sdata ? sdata : "")).result(resultCb).exec();
    } else if (ch->isArray && ch->fieldtype == DBF_CHAR) {
        // long string: write the text bytes (incl. NUL) into the char/byte waveform
        size_t len = strlen(sdata ? sdata : "") + 1;
        qCDebug(pvxsLog) << "pvSetValue: long-string put, length" << (int) len;
        shared_array<uint8_t> bytes(len);
        memcpy(bytes.data(), sdata ? sdata : "", len);
        op = ctxt.put(ch->pvName).set("value", bytes.freeze()).result(resultCb).exec();
    } else if (ch->fieldtype == DBF_DOUBLE) {
        qCDebug(pvxsLog) << "pvSetValue: double put" << rdata;
        op = ctxt.put(ch->pvName).set("value", rdata).result(resultCb).exec();
    } else {
        qCDebug(pvxsLog) << "pvSetValue: integer put" << idata << "(fieldtype" << ch->fieldtype << ")";
        op = ctxt.put(ch->pvName).set("value", idata).result(resultCb).exec();
    }

    std::lock_guard<std::mutex> lk(ch->opMutex);
    ch->pendingPutOp = op;
    return true;
}

bool PvxsPlugin::pvSetWave(knobData *kData, float *fdata, double *ddata, int16_t *data16,
                           int32_t *data32, char *sdata, int nelm, char *object, char *errmess)
{
    Q_UNUSED(object);
    Q_UNUSED(errmess);

    qCDebug(pvxsLog) << "pvSetWave" << kData->pv << "nelm" << nelm
                     << "source" << (ddata ? "double" : fdata ? "float" : data32 ? "int32" : data16 ? "int16" : sdata ? "string" : "none");

    PvxsChannelPtr ch = channelFor(kData);
    if (!ch) {
        qCWarning(pvxsLog) << "pvSetWave: no channel for" << kData->pv;
        return false;
    }

    if (ch->fieldtype == DBF_STRING) {
        logError(QString("pvxs put %1: string array put not supported").arg(QString::fromStdString(ch->pvName)));
        return false;
    }

    std::weak_ptr<PvxsChannel> wch(ch);
    auto resultCb = [this, wch](client::Result &&res) {
        PvxsChannelPtr c = wch.lock();
        try {
            res();
            if (c) qCDebug(pvxsLog) << "wave put ok" << QString::fromStdString(c->pvName);
        } catch (std::exception &e) {
            if (c) logError(QString("pvxs put %1: %2").arg(QString::fromStdString(c->pvName), QString(e.what())));
        }
    };

    std::shared_ptr<client::Operation> op;
    if (ddata || fdata || data32 || data16) {
        shared_array<double> arr(nelm);
        if (ddata) {
            for (int i = 0; i < nelm; i++) arr[i] = ddata[i];
        } else if (fdata) {
            for (int i = 0; i < nelm; i++) arr[i] = fdata[i];
        } else if (data32) {
            for (int i = 0; i < nelm; i++) arr[i] = data32[i];
        } else {
            for (int i = 0; i < nelm; i++) arr[i] = data16[i];
        }
        op = ctxt.put(ch->pvName).set("value", arr.freeze()).result(resultCb).exec();
    } else if (sdata) {
        // text into a char/byte (or other integer) waveform - long-string write
        size_t len = strlen(sdata) + 1;
        if (nelm > 0 && (size_t) nelm < len) len = (size_t) nelm;
        qCDebug(pvxsLog) << "pvSetWave: long-string put, length" << (int) len;
        shared_array<uint8_t> bytes(len);
        memcpy(bytes.data(), sdata, len);
        op = ctxt.put(ch->pvName).set("value", bytes.freeze()).result(resultCb).exec();
    } else {
        qCWarning(pvxsLog) << "pvSetWave: no source array for" << kData->pv;
        return false;
    }

    std::lock_guard<std::mutex> lk(ch->opMutex);
    ch->pendingPutOp = op;
    return true;
}

int PvxsPlugin::pvClearEvent(void *ptr)
{
    auto *chPtr = static_cast<PvxsChannelPtr *>(ptr);
    if (!chPtr || !*chPtr) return true;
    qCDebug(pvxsLog) << "pvClearEvent: pause" << QString::fromStdString((*chPtr)->pvName);
    std::shared_ptr<client::Subscription> sub;
    {
        std::lock_guard<std::mutex> lk((*chPtr)->opMutex);
        sub = (*chPtr)->subscription;
    }
    if (sub) sub->pause(true);
    return true;
}

int PvxsPlugin::pvAddEvent(void *ptr)
{
    auto *chPtr = static_cast<PvxsChannelPtr *>(ptr);
    if (!chPtr || !*chPtr) return true;
    qCDebug(pvxsLog) << "pvAddEvent: resume" << QString::fromStdString((*chPtr)->pvName);
    std::shared_ptr<client::Subscription> sub;
    {
        std::lock_guard<std::mutex> lk((*chPtr)->opMutex);
        sub = (*chPtr)->subscription;
    }
    if (sub) sub->pause(false);
    return true;
}

int PvxsPlugin::pvReconnect(knobData *kData)
{
    PvxsChannelPtr ch = channelFor(kData);
    if (!ch) {
        qCWarning(pvxsLog) << "pvReconnect: no channel for" << (kData ? kData->pv : "(null)");
        return false;
    }
    bool haveSubscription;
    {
        std::lock_guard<std::mutex> lk(ch->opMutex);
        haveSubscription = (bool) ch->subscription;
    }
    qCDebug(pvxsLog) << "pvReconnect" << QString::fromStdString(ch->pvName)
                     << (haveSubscription ? "subscription alive, nothing to do" : "re-subscribing");
    if (!haveSubscription) startMonitor(ch);
    return true;
}

int PvxsPlugin::pvDisconnect(knobData *kData)
{
    PvxsChannelPtr ch = channelFor(kData);
    if (!ch) {
        qCWarning(pvxsLog) << "pvDisconnect: no channel for" << (kData ? kData->pv : "(null)");
        return false;
    }
    qCDebug(pvxsLog) << "pvDisconnect" << QString::fromStdString(ch->pvName);
    stopMonitor(ch);
    return true;
}

int PvxsPlugin::FlushIO()
{
    return true;
}

int PvxsPlugin::TerminateIO()
{
    if (stopDrain.exchange(true)) return true; // already terminated
    qCDebug(pvxsLog) << "TerminateIO: stopping drain thread";
    workQueue.push(nullptr);
    if (drainThread.joinable()) drainThread.join();
    if (ctxt) ctxt.close(); // close() throws on an empty Context (init never ran)
    qCDebug(pvxsLog) << "TerminateIO: done";
    return true;
}

void PvxsPlugin::handleMonitorValue(const PvxsChannelPtr &ch, const Value &val)
{
    knobData kData = mutexKnobData->GetMutexKnobData(ch->index);
    if (kData.index == -1) return;
    bool wantNelm = false;
    mutexKnobData->DataLock(&kData);
    bool ok = pvxsValueMapping::fillValue(val, kData.edata);
    if (ok) {
        // full-structure monitor: limits/units/precision come with the value updates
        pvxsValueMapping::fillLimitsFromDisplayControl(val, kData.edata);
        kData.edata.connected = true;
        bool valueIsArray = val["value"].type().isarray();
        if (ch->fieldtype == -1) {
            qCDebug(pvxsLog) << "first update" << QString::fromStdString(ch->pvName)
                             << "fieldtype" << kData.edata.fieldtype
                             << "isEnum" << (kData.edata.fieldtype == DBF_ENUM)
                             << "isArray" << valueIsArray
                             << "valueCount" << kData.edata.valueCount
                             << "units" << kData.edata.units << "precision" << kData.edata.precision
                             << "disp" << kData.edata.lower_disp_limit << ".." << kData.edata.upper_disp_limit
                             << "ctrl" << kData.edata.lower_ctrl_limit << ".." << kData.edata.upper_ctrl_limit
                             << "warn" << kData.edata.lower_warning_limit << ".." << kData.edata.upper_warning_limit
                             << "alarm" << kData.edata.lower_alarm_limit << ".." << kData.edata.upper_alarm_limit;
            // char arrays double as long strings; caqtdm routes the string write
            // path on edata.nelm > 1, so fetch the record's real capacity once
            wantNelm = valueIsArray && (kData.edata.fieldtype == DBF_CHAR);
        }
        ch->fieldtype = kData.edata.fieldtype;
        ch->isEnum = (kData.edata.fieldtype == DBF_ENUM);
        ch->isArray = valueIsArray;
        mutexKnobData->SetMutexKnobDataReceived(&kData);
    } else {
        qCWarning(pvxsLog) << "monitor update without value field for" << QString::fromStdString(ch->pvName);
    }
    mutexKnobData->DataUnlock(&kData);
    if (wantNelm) fetchNelm(ch);
}

void PvxsPlugin::fetchNelm(const PvxsChannelPtr &ch)
{
    std::string base(ch->pvName.substr(0, ch->pvName.find('.')));
    qCDebug(pvxsLog) << "fetching" << QString::fromStdString(base) << ".NELM";
    std::weak_ptr<PvxsChannel> wch(ch);
    auto op = ctxt.get(base + ".NELM")
                  .result([this, wch](client::Result &&res) {
                      PvxsChannelPtr c = wch.lock();
                      if (!c) return;
                      int32_t nelm = 0;
                      try {
                          res()["value"].as<int32_t>(nelm);
                      } catch (std::exception &e) {
                          // no record behind the pv (e.g. plain pvxs server) - keep nelm 0
                          qCDebug(pvxsLog) << "NELM get failed for" << QString::fromStdString(c->pvName) << e.what();
                          return;
                      }
                      qCDebug(pvxsLog) << "NELM for" << QString::fromStdString(c->pvName) << "=" << nelm;
                      if (nelm <= 0) return;
                      knobData kData = mutexKnobData->GetMutexKnobData(c->index);
                      if (kData.index == -1) return;
                      mutexKnobData->DataLock(&kData);
                      kData.edata.nelm = nelm;
                      mutexKnobData->SetMutexKnobDataReceived(&kData);
                      mutexKnobData->DataUnlock(&kData);
                  })
                  .exec();
    std::lock_guard<std::mutex> lk(ch->opMutex);
    ch->pendingNelmGetOp = op;
}

void PvxsPlugin::handleMonitorConnected(const PvxsChannelPtr &ch, const std::string &peerName)
{
    qCDebug(pvxsLog) << "connected" << QString::fromStdString(ch->pvName)
                     << "from" << QString::fromStdString(peerName);
    // peer address feeds the "IOC:" line of caQtDM's info dialog (edata.fec)
    if (!peerName.empty()) {
        knobData kData = mutexKnobData->GetMutexKnobData(ch->index);
        if (kData.index != -1) {
            mutexKnobData->DataLock(&kData);
            qstrncpy(kData.edata.fec, peerName.c_str(), caqtdm_string_t_length);
            mutexKnobData->SetMutexKnobDataReceived(&kData);
            mutexKnobData->DataUnlock(&kData);
        }
    }
    mutexKnobData->SetMutexKnobDataConnected(ch->index, true);
}

void PvxsPlugin::handleMonitorDisconnected(const PvxsChannelPtr &ch)
{
    qCDebug(pvxsLog) << "disconnected" << QString::fromStdString(ch->pvName);
    mutexKnobData->SetMutexKnobDataConnected(ch->index, false);
}

void PvxsPlugin::drainLoop()
{
    qCDebug(pvxsLog) << "drain thread started";
    while (auto ch = workQueue.pop()) {
        bool more = true;
        while (more) {
            std::shared_ptr<client::Subscription> sub;
            {
                std::lock_guard<std::mutex> lk(ch->opMutex);
                sub = ch->subscription;
            }
            if (!sub) break;

            try {
                Value val = sub->pop();
                if (!val.valid()) {
                    more = false;
                    break;
                }
                handleMonitorValue(ch, val);
            } catch (client::Connected &c) {
                handleMonitorConnected(ch, c.peerName);
            } catch (client::Finished &) {
                more = false;
            } catch (client::Disconnect &) {
                handleMonitorDisconnected(ch);
            } catch (client::RemoteError &e) {
                logError(QString("pvxs %1: %2").arg(QString::fromStdString(ch->pvName), QString(e.what())));
                more = false;
            } catch (std::exception &e) {
                logError(QString("pvxs %1: %2").arg(QString::fromStdString(ch->pvName), QString(e.what())));
                more = false;
            }
        }
    }
    qCDebug(pvxsLog) << "drain thread stopped";
}
