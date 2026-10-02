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
 *
 *  Item model and alarm state machine of caAlarmTree; the rules follow
 *  alLib.c of the EPICS alarm handler (latch, ack, transient, masks,
 *  count filter, group severities from per severity counters).
 */

#ifndef ALHTREEMODEL_H
#define ALHTREEMODEL_H

#include <QAbstractItemModel>
#include <QColor>
#include <QList>
#include <QVariantMap>
#include <QVector>
#include <qtcontrols_global.h>
#include "alhmodel.h"

// runtime state of one node (channel or group)
struct QTCON_EXPORT AlhNodeState {
    int curSevr;              // alarmdefs.h Alarms, NOTCONNECTED = 99
    int unackSevr;
    short status;
    QString value;
    bool connected;
    bool everConnected;
    AlhMask fileMask;         // from the config file
    AlhMask baseMask;         // file mask or override from the config channel
    bool forced;              // $FORCEPV active: forcedMask replaces baseMask (alh alChangeChanMask)
    AlhMask forcedMask;
    qint64 noAckUntilMs;      // "NoAck for One Hour": temporary noAck, shown as 'H'
    int beepSevrOverride;     // beep severity set at runtime (AlhSeverity index), -1 = from config
    bool mute;
    // $ALARMCOUNTFILTER (alh alNewAlarmFilter): transitions normal <-> alarm are delayed by
    // seconds unless the channel toggles 2*count times within the window
    int lastReceivedSevr;
    bool filterPending;
    qint64 filterDueMs;
    int pendingSevr;
    short pendingStatus;
    QString pendingValue;
    QVector<qint64> filterHistory;
    int filterIndex;
    int contribCur;           // severity index contributed to the parent, -1 = none
    int contribUnack;
    int curCount[AlhSevCount];   // groups: channels of the whole subtree per severity index (alh curSev[])
    int unackCount[AlhSevCount];

    AlhNodeState();
};

class QTCON_EXPORT AlhTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    // alh line layout: [ack][sevr][name][arrow][G][P][mask][message]
    enum Column { ColUnackSevr = 0, ColCurSevr, ColName, ColArrow, ColGuidance, ColProcess, ColMask, ColInfo, ColValue, ColPv, ColCount };
    enum Roles { NodeIdRole = Qt::UserRole + 1, IsGroupRole, HasSubGroupsRole, InactiveRole, MaskHighlightRole };

    explicit AlhTreeModel(QObject *parent = Q_NULLPTR);

    void setAlhModel(const AlhModel &model);
    const AlhModel &alhModel() const { return m_alh; }
    bool isEmpty() const { return m_alh.nodeCount() == 0; }

    // QAbstractItemModel
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    int nodeIdFor(const QModelIndex &index) const;
    QModelIndex indexFor(int id, int column = 0) const;
    const AlhNodeState &state(int id) const { return m_state.at(id); }
    const AlhNode &node(int id) const { return m_alh.node(id); }
    AlhMask effectiveMask(int id) const;     // forced ? forcedMask : baseMask, plus temporary noAck
    QString maskText(int id) const;          // effective mask, 'H' while the noAck timer runs
    QString infoText(int id) const;          // group counts (E,I,MA,MI,NO) or <stat,sevr>,<unack>
    bool isMuted(int id) const;              // tree, node or ancestor
    void setTreeMuted(bool muted);
    bool treeMuted() const { return m_treeMuted; }
    QList<int> subtree(int id) const;        // id and all descendants

    // state transitions, the returned maps are alarm events (action, pv, node, group, ...)
    QList<QVariantMap> applyUpdate(int id, bool connected, int sevr, short stat, const QString &value, qint64 nowMs);
    QList<QVariantMap> processDue(qint64 nowMs);          // delayed count filter transitions
    bool hasPendingFilters() const;
    QList<QVariantMap> acknowledge(int id, const QString &user);
    QList<QVariantMap> applyForce(int id, const AlhMask &mask, bool on);   // on: mask replaces, off: back to base
    QList<QVariantMap> setBaseMask(int id, const AlhMask &mask, bool recursive);
    void setMute(int id, bool mute, bool recursive);
    QList<QVariantMap> setNoAckTimer(int id, qint64 untilMs, bool recursive);   // 0 = cancel
    QList<QVariantMap> expireNoAckTimers(qint64 nowMs);
    bool hasNoAckTimers() const;
    void setBeepOverride(int id, int severityIndex, bool recursive);          // -1 = from config

    // beep decision: highest unacknowledged severity (Alarms value) of an unmuted channel at or above its
    // threshold, -1 = nothing to beep for; thresholds are severity indexes (AlhSeverity)
    int beepSeverity(int globalThresholdIndex, int overrideThresholdIndex) const;
    int thresholdIndexFor(int id, int globalThresholdIndex, int overrideThresholdIndex) const;

    static QString severityText(int sevr);
    static QString severityLetter(int sevr);    // alh line buttons: " ", Y, R, V, E
    static QColor severityColor(int sevr);
    static int severityIndex(int sevr);      // Alarms -> AlhSeverity index
    static int severityFromIndex(int index); // AlhSeverity index -> Alarms
    static AlhMask maskOr(const AlhMask &a, const AlhMask &b);
    static AlhMask maskAndNot(const AlhMask &a, const AlhMask &b);

private:
    void contribution(int id, int &cur, int &unack) const;
    void process(int id, bool connected, int newSevr, short stat, const QString &value, QList<QVariantMap> *events);
    void propagate(int id);
    bool groupSeverityFromCounts(int id);
    void notifyRow(int id);
    QVariantMap baseEvent(int id, const char *action) const;
    void changeMask(int id, const AlhMask &oldMask, QList<QVariantMap> *events);

    AlhModel m_alh;
    QVector<AlhNodeState> m_state;
    bool m_treeMuted;
};

#endif // ALHTREEMODEL_H
