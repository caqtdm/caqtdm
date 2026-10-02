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

#include "alhtreemodel.h"
#include "alarmdefs.h"
#include <QFont>

AlhNodeState::AlhNodeState()
    : curSevr(NOTCONNECTED), unackSevr(NO_ALARM), status(0), connected(false), everConnected(false),
      forced(false), noAckUntilMs(0), beepSevrOverride(-1), mute(false), lastReceivedSevr(NOTCONNECTED), filterPending(false), filterDueMs(0),
      pendingSevr(NOTCONNECTED), pendingStatus(0), filterIndex(0), contribCur(-1), contribUnack(-1)
{
    for(int i = 0; i < AlhSevCount; i++) curCount[i] = unackCount[i] = 0;
}

//---------------------------------------------------------------------------- static helpers

int AlhTreeModel::severityIndex(int sevr)
{
    if(sevr == NOTCONNECTED) return AlhSevError;
    if(sevr < NO_ALARM) return AlhSevNoAlarm;
    if(sevr > INVALID_ALARM) return AlhSevInvalid;
    return sevr;
}

int AlhTreeModel::severityFromIndex(int index)
{
    return index == AlhSevError ? (int) NOTCONNECTED : index;
}

QString AlhTreeModel::severityText(int sevr)
{
    switch(severityIndex(sevr)) {
    case AlhSevNoAlarm: return QStringLiteral("NO_ALARM");
    case AlhSevMinor: return QStringLiteral("MINOR");
    case AlhSevMajor: return QStringLiteral("MAJOR");
    case AlhSevInvalid: return QStringLiteral("INVALID");
    default: return QStringLiteral("NC");
    }
}

// alh bg_char[]: NO_ALARM blank, MINOR Y, MAJOR R, INVALID V, ERROR (not connected) E
QString AlhTreeModel::severityLetter(int sevr)
{
    switch(severityIndex(sevr)) {
    case AlhSevNoAlarm: return QString();
    case AlhSevMinor: return QStringLiteral("Y");
    case AlhSevMajor: return QStringLiteral("R");
    case AlhSevInvalid: return QStringLiteral("V");
    default: return QStringLiteral("E");
    }
}

QColor AlhTreeModel::severityColor(int sevr)
{
    switch(severityIndex(sevr)) {
    case AlhSevNoAlarm: return AL_GREEN;
    case AlhSevMinor: return AL_YELLOW;
    case AlhSevMajor: return AL_RED;
    case AlhSevInvalid: return AL_WHITE;
    default: return AL_DEFAULT;
    }
}

AlhMask AlhTreeModel::maskOr(const AlhMask &a, const AlhMask &b)
{
    AlhMask m;
    m.cancel = a.cancel || b.cancel;
    m.disable = a.disable || b.disable;
    m.noAck = a.noAck || b.noAck;
    m.noAckTransient = a.noAckTransient || b.noAckTransient;
    m.noLog = a.noLog || b.noLog;
    return m;
}

AlhMask AlhTreeModel::maskAndNot(const AlhMask &a, const AlhMask &b)
{
    AlhMask m;
    m.cancel = a.cancel && !b.cancel;
    m.disable = a.disable && !b.disable;
    m.noAck = a.noAck && !b.noAck;
    m.noAckTransient = a.noAckTransient && !b.noAckTransient;
    m.noLog = a.noLog && !b.noLog;
    return m;
}

//---------------------------------------------------------------------------- setup

AlhTreeModel::AlhTreeModel(QObject *parent) : QAbstractItemModel(parent), m_treeMuted(false)
{
}

