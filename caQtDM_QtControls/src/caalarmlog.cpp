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

#include "caalarmlog.h"
#include "alarmdefs.h"
#include "alhtreemodel.h"

#include <QAbstractTableModel>
#include <QApplication>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QDateTime>
#include <QHeaderView>
#include <QMenu>

Q_LOGGING_CATEGORY(caAlarmLogLog, "caqtdm.widgets.caalarmlog")

// ring buffer of events
class AlarmLogModel : public QAbstractTableModel
{
public:
    explicit AlarmLogModel(QObject *parent) : QAbstractTableModel(parent), m_max(1000) {}

    int rowCount(const QModelIndex &parent = QModelIndex()) const { return parent.isValid() ? 0 : m_rows.size(); }
    int columnCount(const QModelIndex & = QModelIndex()) const { return caAlarmLog::ColCount; }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const
    {
        if(orientation != Qt::Horizontal || role != Qt::DisplayRole) return QVariant();
        static const char *names[caAlarmLog::ColCount] = { "Time", "Action", "PV", "Node", "Group", "From", "To",
                                                           "Unack", "Status", "Value", "User", "Display" };
        return (section >= 0 && section < caAlarmLog::ColCount) ? QString::fromLatin1(names[section]) : QVariant();
    }

    QVariant data(const QModelIndex &index, int role) const
    {
        if(!index.isValid() || index.row() >= m_rows.size()) return QVariant();
        const QVariantMap &ev = m_rows.at(index.row());
        if(role == Qt::DisplayRole) {
            switch(index.column()) {
            case caAlarmLog::ColTime: {
                const QDateTime ts = QDateTime::fromString(ev.value(QStringLiteral("ts")).toString(), Qt::ISODateWithMs);
                return ts.isValid() ? ts.toLocalTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz")) : ev.value(QStringLiteral("ts"));
            }
            case caAlarmLog::ColAction: return ev.value(QStringLiteral("action"));
            case caAlarmLog::ColPv: return ev.value(QStringLiteral("pv"));
            case caAlarmLog::ColNode: return ev.value(QStringLiteral("node"));
            case caAlarmLog::ColGroup: return ev.value(QStringLiteral("group"));
            case caAlarmLog::ColFrom: return ev.contains(QStringLiteral("mask_old")) ? ev.value(QStringLiteral("mask_old")) : ev.value(QStringLiteral("sevr_old"));
            case caAlarmLog::ColTo: return ev.contains(QStringLiteral("mask_new")) ? ev.value(QStringLiteral("mask_new")) : ev.value(QStringLiteral("sevr_new"));
            case caAlarmLog::ColUnack: return ev.value(QStringLiteral("unack"));
            case caAlarmLog::ColStatus: {
                if(!ev.contains(QStringLiteral("stat"))) return QVariant();
                const int stat = ev.value(QStringLiteral("stat")).toInt();
                const QStringList &names = AlhModel::statusNames();
                return (stat >= 0 && stat < names.size()) ? names.at(stat) : QString::number(stat);
            }
            case caAlarmLog::ColValue: return ev.value(QStringLiteral("value"));
            case caAlarmLog::ColUser: return ev.value(QStringLiteral("user"));
            case caAlarmLog::ColDisplay: return ev.value(QStringLiteral("display"));
            }
        }
        if(role == Qt::BackgroundRole && (index.column() == caAlarmLog::ColFrom || index.column() == caAlarmLog::ColTo)) {
            const QString key = index.column() == caAlarmLog::ColFrom ? QStringLiteral("sevr_old") : QStringLiteral("sevr_new");
            if(!ev.contains(key)) return QVariant();
            return AlhTreeModel::severityColor(severityFromText(ev.value(key).toString()));
        }
        if(role == Qt::ForegroundRole && (index.column() == caAlarmLog::ColFrom || index.column() == caAlarmLog::ColTo)) {
            // black only on the severity colours, mask events use the palette text (dark mode)
            const QString key = index.column() == caAlarmLog::ColFrom ? QStringLiteral("sevr_old") : QStringLiteral("sevr_new");
            return ev.contains(key) ? QVariant(AL_BLACK) : QVariant();
        }
        return QVariant();
    }

