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
 *  caAlarmTree: alarm handler display for alh configurations (.alhConfig).
 *  Layout and functions follow the alh main window: menu bar (Action / View /
 *  Setup), the group tree on the left, the contents of the
 *  selected group on the right, lines with ack, severity, name, arrow, G, P,
 *  mask and message, and the status area with the silence controls. The widget
 *  parses the configuration itself and lists the pvs it needs; caQtDM_Lib
 *  subscribes to them, feeds dataUpdate() and executes the writeRequested()
 *  signal. Latch/ack state is kept per client like alh.
 */

#ifndef CAALARMTREE_H
#define CAALARMTREE_H

#include <QHash>
#include <QJsonObject>
#include <QList>
#include <QMap>
#include <QPointer>
#include <QVariantMap>
#include <QVector>
#include <QWidget>
#include <qtcontrols_global.h>
#include "knobData.h"

class AlhTreeModel;
class caShellCommand;
class caScriptButton;
class caAlarmLog;
class AlhFilterProxy;
class AlhGroupProxy;
class QAction;
class QCheckBox;
class QLabel;
class QMenu;
class QMenuBar;
class QSplitter;
class QTimer;
class QTreeView;

class QTCON_EXPORT caAlarmTree : public QWidget
{
    Q_OBJECT
    Q_ENUMS(CommandMode)
    Q_ENUMS(DisplayFilter)

    Q_PROPERTY(QString configFile READ getConfigFile WRITE setConfigFile)
    Q_PROPERTY(QString includeDir READ getIncludeDir WRITE setIncludeDir)
    Q_PROPERTY(QString macros READ getMacros WRITE setMacros)
    Q_PROPERTY(QString logTarget READ getLogTarget WRITE setLogTarget)
    Q_PROPERTY(bool alarmMode READ getAlarmMode WRITE setAlarmMode)
    Q_PROPERTY(int beepSeverity READ getBeepSeverity WRITE setBeepSeverity)
    Q_PROPERTY(QString configPV READ getConfigPV WRITE setConfigPV)
    Q_PROPERTY(QString configKey READ getConfigKey WRITE setConfigKey)
    Q_PROPERTY(CommandMode commandMode READ getCommandMode WRITE setCommandMode)
    Q_PROPERTY(DisplayFilter displayFilter READ getDisplayFilter WRITE setDisplayFilterEnum)
    Q_PROPERTY(bool silenceForever READ getSilenceForever WRITE setSilenceForever)
    Q_PROPERTY(bool showMenuBar READ getShowMenuBar WRITE setShowMenuBar)
    Q_PROPERTY(bool showTreeWindow READ getShowTreeWindow WRITE setShowTreeWindow)
    Q_PROPERTY(bool showStatusArea READ getShowStatusArea WRITE setShowStatusArea)
    Q_PROPERTY(bool showMaskColumn READ getShowMaskColumn WRITE setShowMaskColumn)
    Q_PROPERTY(bool showValueColumn READ getShowValueColumn WRITE setShowValueColumn)
    Q_PROPERTY(bool showPvColumn READ getShowPvColumn WRITE setShowPvColumn)

    // this will prevent user interference
    Q_PROPERTY(QString styleSheet READ styleSheet WRITE noStyle DESIGNABLE false)

public:
    enum CommandMode { Shell, Script, Ask };
    enum DisplayFilter { FilterNone, FilterActiveAlarms, FilterUnackAlarms };

    explicit caAlarmTree(QWidget *parent = Q_NULLPTR);
    ~caAlarmTree();

    void noStyle(QString style) { Q_UNUSED(style); }

    QString getConfigFile() const { return m_configFile; }
    void setConfigFile(const QString &f) { m_configFile = f; updateStatus(); }
    QString getIncludeDir() const { return m_includeDir; }
    void setIncludeDir(const QString &d) { m_includeDir = d; }
    QString getMacros() const { return m_macros; }
    void setMacros(const QString &m) { m_macros = m; }
    QString getLogTarget() const { return m_logTarget; }
    void setLogTarget(const QString &t) { m_logTarget = t; }
    bool getAlarmMode() const { return m_alarmMode; }
    void setAlarmMode(bool on);
    int getBeepSeverity() const { return m_beepSeverity; }      // -1 = from config, else alarmdefs.h Alarms
    void setBeepSeverity(int sevr);
    QString getConfigPV() const { return m_configPV; }
    void setConfigPV(const QString &pv) { m_configPV = pv; }
    QString getConfigKey() const { return m_configKey; }
    void setConfigKey(const QString &k) { m_configKey = k; }
    CommandMode getCommandMode() const { return m_commandMode; }
    void setCommandMode(CommandMode m) { m_commandMode = m; }
    DisplayFilter getDisplayFilter() const { return m_displayFilter; }
    void setDisplayFilterEnum(DisplayFilter f) { setDisplayFilter((int) f); }
    bool getSilenceForever() const { return m_silenceForever; }
    bool getShowMenuBar() const { return m_showMenuBar; }
    void setShowMenuBar(bool on);
    bool getShowTreeWindow() const { return m_showTreeWindow; }
    void setShowTreeWindow(bool on);
    bool getShowStatusArea() const { return m_showStatusArea; }
    void setShowStatusArea(bool on);
    bool getShowMaskColumn() const { return m_showMask; }
    void setShowMaskColumn(bool on) { m_showMask = on; applyColumns(); }
    bool getShowValueColumn() const { return m_showValue; }
    void setShowValueColumn(bool on) { m_showValue = on; applyColumns(); }
    bool getShowPvColumn() const { return m_showPv; }
    void setShowPvColumn(bool on) { m_showPv = on; applyColumns(); }