void AlhTreeModel::setAlhModel(const AlhModel &model)
{
    beginResetModel();
    m_alh = model;
    m_state.clear();
    m_state.resize(m_alh.nodeCount());
    for(int i = 0; i < m_alh.nodeCount(); i++) {
        const AlhNode &n = m_alh.node(i);
        AlhNodeState &s = m_state[i];
        s.fileMask = s.baseMask = n.mask;
        if(n.hasCountFilter && n.countFilter.count > 0) s.filterHistory.fill(0, 2 * n.countFilter.count);
        // alh: cancel or disable start as NO_ALARM, everything else as not connected
        s.curSevr = (n.isChannel() && !(n.mask.cancel || n.mask.disable)) ? (int) NOTCONNECTED : (int) NO_ALARM;
    }
    // alh: every channel is counted in all of its ancestor groups (alConfig.c), groups derive from the counts
    for(int i = 0; i < m_alh.nodeCount(); i++) {
        if(!m_alh.node(i).isChannel()) continue;
        AlhNodeState &s = m_state[i];
        contribution(i, s.contribCur, s.contribUnack);
        for(int p = m_alh.node(i).parentId; p >= 0; p = m_alh.node(p).parentId) {
            m_state[p].curCount[s.contribCur]++;
            m_state[p].unackCount[s.contribUnack]++;
        }
    }
    for(int i = 0; i < m_alh.nodeCount(); i++) if(m_alh.node(i).isGroup()) groupSeverityFromCounts(i);
    endResetModel();
}

//---------------------------------------------------------------------------- QAbstractItemModel

QModelIndex AlhTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if(row < 0 || column < 0 || column >= ColCount) return QModelIndex();
    if(!parent.isValid()) {
        if(row >= m_alh.roots().size()) return QModelIndex();
        return createIndex(row, column, (quintptr) m_alh.roots().at(row));
    }
    const int pid = nodeIdFor(parent);
    if(pid < 0 || row >= m_alh.node(pid).children.size()) return QModelIndex();
    return createIndex(row, column, (quintptr) m_alh.node(pid).children.at(row));
}

QModelIndex AlhTreeModel::parent(const QModelIndex &child) const
{
    const int id = nodeIdFor(child);
    if(id < 0) return QModelIndex();
    const int p = m_alh.node(id).parentId;
    if(p < 0) return QModelIndex();
    return indexFor(p, 0);
}

int AlhTreeModel::rowCount(const QModelIndex &parent) const
{
    if(!parent.isValid()) return m_alh.roots().size();
    if(parent.column() != 0) return 0;
    const int id = nodeIdFor(parent);
    return id < 0 ? 0 : m_alh.node(id).children.size();
}

int AlhTreeModel::columnCount(const QModelIndex &) const
{
    return ColCount;
}

int AlhTreeModel::nodeIdFor(const QModelIndex &index) const
{
    if(!index.isValid()) return -1;
    const int id = (int) index.internalId();
    return (id >= 0 && id < m_alh.nodeCount()) ? id : -1;
}

QModelIndex AlhTreeModel::indexFor(int id, int column) const
{
    if(id < 0 || id >= m_alh.nodeCount()) return QModelIndex();
    const int p = m_alh.node(id).parentId;
    const QVector<int> &list = (p < 0) ? m_alh.roots() : m_alh.node(p).children;
    const int row = list.indexOf(id);
    if(row < 0) return QModelIndex();
    return createIndex(row, column, (quintptr) id);
}

QVariant AlhTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation != Qt::Horizontal || role != Qt::DisplayRole) return QVariant();
    switch(section) {
    case ColUnackSevr: return tr("Ack");
    case ColCurSevr: return tr("Sevr");
    case ColName: return tr("Name");
    case ColArrow: return QString();
    case ColGuidance: return tr("G");
    case ColProcess: return tr("P");
    case ColMask: return tr("Mask");
    case ColInfo: return tr("Info");
    case ColValue: return tr("Value");
    case ColPv: return tr("PV");
    }
    return QVariant();
}

