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

#include "caalarmtree.h"
#include "alhtreemodel.h"
#include "alarmdefs.h"
#include "alhconfigparser.h"
#include "cashellcommand.h"
#include "cascriptbutton.h"
#include "caalarmlog.h"
#include "searchfile.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDateTime>
#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileInfo>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QShortcut>
#include <QSortFilterProxyModel>
#include <QSplitter>
#include <QStyledItemDelegate>
#include <QSysInfo>
#include <QTextBrowser>
#include <QTimer>
#include <QTreeView>
#include <QUrl>
#include <QVBoxLayout>
#include <algorithm>

Q_LOGGING_CATEGORY(caAlarmTreeLog, "caqtdm.widgets.caalarmtree")
// one JSON line per alarm event, info level: visible with the default logging rules
Q_LOGGING_CATEGORY(caAlarmEventLog, "caqtdm.alarm.events")

static const char *cfgFlagNames[] = { "cancel", "disable", "noAck", "noAckTransient", "noLog", Q_NULLPTR };
static const char *selectFirstText = "Please select an alarm group or channel first.";
static const char *selectGroupText = "Please select an alarm group first.";

static bool *maskFlag(AlhMask &m, int i)
{
    switch(i) {
    case 0: return &m.cancel;
    case 1: return &m.disable;
    case 2: return &m.noAck;
    case 3: return &m.noAckTransient;
    default: return &m.noLog;
    }
}

//---------------------------------------------------------------------------- display filter (alh alFilter.c)

class AlhFilterProxy : public QSortFilterProxyModel
{
public:
    AlhFilterProxy(AlhTreeModel *tree, QObject *parent) : QSortFilterProxyModel(parent), m_tree(tree), m_mode(0)
    {
        setSourceModel(tree);
        setDynamicSortFilter(true);
    }
    void setMode(int mode) { m_mode = mode; invalidateFilter(); }
    int mode() const { return m_mode; }
    void refresh() { invalidateFilter(); }

protected:
    bool filterAcceptsRow(int row, const QModelIndex &parent) const
    {
        if(m_mode == caAlarmTree::FilterNone) return true;
        const int id = m_tree->nodeIdFor(m_tree->index(row, 0, parent));
        if(id < 0) return true;
        const AlhNodeState &s = m_tree->state(id);
        if(m_mode == caAlarmTree::FilterActiveAlarms) return s.curSevr != NO_ALARM || s.unackSevr != NO_ALARM;
        return s.unackSevr != NO_ALARM;
    }

private:
    AlhTreeModel *m_tree;
    int m_mode;
};

// alh tree window: groups only
class AlhGroupProxy : public QSortFilterProxyModel
{
public:
    explicit AlhGroupProxy(AlhFilterProxy *filter, QObject *parent) : QSortFilterProxyModel(parent)
    {
        setSourceModel(filter);
        setDynamicSortFilter(true);
    }

protected:
    bool filterAcceptsRow(int row, const QModelIndex &parent) const
    {
        return sourceModel()->index(row, 0, parent).data(AlhTreeModel::IsGroupRole).toBool();
    }
};

//---------------------------------------------------------------------------- tree window with alh tree lines

class AlhTreeView : public QTreeView
{
public:
    explicit AlhTreeView(QWidget *parent) : QTreeView(parent) {}

protected:
    // alh treeSym: vertical lines for ancestors with further siblings, a branch for the item itself
    void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const
    {
        const int step = indentation();
        if(step <= 0 || !index.parent().isValid()) return;
        painter->save();
        painter->setPen(QPen(palette().color(QPalette::Mid), 1));
        QModelIndex node = index;
        int right = rect.right();
        for(int level = 0; node.parent().isValid() && right - step >= rect.left() - 1; level++) {
            const bool moreSiblings = node.row() < node.model()->rowCount(node.parent()) - 1;
            const int x = right - step / 2;
            const int yMid = rect.center().y();
            if(level == 0) {
                painter->drawLine(x, rect.top(), x, moreSiblings ? rect.bottom() : yMid);
                painter->drawLine(x, yMid, right, yMid);
            } else if(moreSiblings) {
                painter->drawLine(x, rect.top(), x, rect.bottom());
            }
            right -= step;
            node = node.parent();
        }
        painter->restore();
    }
};

// alh group window: flat list, expansion keys are left to the tree window
class AlhGroupView : public QTreeView
{
public:
    explicit AlhGroupView(QWidget *parent) : QTreeView(parent) {}

protected:
    void keyPressEvent(QKeyEvent *event)
    {
        switch(event->key()) {
        case Qt::Key_Plus: case Qt::Key_Minus: case Qt::Key_Asterisk: case Qt::Key_Left: case Qt::Key_Right:
        case Qt::Key_Backspace:
            event->ignore();
            return;
        default:
            QTreeView::keyPressEvent(event);
        }
    }
};

//---------------------------------------------------------------------------- line widgets of alh: ack, sevr, name, arrow, G, P, mask, message

class AlhLineDelegate : public QStyledItemDelegate
{
public:
    AlhLineDelegate(QTreeView *view, bool treeWindow) : QStyledItemDelegate(view), m_view(view), m_treeWindow(treeWindow) {}

    static bool isButtonColumn(int column)
    {
        return column == AlhTreeModel::ColUnackSevr || column == AlhTreeModel::ColName ||
               column == AlhTreeModel::ColArrow || column == AlhTreeModel::ColGuidance || column == AlhTreeModel::ColProcess;
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        opt.font = option.font;
        if(index.column() == AlhTreeModel::ColName) opt.font.setBold(true);
        const int col = index.column();
        const QString text = opt.text;
        const bool hover = (opt.state & QStyle::State_MouseOver) && isButtonColumn(col) && !text.isEmpty();
        const QPalette &pal = opt.palette;
        const QRect cell = opt.rect.adjusted(1, 1, -2, -1);

        painter->save();
        painter->setFont(opt.font);
        if(col == AlhTreeModel::ColUnackSevr || col == AlhTreeModel::ColCurSevr) {
            // ack push button / severity label in the severity colour, grey when nothing is pending
            QColor fill = pal.button().color();
            const QVariant bg = index.data(Qt::BackgroundRole);
            if(bg.isValid()) fill = bg.value<QColor>();
            if(hover) fill = fill.lighter(110);
            QRect box = cell;
            box.setWidth(qMin(cell.width(), QFontMetrics(opt.font).horizontalAdvance(QStringLiteral("W")) + 10));
            drawBox(painter, box, fill, col == AlhTreeModel::ColUnackSevr && !text.isEmpty() ? Raised : Flat);
            painter->setPen(Qt::black);
            painter->drawText(box, Qt::AlignCenter, text);
        } else if(col == AlhTreeModel::ColName) {
            // name push button: channels light blue like alh, the alh selection inverts the button
            const bool group = index.data(AlhTreeModel::IsGroupRole).toBool();
            const bool selected = opt.state & QStyle::State_Selected;
            QColor fill = group ? pal.button().color() : QColor(173, 216, 230);
            if(selected) fill = QApplication::palette().highlight().color();
            else if(hover) fill = fill.lighter(108);
            // button as wide as its text like the alh push button, the column keeps the widest name
            QRect button = cell;
            button.setWidth(qMin(cell.width(), QFontMetrics(opt.font).horizontalAdvance(text) + 12));
            drawBox(painter, button, fill, selected ? Sunken : Raised);
            QColor fg = selected ? QApplication::palette().highlightedText().color() : pal.buttonText().color();
            if(index.data(AlhTreeModel::InactiveRole).toBool() && !selected) fg = pal.color(QPalette::Disabled, QPalette::ButtonText);
            painter->setPen(fg);
            painter->drawText(button.adjusted(5, 0, -5, 0), Qt::AlignVCenter | Qt::AlignLeft, text);
        } else if(col == AlhTreeModel::ColArrow) {
            if(m_treeWindow ? !text.isEmpty() : index.data(AlhTreeModel::IsGroupRole).toBool()) {
                const bool down = m_treeWindow && m_view->isExpanded(index.sibling(index.row(), 0));
                const int h = qMin(cell.height() - 6, 10);
                const QPoint c = cell.center();
                QPolygon tri;
                if(down) tri << QPoint(c.x() - h / 2, c.y() - h / 3) << QPoint(c.x() + h / 2, c.y() - h / 3) << QPoint(c.x(), c.y() + h / 2);
                else tri << QPoint(c.x() - h / 3, c.y() - h / 2) << QPoint(c.x() - h / 3, c.y() + h / 2) << QPoint(c.x() + h / 2, c.y());
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->setPen(Qt::NoPen);
                painter->setBrush(hover ? QApplication::palette().highlight().color() : pal.buttonText().color());
                painter->drawPolygon(tri);
            }
        } else if(col == AlhTreeModel::ColGuidance || col == AlhTreeModel::ColProcess) {
            if(!text.isEmpty()) {
                QColor fill = pal.button().color();
                if(hover) fill = fill.lighter(110);
                drawBox(painter, cell, fill, Raised);
                painter->setPen(pal.buttonText().color());
                painter->drawText(cell, Qt::AlignCenter, text);
            }
        } else if(col == AlhTreeModel::ColMask) {
            // alh (A. Luedeke): blue mask label while D, A/H or T silences the line
            const bool highlight = index.data(AlhTreeModel::MaskHighlightRole).toBool();
            if(highlight) {
                painter->fillRect(cell, QColor(0, 0, 255));
                painter->setPen(Qt::white);
            } else {
                painter->setPen(pal.text().color());
            }
            painter->drawText(cell.adjusted(3, 0, -3, 0), Qt::AlignVCenter | Qt::AlignLeft, text);
        } else {
            const QVariant fg = index.data(Qt::ForegroundRole);
            painter->setPen(fg.isValid() ? fg.value<QColor>() : pal.text().color());
            painter->drawText(cell.adjusted(3, 0, -3, 0), Qt::AlignVCenter | Qt::AlignLeft, text);
        }
        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        opt.font = option.font;
        if(index.column() == AlhTreeModel::ColName) opt.font.setBold(true);
        const QFontMetrics fm(opt.font);
        QSize s(fm.horizontalAdvance(opt.text) + 8, fm.height() + 8);
        switch(index.column()) {
        case AlhTreeModel::ColUnackSevr:
        case AlhTreeModel::ColCurSevr: s.setWidth(fm.horizontalAdvance(QStringLiteral("W")) + 14); break;
        case AlhTreeModel::ColArrow: s.setWidth((m_treeWindow ? opt.text.isEmpty() : !index.data(AlhTreeModel::IsGroupRole).toBool()) ? 4 : 18); break;
        case AlhTreeModel::ColGuidance:
        case AlhTreeModel::ColProcess: s.setWidth(opt.text.isEmpty() ? 4 : fm.horizontalAdvance(QStringLiteral("W")) + 14); break;
        case AlhTreeModel::ColName: s.setWidth(fm.horizontalAdvance(opt.text) + 14); break;
        default: break;
        }
        return s;
    }

private:
    enum Relief { Flat, Raised, Sunken };

    static void drawBox(QPainter *painter, const QRect &r, const QColor &fill, Relief relief)
    {
        painter->fillRect(r, fill);
        if(relief == Flat) {                   // alh: insensitive button / label, outline only
            painter->setPen(fill.darker(115));
            painter->drawRect(r.adjusted(0, 0, -1, -1));
            return;
        }
        const QColor light = fill.lighter(140), dark = fill.darker(150);
        painter->setPen(relief == Raised ? light : dark);
        painter->drawLine(r.topLeft(), r.topRight());
        painter->drawLine(r.topLeft(), r.bottomLeft());
        painter->setPen(relief == Raised ? dark : light);
        painter->drawLine(r.bottomLeft(), r.bottomRight());
        painter->drawLine(r.topRight(), r.bottomRight());
    }

    QTreeView *m_view;
    bool m_treeWindow;
};