    // contract with caQtDM_Lib (HandleWidget / Callback_UpdateWidget / DisplayContextMenu)
    struct PvRequest { int specId; QString pv; };        // specId: node id, or AuxBase + index for auxiliary pvs
    void loadConfiguration(const QMap<QString, QString> &panelMacros);   // parse and build the pv list
    QList<PvRequest> requestedPvs() const { return m_requests; }
    void setResolvedPv(int specId, const QString &pvRep);               // pv name as registered by the lib
    void dataUpdate(const QString &String, const knobData &data);       // routed by data.specData[0]
    QString pvUnderCursor() const;                                       // channel pv for "Get Info", empty for groups
    QString dragText() const;
    void addContextActions(QMenu &menu);                                 // alh actions for the node under the cursor

    const AlhTreeModel *treeModel() const { return m_tree; }
    QTreeView *view() const { return m_treeView; }          // alh tree window (groups)
    QTreeView *groupView() const { return m_groupView; }    // alh group window (contents of the selected group)
    bool isActivated() const { return m_activated; }
    QString resolvedConfigFile() const { return m_resolvedConfig; }
    bool getSilenceCurrent() const { return m_silenceCurrent; }
    bool getSilenceOneHour() const { return m_silenceOneHour; }
    int selectedNode() const { return m_selected; }
    int groupWindowNode() const { return m_groupRoot; }

signals:
    void alarmEvent(const QVariantMap &event);
    // executed by caQtDM_Lib: pv writes ($SEVRPV, $ACKPV, $HEARTBEATPV, $FORCEPV, configPV) and EPICS calc
    void writeRequested(const QString &pv, const QString &text, bool asString);
    void calcRequested(const QString &expr, const QVector<double> &inputs, double *result, bool *ok);
    // beep decision (sound itself comes from a separate widget, see CAQTDM_ALARM_SOUND)
    void beepRequested(int severity);
    void beepStopped();

public slots:
    // Action menu of alh
    void acknowledgeCurrent();
    void acknowledgeSubtree();
    void acknowledgeAll();
    void showGuidance();
    void runRelatedDisplay();
    void runCommandWithOutput();
    void forceProcessVariableDialog();
    void forceMaskDialog();
    void beepSeverityDialog();
    void noAckOneHourDialog();
    void showProperties();
    void showMask() { showProperties(); }
    // View menu of alh
    void expandOneLevel();
    void expandBranch();
    void expandAllNodes();
    void collapseBranch();
    void collapseAllNodes();
    void showAlarmHistory();
    void showConfigFile();
    // Setup menu / status area of alh
    void setDisplayFilter(int filter);
    void setAlhBeepSeverity(int severity);          // alarmdefs.h Alarms value
    void setSilenceCurrent(bool on);
    void setSilenceOneHour(bool on);
    void setSilenceForever(bool on);
    // navigation
    void selectNode(int id);                        // alh selection (name button marked)
    void showGroupContents(int groupId);            // group window shows the children of this group
    // shared runtime configuration
    void disableCurrent();
    void enableCurrent();
    void muteCurrent();
    void unmuteCurrent();
    void muteAll();
    void unmuteAll();
    void resetCurrentToConfig();

private slots:
    void onTreeClicked(const QModelIndex &index);
    void onTreeDoubleClicked(const QModelIndex &index);
    void onGroupClicked(const QModelIndex &index);
    void onGroupDoubleClicked(const QModelIndex &index);
    void onTreeExpansionChanged();
    void onFilterAction(QAction *action);
    void onBeepAction(QAction *action);
    void onMuteAllToggled(bool on);
    void beepTimeout();
    void heartbeatTimeout();
    void filterTimeout();
    void noAckTimeout();
    void silenceOneHourTimeout();
    void refilter();

private:
    enum { AuxBase = 100000, BeepRepeatMs = 5000, HistorySize = 500 };