QVariant AlhTreeModel::data(const QModelIndex &index, int role) const
{
    const int id = nodeIdFor(index);
    if(id < 0) return QVariant();
    const AlhNode &n = m_alh.node(id);
    const AlhNodeState &s = m_state.at(id);
    const AlhMask mask = effectiveMask(id);
    const bool inactive = n.isChannel() && (mask.cancel || mask.disable);

    if(role == NodeIdRole) return id;
    if(role == IsGroupRole) return n.isGroup();
    if(role == InactiveRole) return inactive;
    if(role == HasSubGroupsRole) {
        foreach(int child, n.children) if(m_alh.node(child).isGroup()) return true;
        return false;
    }
    if(role == MaskHighlightRole) return mask.disable || mask.noAck || mask.noAckTransient || s.noAckUntilMs > 0;

    if(role == Qt::DisplayRole) {
        switch(index.column()) {
        case ColName: return n.displayName();
        case ColCurSevr: return inactive ? QString() : severityLetter(s.curSevr);
        case ColUnackSevr: return (inactive || s.unackSevr == NO_ALARM) ? QString() : severityLetter(s.unackSevr);
        case ColArrow: return data(index, HasSubGroupsRole).toBool() ? QStringLiteral(">") : QString();
        case ColGuidance: return (!n.guidanceText.isEmpty() || !n.guidanceLocation.isEmpty()) ? QStringLiteral("G") : QString();
        case ColProcess: return n.command.isEmpty() ? QString() : QStringLiteral("P");
        case ColInfo: return infoText(id);
        case ColValue: return n.isChannel() ? s.value : QString();
        case ColMask: {
            QString m = QLatin1Char('<') + maskText(id) + QLatin1Char('>');
            if(isMuted(id)) m += QStringLiteral(" M");
            return m;
        }
        case ColPv: return n.isChannel() ? n.name : QString();
        }
    }
    if(role == Qt::BackgroundRole) {
        if(index.column() == ColCurSevr) return (inactive || s.curSevr == NO_ALARM) ? QVariant() : severityColor(s.curSevr);
        if(index.column() == ColUnackSevr) return (inactive || s.unackSevr == NO_ALARM) ? QVariant() : severityColor(s.unackSevr);
    }
    if(role == Qt::ForegroundRole) {
        if(index.column() == ColCurSevr || index.column() == ColUnackSevr) return AL_BLACK;
        if(inactive) return AL_DEFAULT;
    }
    if(role == Qt::TextAlignmentRole && (index.column() == ColGuidance || index.column() == ColProcess ||
                                         index.column() == ColCurSevr || index.column() == ColUnackSevr))
        return (int) Qt::AlignCenter;
    if(role == Qt::FontRole && index.column() == ColName) {
        QFont f;
        f.setBold(true);
        return f;
    }
    if(role == Qt::ToolTipRole) {
        QString tip = m_alh.nodePath(id);
        if(n.isChannel()) {
            tip += tr("\nmask file %1, base %2, forced %3, effective %4")
                    .arg(s.fileMask.toString(), s.baseMask.toString(),
                         s.forced ? s.forcedMask.toString() : tr("no"), mask.toString());
        }
        if(!n.alias.isEmpty()) tip += QStringLiteral("\n") + n.name;
        if(!n.guidanceText.isEmpty()) tip += QStringLiteral("\n") + n.guidanceText.first().trimmed();
        if(!n.guidanceLocation.isEmpty()) tip += QStringLiteral("\n") + n.guidanceLocation;
        if(!n.command.isEmpty()) tip += tr("\ncommand: %1").arg(n.command);
        return tip;
    }
    return QVariant();
}

//---------------------------------------------------------------------------- masks, mute

AlhMask AlhTreeModel::effectiveMask(int id) const
{
    const AlhNodeState &s = m_state.at(id);
    AlhMask m = s.forced ? s.forcedMask : s.baseMask;
    if(s.noAckUntilMs > 0) m.noAck = true;
    return m;
}

QString AlhTreeModel::maskText(int id) const
{
    QString m = effectiveMask(id).toString();
    if(m_state.at(id).noAckUntilMs > 0) m[2] = QLatin1Char('H');
    return m;
}

// alh line message: groups "(ERROR,INVALID,MAJOR,MINOR,NOALARM)", channels "<STATUS,SEVERITY>,<UNACK>"
QString AlhTreeModel::infoText(int id) const
{
    const AlhNode &n = m_alh.node(id);
    const AlhNodeState &s = m_state.at(id);
    if(n.isGroup()) {
        return QString("(%1,%2,%3,%4,%5)").arg(s.curCount[AlhSevError]).arg(s.curCount[AlhSevInvalid])
                .arg(s.curCount[AlhSevMajor]).arg(s.curCount[AlhSevMinor]).arg(s.curCount[AlhSevNoAlarm]);
    }
    QString stat;
    if(!s.connected) stat = QStringLiteral("NOT_CONNECTED");
    else {
        const QStringList &names = AlhModel::statusNames();
        stat = (s.status >= 0 && s.status < names.size()) ? names.at(s.status) : QString::number(s.status);
    }
    return QString("<%1,%2>,<%3>").arg(stat, severityText(s.curSevr), severityText(s.unackSevr));
}

