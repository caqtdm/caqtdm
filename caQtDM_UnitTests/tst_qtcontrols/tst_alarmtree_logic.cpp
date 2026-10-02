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

#include "tst_alarmtree_logic.h"
#include "alarmdefs.h"
#include "alhconfigparser.h"

static const char *testConfig =
        "$BEEPSEVERITY MAJOR\n"
        "GROUP NULL ROOT\n"
        "GROUP ROOT G1\n"
        "CHANNEL G1 a:ch1 -----\n"
        "CHANNEL G1 a:ch2 ---T-\n"
        "CHANNEL G1 a:ch3 --A--\n"
        "CHANNEL G1 a:ch4 -D---\n"
        "CHANNEL G1 a:ch5 ----L\n"
        "GROUP ROOT G2\n"
        "$BEEPSEVR INVALID\n"
        "CHANNEL G2 b:ch1 -----\n"
        "$ALARMCOUNTFILTER 3 10\n"
        "CHANNEL G2 b:ch2 C----\n";

static int countAction(const QList<QVariantMap> &events, const char *action)
{
    int n = 0;
    foreach(const QVariantMap &ev, events) if(ev.value("action").toString() == QLatin1String(action)) n++;
    return n;
}

void TestAlarmTreeLogic::init()
{
    AlhModel alh;
    AlhConfigParser parser;
    QVERIFY(parser.parseText(QString::fromLatin1(testConfig), "logic.alhConfig", &alh));
    QVERIFY(alh.warnings().isEmpty());
    m_model.setAlhModel(alh);
}

int TestAlarmTreeLogic::id(const char *path) const
{
    return m_model.alhModel().findByPath(QString::fromLatin1(path));
}

QList<QVariantMap> TestAlarmTreeLogic::update(const char *path, int sevr, bool connected, qint64 nowMs)
{
    return m_model.applyUpdate(id(path), connected, sevr, 0, QStringLiteral("1"), nowMs);
}

void TestAlarmTreeLogic::connectAll(int group, int sevr)
{
    foreach(int c, m_model.subtree(group)) {
        const AlhNode &n = m_model.node(c);
        if(n.isChannel() && !m_model.effectiveMask(c).cancel) m_model.applyUpdate(c, true, sevr, 0, QStringLiteral("0"), 0);
    }
}

void TestAlarmTreeLogic::initialState()
{
    QVERIFY(id("ROOT") >= 0 && id("ROOT/G1/a:ch1") >= 0);
    // alh: channels start as not connected (ERROR), cancel/disable as NO_ALARM
    QCOMPARE(m_model.state(id("ROOT/G1/a:ch1")).curSevr, (int) NOTCONNECTED);
    QCOMPARE(m_model.state(id("ROOT/G1/a:ch4")).curSevr, (int) NO_ALARM);
    QCOMPARE(m_model.state(id("ROOT/G2/b:ch2")).curSevr, (int) NO_ALARM);
    // groups aggregate the not connected children
    QCOMPARE(m_model.state(id("ROOT/G1")).curSevr, (int) NOTCONNECTED);
    QCOMPARE(m_model.state(id("ROOT")).curSevr, (int) NOTCONNECTED);
    QCOMPARE(m_model.state(id("ROOT")).unackSevr, (int) NO_ALARM);
    // the disabled channel counts as NO_ALARM in every ancestor (alh)
    QCOMPARE(m_model.state(id("ROOT/G1/a:ch4")).contribCur, (int) AlhSevNoAlarm);   // alh: disabled counts as NO_ALARM
    QCOMPARE(m_model.state(id("ROOT/G1")).curCount[AlhSevError], 4);
}