    struct AuxPv {
        enum Kind { ForcePv, CalcInput, AckPv, SevrPv, HeartbeatPv, ConfigPv };
        int kind;
        int node;
        int input;
        QString pv;
        bool connected;
        bool seen;
        bool writable;
        double value;
        AuxPv() : kind(ForcePv), node(-1), input(-1), connected(false), seen(false), writable(false), value(0.0) {}
    };
    struct CalcState {
        double in[6];
        bool known[6];
        CalcState() { for(int i = 0; i < 6; i++) { in[i] = 0.0; known[i] = false; } }
    };

    void buildUi();
    void buildMenuBar();
    void buildStatusArea();
    QAction *menuAction(QMenu *menu, const QString &text, const char *slot, const QString &shortcut = QString());
    QString resolveConfigFile(const QMap<QString, QString> &map) const;
    QMap<QString, QString> macroMap(const QMap<QString, QString> &libMap) const;
    int addAux(int kind, int node, int input, const QString &pv);
    void handleChannelUpdate(int id, const QString &String, const knobData &data);
    void handleAuxUpdate(int auxIndex, const QString &String, const knobData &data);
    void evaluateForce(int nodeId, double value, bool asFloat = false);
    void evaluateCalcForce(int nodeId);
    void writeSevrPvs();
    void writePv(const QString &pv, const QString &text, bool asString);
    void acknowledgeNode(int id);
    void afterStateChange(const QList<QVariantMap> &events);
    void emitEvents(const QList<QVariantMap> &events);
    void emitEvent(QVariantMap event);
    void updateBeepState();
    void updateStatus();
    void applyColumns();
    void syncSetupActions();
    void initViews();
    void fixColumnWidths();
    void restoreGroupRoot();
    bool lineButtonClicked(int id, int column);     // ack, G, P columns of both windows
    int nodeIdFor(const QModelIndex &viewIndex) const;
    QModelIndex treeIndexFor(int id) const;         // index in the tree window
    QModelIndex groupIndexFor(int id) const;        // index in the group window
    void expandTreeTo(int id);
    bool treeShows(int id) const;                   // all ancestors expanded in the tree window
    void descendInto(int id);                       // group window shows this group
    int nodeUnderCursor() const;
    int currentNode() const;
    int currentGroup() const;                       // selected group or the group of the selected channel
    int selectedOrWarn();                           // alh: "Please select an alarm group or channel first."
    QString valueText(const QString &String, const knobData &data) const;
    QString userName() const;
    void runCommand(int nodeId, bool withOutput);
    void expandRecursively(const QModelIndex &index, bool expand);
    QString propertiesText(int id) const;

    // config channel (JSON in a char waveform)
    QString effectiveConfigKey() const;
    void applyConfigDocument(const QString &json, bool fromPv);
    void applyOverrides();
    void setLocalOverride(int id, const QString &field, const QVariant &value);
    void setLocalOverrideFields(int id, const QMap<QString, QVariant> &fields);   // invalid value removes a field
    void writeConfigDocument();

    QString m_configFile, m_includeDir, m_macros, m_logTarget, m_configPV, m_configKey;
    bool m_alarmMode;
    int m_beepSeverity;
    CommandMode m_commandMode;
    DisplayFilter m_displayFilter;
    bool m_silenceForever, m_silenceOneHour, m_silenceCurrent;
    bool m_showMenuBar, m_showTreeWindow, m_showStatusArea, m_showMask, m_showValue, m_showPv;

    bool m_activated;
    QList<PvRequest> m_requests;
    QMap<QString, QString> m_map;
    AlhTreeModel *m_tree;
    AlhFilterProxy *m_proxy;
    AlhGroupProxy *m_groupProxy;
    QMenuBar *m_menuBar;
    QSplitter *m_splitter;
    QTreeView *m_treeView, *m_groupView;
    QWidget *m_statusArea;
    QLabel *m_execLabel, *m_fileLabel, *m_silenceForeverLabel, *m_beepLabel, *m_forceCountLabel;
    QCheckBox *m_silenceOneHourBox, *m_silenceCurrentBox;
    QAction *m_filterActions[3], *m_beepActions[3], *m_silenceForeverAction, *m_muteAllAction;
    int m_selected, m_groupRoot;
    QString m_resolvedConfig;
    caShellCommand *m_shell;
    caScriptButton *m_script;

    QVector<AuxPv> m_aux;
    int m_configAux;
    QHash<int, bool> m_forced;
    QHash<int, CalcState> m_calc;
    QHash<int, int> m_sevrWritten;
    QList<int> m_sevrPvNodes;
    QTimer *m_beepTimer, *m_heartbeat, *m_filterTimer, *m_noAckTimer, *m_silenceOneHourTimer, *m_refilterTimer;
    bool m_beepActive;
    int m_lastBeepSevr;
    int m_menuNode;
    QList<QVariantMap> m_history;
    QPointer<caAlarmLog> m_historyLog;

    QJsonObject m_configDoc;
    QMap<QString, QJsonObject> m_cfgNodes;
    bool m_cfgTreeMute;
    int m_configNelm;

    QString m_host;
    qint64 m_pid;
};

#endif // CAALARMTREE_H