//---------------------------------------------------------------------------- construction

caAlarmTree::caAlarmTree(QWidget *parent)
    : QWidget(parent), m_alarmMode(true), m_beepSeverity(-1), m_commandMode(Shell), m_displayFilter(FilterNone),
      m_silenceForever(false), m_silenceOneHour(false), m_silenceCurrent(false),
      m_showMenuBar(true), m_showTreeWindow(true), m_showStatusArea(true), m_showMask(true), m_showValue(true), m_showPv(false),
      m_activated(false), m_selected(-1), m_groupRoot(-1), m_configAux(-1),
      m_beepActive(false), m_lastBeepSevr(-1), m_menuNode(-1), m_cfgTreeMute(false), m_configNelm(0)
{
    m_tree = new AlhTreeModel(this);
    m_proxy = new AlhFilterProxy(m_tree, this);
    m_groupProxy = new AlhGroupProxy(m_proxy, this);
    buildUi();

    // the lib wires these like any other widget of the panel; the tree only fills them on demand
    m_shell = new caShellCommand(this);
    m_shell->hide();
    m_script = new caScriptButton(this);
    m_script->hide();

    m_beepTimer = new QTimer(this);
    m_beepTimer->setInterval(BeepRepeatMs);
    connect(m_beepTimer, SIGNAL(timeout()), this, SLOT(beepTimeout()));
    m_heartbeat = new QTimer(this);
    connect(m_heartbeat, SIGNAL(timeout()), this, SLOT(heartbeatTimeout()));
    m_filterTimer = new QTimer(this);          // delayed $ALARMCOUNTFILTER transitions
    m_filterTimer->setInterval(1000);
    connect(m_filterTimer, SIGNAL(timeout()), this, SLOT(filterTimeout()));
    m_noAckTimer = new QTimer(this);           // "NoAck for One Hour" expiry
    m_noAckTimer->setInterval(10000);
    connect(m_noAckTimer, SIGNAL(timeout()), this, SLOT(noAckTimeout()));
    m_silenceOneHourTimer = new QTimer(this);
    m_silenceOneHourTimer->setSingleShot(true);
    m_silenceOneHourTimer->setInterval(3600 * 1000);
    connect(m_silenceOneHourTimer, SIGNAL(timeout()), this, SLOT(silenceOneHourTimeout()));
    m_refilterTimer = new QTimer(this);
    m_refilterTimer->setSingleShot(true);
    m_refilterTimer->setInterval(200);
    connect(m_refilterTimer, SIGNAL(timeout()), this, SLOT(refilter()));

    m_host = QSysInfo::machineHostName();
    m_pid = QCoreApplication::applicationPid();
    updateStatus();
}

caAlarmTree::~caAlarmTree()
{
}

QAction *caAlarmTree::menuAction(QMenu *menu, const QString &text, const char *slot, const QString &shortcut)
{
    QAction *a = new QAction(text, this);
    if(!shortcut.isEmpty()) {
        a->setShortcut(QKeySequence(shortcut));
        a->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        addAction(a);                          // shortcut active while the tree has the focus
    }
    if(slot) connect(a, SIGNAL(triggered()), this, slot);
    menu->addAction(a);
    return a;
}

static QTreeView *configureView(QTreeView *view, bool treeWindow)
{
    view->setItemDelegate(new AlhLineDelegate(view, treeWindow));
    view->setHeaderHidden(true);
    view->setUniformRowHeights(true);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setExpandsOnDoubleClick(false);
    view->setAllColumnsShowFocus(true);
    view->setMouseTracking(true);
    view->setFocusPolicy(Qt::StrongFocus);
    view->setRootIsDecorated(false);
    view->setItemsExpandable(treeWindow);
    view->setIndentation(treeWindow ? 20 : 0);
    view->setTreePosition(AlhTreeModel::ColName);  // tree lines in front of the name button, ack/sevr stay in place
    view->header()->setStretchLastSection(false);
    view->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    view->header()->setMinimumSectionSize(4);
    QPalette pal = view->palette();           // alh marks the name button only, no row highlight
    pal.setColor(QPalette::Highlight, Qt::transparent);
    view->setPalette(pal);
    return view;
}

void caAlarmTree::buildUi()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(1);

    buildMenuBar();
    layout->addWidget(m_menuBar);

    // alh main window: tree window (groups) on the left, group window (contents of the selected group) on the right
    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_splitter->setChildrenCollapsible(false);
    m_treeView = configureView(new AlhTreeView(m_splitter), true);
    m_treeView->setModel(m_groupProxy);
    connect(m_treeView, SIGNAL(clicked(QModelIndex)), this, SLOT(onTreeClicked(QModelIndex)));
    connect(m_treeView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onTreeDoubleClicked(QModelIndex)));
    connect(m_treeView, SIGNAL(expanded(QModelIndex)), this, SLOT(onTreeExpansionChanged()));
    connect(m_treeView, SIGNAL(collapsed(QModelIndex)), this, SLOT(onTreeExpansionChanged()));
    m_groupView = configureView(new AlhGroupView(m_splitter), false);
    m_groupView->setModel(m_proxy);
    connect(m_groupView, SIGNAL(clicked(QModelIndex)), this, SLOT(onGroupClicked(QModelIndex)));
    connect(m_groupView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onGroupDoubleClicked(QModelIndex)));
    m_splitter->addWidget(m_treeView);
    m_splitter->addWidget(m_groupView);
    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 1);
    layout->addWidget(m_splitter, 1);
    setFocusProxy(m_groupView);

    buildStatusArea();
    layout->addWidget(m_statusArea);
    applyColumns();

    // alh: space bar acknowledges the selected line
    QShortcut *space = new QShortcut(QKeySequence(Qt::Key_Space), this);
    space->setContext(Qt::WidgetWithChildrenShortcut);
    connect(space, SIGNAL(activated()), this, SLOT(acknowledgeCurrent()));
}

// menu bar of the alh main window; the Setup entries show the current state
void caAlarmTree::buildMenuBar()
{
    m_menuBar = new QMenuBar(this);
    m_menuBar->setNativeMenuBar(false);

    QMenu *action = m_menuBar->addMenu(tr("Action"));
    menuAction(action, tr("Acknowledge Alarm"), SLOT(acknowledgeCurrent()), QStringLiteral("Ctrl+A"));
    menuAction(action, tr("Display Guidance"), SLOT(showGuidance()), QStringLiteral("Ctrl+G"));
    menuAction(action, tr("Start Related Process"), SLOT(runRelatedDisplay()), QStringLiteral("Ctrl+P"));
    menuAction(action, tr("Start Related Process with Output..."), SLOT(runCommandWithOutput()));
    menuAction(action, tr("Force Process Variable..."), SLOT(forceProcessVariableDialog()), QStringLiteral("Ctrl+V"));
    menuAction(action, tr("Force Mask..."), SLOT(forceMaskDialog()), QStringLiteral("Ctrl+M"));
    menuAction(action, tr("Beep Severity..."), SLOT(beepSeverityDialog()), QStringLiteral("Ctrl+B"));
    menuAction(action, tr("NoAck for One Hour..."), SLOT(noAckOneHourDialog()), QStringLiteral("Ctrl+N"));
    action->addSeparator();
    menuAction(action, tr("Acknowledge All"), SLOT(acknowledgeAll()));
    menuAction(action, tr("Disable"), SLOT(disableCurrent()));
    menuAction(action, tr("Enable"), SLOT(enableCurrent()));
    menuAction(action, tr("Mute"), SLOT(muteCurrent()));
    menuAction(action, tr("Unmute"), SLOT(unmuteCurrent()));
    menuAction(action, tr("Reset to Configuration"), SLOT(resetCurrentToConfig()));

    QMenu *view = m_menuBar->addMenu(tr("View"));
    menuAction(view, tr("Expand One Level"), SLOT(expandOneLevel()));
    menuAction(view, tr("Expand Branch"), SLOT(expandBranch()));
    menuAction(view, tr("Expand All"), SLOT(expandAllNodes()));
    menuAction(view, tr("Collapse Branch"), SLOT(collapseBranch()));
    view->addSeparator();
    menuAction(view, tr("Current Alarm History Window"), SLOT(showAlarmHistory()));
    menuAction(view, tr("Configuration File Window"), SLOT(showConfigFile()));
    menuAction(view, tr("Group/Channel Properties Window"), SLOT(showProperties()));

    QMenu *setup = m_menuBar->addMenu(tr("Setup"));
    QMenu *filter = setup->addMenu(tr("Display Filter"));
    QActionGroup *filterGroup = new QActionGroup(this);
    const char *filterTexts[3] = { "No filter", "Active Alarms Only", "Unacknowledged Alarms Only" };
    for(int i = 0; i < 3; i++) {
        m_filterActions[i] = filter->addAction(tr(filterTexts[i]));
        m_filterActions[i]->setCheckable(true);
        m_filterActions[i]->setData(i);
        filterGroup->addAction(m_filterActions[i]);
    }
    connect(filterGroup, SIGNAL(triggered(QAction*)), this, SLOT(onFilterAction(QAction*)));
    QMenu *beep = setup->addMenu(tr("ALH Beep Severity"));
    QActionGroup *beepGroup = new QActionGroup(this);
    const char *beepTexts[3] = { "Minor", "Major", "Invalid" };
    const int beepValues[3] = { MINOR_ALARM, MAJOR_ALARM, INVALID_ALARM };
    for(int i = 0; i < 3; i++) {
        m_beepActions[i] = beep->addAction(tr(beepTexts[i]));
        m_beepActions[i]->setCheckable(true);
        m_beepActions[i]->setData(beepValues[i]);
        beepGroup->addAction(m_beepActions[i]);
    }
    connect(beepGroup, SIGNAL(triggered(QAction*)), this, SLOT(onBeepAction(QAction*)));
    m_silenceForeverAction = setup->addAction(tr("Silence Forever"));
    m_silenceForeverAction->setCheckable(true);
    connect(m_silenceForeverAction, SIGNAL(toggled(bool)), this, SLOT(setSilenceForever(bool)));
    m_muteAllAction = setup->addAction(tr("Mute All"));
    m_muteAllAction->setCheckable(true);
    connect(m_muteAllAction, SIGNAL(toggled(bool)), this, SLOT(onMuteAllToggled(bool)));
    syncSetupActions();
}

// message area of the alh main window: legend lines on the left, silence controls on the right
void caAlarmTree::buildStatusArea()
{
    m_statusArea = new QWidget(this);
    m_statusArea->setObjectName(QStringLiteral("alhMessageArea"));
    QGridLayout *grid = new QGridLayout(m_statusArea);
    grid->setContentsMargins(6, 2, 6, 2);
    grid->setHorizontalSpacing(12);
    grid->setVerticalSpacing(1);

    m_execLabel = new QLabel(m_statusArea);
    grid->addWidget(m_execLabel, 0, 0);
    grid->addWidget(new QLabel(tr("Mask <CDATL>:  <Cancel,Disable,noAck,noackT,noLog>      H=noAck 1hr timer"), m_statusArea), 1, 0);
    grid->addWidget(new QLabel(tr("Group Alarm Counts:  (ERROR,INVALID,MAJOR,MINOR,NOALARM)"), m_statusArea), 2, 0);
    grid->addWidget(new QLabel(tr("Channel Alarm Data:  <Status,Severity>,<Unack Severity>"), m_statusArea), 3, 0);
    m_fileLabel = new QLabel(m_statusArea);
    m_fileLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    grid->addWidget(m_fileLabel, 4, 0);
    grid->setColumnStretch(1, 1);

    QVBoxLayout *right = new QVBoxLayout;
    right->setContentsMargins(0, 0, 0, 0);
    right->setSpacing(1);
    m_silenceOneHourBox = new QCheckBox(tr("SilenceOneHour"), m_statusArea);
    m_silenceOneHourBox->setFocusPolicy(Qt::NoFocus);
    connect(m_silenceOneHourBox, SIGNAL(toggled(bool)), this, SLOT(setSilenceOneHour(bool)));
    right->addWidget(m_silenceOneHourBox, 0, Qt::AlignLeft);
    m_silenceCurrentBox = new QCheckBox(tr("SilenceCurrent"), m_statusArea);
    m_silenceCurrentBox->setFocusPolicy(Qt::NoFocus);
    connect(m_silenceCurrentBox, SIGNAL(toggled(bool)), this, SLOT(setSilenceCurrent(bool)));
    right->addWidget(m_silenceCurrentBox, 0, Qt::AlignLeft);
    m_silenceForeverLabel = new QLabel(m_statusArea);
    right->addWidget(m_silenceForeverLabel, 0, Qt::AlignLeft);
    m_beepLabel = new QLabel(m_statusArea);
    right->addWidget(m_beepLabel, 0, Qt::AlignLeft);
    m_forceCountLabel = new QLabel(m_statusArea);
    right->addWidget(m_forceCountLabel, 0, Qt::AlignLeft);
    right->addStretch(1);
    grid->addLayout(right, 0, 2, 5, 1, Qt::AlignTop);
}