bool AlhTreeModel::isMuted(int id) const
{
    if(m_treeMuted) return true;
    while(id >= 0) {
        if(m_state.at(id).mute) return true;
        id = m_alh.node(id).parentId;
    }
    return false;
}

void AlhTreeModel::setTreeMuted(bool muted)
{
    if(m_treeMuted == muted) return;
    m_treeMuted = muted;
    for(int i = 0; i < m_alh.nodeCount(); i++) emit dataChanged(indexFor(i, ColMask), indexFor(i, ColMask));
}

QList<int> AlhTreeModel::subtree(int id) const
{
    QList<int> list;
    QList<int> todo;
    todo.append(id);
    while(!todo.isEmpty()) {
        const int cur = todo.takeFirst();
        list.append(cur);
        foreach(int c, m_alh.node(cur).children) todo.append(c);
    }
    return list;
}

void AlhTreeModel::setMute(int id, bool mute, bool recursive)
{
    foreach(int n, recursive ? subtree(id) : QList<int>() << id) {
        if(m_state[n].mute != mute) {
            m_state[n].mute = mute;
            notifyRow(n);
        }
    }
}

//---------------------------------------------------------------------------- propagation

// severity indexes a channel adds to the counters of its ancestors; cancel/disable count as NO_ALARM like alh
void AlhTreeModel::contribution(int id, int &cur, int &unack) const
{
    if(!m_alh.node(id).isChannel()) { cur = unack = -1; return; }
    const AlhMask mask = effectiveMask(id);
    if(mask.cancel || mask.disable) { cur = unack = AlhSevNoAlarm; return; }
    cur = severityIndex(m_state.at(id).curSevr);
    unack = severityIndex(m_state.at(id).unackSevr);
}

// alh alHighestSeverity: highest index with a count, else NO_ALARM
bool AlhTreeModel::groupSeverityFromCounts(int id)
{
    AlhNodeState &s = m_state[id];
    int cur = 0, unack = 0;
    for(int k = AlhSevCount - 1; k > 0; k--) { if(s.curCount[k] > 0) { cur = k; break; } }
    for(int k = AlhSevCount - 1; k > 0; k--) { if(s.unackCount[k] > 0) { unack = k; break; } }
    const int newCur = severityFromIndex(cur), newUnack = severityFromIndex(unack);
    if(newCur == s.curSevr && newUnack == s.unackSevr) return false;
    s.curSevr = newCur;
    s.unackSevr = newUnack;
    return true;
}

// moves the channel's count in every ancestor group (alh: while(glink) {curSev[prev]--; curSev[sev]++; ...})
void AlhTreeModel::propagate(int id)
{
    if(id < 0 || !m_alh.node(id).isChannel()) return;
    AlhNodeState &s = m_state[id];
    int cur, unack;
    contribution(id, cur, unack);
    if(cur == s.contribCur && unack == s.contribUnack) return;
    for(int p = m_alh.node(id).parentId; p >= 0; p = m_alh.node(p).parentId) {
        AlhNodeState &ps = m_state[p];
        if(s.contribCur >= 0) { ps.curCount[s.contribCur]--; ps.unackCount[s.contribUnack]--; }
        ps.curCount[cur]++;
        ps.unackCount[unack]++;
        groupSeverityFromCounts(p);
        notifyRow(p);                      // counts changed even when the severity did not
    }
    s.contribCur = cur;
    s.contribUnack = unack;
}

void AlhTreeModel::notifyRow(int id)
{
    emit dataChanged(indexFor(id, 0), indexFor(id, ColCount - 1));
}

