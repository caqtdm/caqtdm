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
 *  caAlarmLog: operator log window for caAlarmTree events (alarm and operator log of alh in one list)
 */

#ifndef CAALARMLOG_H
#define CAALARMLOG_H

#include <QTableView>
#include <QVariantMap>
#include <qtcontrols_global.h>

class AlarmLogModel;

class QTCON_EXPORT caAlarmLog : public QTableView
{
    Q_OBJECT
    Q_PROPERTY(int maxEntries READ getMaxEntries WRITE setMaxEntries)
    Q_PROPERTY(bool autoScroll READ getAutoScroll WRITE setAutoScroll)
    Q_PROPERTY(bool showDisplayColumn READ getShowDisplayColumn WRITE setShowDisplayColumn)

    // this will prevent user interference
    Q_PROPERTY(QString styleSheet READ styleSheet WRITE noStyle DESIGNABLE false)

public:
    enum Column { ColTime = 0, ColAction, ColPv, ColNode, ColGroup, ColFrom, ColTo, ColUnack, ColStatus, ColValue, ColUser, ColDisplay, ColCount };

    explicit caAlarmLog(QWidget *parent = Q_NULLPTR);

    void noStyle(QString style) { Q_UNUSED(style); }
    int getMaxEntries() const;
    void setMaxEntries(int n);
    bool getAutoScroll() const { return m_autoScroll; }
    void setAutoScroll(bool on) { m_autoScroll = on; }
    bool getShowDisplayColumn() const { return m_showDisplay; }
    void setShowDisplayColumn(bool on) { m_showDisplay = on; setColumnHidden(ColDisplay, !on); }
    int entryCount() const;
    QVariantMap entry(int row) const;

public slots:
    void appendEvent(const QVariantMap &event);
    void clear();
    void copySelection();
    void toggleAutoScroll();

protected:
    void contextMenuEvent(QContextMenuEvent *event);

private:
    AlarmLogModel *m_model;
    bool m_autoScroll;
    bool m_showDisplay;
};

#endif // CAALARMLOG_H