void TestAlarmTreeLogic::modelIndexes()
{
    QCOMPARE(m_model.rowCount(), 1);
    const QModelIndex root = m_model.index(0, 0);
    QCOMPARE(m_model.nodeIdFor(root), id("ROOT"));
    QCOMPARE(m_model.rowCount(root), 2);
    const QModelIndex g1 = m_model.index(0, 0, root);
    QCOMPARE(m_model.nodeIdFor(g1), id("ROOT/G1"));
    QCOMPARE(m_model.rowCount(g1), 5);
    QCOMPARE(m_model.parent(g1), root);
    const QModelIndex ch2 = m_model.index(1, AlhTreeModel::ColName, g1);
    QCOMPARE(m_model.data(ch2, Qt::DisplayRole).toString(), QStringLiteral("a:ch2"));
    QCOMPARE(m_model.data(m_model.index(1, AlhTreeModel::ColMask, g1), Qt::DisplayRole).toString(), QStringLiteral("<---T->"));
    QCOMPARE(m_model.indexFor(id("ROOT/G1/a:ch2")), m_model.index(1, 0, g1));
    QCOMPARE(m_model.columnCount(), (int) AlhTreeModel::ColCount);
}

void TestAlarmTreeLogic::latchPropagateAcknowledge()
{
    connectAll(id("ROOT"));
    QCOMPARE(m_model.state(id("ROOT")).curSevr, (int) NO_ALARM);
    QCOMPARE(m_model.state(id("ROOT")).unackSevr, (int) NO_ALARM);

    QList<QVariantMap> events = update("ROOT/G1/a:ch1", MAJOR_ALARM);
    QCOMPARE(countAction(events, "transition"), 1);
    QCOMPARE(events.first().value("sevr_old").toString(), QStringLiteral("NO_ALARM"));
    QCOMPARE(events.first().value("sevr_new").toString(), QStringLiteral("MAJOR"));
    QCOMPARE(events.first().value("group").toString(), QStringLiteral("ROOT/G1"));
    QCOMPARE(m_model.state(id("ROOT/G1/a:ch1")).unackSevr, (int) MAJOR_ALARM);
    QCOMPARE(m_model.state(id("ROOT/G1")).curSevr, (int) MAJOR_ALARM);
    QCOMPARE(m_model.state(id("ROOT")).curSevr, (int) MAJOR_ALARM);
    QCOMPARE(m_model.state(id("ROOT")).unackSevr, (int) MAJOR_ALARM);

    // alarm goes away: current follows, unacknowledged stays latched
    update("ROOT/G1/a:ch1", NO_ALARM);
    QCOMPARE(m_model.state(id("ROOT/G1/a:ch1")).curSevr, (int) NO_ALARM);
    QCOMPARE(m_model.state(id("ROOT/G1/a:ch1")).unackSevr, (int) MAJOR_ALARM);
    QCOMPARE(m_model.state(id("ROOT/G1")).curSevr, (int) NO_ALARM);
    QCOMPARE(m_model.state(id("ROOT/G1")).unackSevr, (int) MAJOR_ALARM);
    QCOMPARE(m_model.state(id("ROOT")).unackSevr, (int) MAJOR_ALARM);

    // a lower alarm does not lower the latch
    update("ROOT/G1/a:ch1", MINOR_ALARM);
    QCOMPARE(m_model.state(id("ROOT/G1/a:ch1")).unackSevr, (int) MAJOR_ALARM);
    update("ROOT/G1/a:ch1", NO_ALARM);

    // group acknowledge clears all descendants
    events = m_model.acknowledge(id("ROOT"), QStringLiteral("tester"));
    QCOMPARE(countAction(events, "ack"), 1);
    QCOMPARE(events.first().value("user").toString(), QStringLiteral("tester"));
    QCOMPARE(m_model.state(id("ROOT/G1/a:ch1")).unackSevr, (int) NO_ALARM);
    QCOMPARE(m_model.state(id("ROOT")).unackSevr, (int) NO_ALARM);
    QVERIFY(m_model.acknowledge(id("ROOT"), QStringLiteral("tester")).isEmpty());
}