QVariantMap AlhTreeModel::baseEvent(int id, const char *action) const
{
    const AlhNode &n = m_alh.node(id);
    QVariantMap ev;
    ev.insert(QStringLiteral("action"), QString::fromLatin1(action));
    ev.insert(QStringLiteral("nodeId"), id);
    ev.insert(QStringLiteral("pv"), n.isChannel() ? n.name : QString());
    ev.insert(QStringLiteral("node"), n.displayName());
    ev.insert(QStringLiteral("group"), n.parentId >= 0 ? m_alh.nodePath(n.parentId) : QString());
    ev.insert(QStringLiteral("mask"), effectiveMask(id).toString());
    return ev;
}

//---------------------------------------------------------------------------- transitions

QList<QVariantMap> AlhTreeModel::applyUpdate(int id, bool connected, int sevr, short stat, const QString &value, qint64 nowMs)
{
    QList<QVariantMap> events;
    if(id < 0 || id >= m_alh.nodeCount() || !m_alh.node(id).isChannel()) return events;
    const AlhNode &n = m_alh.node(id);
    AlhNodeState &s = m_state[id];
    const int newSevr = connected ? sevr : (int) NOTCONNECTED;

    // alNewAlarmFilter: not for the initial state, connection changes or filters with count/seconds 0
    const bool filtered = n.hasCountFilter && n.countFilter.count > 0 && n.countFilter.seconds > 0 &&
                          connected && s.connected && s.everConnected && !s.filterHistory.isEmpty();
    if(!filtered) {
        s.filterPending = false;
        s.lastReceivedSevr = newSevr;
        process(id, connected, newSevr, stat, value, &events);
        return events;
    }

    const int prevReceived = s.lastReceivedSevr;
    s.lastReceivedSevr = newSevr;
    s.pendingSevr = newSevr;
    s.pendingStatus = stat;
    s.pendingValue = value;
    const bool curAlarm = (s.curSevr != NO_ALARM), newAlarm = (newSevr != NO_ALARM);
    const qint64 window = (qint64) n.countFilter.seconds * 1000;

    if(curAlarm == newAlarm) {
        s.filterPending = false;
        if(curAlarm) process(id, connected, newSevr, stat, value, &events);   // changes inside an alarm go through
        else { s.value = value; notifyRow(id); }
    } else if(!s.filterPending) {
        s.filterPending = true;                                              // delayed by seconds
        s.filterDueMs = nowMs + window;
        s.value = value;
        notifyRow(id);
    }

    // toggling 2*count times within the window: process at once
    if((prevReceived != NO_ALARM) != newAlarm) {
        const int i = s.filterIndex;
        if(s.filterHistory.at(i) != 0 && nowMs - s.filterHistory.at(i) <= window) {
            s.filterHistory.fill(0);
            s.filterIndex = 0;
            s.filterPending = false;
            process(id, connected, newSevr, stat, value, &events);
        } else {
            s.filterHistory[i] = nowMs;
            s.filterIndex = (i + 1) % s.filterHistory.size();
        }
    }
    return events;
}

QList<QVariantMap> AlhTreeModel::processDue(qint64 nowMs)
{
    QList<QVariantMap> events;
    for(int id = 0; id < m_state.size(); id++) {
        AlhNodeState &s = m_state[id];
        if(!s.filterPending || nowMs < s.filterDueMs) continue;
        s.filterPending = false;
        s.filterHistory.fill(0);
        s.filterIndex = 0;
        process(id, s.connected, s.pendingSevr, s.pendingStatus, s.pendingValue, &events);
    }
    return events;
}

bool AlhTreeModel::hasPendingFilters() const
{
    foreach(const AlhNodeState &s, m_state) if(s.filterPending) return true;
    return false;
}