void caAlarmTree::setShowMenuBar(bool on)
{
    m_showMenuBar = on;
    m_menuBar->setVisible(on);
}

void caAlarmTree::setShowTreeWindow(bool on)
{
    m_showTreeWindow = on;
    m_treeView->setVisible(on);
}

void caAlarmTree::setShowStatusArea(bool on)
{
    m_showStatusArea = on;
    m_statusArea->setVisible(on);
}

void caAlarmTree::setAlarmMode(bool on)
{
    m_alarmMode = on;
    if(m_heartbeat->interval() > 0) {
        if(on && m_activated) m_heartbeat->start();
        else m_heartbeat->stop();
    }
    updateBeepState();
    updateStatus();
}

void caAlarmTree::setBeepSeverity(int sevr)
{
    m_beepSeverity = sevr;
    syncSetupActions();
    updateBeepState();
    updateStatus();
}

void caAlarmTree::applyColumns()
{
    // the tree window shows the alh line only, value and pv are extensions of the group window
    m_treeView->setColumnHidden(AlhTreeModel::ColMask, !m_showMask);
    m_treeView->setColumnHidden(AlhTreeModel::ColValue, true);
    m_treeView->setColumnHidden(AlhTreeModel::ColPv, true);
    m_groupView->setColumnHidden(AlhTreeModel::ColMask, !m_showMask);
    m_groupView->setColumnHidden(AlhTreeModel::ColValue, !m_showValue);
    m_groupView->setColumnHidden(AlhTreeModel::ColPv, !m_showPv);
}

// Setup menu entries follow the properties and the status area
void caAlarmTree::syncSetupActions()
{
    for(int i = 0; i < 3; i++) {
        m_filterActions[i]->blockSignals(true);
        m_filterActions[i]->setChecked(i == (int) m_displayFilter);
        m_filterActions[i]->blockSignals(false);
    }
    const int beep = m_beepSeverity >= 0 ? m_beepSeverity : AlhTreeModel::severityFromIndex(m_tree->alhModel().effectiveBeepSeverity());
    for(int i = 0; i < 3; i++) {
        m_beepActions[i]->blockSignals(true);
        m_beepActions[i]->setChecked(m_beepActions[i]->data().toInt() == beep);
        m_beepActions[i]->blockSignals(false);
    }
    m_silenceForeverAction->blockSignals(true);
    m_silenceForeverAction->setChecked(m_silenceForever);
    m_silenceForeverAction->blockSignals(false);
    m_muteAllAction->blockSignals(true);
    m_muteAllAction->setChecked(m_tree->treeMuted());
    m_muteAllAction->blockSignals(false);
}

QString caAlarmTree::userName() const
{
    QString user = QString::fromLocal8Bit(qgetenv("USER"));
    if(user.isEmpty()) user = QString::fromLocal8Bit(qgetenv("USERNAME"));
    return user;
}

void caAlarmTree::updateStatus()
{
    QString exec = tr("Execution Status:  %1").arg(m_alarmMode ? tr("Local Active") : tr("Local Passive"));
    if(m_displayFilter != FilterNone) exec += tr("      Filter:  %1").arg(m_filterActions[m_displayFilter]->text());
    if(m_silenceOneHour || m_silenceForever || m_tree->treeMuted()) exec += tr("      (silenced)");
    m_execLabel->setText(exec);
    m_fileLabel->setText(tr("Filename:  %1").arg(m_resolvedConfig.isEmpty() ? m_configFile : m_resolvedConfig));
    m_silenceForeverLabel->setText(tr("Silence Forever:  %1").arg(m_silenceForever ? tr("On") : tr("Off")));
    const int beep = m_beepSeverity >= 0 ? m_beepSeverity : AlhTreeModel::severityFromIndex(m_tree->alhModel().effectiveBeepSeverity());
    m_beepLabel->setText(tr("ALH Beep Severity:  %1").arg(AlhTreeModel::severityText(beep)));
    int disabledForce = 0;
    for(QHash<int, bool>::const_iterator it = m_forced.constBegin(); it != m_forced.constEnd(); ++it) if(it.value()) disabledForce++;
    m_forceCountLabel->setText(disabledForce > 0 ? tr("Disabled ForcePV Count:  %1").arg(disabledForce) : QString());
    // alh: message area turns blue while "silence one hour" runs; both states keep a style sheet so the look is constant
    m_statusArea->setStyleSheet(m_silenceOneHour
        ? QStringLiteral("QWidget#alhMessageArea { background-color: blue; } QWidget#alhMessageArea QLabel, QWidget#alhMessageArea QCheckBox { color: white; }")
        : QStringLiteral("QWidget#alhMessageArea { background-color: palette(window); }"));
    syncSetupActions();
}

//---------------------------------------------------------------------------- index helpers and navigation

int caAlarmTree::nodeIdFor(const QModelIndex &viewIndex) const
{
    if(!viewIndex.isValid()) return -1;
    return viewIndex.data(AlhTreeModel::NodeIdRole).toInt();
}

QModelIndex caAlarmTree::groupIndexFor(int id) const
{
    if(id < 0) return QModelIndex();
    return m_proxy->mapFromSource(m_tree->indexFor(id, 0));
}

QModelIndex caAlarmTree::treeIndexFor(int id) const
{
    return m_groupProxy->mapFromSource(groupIndexFor(id));
}

int caAlarmTree::nodeUnderCursor() const
{
    QPoint pos = m_treeView->viewport()->mapFromGlobal(QCursor::pos());
    if(m_treeView->isVisible() && m_treeView->viewport()->rect().contains(pos)) return nodeIdFor(m_treeView->indexAt(pos));
    pos = m_groupView->viewport()->mapFromGlobal(QCursor::pos());
    if(m_groupView->viewport()->rect().contains(pos)) return nodeIdFor(m_groupView->indexAt(pos));
    return -1;
}

int caAlarmTree::currentNode() const
{
    if(m_menuNode >= 0) return m_menuNode;
    return m_selected;
}

int caAlarmTree::currentGroup() const
{
    int id = currentNode();
    if(id >= 0 && m_tree->node(id).isChannel()) id = m_tree->node(id).parentId;
    return id;
}

int caAlarmTree::selectedOrWarn()
{
    const int id = currentNode();
    m_menuNode = -1;
    if(id < 0) QMessageBox::warning(window(), tr("Alarm Handler"), tr(selectFirstText));
    return id;
}

// alh markSelection: one marked line in either window
void caAlarmTree::selectNode(int id)
{
    m_selected = id;
    const QModelIndex gi = groupIndexFor(id);
    const QModelIndex ti = treeIndexFor(id);
    m_groupView->selectionModel()->clear();
    m_treeView->selectionModel()->clear();
    if(gi.isValid() && gi.parent() == m_groupView->rootIndex()) {
        m_groupView->setCurrentIndex(gi);
    } else if(ti.isValid() && treeShows(id)) {
        m_treeView->setCurrentIndex(ti);
    }
}

// all ancestors expanded in the tree window
bool caAlarmTree::treeShows(int id) const
{
    for(int p = (id >= 0) ? m_tree->node(id).parentId : -1; p >= 0; p = m_tree->node(p).parentId) {
        const QModelIndex ti = treeIndexFor(p);
        if(!ti.isValid() || !m_treeView->isExpanded(ti)) return false;
    }
    return true;
}

// group window: the group's contents replace the window, the tree window follows if it shows the group
void caAlarmTree::descendInto(int id)
{
    if(id < 0 || !m_tree->node(id).isGroup()) return;
    showGroupContents(id);
    selectNode(id);
}

// alh group window: contents of the group selected in the tree window
void caAlarmTree::showGroupContents(int groupId)
{
    if(groupId >= 0 && !m_tree->node(groupId).isGroup()) groupId = m_tree->node(groupId).parentId;
    m_groupRoot = groupId;
    m_groupView->setRootIndex(groupIndexFor(groupId));
    m_groupView->selectionModel()->clear();
    m_groupView->scrollToTop();
}

void caAlarmTree::expandTreeTo(int id)
{
    for(int p = (id >= 0) ? m_tree->node(id).parentId : -1; p >= 0; p = m_tree->node(p).parentId) {
        const QModelIndex ti = treeIndexFor(p);
        if(ti.isValid()) m_treeView->expand(ti);
    }
}

// the proxies rebuild their indexes on every filter change
void caAlarmTree::restoreGroupRoot()
{
    if(m_groupRoot >= 0 && !m_groupView->rootIndex().isValid()) {
        const QModelIndex gi = groupIndexFor(m_groupRoot);
        if(gi.isValid()) m_groupView->setRootIndex(gi);
    }
}

// column widths once from the whole configuration, so the lines do not shift with the visible rows
void caAlarmTree::fixColumnWidths()
{
    const AlhModel &m = m_tree->alhModel();
    QFont bold = m_groupView->font();
    bold.setBold(true);
    const QFontMetrics fm(m_groupView->font()), fmBold(bold);
    int nameW = fmBold.horizontalAdvance(QStringLiteral("Name")) + 14, groupNameW = nameW, maxDepth = 0, channels = 0;
    bool anyGuidance = false, anyCommand = false, anySubGroup = false, anyGroup = false;
    for(int id = 0; id < m.nodeCount(); id++) {
        const AlhNode &n = m.node(id);
        nameW = qMax(nameW, fmBold.horizontalAdvance(n.displayName()) + 14);
        if(n.isGroup()) groupNameW = qMax(groupNameW, fmBold.horizontalAdvance(n.displayName()) + 14);
        anyGuidance = anyGuidance || !n.guidanceText.isEmpty() || !n.guidanceLocation.isEmpty();
        anyCommand = anyCommand || !n.command.isEmpty();
        if(n.isChannel()) { channels++; continue; }
        anyGroup = true;
        int depth = 0;
        for(int p = n.parentId; p >= 0; p = m.node(p).parentId) depth++;
        maxDepth = qMax(maxDepth, depth);
        anySubGroup = anySubGroup || (n.parentId >= 0);
    }
    const int letterW = fm.horizontalAdvance(QStringLiteral("W")) + 14;
    const QString digits(QString::number(qMax(channels, 1)).size(), QLatin1Char('9'));
    int longestStat = 0;
    foreach(const QString &stat, AlhModel::statusNames()) longestStat = qMax(longestStat, fm.horizontalAdvance(stat));
    const int infoW = qMax(fm.horizontalAdvance(QString("(%1,%1,%1,%1,%1)").arg(digits)),
                           longestStat + fm.horizontalAdvance(QStringLiteral("<,NO_ALARM>,<NO_ALARM>"))) + 8;
    const int maskW = fm.horizontalAdvance(QStringLiteral("<CDATL> M")) + 8;

    QTreeView *views[2] = { m_treeView, m_groupView };
    for(int v = 0; v < 2; v++) {
        QHeaderView *h = views[v]->header();
        const bool tree = (v == 0);
        for(int c = AlhTreeModel::ColUnackSevr; c <= AlhTreeModel::ColInfo; c++) h->setSectionResizeMode(c, QHeaderView::Fixed);
        h->resizeSection(AlhTreeModel::ColUnackSevr, letterW);
        h->resizeSection(AlhTreeModel::ColCurSevr, letterW);
        h->resizeSection(AlhTreeModel::ColName, tree ? groupNameW + maxDepth * views[v]->indentation() : nameW);
        h->resizeSection(AlhTreeModel::ColArrow, (tree ? anySubGroup : anyGroup) ? 18 : 4);
        h->resizeSection(AlhTreeModel::ColGuidance, anyGuidance ? letterW : 4);
        h->resizeSection(AlhTreeModel::ColProcess, anyCommand ? letterW : 4);
        h->resizeSection(AlhTreeModel::ColMask, maskW);
        h->resizeSection(AlhTreeModel::ColInfo, infoW);
    }
}