    void append(const QVariantMap &ev)
    {
        if(m_max > 0 && m_rows.size() >= m_max) {
            const int drop = m_rows.size() - m_max + 1;
            beginRemoveRows(QModelIndex(), 0, drop - 1);
            for(int i = 0; i < drop; i++) m_rows.removeFirst();
            endRemoveRows();
        }
        beginInsertRows(QModelIndex(), m_rows.size(), m_rows.size());
        m_rows.append(ev);
        endInsertRows();
    }

    void clearAll()
    {
        beginResetModel();
        m_rows.clear();
        endResetModel();
    }

    void setMax(int n) { m_max = n; }
    int max() const { return m_max; }
    const QList<QVariantMap> &rows() const { return m_rows; }

    static int severityFromText(const QString &text)
    {
        if(text == QLatin1String("MINOR")) return MINOR_ALARM;
        if(text == QLatin1String("MAJOR")) return MAJOR_ALARM;
        if(text == QLatin1String("INVALID")) return INVALID_ALARM;
        if(text == QLatin1String("NC")) return NOTCONNECTED;
        return NO_ALARM;
    }

private:
    QList<QVariantMap> m_rows;
    int m_max;
};

caAlarmLog::caAlarmLog(QWidget *parent) : QTableView(parent), m_autoScroll(true), m_showDisplay(false)
{
    m_model = new AlarmLogModel(this);
    setModel(m_model);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setAlternatingRowColors(true);
    setWordWrap(false);
    verticalHeader()->setVisible(false);
    verticalHeader()->setDefaultSectionSize(fontMetrics().height() + 4);
    horizontalHeader()->setStretchLastSection(true);
    setColumnHidden(ColDisplay, !m_showDisplay);
}

int caAlarmLog::getMaxEntries() const { return m_model->max(); }
void caAlarmLog::setMaxEntries(int n) { m_model->setMax(qMax(1, n)); }
int caAlarmLog::entryCount() const { return m_model->rowCount(); }

QVariantMap caAlarmLog::entry(int row) const
{
    return (row >= 0 && row < m_model->rows().size()) ? m_model->rows().at(row) : QVariantMap();
}

void caAlarmLog::appendEvent(const QVariantMap &event)
{
    m_model->append(event);
    if(m_autoScroll) scrollToBottom();
}

void caAlarmLog::clear()
{
    m_model->clearAll();
}

void caAlarmLog::toggleAutoScroll()
{
    m_autoScroll = !m_autoScroll;
    if(m_autoScroll) scrollToBottom();
}

void caAlarmLog::copySelection()
{
    QStringList lines;
    QModelIndexList rows = selectionModel()->selectedRows();
    std::sort(rows.begin(), rows.end());
    foreach(const QModelIndex &idx, rows) {
        QStringList cells;
        for(int c = 0; c < ColCount; c++) {
            if(isColumnHidden(c)) continue;
            cells.append(m_model->data(m_model->index(idx.row(), c), Qt::DisplayRole).toString());
        }
        lines.append(cells.join(QLatin1Char('\t')));
    }
    if(!lines.isEmpty()) QApplication::clipboard()->setText(lines.join(QLatin1Char('\n')));
}

void caAlarmLog::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    connect(menu.addAction(tr("Copy selected lines")), SIGNAL(triggered()), this, SLOT(copySelection()));
    QAction *scroll = menu.addAction(tr("Auto scroll"));
    scroll->setCheckable(true);
    scroll->setChecked(m_autoScroll);
    connect(scroll, SIGNAL(triggered()), this, SLOT(toggleAutoScroll()));
    menu.addSeparator();
    connect(menu.addAction(tr("Clear")), SIGNAL(triggered()), this, SLOT(clear()));
    menu.exec(event->globalPos());
}