// alNewAlarmProcess: log (also for disabled channels, not for the initial connection), then latch and propagate
void AlhTreeModel::process(int id, bool connected, int newSevr, short stat, const QString &value, QList<QVariantMap> *events)
{
    AlhNodeState &s = m_state[id];
    const AlhMask mask = effectiveMask(id);
    const int oldSevr = s.curSevr;
    const bool wasConnected = s.connected;
    const bool initial = !s.everConnected;
    const short oldStat = s.status;

    s.connected = connected;
    if(connected) s.everConnected = true;
    s.status = stat;
    s.value = value;
    s.curSevr = newSevr;

    if(!initial) {
        if(wasConnected != connected) events->append(baseEvent(id, connected ? "connect" : "disconnect"));
        if(newSevr != oldSevr) {
            QVariantMap ev = baseEvent(id, "transition");
            ev.insert(QStringLiteral("sevr_old"), severityText(oldSevr));
            ev.insert(QStringLiteral("sevr_new"), severityText(newSevr));
            ev.insert(QStringLiteral("stat"), (int) stat);
            ev.insert(QStringLiteral("value"), value);
            events->append(ev);
        } else if(stat != oldStat && connected) {
            QVariantMap ev = baseEvent(id, "status");
            ev.insert(QStringLiteral("sevr_new"), severityText(newSevr));
            ev.insert(QStringLiteral("stat"), (int) stat);
            ev.insert(QStringLiteral("value"), value);
            events->append(ev);
        }
    }

    const int oldUnackIdx = severityIndex(s.unackSevr);
    if(!mask.disable && !mask.cancel && !mask.noAck) {
        const int newIdx = severityIndex(newSevr);
        if(newIdx >= oldUnackIdx) s.unackSevr = newSevr;              // latch
        else if(mask.noAckTransient) s.unackSevr = newSevr;           // transient alarms follow downwards
    }
    if(!events->isEmpty() && events->last().value(QStringLiteral("action")) == QLatin1String("transition")) {
        events->last().insert(QStringLiteral("unack"), severityText(s.unackSevr));
        // a new unacknowledged alarm (alh: newUnackBeepSevr, resets "silence current")
        if(severityIndex(s.unackSevr) > oldUnackIdx) events->last().insert(QStringLiteral("latched"), true);
    }

    notifyRow(id);
    propagate(id);
}

QList<QVariantMap> AlhTreeModel::acknowledge(int id, const QString &user)
{
    QList<QVariantMap> events;
    if(id < 0 || id >= m_alh.nodeCount()) return events;
    foreach(int c, subtree(id)) {
        if(!m_alh.node(c).isChannel()) continue;
        AlhNodeState &s = m_state[c];
        if(s.unackSevr == NO_ALARM) continue;
        QVariantMap ev = baseEvent(c, "ack");
        ev.insert(QStringLiteral("sevr_old"), severityText(s.unackSevr));
        ev.insert(QStringLiteral("sevr_new"), severityText(s.curSevr));
        ev.insert(QStringLiteral("user"), user);
        s.unackSevr = NO_ALARM;
        events.append(ev);
        notifyRow(c);
        propagate(c);
    }
    return events;
}

// alChangeChanMask: disabling clears the acknowledge state, enabling an alarming channel latches it again
void AlhTreeModel::changeMask(int id, const AlhMask &oldMask, QList<QVariantMap> *events)
{
    const AlhMask newMask = effectiveMask(id);
    if(newMask == oldMask) return;
    AlhNodeState &s = m_state[id];
    if(m_alh.node(id).isChannel()) {
        const bool wasActive = !(oldMask.cancel || oldMask.disable);
        const bool isActive = !(newMask.cancel || newMask.disable);
        if(wasActive && !isActive) s.unackSevr = NO_ALARM;
        if(newMask.noAck && !oldMask.noAck) s.unackSevr = NO_ALARM;
        if(isActive && !newMask.noAck && s.curSevr != NO_ALARM && (!wasActive || oldMask.noAck)) {
            s.unackSevr = s.curSevr;
            if(!wasActive) {
                QVariantMap ev = baseEvent(id, "transition");
                ev.insert(QStringLiteral("sevr_old"), severityText(NO_ALARM));
                ev.insert(QStringLiteral("sevr_new"), severityText(s.curSevr));
                ev.insert(QStringLiteral("unack"), severityText(s.unackSevr));
                ev.insert(QStringLiteral("stat"), (int) s.status);
                ev.insert(QStringLiteral("value"), s.value);
                events->append(ev);
            }
        }
    }
    QVariantMap ev = baseEvent(id, "mask");
    ev.insert(QStringLiteral("mask_old"), oldMask.toString());
    ev.insert(QStringLiteral("mask_new"), newMask.toString());
    events->append(ev);
    notifyRow(id);
    propagate(id);
}

