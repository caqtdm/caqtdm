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
#ifndef TST_PVXSVALUEMAPPING_H
#define TST_PVXSVALUEMAPPING_H

#include <QObject>
#include <QTest>

// No Context/server/network - safe to run in CI.
class TestPvxsValueMapping : public QObject
{
    Q_OBJECT
public:
    TestPvxsValueMapping() = default;

private slots:
    void scalarDouble();
    void scalarBool();
    void scalarString();
    void scalarArrayDouble();
    void scalarArrayInt();
    void enumValue();
    void enumIndexLookup();
    void alarmSeverity();
    void displayControlLimits();
    void missingValueField();
};

#endif // TST_PVXSVALUEMAPPING_H