void TestAlarmTreeLogic::transientMask()
{
    connectAll(id("ROOT"));
    update("ROOT/G1/a:ch2", MAJOR_ALARM);
    QCOMPARE(m_model.state(id("ROOT/G1/a:ch2")).unackSevr, (int) MAJOR_ALARM);
    update("ROOT/G1/a:ch2", MINOR_ALARM);
    QCOMPARE(m_model.state(id("ROOT/G1/a:ch2")).unackSevr, (int) MINOR_ALARM);   // follows downwards
    update("ROOT/G1/a:ch2", NO_ALARM);
    QCOMPARE(m_model.state(id("ROOT/G1/a:ch2")).unackSevr, (int) NO_ALARM);     // transient needs no ack
    QCOMPARE(m_model.state(id("ROOT")).unackSevr, (int) NO_ALARM);
}

void TestAlarmTreeLogic::noAckMask()
{
    connectAll(id("ROOT"));
    update("ROOT/G1/a:ch3", MAJOR_ALARM);
    QCOMPARE(m_model.state(id("ROOT/G1/a:ch3")).curSevr, (int) MAJOR_ALARM);
    QCOMPARE(m_model.state(id("ROOT/G1/a:ch3")).unackSevr, (int) NO_ALARM);
    QCOMPARE(m_model.state(id("ROOT")).curSevr, (int) MAJOR_ALARM);
    QCOMPARE(m_model.state(id("ROOT")).unackSevr, (int) NO_ALARM);
}

void TestAlarmTreeLogic::disableMask()
{
    connectAll(id("ROOT"));
    const QList<QVariantMap> events = update("ROOT/G1/a:ch4", MAJOR_ALARM);
    QCOMPARE(countAction(events, "transition"), 1);                              // alh logs disabled channels
    QCOMPARE(m_model.state(id("ROOT/G1/a:ch4")).curSevr, (int) MAJOR_ALARM);     // and shows them
    QCOMPARE(m_model.state(id("ROOT/G1/a:ch4")).unackSevr, (int) NO_ALARM);
    QCOMPARE(m_model.state(id("ROOT/G1")).curSevr, (int) NO_ALARM);              // does not count
}

void TestAlarmTreeLogic::disconnectIsAnAlarm()
{
    // the initial connection is not logged (alh)
    QVERIFY(update("ROOT/G1/a:ch1", NO_ALARM, true).isEmpty());
    connectAll(id("ROOT"));
    QList<QVariantMap> events = update("ROOT/G1/a:ch1", NO_ALARM, false);
    QCOMPARE(countAction(events, "disconnect"), 1);
    QCOMPARE(countAction(events, "transition"), 1);
    QCOMPARE(m_model.state(id("ROOT/G1/a:ch1")).curSevr, (int) NOTCONNECTED);
    QCOMPARE(m_model.state(id("ROOT/G1/a:ch1")).unackSevr, (int) NOTCONNECTED);
    QCOMPARE(m_model.state(id("ROOT")).curSevr, (int) NOTCONNECTED);
    events = update("ROOT/G1/a:ch1", NO_ALARM, true);
    QCOMPARE(countAction(events, "connect"), 1);
    QCOMPARE(m_model.state(id("ROOT")).curSevr, (int) NO_ALARM);
    QCOMPARE(m_model.state(id("ROOT")).unackSevr, (int) NOTCONNECTED);          // needs an acknowledge
    m_model.acknowledge(id("ROOT/G1/a:ch1"), QString());
    QCOMPARE(m_model.state(id("ROOT")).unackSevr, (int) NO_ALARM);
}