// $FORCEPV: alh replaces the mask of every channel below by the force mask (alChangeGroupMask) and
// restores the configured mask on reset (alResetGroupMask)
QList<QVariantMap> AlhTreeModel::applyForce(int id, const AlhMask &mask, bool on)
{
    QList<QVariantMap> events;
    if(id < 0 || id >= m_alh.nodeCount()) return events;
    foreach(int n, subtree(id)) {
        const AlhMask old = effectiveMask(n);
        m_state[n].forced = on;
        m_state[n].forcedMask = mask;
        changeMask(n, old, &events);
    }
    return events;
}

QList<QVariantMap> AlhTreeModel::setBaseMask(int id, const AlhMask &mask, bool recursive)
{
    QList<QVariantMap> events;
    if(id < 0 || id >= m_alh.nodeCount()) return events;
    foreach(int n, recursive ? subtree(id) : QList<int>() << id) {
        const AlhMask old = effectiveMask(n);
        m_state[n].baseMask = mask;
        changeMask(n, old, &events);
    }
    return events;
}

//---------------------------------------------------------------------------- beep

int AlhTreeModel::thresholdIndexFor(int id, int globalThresholdIndex, int overrideThresholdIndex) const
{
    int n = id;
    while(n >= 0) {
        if(m_state.at(n).beepSevrOverride >= 0) return m_state.at(n).beepSevrOverride;   // set at runtime
        if(m_alh.node(n).beepSevr >= 0) return m_alh.node(n).beepSevr;
        n = m_alh.node(n).parentId;
    }
    if(overrideThresholdIndex >= 0) return overrideThresholdIndex;                        // ALH beep severity
    return globalThresholdIndex;
}

void AlhTreeModel::setBeepOverride(int id, int severityIndex, bool recursive)
{
    foreach(int n, recursive ? subtree(id) : QList<int>() << id) m_state[n].beepSevrOverride = severityIndex;
}

QList<QVariantMap> AlhTreeModel::setNoAckTimer(int id, qint64 untilMs, bool recursive)
{
    QList<QVariantMap> events;
    if(id < 0 || id >= m_alh.nodeCount()) return events;
    foreach(int n, recursive ? subtree(id) : QList<int>() << id) {
        const AlhMask old = effectiveMask(n);
        m_state[n].noAckUntilMs = untilMs;
        changeMask(n, old, &events);
        if(m_state[n].noAckUntilMs != 0 || old == effectiveMask(n)) notifyRow(n);   // 'H' indicator
    }
    return events;
}

QList<QVariantMap> AlhTreeModel::expireNoAckTimers(qint64 nowMs)
{
    QList<QVariantMap> events;
    for(int n = 0; n < m_state.size(); n++) {
        if(m_state.at(n).noAckUntilMs > 0 && m_state.at(n).noAckUntilMs <= nowMs) events += setNoAckTimer(n, 0, false);
    }
    return events;
}

bool AlhTreeModel::hasNoAckTimers() const
{
    foreach(const AlhNodeState &s, m_state) if(s.noAckUntilMs > 0) return true;
    return false;
}

int AlhTreeModel::beepSeverity(int globalThresholdIndex, int overrideThresholdIndex) const
{
    int best = -1;
    for(int i = 0; i < m_alh.nodeCount(); i++) {
        if(!m_alh.node(i).isChannel()) continue;
        const AlhNodeState &s = m_state.at(i);
        if(s.unackSevr == NO_ALARM) continue;
        const AlhMask mask = effectiveMask(i);
        if(mask.cancel || mask.disable || isMuted(i)) continue;
        const int idx = severityIndex(s.unackSevr);
        if(idx < thresholdIndexFor(i, globalThresholdIndex, overrideThresholdIndex)) continue;
        if(best < 0 || idx > severityIndex(best)) best = s.unackSevr;
    }
    return best;
}
