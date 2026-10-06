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
#ifndef TST_ALARMTREE_LOGIC_H
#define TST_ALARMTREE_LOGIC_H

#include <QObject>
#include <QTest>

#include "alhtreemodel.h"

// alarm state machine of caAlarmTree (AlhTreeModel) without any control system
class TestAlarmTreeLogic : public QObject
{
    Q_OBJECT
public:
    TestAlarmTreeLogic() = default;

private slots:
    void init();
    void initialState();
    void modelIndexes();
    void latchPropagateAcknowledge();
    void transientMask();
    void noAckMask();
    void disableMask();
    void disconnectIsAnAlarm();
    void countFilter();
    void countFilterSecondsOnly();
    void forceBits();
    void baseMaskOverride();
    void beepDecision();
    void noAckTimerAndBeepOverride();
    void lineTexts();

private:
    int id(const char *path) const;
    void connectAll(int group, int sevr = 0);
    QList<QVariantMap> update(const char *path, int sevr, bool connected = true, qint64 nowMs = 0);
    AlhTreeModel m_model;
};

#endif // TST_ALARMTREE_LOGIC_H