void TestAlarmTreeLogic::countFilter()
{
    connectAll(id("ROOT"));
    const int ch = id("ROOT/G2/b:ch1");                                           // count 3, seconds 10
    // going into alarm is delayed by seconds
    QVERIFY(update("ROOT/G2/b:ch1", MAJOR_ALARM, true, 0).isEmpty());
    QCOMPARE(m_model.state(ch).curSevr, (int) NO_ALARM);
    QVERIFY(m_model.hasPendingFilters());
    QVERIFY(m_model.processDue(5000).isEmpty());
    QList<QVariantMap> events = m_model.processDue(10000);
    QCOMPARE(countAction(events, "transition"), 1);
    QCOMPARE(m_model.state(ch).curSevr, (int) MAJOR_ALARM);
    QCOMPARE(m_model.state(id("ROOT/G2")).curSevr, (int) MAJOR_ALARM);
    QVERIFY(!m_model.hasPendingFilters());
    // changes inside the alarm go through at once
    events = update("ROOT/G2/b:ch1", MINOR_ALARM, true, 11000);
    QCOMPARE(countAction(events, "transition"), 1);
    QCOMPARE(m_model.state(ch).curSevr, (int) MINOR_ALARM);
    // leaving the alarm is delayed as well, the displayed state stays
    QVERIFY(update("ROOT/G2/b:ch1", NO_ALARM, true, 12000).isEmpty());
    QCOMPARE(m_model.state(ch).curSevr, (int) MINOR_ALARM);
    QVERIFY(m_model.processDue(21000).isEmpty());
    QCOMPARE(countAction(m_model.processDue(22000), "transition"), 1);
    QCOMPARE(m_model.state(ch).curSevr, (int) NO_ALARM);
    // toggling 2*count times within the window is processed immediately
    qint64 t = 100000;
    for(int i = 0; i < 6; i++, t += 100) QVERIFY(update("ROOT/G2/b:ch1", (i % 2 == 0) ? MAJOR_ALARM : NO_ALARM, true, t).isEmpty());
    QCOMPARE(m_model.state(ch).curSevr, (int) NO_ALARM);
    events = update("ROOT/G2/b:ch1", MAJOR_ALARM, true, t);                      // 7th transition
    QCOMPARE(countAction(events, "transition"), 1);
    QCOMPARE(m_model.state(ch).curSevr, (int) MAJOR_ALARM);
    QVERIFY(!m_model.hasPendingFilters());
}

void TestAlarmTreeLogic::forceBits()
{
    connectAll(id("ROOT"));
    update("ROOT/G1/a:ch1", MAJOR_ALARM);
    QCOMPARE(m_model.state(id("ROOT")).curSevr, (int) MAJOR_ALARM);

    // alh replaces the masks of the subtree by the force mask
    AlhMask force = AlhMask::fromString("-D---");
    QList<QVariantMap> events = m_model.applyForce(id("ROOT/G1"), force, true);
    QCOMPARE(countAction(events, "mask"), 5);                                    // group + 4 channels, ch4 already -D---
    QVERIFY(m_model.effectiveMask(id("ROOT/G1/a:ch1")).disable);
    QVERIFY(!m_model.effectiveMask(id("ROOT/G1/a:ch2")).noAckTransient);        // T replaced
    QCOMPARE(m_model.state(id("ROOT/G1/a:ch1")).unackSevr, (int) NO_ALARM);     // disabling clears the ack state
    QCOMPARE(m_model.state(id("ROOT/G1")).curSevr, (int) NO_ALARM);
    QCOMPARE(m_model.state(id("ROOT")).curSevr, (int) NO_ALARM);
    QCOMPARE(m_model.state(id("ROOT")).unackSevr, (int) NO_ALARM);

    QVERIFY(m_model.applyForce(id("ROOT/G1"), force, true).isEmpty());         // idempotent
    events = m_model.applyForce(id("ROOT/G1"), force, false);                    // reset: configured masks
    QCOMPARE(countAction(events, "mask"), 5);
    QCOMPARE(countAction(events, "transition"), 1);                              // ch1 re-enabled in alarm
    QVERIFY(!m_model.effectiveMask(id("ROOT/G1/a:ch1")).disable);
    QVERIFY(m_model.effectiveMask(id("ROOT/G1/a:ch2")).noAckTransient);
    QVERIFY(m_model.effectiveMask(id("ROOT/G1/a:ch4")).disable);                // file mask kept
    QCOMPARE(m_model.state(id("ROOT")).curSevr, (int) MAJOR_ALARM);
    QCOMPARE(m_model.state(id("ROOT")).unackSevr, (int) MAJOR_ALARM);           // latched again
}