// after loading: one root group selected and its contents in the group window
void caAlarmTree::initViews()
{
    m_selected = -1;
    m_groupRoot = -1;
    fixColumnWidths();
    m_treeView->expandToDepth(0);
    const QVector<int> &roots = m_tree->alhModel().roots();      // QVector: Qt5 has no implicit QList conversion
    if(roots.size() == 1) {
        showGroupContents(roots.first());
        selectNode(roots.first());
    } else {
        m_groupView->setRootIndex(QModelIndex());
    }
}

//---------------------------------------------------------------------------- clicks in the two windows (alh awView.c)

// ack, G and P buttons of a line; true when the click was consumed
bool caAlarmTree::lineButtonClicked(int id, int column)
{
    switch(column) {
    case AlhTreeModel::ColUnackSevr:
        if(m_tree->state(id).unackSevr != NO_ALARM) acknowledgeNode(id);
        return true;
    case AlhTreeModel::ColGuidance: {
        const AlhNode &n = m_tree->node(id);
        if(!n.guidanceText.isEmpty() || !n.guidanceLocation.isEmpty()) { m_menuNode = id; showGuidance(); }
        return true;
    }
    case AlhTreeModel::ColProcess:
        if(!m_tree->node(id).command.isEmpty()) runCommand(id, m_commandMode == Script);
        return true;
    default:
        return false;
    }
}

// tree window: name selects the group and fills the group window, arrow expands/collapses one level
void caAlarmTree::onTreeClicked(const QModelIndex &index)
{
    const int id = nodeIdFor(index);
    if(id < 0) return;
    m_menuNode = -1;
    if(lineButtonClicked(id, index.column())) { m_treeView->selectionModel()->clear(); return; }
    if(index.column() == AlhTreeModel::ColArrow) {
        const QModelIndex ti = index.sibling(index.row(), 0);
        if(!index.data(Qt::DisplayRole).toString().isEmpty()) m_treeView->setExpanded(ti, !m_treeView->isExpanded(ti));
        m_treeView->selectionModel()->clear();
        return;
    }
    showGroupContents(id);
    selectNode(id);
}

// tree window: double click on the arrow expands the branch
void caAlarmTree::onTreeDoubleClicked(const QModelIndex &index)
{
    const int id = nodeIdFor(index);
    if(id < 0 || index.column() != AlhTreeModel::ColArrow) return;
    expandRecursively(index.sibling(index.row(), 0), true);
}

// group window: name marks the line, arrow descends into the group
void caAlarmTree::onGroupClicked(const QModelIndex &index)
{
    const int id = nodeIdFor(index);
    if(id < 0) return;
    m_menuNode = -1;
    if(lineButtonClicked(id, index.column())) { m_groupView->selectionModel()->clear(); return; }
    if(index.column() == AlhTreeModel::ColArrow) {
        m_groupView->selectionModel()->clear();
        descendInto(id);
        return;
    }
    selectNode(id);
}

// group window: double click on a group descends into it, on a channel acknowledges it
void caAlarmTree::onGroupDoubleClicked(const QModelIndex &index)
{
    const int id = nodeIdFor(index);
    if(id < 0 || (AlhLineDelegate::isButtonColumn(index.column()) && index.column() != AlhTreeModel::ColName)) return;
    if(m_tree->node(id).isGroup()) descendInto(id);
    else acknowledgeNode(id);
}

void caAlarmTree::onTreeExpansionChanged()
{
    m_treeView->viewport()->update();          // arrow direction
}

void caAlarmTree::onFilterAction(QAction *action)
{
    setDisplayFilter(action->data().toInt());
}

void caAlarmTree::onBeepAction(QAction *action)
{
    setBeepSeverity(action->data().toInt());
}

void caAlarmTree::onMuteAllToggled(bool on)
{
    if(on) muteAll(); else unmuteAll();
}

//---------------------------------------------------------------------------- activation

QMap<QString, QString> caAlarmTree::macroMap(const QMap<QString, QString> &libMap) const
{
    QMap<QString, QString> map = libMap;
    // the macros property wins over the panel macros
    foreach(const QString &part, m_macros.split(QLatin1Char(','), ALH_SKIP_EMPTY)) {
        const int eq = part.indexOf(QLatin1Char('='));
        if(eq > 0) map.insert(part.left(eq).trimmed(), part.mid(eq + 1).trimmed());
    }
    return map;
}

QString caAlarmTree::resolveConfigFile(const QMap<QString, QString> &map) const
{
    QString f = AlhConfigParser::expandMacros(m_configFile.trimmed(), map);
    if(f.isEmpty()) return f;
    if(QFileInfo(f).isAbsolute()) return f;
    const QString uiPath = map.value(QStringLiteral("CAQTDM_INTERNAL_UIPATH"));
    if(!uiPath.isEmpty() && QFile::exists(uiPath + f)) return uiPath + f;
    searchFile search(f);
    const QString found = search.findFile();
    return found.isNull() ? f : found;
}

int caAlarmTree::addAux(int kind, int node, int input, const QString &pv)
{
    AuxPv a;
    a.kind = kind;
    a.node = node;
    a.input = input;
    a.pv = pv;
    m_aux.append(a);
    const int index = m_aux.size() - 1;
    PvRequest r;
    r.specId = AuxBase + index;
    r.pv = pv;
    m_requests.append(r);
    return index;
}

void caAlarmTree::setResolvedPv(int specId, const QString &pvRep)
{
    if(specId >= AuxBase && specId - AuxBase < m_aux.size()) m_aux[specId - AuxBase].pv = pvRep;
}

// called by caQtDM_Lib (HandleWidget) before it subscribes to requestedPvs()
void caAlarmTree::loadConfiguration(const QMap<QString, QString> &map)
{
    m_map = map;
    m_requests.clear();
    m_aux.clear();
    m_shell->setObjectName(objectName() + QStringLiteral("_shell"));
    m_script->setObjectName(objectName() + QStringLiteral("_script"));

    AlhModel model;
    AlhConfigParser::Options options;
    options.includeDir = AlhConfigParser::expandMacros(m_includeDir.trimmed(), map);
    options.macros = macroMap(map);
    m_resolvedConfig = resolveConfigFile(options.macros);
    if(m_resolvedConfig.isEmpty()) {
        qCWarning(caAlarmTreeLog) << objectName() << "no configFile set";
    } else {
        AlhConfigParser parser(options);
        if(!parser.parseFile(m_resolvedConfig, &model))
            qCWarning(caAlarmTreeLog) << objectName() << "cannot read" << m_resolvedConfig;
        foreach(const AlhWarning &w, model.warnings()) qCWarning(caAlarmTreeLog) << w.toString();
    }
    m_tree->setAlhModel(model);
    initViews();
    syncSetupActions();

    // channel pvs: specData[0] = node id
    for(int id = 0; id < model.nodeCount(); id++) {
        const AlhNode &n = model.node(id);
        if(!n.isChannel() || n.mask.cancel) continue;
        PvRequest r;
        r.specId = id;
        r.pv = n.name;
        m_requests.append(r);
    }
    // auxiliary pvs: specData[0] = AuxBase + index
    for(int id = 0; id < model.nodeCount(); id++) {
        const AlhNode &n = model.node(id);
        if(n.hasForcePv) {
            if(n.forcePv.isCalc) {
                CalcState &c = m_calc[id];
                for(int i = 0; i < 6; i++) {
                    const QString in = n.forcePv.calcInput[i];
                    if(in.isEmpty()) continue;
                    double v;
                    if(AlhForcePv::inputIsConstant(in, &v)) { c.in[i] = v; c.known[i] = true; }
                    else addAux(AuxPv::CalcInput, id, i, in);
                }
            } else {
                addAux(AuxPv::ForcePv, id, -1, n.forcePv.pv);
            }
        }
        if(!n.ackPv.isEmpty()) addAux(AuxPv::AckPv, id, -1, n.ackPv);
        if(!n.sevrPv.isEmpty()) {
            addAux(AuxPv::SevrPv, id, -1, n.sevrPv);
            m_sevrPvNodes.append(id);
        }
    }
    if(model.hasHeartbeat()) {
        addAux(AuxPv::HeartbeatPv, -1, -1, model.heartbeatPv());
        m_heartbeat->setInterval(qMax(1, (int) (model.heartbeatRate() * 1000.0)));
        if(m_alarmMode) m_heartbeat->start();
    }
    for(int id = 0; id < model.nodeCount(); id++) {
        if(model.node(id).hasCountFilter) { m_filterTimer->start(); break; }
    }
    const QString configPv = AlhConfigParser::expandMacros(m_configPV.trimmed(), map);
    if(!configPv.isEmpty()) m_configAux = addAux(AuxPv::ConfigPv, -1, -1, configPv);
    m_activated = true;
    updateStatus();

    qCInfo(caAlarmTreeLog) << objectName() << "config" << m_resolvedConfig << "groups" << model.groupCount()
                           << "channels" << model.channelCount() << "aux" << m_aux.size() << "warnings" << model.warnings().size();
    QVariantMap ev;
    ev.insert(QStringLiteral("action"), QStringLiteral("start"));
    ev.insert(QStringLiteral("channels"), model.channelCount());
    ev.insert(QStringLiteral("groups"), model.groupCount());
    ev.insert(QStringLiteral("warnings"), model.warnings().size());
    emitEvent(ev);
}

//---------------------------------------------------------------------------- data updates

QString caAlarmTree::valueText(const QString &String, const knobData &data) const
{
    if(data.edata.fieldtype == caENUM) {
        const QStringList list = String.split(QChar(27));
        const int i = (int) data.edata.ivalue;
        if(i >= 0 && i < list.size() && !list.at(i).trimmed().isEmpty()) return list.at(i);
        return QString::number(i);
    }
    if(data.edata.fieldtype == caSTRING) return String;
    if(data.edata.fieldtype == caCHAR) {
        if(data.edata.nelm > 1) return String;
        return QString::number(data.edata.ivalue);
    }
    if(data.edata.precision > 0 && data.edata.precision < 10 && data.edata.fieldtype != caINT && data.edata.fieldtype != caLONG)
        return QString::number(data.edata.rvalue, 'f', data.edata.precision);
    return QString::number(data.edata.rvalue, 'g', 10);
}

// called by caQtDM_Lib (Callback_UpdateWidget) for every monitored pv
void caAlarmTree::dataUpdate(const QString &String, const knobData &data)
{
    const int sd = data.specData[0];
    if(sd >= AuxBase) handleAuxUpdate(sd - AuxBase, String, data);
    else handleChannelUpdate(sd, String, data);
}

