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
#ifndef TST_INTERNAL_PLUGIN_H
#define TST_INTERNAL_PLUGIN_H

#include <QList>
#include <QObject>
#include <QTest>
#include <QWidget>

#include "internal_plugin.h"

class TestInternalPlugin : public QObject
{
    Q_OBJECT
public:
    TestInternalPlugin() = default;

private slots:
    void initTestCase();
    void init();
    void cleanupTestCase();
    void cleanup();

    void pluginNameIsInternal();
    void addMonitorPublishesInitialValue();
    void configFromWidgetPropertyWorks();
    void filterSuffixIsIgnored();
    void channelsAreSharedByBaseName();
    void invalidConfigLeavesChannelUnconnected();
    void channelConnectsOnlyAfterConfiguration();
    void counterAdvancesWithTimerTicks();
    void writeThroughPluginWorks();
    void channelDeletedWhenUnreferenced();
    void persistentChannelKeepsRunning();
    void fieldMonitorsAndWritesWork();
    void unknownExtensionIsRejected();
    void fieldMonitorsTriggerOnlyOnChange();
    void controlInfoWriteForcesReinitialize();
    void drvlDrvhClampThroughPlugin();
    void schemePrefixInDirectCallsIsStripped();
    void matrixChannelThroughPluginWorks();
    void duplicateDefinitionIsReportedAndIgnored();

private:
    // registers a monitor; the configuration travels like in the real
    // application through the channelConfigJSON property of the widget
    int createMonitor(const QString &pv, const QString &configJSON = QString(), const QString &widgetName = QString());
    void pumpTimerOnce();

    InternalPlugin *m_plugin;
    MutexKnobData *m_mutexKnobData;
    QList<QWidget *> m_widgets;
    QList<int> m_indexes;
};

#endif // TST_INTERNAL_PLUGIN_H