void TestAlarmTreeLogic::baseMaskOverride()
{
    connectAll(id("ROOT"));
    update("ROOT/G1/a:ch1", MAJOR_ALARM);
    const int ch = id("ROOT/G1/a:ch1");
    QList<QVariantMap> events = m_model.setBaseMask(ch, AlhMask::fromString("-D---"), false);
    QCOMPARE(events.size(), 1);
    QCOMPARE(events.first().value("mask_old").toString(), QStringLiteral("-----"));
    QCOMPARE(events.first().value("mask_new").toString(), QStringLiteral("-D---"));
    QCOMPARE(m_model.state(id("ROOT")).curSevr, (int) NO_ALARM);
    // an active force replaces the base mask, the base mask comes back on reset
    m_model.applyForce(ch, AlhMask::fromString("----L"), true);
    QCOMPARE(m_model.effectiveMask(ch).toString(), QStringLiteral("----L"));
    QCOMPARE(m_model.state(id("ROOT")).curSevr, (int) MAJOR_ALARM);
    m_model.applyForce(ch, AlhMask::fromString("----L"), false);
    QCOMPARE(m_model.effectiveMask(ch).toString(), QStringLiteral("-D---"));
    m_model.setBaseMask(ch, m_model.state(ch).fileMask, false);                  // reset to config
    QCOMPARE(m_model.effectiveMask(ch).toString(), QStringLiteral("-----"));
    QCOMPARE(m_model.state(id("ROOT")).curSevr, (int) MAJOR_ALARM);
}

void TestAlarmTreeLogic::beepDecision()
{
    connectAll(id("ROOT"));
    const int global = m_model.alhModel().effectiveBeepSeverity();
    QCOMPARE(global, (int) AlhSevMajor);
    QCOMPARE(m_model.beepSeverity(global, -1), -1);

    update("ROOT/G1/a:ch1", MINOR_ALARM);
    QCOMPARE(m_model.beepSeverity(global, -1), -1);                              // below threshold
    QCOMPARE(m_model.beepSeverity(global, AlhSevMinor), (int) MINOR_ALARM);      // override threshold
    update("ROOT/G1/a:ch1", MAJOR_ALARM);
    QCOMPARE(m_model.beepSeverity(global, -1), (int) MAJOR_ALARM);
    update("ROOT/G1/a:ch1", NO_ALARM);
    QCOMPARE(m_model.beepSeverity(global, -1), (int) MAJOR_ALARM);               // until acknowledged

    m_model.setMute(id("ROOT/G1"), true, true);
    QCOMPARE(m_model.beepSeverity(global, -1), -1);
    m_model.setMute(id("ROOT/G1"), false, true);
    m_model.setTreeMuted(true);
    QCOMPARE(m_model.beepSeverity(global, -1), -1);
    m_model.setTreeMuted(false);
    m_model.acknowledge(id("ROOT"), QString());
    QCOMPARE(m_model.beepSeverity(global, -1), -1);

    // $BEEPSEVR INVALID on G2 raises the threshold for its channels
    QCOMPARE(m_model.thresholdIndexFor(id("ROOT/G2/b:ch1"), global, -1), (int) AlhSevInvalid);
    update("ROOT/G2/b:ch1", MAJOR_ALARM, true, 0);
    m_model.processDue(10000);                                                   // count filter delay over
    QCOMPARE(m_model.state(id("ROOT/G2/b:ch1")).unackSevr, (int) MAJOR_ALARM);
    QCOMPARE(m_model.beepSeverity(global, -1), -1);
}