void caAlarmTree::handleChannelUpdate(int id, const QString &String, const knobData &data)
{
    if(id < 0 || id >= m_tree->alhModel().nodeCount()) return;
    const AlhNodeState &s = m_tree->state(id);
    const bool connected = data.edata.connected != 0;
    const int sevr = data.edata.severity;
    const short stat = data.edata.status;
    const QString value = connected ? valueText(String, data) : QString();

    // disconnected pvs are re-emitted periodically by the lib; only real changes go through
    if(s.connected == connected && (!connected || (s.lastReceivedSevr == sevr && s.status == stat && s.value == value))) return;

    afterStateChange(m_tree->applyUpdate(id, connected, sevr, stat, value, QDateTime::currentMSecsSinceEpoch()));
}

void caAlarmTree::handleAuxUpdate(int auxIndex, const QString &String, const knobData &data)
{
    if(auxIndex < 0 || auxIndex >= m_aux.size()) return;
    AuxPv &a = m_aux[auxIndex];
    const bool wasConnected = a.connected;
    a.connected = data.edata.connected != 0;
    if(!a.connected) {
        if(wasConnected) qCWarning(caAlarmTreeLog) << objectName() << "auxiliary pv disconnected" << a.pv;
        return;
    }
    const double v = (data.edata.fieldtype == caENUM) ? (double) data.edata.ivalue : data.edata.rvalue;
    switch(a.kind) {
    case AuxPv::ForcePv:
        if(a.seen && a.value == v) return;
        a.seen = true;
        a.value = v;
        evaluateForce(a.node, v);
        break;
    case AuxPv::CalcInput: {
        if(a.seen && a.value == v) return;
        a.seen = true;
        a.value = v;
        CalcState &c = m_calc[a.node];
        c.in[a.input] = v;
        c.known[a.input] = true;
        evaluateCalcForce(a.node);
        break;
    }
    case AuxPv::SevrPv:
        // write (again) on connect and once write access is granted
        if(!wasConnected || (data.edata.accessW && !a.writable)) {
            a.writable = data.edata.accessW;
            m_sevrWritten.remove(a.node);
            writeSevrPvs();
        }
        break;
    case AuxPv::ConfigPv:
        m_configNelm = data.edata.nelm;
        if(data.edata.fieldtype == caCHAR || data.edata.fieldtype == caSTRING) {
            const QString json = (data.edata.fieldtype == caCHAR && data.edata.nelm == 1) ? QString() : String;
            applyConfigDocument(json, true);
        }
        break;
    default:
        break;
    }
}

void caAlarmTree::evaluateForce(int nodeId, double value, bool asFloat)
{
    const AlhNode &n = m_tree->node(nodeId);
    const AlhForcePv &f = n.forcePv;
    bool &forced = m_forced[nodeId];
    QList<QVariantMap> events;
    // alh compares the CALC result as float, a plain force pv as double
    const bool isForce = asFloat ? ((float) value == (float) f.forceValue) : (value == f.forceValue);
    const bool isReset = f.resetNE ? !isForce : (asFloat ? ((float) value == (float) f.resetValue) : (value == f.resetValue));
    if(isForce) {
        if(!forced) {
            forced = true;
            events = m_tree->applyForce(nodeId, f.mask, true);
            QVariantMap ev;
            ev.insert(QStringLiteral("action"), QStringLiteral("force"));
            ev.insert(QStringLiteral("node"), n.displayName());
            ev.insert(QStringLiteral("group"), m_tree->alhModel().nodePath(nodeId));
            ev.insert(QStringLiteral("mask"), f.mask.toString());
            ev.insert(QStringLiteral("state"), QStringLiteral("on"));
            events.prepend(ev);
        }
    } else if(isReset) {
        if(forced) {
            forced = false;
            events = m_tree->applyForce(nodeId, f.mask, false);
            QVariantMap ev;
            ev.insert(QStringLiteral("action"), QStringLiteral("force"));
            ev.insert(QStringLiteral("node"), n.displayName());
            ev.insert(QStringLiteral("group"), m_tree->alhModel().nodePath(nodeId));
            ev.insert(QStringLiteral("mask"), f.mask.toString());
            ev.insert(QStringLiteral("state"), QStringLiteral("off"));
            events.prepend(ev);
        }
    }
    if(!events.isEmpty()) afterStateChange(events);
}

// the EPICS calc engine lives in caQtDM_Lib: directly connected signal, ok stays false without a receiver
void caAlarmTree::evaluateCalcForce(int nodeId)
{
    if(!m_activated) return;
    const AlhNode &n = m_tree->node(nodeId);
    const CalcState &c = m_calc[nodeId];
    for(int i = 0; i < 6; i++) if(!n.forcePv.calcInput[i].isEmpty() && !c.known[i]) return;   // alh waits for all inputs
    QVector<double> inputs(6);
    for(int i = 0; i < 6; i++) inputs[i] = c.in[i];
    double result = 0.0;
    bool ok = false;
    emit calcRequested(n.forcePv.calcExpr, inputs, &result, &ok);
    if(!ok) {
        qCWarning(caAlarmTreeLog) << objectName() << "FORCEPV_CALC" << n.displayName() << "not evaluated";
        return;
    }
    evaluateForce(nodeId, result, true);
}

void caAlarmTree::writePv(const QString &pv, const QString &text, bool asString)
{
    if(!m_activated || pv.isEmpty()) return;
    emit writeRequested(pv, text, asString);
}

// $SEVRPV: current severity of the node, NOTCONNECTED written as INVALID
void caAlarmTree::writeSevrPvs()
{
    if(!m_alarmMode) return;
    foreach(int id, m_sevrPvNodes) {
        const int idx = AlhTreeModel::severityIndex(m_tree->state(id).curSevr);
        const int out = qMin(idx, (int) AlhSevInvalid);
        if(m_sevrWritten.value(id, -1) == out) continue;
        m_sevrWritten.insert(id, out);
        writePv(m_tree->node(id).sevrPv, QString::number(out), false);
    }
}

void caAlarmTree::heartbeatTimeout()
{
    if(!m_alarmMode || !m_activated) return;
    const AlhModel &alh = m_tree->alhModel();
    if(alh.hasHeartbeat()) writePv(alh.heartbeatPv(), QString::number(alh.heartbeatValue()), false);
}

void caAlarmTree::filterTimeout()
{
    if(!m_tree->hasPendingFilters()) return;
    afterStateChange(m_tree->processDue(QDateTime::currentMSecsSinceEpoch()));
}

void caAlarmTree::noAckTimeout()
{
    if(!m_tree->hasNoAckTimers()) { m_noAckTimer->stop(); return; }
    afterStateChange(m_tree->expireNoAckTimers(QDateTime::currentMSecsSinceEpoch()));
}

void caAlarmTree::refilter()
{
    if(m_displayFilter != FilterNone) m_proxy->refresh();
    restoreGroupRoot();
}

// everything that follows a state change: events, $SEVRPV, beep, silence current, filter, status
void caAlarmTree::afterStateChange(const QList<QVariantMap> &events)
{
    // alh: a new unacknowledged alarm at or above the beep severity ends "silence current"
    if(m_silenceCurrent) {
        const int global = m_tree->alhModel().effectiveBeepSeverity();
        const int override = m_beepSeverity >= 0 ? AlhTreeModel::severityIndex(m_beepSeverity) : -1;
        foreach(const QVariantMap &ev, events) {
            if(!ev.value(QStringLiteral("latched")).toBool()) continue;
            const int node = ev.value(QStringLiteral("nodeId")).toInt();
            if(AlhTreeModel::severityIndex(m_tree->state(node).unackSevr) >= m_tree->thresholdIndexFor(node, global, override)) {
                setSilenceCurrent(false);
                break;
            }
        }
    }
    emitEvents(events);
    writeSevrPvs();
    updateBeepState();
    if(m_displayFilter != FilterNone && !m_refilterTimer->isActive()) m_refilterTimer->start();
    if(!events.isEmpty()) updateStatus();
}

//---------------------------------------------------------------------------- events

void caAlarmTree::emitEvents(const QList<QVariantMap> &events)
{
    foreach(const QVariantMap &ev, events) emitEvent(ev);
}

void caAlarmTree::emitEvent(QVariantMap event)
{
    const int nodeId = event.take(QStringLiteral("nodeId")).toInt();
    event.remove(QStringLiteral("latched"));
    event.insert(QStringLiteral("ts"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    if(!event.contains(QStringLiteral("user"))) event.insert(QStringLiteral("user"), userName());
    event.insert(QStringLiteral("host"), m_host);
    event.insert(QStringLiteral("display"), m_resolvedConfig);
    event.insert(QStringLiteral("pid"), m_pid);

    // current alarm history (alh window), independent of the alarm mode
    const QString action = event.value(QStringLiteral("action")).toString();
    if(action != QLatin1String("start") && action != QLatin1String("config")) {
        m_history.append(event);
        while(m_history.size() > HistorySize) m_history.removeFirst();
        if(!m_historyLog.isNull()) m_historyLog->appendEvent(event);
    }

    if(!m_alarmMode) return;
    if(event.contains(QStringLiteral("pv")) && nodeId >= 0 && nodeId < m_tree->alhModel().nodeCount()) {
        if(m_tree->effectiveMask(nodeId).noLog) return;             // mask L
    }
    qCInfo(caAlarmEventLog).noquote() << QString::fromUtf8(QJsonDocument(QJsonObject::fromVariantMap(event)).toJson(QJsonDocument::Compact));
    emit alarmEvent(event);
}

//---------------------------------------------------------------------------- beep (extension point) and silence

void caAlarmTree::updateBeepState()
{
    int sevr = -1;
    if(m_alarmMode && m_activated) {
        const int global = m_tree->alhModel().effectiveBeepSeverity();
        const int override = m_beepSeverity >= 0 ? AlhTreeModel::severityIndex(m_beepSeverity) : -1;
        sevr = m_tree->beepSeverity(global, override);
    }
    const bool silenced = m_silenceCurrent || m_silenceOneHour || m_silenceForever;
    const bool wanted = sevr >= 0 && !silenced;
    if(wanted && !m_beepActive) {
        m_beepActive = true;
        emit beepRequested(sevr);
        m_beepTimer->start();
#ifdef CAQTDM_ALARM_SOUND
        // reserved for the sound widget integration
#endif
    } else if(!wanted && m_beepActive) {
        m_beepActive = false;
        m_beepTimer->stop();
        emit beepStopped();
    }
    m_lastBeepSevr = sevr;
}

void caAlarmTree::beepTimeout()
{
    if(!m_beepActive) return;
    const int global = m_tree->alhModel().effectiveBeepSeverity();
    const int override = m_beepSeverity >= 0 ? AlhTreeModel::severityIndex(m_beepSeverity) : -1;
    const int sevr = m_tree->beepSeverity(global, override);
    if(sevr >= 0) emit beepRequested(sevr);
    else updateBeepState();
}

void caAlarmTree::setSilenceCurrent(bool on)
{
    if(m_silenceCurrent == on) return;
    m_silenceCurrent = on;
    m_silenceCurrentBox->blockSignals(true);
    m_silenceCurrentBox->setChecked(on);
    m_silenceCurrentBox->blockSignals(false);
    QVariantMap ev;
    ev.insert(QStringLiteral("action"), QStringLiteral("silence"));
    ev.insert(QStringLiteral("mode"), QStringLiteral("current"));
    ev.insert(QStringLiteral("state"), on);
    emitEvent(ev);
    updateBeepState();
    updateStatus();
}

void caAlarmTree::setSilenceOneHour(bool on)
{
    if(m_silenceOneHour == on) return;
    m_silenceOneHour = on;
    m_silenceOneHourBox->blockSignals(true);
    m_silenceOneHourBox->setChecked(on);
    m_silenceOneHourBox->blockSignals(false);
    if(on) m_silenceOneHourTimer->start();
    else m_silenceOneHourTimer->stop();
    QVariantMap ev;
    ev.insert(QStringLiteral("action"), QStringLiteral("silence"));
    ev.insert(QStringLiteral("mode"), QStringLiteral("one hour"));
    ev.insert(QStringLiteral("state"), on);
    emitEvent(ev);
    updateBeepState();
    updateStatus();
}

void caAlarmTree::silenceOneHourTimeout()
{
    setSilenceOneHour(false);
}

void caAlarmTree::setSilenceForever(bool on)
{
    if(m_silenceForever == on) return;
    m_silenceForever = on;
    syncSetupActions();
    QVariantMap ev;
    ev.insert(QStringLiteral("action"), QStringLiteral("silence"));
    ev.insert(QStringLiteral("mode"), QStringLiteral("forever"));
    ev.insert(QStringLiteral("state"), on);
    emitEvent(ev);
    updateBeepState();
    updateStatus();
}

void caAlarmTree::setAlhBeepSeverity(int severity)
{
    setBeepSeverity(severity);
}

void caAlarmTree::setDisplayFilter(int filter)
{
    if(filter < FilterNone || filter > FilterUnackAlarms) filter = FilterNone;
    m_displayFilter = (DisplayFilter) filter;
    m_proxy->setMode(filter);
    restoreGroupRoot();
    m_treeView->expandToDepth(0);
    if(m_selected >= 0) selectNode(m_selected);
    syncSetupActions();
    updateStatus();
}

//---------------------------------------------------------------------------- acknowledge

void caAlarmTree::acknowledgeNode(int id)
{
    if(id < 0) return;
    const QList<QVariantMap> events = m_tree->acknowledge(id, userName());
    if(m_alarmMode) {
        foreach(const QVariantMap &ev, events) {
            const int node = ev.value(QStringLiteral("nodeId")).toInt();
            const AlhNode &n = m_tree->node(node);
            if(!n.ackPv.isEmpty()) writePv(n.ackPv, n.ackValue, false);
        }
    }
    afterStateChange(events);
}

void caAlarmTree::acknowledgeCurrent()
{
    const int id = selectedOrWarn();
    if(id >= 0) acknowledgeNode(id);
}

void caAlarmTree::acknowledgeSubtree()
{
    acknowledgeCurrent();
}

void caAlarmTree::acknowledgeAll()
{
    m_menuNode = -1;
    foreach(int root, m_tree->alhModel().roots()) acknowledgeNode(root);
}

// alh line buttons: ack button, G button, P button
//---------------------------------------------------------------------------- context menu (lib), info, drag

void caAlarmTree::addContextActions(QMenu &menu)
{
    m_menuNode = nodeUnderCursor();
    if(m_menuNode >= 0) selectNode(m_menuNode);
    if(m_menuNode < 0) {
        connect(menu.addAction(tr("Acknowledge All")), SIGNAL(triggered()), this, SLOT(acknowledgeAll()));
        connect(menu.addAction(tr("Expand All")), SIGNAL(triggered()), this, SLOT(expandAllNodes()));
        connect(menu.addAction(tr("Collapse All")), SIGNAL(triggered()), this, SLOT(collapseAllNodes()));
        connect(menu.addAction(tr("Current Alarm History Window")), SIGNAL(triggered()), this, SLOT(showAlarmHistory()));
        connect(menu.addAction(tr("Configuration File Window")), SIGNAL(triggered()), this, SLOT(showConfigFile()));
        return;
    }
    const AlhNode &n = m_tree->node(m_menuNode);
    connect(menu.addAction(n.isGroup() ? tr("Acknowledge Group") : tr("Acknowledge Alarm")), SIGNAL(triggered()), this, SLOT(acknowledgeCurrent()));
    if(!n.guidanceText.isEmpty() || !n.guidanceLocation.isEmpty())
        connect(menu.addAction(tr("Display Guidance")), SIGNAL(triggered()), this, SLOT(showGuidance()));
    if(!n.command.isEmpty()) {
        if(m_commandMode != Script)
            connect(menu.addAction(tr("Start Related Process")), SIGNAL(triggered()), this, SLOT(runRelatedDisplay()));
        if(m_commandMode != Shell)
            connect(menu.addAction(tr("Start Related Process with Output...")), SIGNAL(triggered()), this, SLOT(runCommandWithOutput()));
    }
    connect(menu.addAction(tr("Force Mask...")), SIGNAL(triggered()), this, SLOT(forceMaskDialog()));
    connect(menu.addAction(tr("Beep Severity...")), SIGNAL(triggered()), this, SLOT(beepSeverityDialog()));
    connect(menu.addAction(tr("NoAck for One Hour...")), SIGNAL(triggered()), this, SLOT(noAckOneHourDialog()));
    connect(menu.addAction(tr("Group/Channel Properties Window")), SIGNAL(triggered()), this, SLOT(showProperties()));
    menu.addSeparator();
    const bool disabled = m_tree->state(m_menuNode).baseMask.disable;
    connect(menu.addAction(disabled ? tr("Enable") : tr("Disable")), SIGNAL(triggered()), this,
            disabled ? SLOT(enableCurrent()) : SLOT(disableCurrent()));
    const bool muted = m_tree->state(m_menuNode).mute;
    connect(menu.addAction(muted ? tr("Unmute") : tr("Mute")), SIGNAL(triggered()), this,
            muted ? SLOT(unmuteCurrent()) : SLOT(muteCurrent()));
    connect(menu.addAction(m_tree->treeMuted() ? tr("Unmute All") : tr("Mute All")), SIGNAL(triggered()), this,
            m_tree->treeMuted() ? SLOT(unmuteAll()) : SLOT(muteAll()));
    connect(menu.addAction(tr("Reset to Configuration")), SIGNAL(triggered()), this, SLOT(resetCurrentToConfig()));
    menu.addSeparator();
    connect(menu.addAction(tr("Expand Branch")), SIGNAL(triggered()), this, SLOT(expandBranch()));
    connect(menu.addAction(tr("Collapse Branch")), SIGNAL(triggered()), this, SLOT(collapseBranch()));
}

QString caAlarmTree::pvUnderCursor() const
{
    const int id = nodeUnderCursor();
    if(id >= 0 && m_tree->node(id).isChannel()) return m_tree->node(id).name;
    return QString();
}

QString caAlarmTree::dragText() const
{
    return pvUnderCursor();
}

//---------------------------------------------------------------------------- guidance, related process, properties

void caAlarmTree::showGuidance()
{
    const int id = selectedOrWarn();
    if(id < 0) return;
    const AlhNode &n = m_tree->node(id);
    if(n.guidanceText.isEmpty() && n.guidanceLocation.isEmpty()) {
        QMessageBox::information(window(), tr("Alarm Handler"), tr("No guidance for %1").arg(n.displayName()));
        return;
    }
    if(!n.guidanceText.isEmpty()) {
        QDialog *dialog = new QDialog(window());
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->setWindowTitle(tr("Guidance: %1").arg(n.displayName()));
        QVBoxLayout *layout = new QVBoxLayout(dialog);
        QTextBrowser *browser = new QTextBrowser(dialog);
        browser->setPlainText(n.guidanceText.join(QStringLiteral("\n")));
        layout->addWidget(browser);
        QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Close, dialog);
        connect(buttons, SIGNAL(rejected()), dialog, SLOT(close()));
        connect(buttons, SIGNAL(accepted()), dialog, SLOT(close()));
        layout->addWidget(buttons);
        dialog->resize(500, 300);
        dialog->show();
    }
    if(!n.guidanceLocation.isEmpty()) QDesktopServices::openUrl(QUrl::fromUserInput(n.guidanceLocation));
}

void caAlarmTree::runCommand(int nodeId, bool withOutput)
{
    if(nodeId < 0) return;
    const AlhNode &n = m_tree->node(nodeId);
    if(n.command.isEmpty()) {
        QMessageBox::warning(window(), tr("Alarm Handler"), tr("No related process for %1").arg(n.displayName()));
        return;
    }
    if(withOutput) {
        m_script->setScriptCommand(n.command);
        m_script->setScriptParam(QString());
        QMetaObject::invokeMethod(m_script, "scriptButtonSignal");
    } else {
        m_shell->setFiles(n.command);
        m_shell->setArgs(QString());
        QMetaObject::invokeMethod(m_shell, "clicked", Q_ARG(int, 0));
    }
}

void caAlarmTree::runRelatedDisplay()
{
    const int id = selectedOrWarn();
    if(id >= 0) runCommand(id, m_commandMode == Script);
}

void caAlarmTree::runCommandWithOutput()
{
    const int id = selectedOrWarn();
    if(id >= 0) runCommand(id, true);
}

QString caAlarmTree::propertiesText(int id) const
{
    const AlhNodeState &s = m_tree->state(id);
    const AlhNode &n = m_tree->node(id);
    QString t = QString("<table>");
    t += tr("<tr><td>%1</td><td><b>%2</b></td></tr>").arg(n.isGroup() ? tr("Group") : tr("Channel"), n.name);
    t += tr("<tr><td>Path</td><td>%1</td></tr>").arg(m_tree->alhModel().nodePath(id));
    if(!n.alias.isEmpty()) t += tr("<tr><td>Alias</td><td>%1</td></tr>").arg(n.alias);
    t += tr("<tr><td>Current severity</td><td>%1</td></tr>").arg(AlhTreeModel::severityText(s.curSevr));
    t += tr("<tr><td>Unacknowledged</td><td>%1</td></tr>").arg(AlhTreeModel::severityText(s.unackSevr));
    t += tr("<tr><td>Info</td><td>%1</td></tr>").arg(m_tree->infoText(id).toHtmlEscaped());
    t += tr("<tr><td>Mask file</td><td>%1</td></tr>").arg(s.fileMask.toString());
    t += tr("<tr><td>Mask config channel</td><td>%1</td></tr>").arg(s.baseMask.toString());
    t += tr("<tr><td>Force mask</td><td>%1</td></tr>").arg(s.forced ? s.forcedMask.toString() : tr("not active"));
    t += tr("<tr><td>Effective mask</td><td><b>%1</b></td></tr>").arg(m_tree->maskText(id));
    if(s.noAckUntilMs > 0) t += tr("<tr><td>NoAck timer until</td><td>%1</td></tr>").arg(QDateTime::fromMSecsSinceEpoch(s.noAckUntilMs).toString(Qt::ISODate));
    t += tr("<tr><td>Beep severity</td><td>%1</td></tr>").arg(AlhModel::severityName(m_tree->thresholdIndexFor(id,
            m_tree->alhModel().effectiveBeepSeverity(), m_beepSeverity >= 0 ? AlhTreeModel::severityIndex(m_beepSeverity) : -1)));
    t += tr("<tr><td>Muted</td><td>%1</td></tr>").arg(m_tree->isMuted(id) ? tr("yes") : tr("no"));
    if(n.hasForcePv) t += tr("<tr><td>Force process variable</td><td>%1 mask %2 force=%3 %4</td></tr>").arg(n.forcePv.pv, n.forcePv.mask.toString())
            .arg(n.forcePv.forceValue).arg(n.forcePv.resetNE ? QStringLiteral("reset=NE") : QStringLiteral("reset=") + QString::number(n.forcePv.resetValue));
    if(n.hasForcePv && n.forcePv.isCalc) {
        QStringList in;
        for(int i = 0; i < 6; i++) if(!n.forcePv.calcInput[i].isEmpty()) in << QString("%1=%2").arg(QChar('A' + i)).arg(n.forcePv.calcInput[i]);
        t += tr("<tr><td>Force CALC</td><td>%1 (%2)</td></tr>").arg(n.forcePv.calcExpr.toHtmlEscaped(), in.join(QStringLiteral(", ")));
    }
    if(!n.sevrPv.isEmpty()) t += tr("<tr><td>Severity pv</td><td>%1</td></tr>").arg(n.sevrPv);
    if(!n.ackPv.isEmpty()) t += tr("<tr><td>Ack pv</td><td>%1 = %2</td></tr>").arg(n.ackPv, n.ackValue);
    if(n.hasCountFilter) t += tr("<tr><td>Alarm count filter</td><td>%1 in %2 s</td></tr>").arg(n.countFilter.count).arg(n.countFilter.seconds);
    if(!n.command.isEmpty()) t += tr("<tr><td>Related process</td><td>%1</td></tr>").arg(n.command.toHtmlEscaped());
    if(!n.guidanceLocation.isEmpty()) t += tr("<tr><td>Guidance URL</td><td>%1</td></tr>").arg(n.guidanceLocation.toHtmlEscaped());
    if(!n.guidanceText.isEmpty()) t += tr("<tr><td>Guidance text</td><td>%1</td></tr>").arg(n.guidanceText.join(QStringLiteral("<br>")).toHtmlEscaped().replace(QStringLiteral("&lt;br&gt;"), QStringLiteral("<br>")));
    foreach(const AlhSevrCommand &c, n.sevrCommands) t += tr("<tr><td>Severity command</td><td>%1 %2 (not executed)</td></tr>").arg(c.token, c.command.toHtmlEscaped());
    foreach(const AlhStatCommand &c, n.statCommands) t += tr("<tr><td>Status command</td><td>%1 %2 (not executed)</td></tr>").arg(c.status, c.command.toHtmlEscaped());
    t += QStringLiteral("</table>");
    t += tr("<p>Mask: C cancel, D disable, A no ack, T no ack transient, L no log, H no ack timer</p>");
    return t;
}

void caAlarmTree::showProperties()
{
    const int id = selectedOrWarn();
    if(id < 0) return;
    QDialog *dialog = new QDialog(window());
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle(tr("Alarm Handler Properties: %1").arg(m_tree->node(id).displayName()));
    QVBoxLayout *layout = new QVBoxLayout(dialog);
    QTextBrowser *browser = new QTextBrowser(dialog);
    browser->setHtml(propertiesText(id));
    layout->addWidget(browser);
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Close, dialog);
    connect(buttons, SIGNAL(rejected()), dialog, SLOT(close()));
    layout->addWidget(buttons);
    dialog->resize(560, 420);
    dialog->show();
}