void TestAlarmTreeLogic::noAckTimerAndBeepOverride()
{
    connectAll(id("ROOT"));
    const int ch = id("ROOT/G1/a:ch1");
    update("ROOT/G1/a:ch1", MAJOR_ALARM);
    QCOMPARE(m_model.state(ch).unackSevr, (int) MAJOR_ALARM);

    // "NoAck for One Hour": ack mask set, shown as H, acknowledge state cleared
    QList<QVariantMap> events = m_model.setNoAckTimer(id("ROOT/G1"), 5000, true);
    QVERIFY(countAction(events, "mask") >= 1);
    QVERIFY(m_model.effectiveMask(ch).noAck);
    QCOMPARE(m_model.maskText(ch), QStringLiteral("--H--"));
    QCOMPARE(m_model.state(ch).unackSevr, (int) NO_ALARM);
    QVERIFY(m_model.hasNoAckTimers());
    update("ROOT/G1/a:ch1", MINOR_ALARM);
    update("ROOT/G1/a:ch1", MAJOR_ALARM);
    QCOMPARE(m_model.state(ch).unackSevr, (int) NO_ALARM);                     // nothing latches while noAck
    QVERIFY(m_model.expireNoAckTimers(4000).isEmpty());
    events = m_model.expireNoAckTimers(5000);
    QVERIFY(!m_model.hasNoAckTimers());
    QCOMPARE(m_model.maskText(ch), QStringLiteral("-----"));
    QCOMPARE(m_model.state(ch).unackSevr, (int) MAJOR_ALARM);                  // alarming channel latches again
    QCOMPARE(m_model.state(id("ROOT")).unackSevr, (int) MAJOR_ALARM);

    // beep severity set at runtime wins over the configuration
    const int global = m_model.alhModel().effectiveBeepSeverity();
    QCOMPARE(m_model.thresholdIndexFor(ch, global, -1), (int) AlhSevMajor);
    m_model.setBeepOverride(id("ROOT/G1"), AlhSevInvalid, true);
    QCOMPARE(m_model.thresholdIndexFor(ch, global, -1), (int) AlhSevInvalid);
    QCOMPARE(m_model.beepSeverity(global, -1), -1);
    m_model.setBeepOverride(id("ROOT/G1"), -1, true);
    QCOMPARE(m_model.beepSeverity(global, -1), (int) MAJOR_ALARM);
    // the ALH beep severity (toolbar) applies below node settings only
    QCOMPARE(m_model.thresholdIndexFor(ch, global, AlhSevMinor), (int) AlhSevMinor);
    QCOMPARE(m_model.thresholdIndexFor(id("ROOT/G2/b:ch1"), global, AlhSevMinor), (int) AlhSevInvalid);   // $BEEPSEVR
}

void TestAlarmTreeLogic::lineTexts()
{
    connectAll(id("ROOT"));
    update("ROOT/G1/a:ch1", MAJOR_ALARM);
    // alh line messages: channel <STATUS,SEVERITY>,<UNACK>, group (ERROR,INVALID,MAJOR,MINOR,NOALARM)
    QCOMPARE(m_model.infoText(id("ROOT/G1/a:ch1")), QStringLiteral("<NO_ALARM,MAJOR>,<MAJOR>"));
    QCOMPARE(m_model.infoText(id("ROOT/G1")), QStringLiteral("(0,0,1,0,4)"));         // ch4 disabled counts as NO_ALARM
    QCOMPARE(m_model.infoText(id("ROOT")), QStringLiteral("(0,0,1,0,6)"));            // alh: all 7 channels of the subtree
    update("ROOT/G1/a:ch1", NO_ALARM, false);
    QCOMPARE(m_model.infoText(id("ROOT/G1/a:ch1")), QStringLiteral("<NOT_CONNECTED,NC>,<NC>"));
    const QModelIndex g1 = m_model.index(0, 0, m_model.index(0, 0));
    QCOMPARE(m_model.data(m_model.index(0, AlhTreeModel::ColUnackSevr, g1), Qt::DisplayRole).toString(), QStringLiteral("E"));
    QCOMPARE(m_model.data(m_model.index(0, AlhTreeModel::ColArrow, m_model.index(0, 0)), Qt::DisplayRole).toString(), QString());
    QCOMPARE(m_model.data(m_model.index(0, AlhTreeModel::ColArrow), Qt::DisplayRole).toString(), QStringLiteral(">"));
    QCOMPARE(m_model.data(m_model.index(0, AlhTreeModel::ColGuidance, g1), Qt::DisplayRole).toString(), QString());
    QCOMPARE(m_model.headerData(AlhTreeModel::ColProcess, Qt::Horizontal, Qt::DisplayRole).toString(), QStringLiteral("P"));
}