//---------------------------------------------------------------------------- dialogs: force mask, beep severity, noAck one hour

// alh "Force Process Variable": shows the $FORCEPV of the node and writes its force or reset value
void caAlarmTree::forceProcessVariableDialog()
{
    const int id = selectedOrWarn();
    if(id < 0) return;
    const AlhNode &n = m_tree->node(id);
    if(!n.hasForcePv) {
        QMessageBox::information(window(), tr("Force Process Variable"), tr("%1 has no force process variable.").arg(n.displayName()));
        return;
    }
    const AlhForcePv &f = n.forcePv;
    QMessageBox box(window());
    box.setWindowTitle(tr("Force Process Variable"));
    box.setTextFormat(Qt::RichText);
    box.setText(tr("%1: <b>%2</b><table>"
                   "<tr><td>Force PV:</td><td>%3</td></tr>"
                   "<tr><td>Force Mask:</td><td>%4</td></tr>"
                   "<tr><td>Force Value:</td><td>%5</td></tr>"
                   "<tr><td>Reset Value:</td><td>%6</td></tr>"
                   "<tr><td>State:</td><td>%7</td></tr></table>")
                .arg(n.isGroup() ? tr("Group") : tr("Channel"), n.displayName().toHtmlEscaped(),
                     (f.isCalc ? tr("CALC  %1").arg(f.calcExpr) : f.pv).toHtmlEscaped(), f.mask.toString(),
                     QString::number(f.forceValue), f.resetNE ? tr("not equal %1").arg(f.forceValue) : QString::number(f.resetValue),
                     m_forced.value(id, false) ? tr("forced") : tr("not forced")));
    QPushButton *force = box.addButton(tr("Force"), QMessageBox::AcceptRole);
    QPushButton *reset = box.addButton(tr("Reset"), QMessageBox::DestructiveRole);
    box.addButton(QMessageBox::Close);
    const bool canWrite = !f.isCalc && m_alarmMode && m_activated;
    force->setEnabled(canWrite);
    reset->setEnabled(canWrite && !f.resetNE);
    box.exec();
    if(box.clickedButton() == force) writePv(f.pv, QString::number(f.forceValue), false);
    else if(box.clickedButton() == reset) writePv(f.pv, QString::number(f.resetValue), false);
}

// alh "Force Mask": per mask field Add/Cancel, Enable/Disable, Ack/NoAck, AckT/NoAckT, Log/NoLog or Reset
void caAlarmTree::forceMaskDialog()
{
    const int id = selectedOrWarn();
    if(id < 0) return;
    const AlhNode &n = m_tree->node(id);
    QDialog dialog(window());
    dialog.setWindowTitle(tr("Force Mask"));
    QGridLayout *grid = new QGridLayout(&dialog);
    grid->addWidget(new QLabel(tr("%1: <b>%2</b>").arg(n.isGroup() ? tr("Group") : tr("Channel"), n.displayName()), &dialog), 0, 0, 1, 4);
    static const char *rowTitles[5] = { "Add/Cancel Alarms", "Enable/Disable Alarms", "Ack/NoAck Alarms", "Ack/NoAck Transient Alarms", "Log/NoLog Alarms" };
    static const char *onTexts[5] = { "Add", "Enable", "Ack", "AckT", "Log" };
    static const char *offTexts[5] = { "Cancel", "Disable", "NoAck", "NoAckT", "NoLog" };
    QList<QButtonGroup *> groups;
    const QJsonObject current = m_cfgNodes.value(m_tree->alhModel().nodePath(id));
    for(int row = 0; row < 5; row++) {
        grid->addWidget(new QLabel(tr(rowTitles[row]), &dialog), row + 1, 0);
        QButtonGroup *g = new QButtonGroup(&dialog);
        QRadioButton *on = new QRadioButton(tr(onTexts[row]), &dialog);
        QRadioButton *off = new QRadioButton(tr(offTexts[row]), &dialog);
        QRadioButton *reset = new QRadioButton(tr("Reset"), &dialog);
        g->addButton(on, 0);
        g->addButton(off, 1);
        g->addButton(reset, 2);
        const QString field = QLatin1String(cfgFlagNames[row]);
        if(current.contains(field)) (current.value(field).toBool() ? off : on)->setChecked(true);
        else reset->setChecked(true);
        grid->addWidget(on, row + 1, 1);
        grid->addWidget(off, row + 1, 2);
        grid->addWidget(reset, row + 1, 3);
        groups.append(g);
    }
    grid->addWidget(new QLabel(tr("Reset returns to the mask of the configuration file; a group applies to all channels below."), &dialog), 6, 0, 1, 4);
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Apply | QDialogButtonBox::Cancel, &dialog);
    connect(buttons->button(QDialogButtonBox::Apply), SIGNAL(clicked()), &dialog, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), &dialog, SLOT(reject()));
    grid->addWidget(buttons, 7, 0, 1, 4);
    if(dialog.exec() != QDialog::Accepted) return;

    QMap<QString, QVariant> fields;
    for(int row = 0; row < 5; row++) {
        const int choice = groups.at(row)->checkedId();
        const QString field = QLatin1String(cfgFlagNames[row]);
        if(choice == 2) fields.insert(field, QVariant());             // reset: no override
        else fields.insert(field, choice == 1);                       // off text = mask bit set
    }
    setLocalOverrideFields(id, fields);
}

// alh "Set Beep Severity" for a group or channel
void caAlarmTree::beepSeverityDialog()
{
    const int id = selectedOrWarn();
    if(id < 0) return;
    const AlhNode &n = m_tree->node(id);
    QDialog dialog(window());
    dialog.setWindowTitle(tr("Set Beep Severity"));
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    layout->addWidget(new QLabel(tr("%1: <b>%2</b>").arg(n.isGroup() ? tr("Group") : tr("Channel"), n.displayName()), &dialog));
    layout->addWidget(new QLabel(tr("Beep severity is the minimum severity level required for beeping."), &dialog));
    QComboBox *combo = new QComboBox(&dialog);
    combo->addItem(tr("From configuration"), -1);
    combo->addItem(tr("Minor"), (int) AlhSevMinor);
    combo->addItem(tr("Major"), (int) AlhSevMajor);
    combo->addItem(tr("Invalid"), (int) AlhSevInvalid);
    combo->setCurrentIndex(qMax(0, combo->findData(m_tree->state(id).beepSevrOverride)));
    layout->addWidget(combo);
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Apply | QDialogButtonBox::Cancel, &dialog);
    connect(buttons->button(QDialogButtonBox::Apply), SIGNAL(clicked()), &dialog, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), &dialog, SLOT(reject()));
    layout->addWidget(buttons);
    if(dialog.exec() != QDialog::Accepted) return;
    const int value = combo->itemData(combo->currentIndex()).toInt();
    setLocalOverride(id, QStringLiteral("beepSevr"), value < 0 ? QVariant() : QVariant(value));
}

// alh "NoAck For One Hour": ack mask set to noAck ('H'), reset by a timer
void caAlarmTree::noAckOneHourDialog()
{
    const int id = selectedOrWarn();
    if(id < 0) return;
    const AlhNode &n = m_tree->node(id);
    const bool active = m_tree->state(id).noAckUntilMs > 0;
    QMessageBox box(window());
    box.setWindowTitle(tr("NoAck For One Hour"));
    box.setText(tr("%1: <b>%2</b><br>%3").arg(n.isGroup() ? tr("Group") : tr("Channel"), n.displayName(),
                active ? tr("The noAck one hour timer is running (mask 'H').") : tr("Set the ack mask to noAck for one hour.")));
    QPushButton *set = box.addButton(tr("Set NoAck and start timer"), QMessageBox::AcceptRole);
    QPushButton *reset = box.addButton(tr("Reset Ack mask and cancel timer"), QMessageBox::DestructiveRole);
    box.addButton(QMessageBox::Cancel);
    set->setEnabled(!active);
    reset->setEnabled(active);
    box.exec();
    QList<QVariantMap> events;
    if(box.clickedButton() == set) {
        events = m_tree->setNoAckTimer(id, QDateTime::currentMSecsSinceEpoch() + 3600 * 1000, n.isGroup());
        m_noAckTimer->start();
    } else if(box.clickedButton() == reset) {
        events = m_tree->setNoAckTimer(id, 0, n.isGroup());
    } else {
        return;
    }
    QVariantMap ev;
    ev.insert(QStringLiteral("action"), QStringLiteral("noack"));
    ev.insert(QStringLiteral("node"), n.displayName());
    ev.insert(QStringLiteral("group"), m_tree->alhModel().nodePath(id));
    ev.insert(QStringLiteral("state"), box.clickedButton() == set ? QStringLiteral("on") : QStringLiteral("off"));
    events.prepend(ev);
    afterStateChange(events);
}

//---------------------------------------------------------------------------- view menu

void caAlarmTree::expandRecursively(const QModelIndex &idx, bool expand)
{
    const QModelIndex index = idx.sibling(idx.row(), 0);
    if(!index.isValid()) return;
    if(expand) m_treeView->expand(index); else m_treeView->collapse(index);
    for(int r = 0; r < m_groupProxy->rowCount(index); r++) expandRecursively(m_groupProxy->index(r, 0, index), expand);
}

// alh EXPANDCOLLAPSE1: toggles the subgroups of the selected group
void caAlarmTree::expandOneLevel()
{
    const int id = currentGroup();
    m_menuNode = -1;
    if(id < 0) { QMessageBox::warning(window(), tr("Alarm Handler"), tr(selectGroupText)); return; }
    const QModelIndex ti = treeIndexFor(id);
    m_treeView->setExpanded(ti, !m_treeView->isExpanded(ti));
}

void caAlarmTree::expandBranch()
{
    const int id = currentGroup();
    m_menuNode = -1;
    if(id < 0) { QMessageBox::warning(window(), tr("Alarm Handler"), tr(selectGroupText)); return; }
    expandRecursively(treeIndexFor(id), true);
}

void caAlarmTree::collapseBranch()
{
    const int id = currentGroup();
    m_menuNode = -1;
    if(id < 0) { QMessageBox::warning(window(), tr("Alarm Handler"), tr(selectGroupText)); return; }
    expandRecursively(treeIndexFor(id), false);
}

void caAlarmTree::expandAllNodes()   { m_menuNode = -1; m_treeView->expandAll(); }
void caAlarmTree::collapseAllNodes() { m_menuNode = -1; m_treeView->collapseAll(); m_treeView->expandToDepth(0); }

void caAlarmTree::showAlarmHistory()
{
    m_menuNode = -1;
    if(!m_historyLog.isNull()) {
        m_historyLog->window()->raise();
        m_historyLog->window()->activateWindow();
        return;
    }
    QDialog *dialog = new QDialog(window());
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle(tr("Alarm Handler: Current Alarm History"));
    QVBoxLayout *layout = new QVBoxLayout(dialog);
    caAlarmLog *log = new caAlarmLog(dialog);
    log->setMaxEntries(HistorySize);
    foreach(const QVariantMap &ev, m_history) log->appendEvent(ev);
    layout->addWidget(log);
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Close, dialog);
    connect(buttons, SIGNAL(rejected()), dialog, SLOT(close()));
    layout->addWidget(buttons);
    dialog->resize(900, 400);
    m_historyLog = log;
    dialog->show();
}

void caAlarmTree::showConfigFile()
{
    m_menuNode = -1;
    QFile f(m_resolvedConfig);
    QString text;
    if(f.open(QIODevice::ReadOnly)) text = QString::fromUtf8(f.readAll());
    else text = tr("cannot read %1").arg(m_resolvedConfig);
    QDialog *dialog = new QDialog(window());
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle(tr("Alarm Handler: Configuration File %1").arg(QFileInfo(m_resolvedConfig).fileName()));
    QVBoxLayout *layout = new QVBoxLayout(dialog);
    QPlainTextEdit *edit = new QPlainTextEdit(dialog);
    edit->setReadOnly(true);
    edit->setLineWrapMode(QPlainTextEdit::NoWrap);
    edit->setPlainText(text);
    layout->addWidget(edit);
    const QStringList includes = m_tree->alhModel().includedFiles();
    if(!includes.isEmpty()) layout->addWidget(new QLabel(tr("Included files: %1").arg(includes.join(QStringLiteral(", "))), dialog));
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Close, dialog);
    connect(buttons, SIGNAL(rejected()), dialog, SLOT(close()));
    layout->addWidget(buttons);
    dialog->resize(700, 500);
    dialog->show();
}

//---------------------------------------------------------------------------- config channel

void caAlarmTree::disableCurrent()  { const int id = selectedOrWarn(); if(id >= 0) setLocalOverride(id, QStringLiteral("disable"), true); }
void caAlarmTree::enableCurrent()   { const int id = selectedOrWarn(); if(id >= 0) setLocalOverride(id, QStringLiteral("disable"), false); }
void caAlarmTree::muteCurrent()     { const int id = selectedOrWarn(); if(id >= 0) setLocalOverride(id, QStringLiteral("mute"), true); }
void caAlarmTree::unmuteCurrent()   { const int id = selectedOrWarn(); if(id >= 0) setLocalOverride(id, QStringLiteral("mute"), false); }
void caAlarmTree::resetCurrentToConfig() { const int id = selectedOrWarn(); if(id >= 0) setLocalOverride(id, QString(), QVariant()); }

void caAlarmTree::muteAll()
{
    m_menuNode = -1;
    m_cfgTreeMute = true;
    applyOverrides();
    writeConfigDocument();
}

void caAlarmTree::unmuteAll()
{
    m_menuNode = -1;
    m_cfgTreeMute = false;
    applyOverrides();
    writeConfigDocument();
}

QString caAlarmTree::effectiveConfigKey() const
{
    const QString key = AlhConfigParser::expandMacros(m_configKey.trimmed(), m_map);
    if(!key.isEmpty()) return key;
    return QFileInfo(m_resolvedConfig).fileName();
}

// the document is declarative: the override set of our key replaces the current one
void caAlarmTree::applyConfigDocument(const QString &json, bool fromPv)
{
    QJsonObject root;
    if(!json.trimmed().isEmpty()) {
        QJsonParseError error;
        const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &error);
        if(doc.isNull() || !doc.isObject()) {
            qCWarning(caAlarmTreeLog) << objectName() << "configPV: invalid JSON" << error.errorString();
            return;
        }
        root = doc.object();
    }
    m_configDoc = root;
    const QString key = effectiveConfigKey();
    m_cfgNodes.clear();
    m_cfgTreeMute = false;
    QString by;
    if(root.contains(key)) {
        const QJsonObject section = root.value(key).toObject();
        m_cfgTreeMute = section.value(QStringLiteral("mute")).toBool(false);
        by = section.value(QStringLiteral("by")).toString();
        const QJsonObject nodes = section.value(QStringLiteral("nodes")).toObject();
        foreach(const QString &path, nodes.keys()) m_cfgNodes.insert(path, nodes.value(path).toObject());
    } else if(!root.isEmpty()) {
        qCDebug(caAlarmTreeLog) << objectName() << "configPV: no section for key" << key;
    }
    applyOverrides();

    QVariantMap ev;
    ev.insert(QStringLiteral("action"), QStringLiteral("config"));
    ev.insert(QStringLiteral("source"), fromPv ? QStringLiteral("pv") : QStringLiteral("local"));
    ev.insert(QStringLiteral("key"), key);
    ev.insert(QStringLiteral("overrides"), m_cfgNodes.size());
    ev.insert(QStringLiteral("mute"), m_cfgTreeMute);
    if(!by.isEmpty()) ev.insert(QStringLiteral("by"), by);
    emitEvent(ev);
}

static bool pathDepthLess(const QString &a, const QString &b)
{
    return a.count(QLatin1Char('/')) < b.count(QLatin1Char('/'));
}

void caAlarmTree::applyOverrides()
{
    const AlhModel &alh = m_tree->alhModel();
    const int count = alh.nodeCount();
    QVector<AlhMask> desired(count);
    QVector<bool> mute(count, false);
    QVector<int> beep(count, -1);
    for(int i = 0; i < count; i++) desired[i] = m_tree->state(i).fileMask;

    // shallow entries first, deeper entries win
    QStringList paths = m_cfgNodes.keys();
    std::stable_sort(paths.begin(), paths.end(), pathDepthLess);
    foreach(const QString &path, paths) {
        const int id = alh.findByPath(path);
        if(id < 0) {
            qCWarning(caAlarmTreeLog) << objectName() << "configPV: unknown node" << path;
            continue;
        }
        const QJsonObject o = m_cfgNodes.value(path);
        foreach(int t, m_tree->subtree(id)) {
            AlhMask m = desired[t];
            if(o.contains(QStringLiteral("mask"))) m = AlhMask::fromString(o.value(QStringLiteral("mask")).toString());
            for(int f = 0; cfgFlagNames[f] != Q_NULLPTR; f++) {
                const QString name = QLatin1String(cfgFlagNames[f]);
                if(o.contains(name)) *maskFlag(m, f) = o.value(name).toBool();
            }
            desired[t] = m;
            if(o.contains(QStringLiteral("mute"))) mute[t] = o.value(QStringLiteral("mute")).toBool();
            if(o.contains(QStringLiteral("beepSevr"))) {
                const QJsonValue v = o.value(QStringLiteral("beepSevr"));
                beep[t] = v.isString() ? AlhModel::severityFromToken(v.toString()) : v.toInt(-1);
            }
        }
    }

    QList<QVariantMap> events;
    for(int i = 0; i < count; i++) {
        if(desired.at(i) != m_tree->state(i).baseMask) events += m_tree->setBaseMask(i, desired.at(i), false);
        if(mute.at(i) != m_tree->state(i).mute) m_tree->setMute(i, mute.at(i), false);
        if(beep.at(i) != m_tree->state(i).beepSevrOverride) m_tree->setBeepOverride(i, beep.at(i), false);
    }
    m_tree->setTreeMuted(m_cfgTreeMute);
    afterStateChange(events);
    updateStatus();
}

void caAlarmTree::setLocalOverride(int id, const QString &field, const QVariant &value)
{
    QMap<QString, QVariant> fields;
    if(!field.isEmpty()) fields.insert(field, value);
    setLocalOverrideFields(id, fields);
}

// local change: update our entry (empty map = reset node and everything below), apply and publish
void caAlarmTree::setLocalOverrideFields(int id, const QMap<QString, QVariant> &fields)
{
    if(id < 0) return;
    const AlhModel &alh = m_tree->alhModel();
    const QString path = alh.nodePath(id);
    if(fields.isEmpty()) {
        foreach(const QString &key, m_cfgNodes.keys())
            if(key == path || key.startsWith(path + QLatin1Char('/'))) m_cfgNodes.remove(key);
    } else {
        QJsonObject o = m_cfgNodes.value(path);
        for(QMap<QString, QVariant>::const_iterator it = fields.constBegin(); it != fields.constEnd(); ++it) {
            if(it.value().isValid()) o.insert(it.key(), QJsonValue::fromVariant(it.value()));
            else o.remove(it.key());
        }
        if(alh.node(id).isChannel()) {
            // flags equal to the file default are no override
            AlhMask file = m_tree->state(id).fileMask;
            for(int f = 0; cfgFlagNames[f] != Q_NULLPTR; f++) {
                const QString name = QLatin1String(cfgFlagNames[f]);
                if(o.contains(name) && o.value(name).toBool() == *maskFlag(file, f)) o.remove(name);
            }
            if(o.contains(QStringLiteral("mute")) && !o.value(QStringLiteral("mute")).toBool()) o.remove(QStringLiteral("mute"));
        }
        if(o.isEmpty()) m_cfgNodes.remove(path);
        else m_cfgNodes.insert(path, o);
    }
    applyOverrides();
    writeConfigDocument();
}

void caAlarmTree::writeConfigDocument()
{
    if(m_configAux < 0 || !m_activated) return;
    const QString key = effectiveConfigKey();
    QJsonObject root = m_configDoc;
    QJsonObject section = root.value(key).toObject();
    section.insert(QStringLiteral("updated"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    section.insert(QStringLiteral("by"), userName() + QLatin1Char('@') + m_host);
    if(m_cfgTreeMute) section.insert(QStringLiteral("mute"), true);
    else section.remove(QStringLiteral("mute"));
    QJsonObject nodes;
    for(QMap<QString, QJsonObject>::const_iterator it = m_cfgNodes.constBegin(); it != m_cfgNodes.constEnd(); ++it)
        nodes.insert(it.key(), it.value());
    if(nodes.isEmpty()) section.remove(QStringLiteral("nodes"));
    else section.insert(QStringLiteral("nodes"), nodes);
    if(nodes.isEmpty() && !m_cfgTreeMute) root.remove(key);
    else root.insert(key, section);

    const QByteArray json = root.isEmpty() ? QByteArray() : QJsonDocument(root).toJson(QJsonDocument::Compact);
    if(m_configNelm > 0 && json.size() >= m_configNelm) {
        qCWarning(caAlarmTreeLog) << objectName() << "configPV: document too long for the waveform" << json.size() << ">=" << m_configNelm;
        QMessageBox::warning(window(), tr("Alarm configuration"),
                             tr("The configuration document (%1 bytes) does not fit into %2 (%3 elements). Change not published.")
                             .arg(json.size()).arg(m_aux.at(m_configAux).pv).arg(m_configNelm));
        return;
    }
    m_configDoc = root;
    writePv(m_aux.at(m_configAux).pv, QString::fromUtf8(json), true);
}
