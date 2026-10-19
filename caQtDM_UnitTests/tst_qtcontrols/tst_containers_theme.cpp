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
 *  Copyright (c) 2026
 */

#include "tst_containers_theme.h"
#include "tst_theme_helpers.h"

#include <QLabel>
#include <QTest>

#include "alarmdefs.h"
#include "caframe.h"
#include "cainclude.h"
#include "catable.h"
#include "cawavetable.h"
#include "cadoubletabwidget.h"

using namespace ThemeTestHelpers;

void TestContainersTheme::frameOutlineAndFilled()
{
    QScopedPointer<QWidget> root(darkRoot());
    caFrame *frame = new caFrame(root.data());
    QCOMPARE(frame->getBackground(), ThemeColors::color(ThemeColors::FrameLine));
    frame->setBackground(Qt::red);
    QCOMPARE(frame->getBackgroundMode(), caFrame::Outline);
    QCOMPARE(frame->palette().color(QPalette::WindowText), QColor(Qt::red));
    QVERIFY(!frame->autoFillBackground());
    frame->setBackgroundMode(caFrame::Filled);
    QVERIFY(frame->autoFillBackground());
    QCOMPARE(frame->palette().color(QPalette::Window), QColor(Qt::red));
}

void TestContainersTheme::includeFrameColorReachesFrame()
{
    QScopedPointer<QWidget> root(darkRoot());
    caInclude *include = new caInclude(root.data());
    QCOMPARE(include->getFrameColor(), ThemeColors::color(ThemeColors::FrameLine));
    include->setFrameColor(Qt::red);                     // runtime path (loadIncludes is false)
    QVERIFY(include->getIncludeFrame() != Q_NULLPTR);
    QCOMPARE(include->getIncludeFrame()->palette().color(QPalette::WindowText), QColor(Qt::red));
    QCOMPARE(include->getIncludeFrame()->palette().color(QPalette::Window), QColor(Qt::red));
}

void TestContainersTheme::tableStaticUsesPaletteText()
{
    QScopedPointer<QWidget> root(darkRoot());
    caTable *table = new caTable(root.data());
    table->setRowCount(1);
    table->setColumnCount(3);
    table->displayText(0, 1, NO_ALARM, "x");
    QVERIFY(table->item(0, 1) != Q_NULLPTR);
    QCOMPARE(table->item(0, 1)->foreground().style(), Qt::NoBrush);
    table->setColorMode(caTable::Alarm);
    table->displayText(0, 1, MAJOR_ALARM, "x");
    QCOMPARE(table->item(0, 1)->foreground().color(), ThemeColors::color(ThemeColors::AlarmMajor));
}

void TestContainersTheme::tableBuiltInStyleFollowsScheme()
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QSKIP("built-in table style is generated on Qt 6 only");
#else
    QScopedPointer<QWidget> root(darkRoot());
    caTable *table = new caTable(root.data());
    settleRoot(root.data());
    root->show();
    QTest::qWait(20);
    QVERIFY(table->property(ThemeColors::SelfStyledProperty).toBool());
    QCOMPARE(table->palette().color(QPalette::AlternateBase),
             ThemeColors::color(ThemeColors::TableAlternateBase, ThemeColors::Dark));
    QVERIFY(table->styleSheet().contains(ThemeColors::css(ThemeColors::TableHeader, ThemeColors::Dark)));
    switchRoot(root.data(), "light");
    QCOMPARE(table->palette().color(QPalette::AlternateBase),
             ThemeColors::color(ThemeColors::TableAlternateBase, ThemeColors::Light));
    QVERIFY(table->styleSheet().contains(ThemeColors::css(ThemeColors::TableHeader, ThemeColors::Light)));

    // an ancestor stylesheet at construction time disables the built-in look for good
    // (the top-level window itself is not inspected, as before)
    QScopedPointer<QWidget> styled(darkRoot());
    QWidget *holder = new QWidget(styled.data());
    holder->setStyleSheet("QLabel { color: black; }");
    caTable *plain = new caTable(holder);
    settleRoot(styled.data());
    QVERIFY(plain->styleSheet().isEmpty());
    QVERIFY(!plain->property(ThemeColors::SelfStyledProperty).toBool());
#endif
}

void TestContainersTheme::waveTableBuiltInStyleFollowsScheme()
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QSKIP("built-in table style is generated on Qt 6 only");
#else
    QScopedPointer<QWidget> root(darkRoot());
    caWaveTable *table = new caWaveTable(root.data());
    settleRoot(root.data());
    root->show();
    QTest::qWait(20);
    QVERIFY(table->property(ThemeColors::SelfStyledProperty).toBool());
    QCOMPARE(table->palette().color(QPalette::AlternateBase),
             ThemeColors::color(ThemeColors::TableAlternateBase, ThemeColors::Dark));
    QVERIFY(table->styleSheet().contains(ThemeColors::css(ThemeColors::TableGrid, ThemeColors::Dark)));
    switchRoot(root.data(), "light");
    QVERIFY(table->styleSheet().contains(ThemeColors::css(ThemeColors::TableGrid, ThemeColors::Light)));

    // an authored sheet replaces the built-in one and is left to the matrix
    table->noStyle("color: yellow;");
    QVERIFY(!table->property(ThemeColors::SelfStyledProperty).toBool());
    switchRoot(root.data(), "dark");
    QCOMPARE(table->styleSheet(), QString("color: yellow;"));
#endif
}

void TestContainersTheme::doubleTabWidgetFollowsScheme()
{
    QScopedPointer<QWidget> root(darkRoot());
    caDoubleTabWidget *tabs = new caDoubleTabWidget(root.data());
    tabs->setItemsHor("A;B");
    tabs->setItemsVer("1;2");
    QWidget *page = new QWidget;
    tabs->addPage(page);
    settleRoot(root.data());
    root->show();
    QTest::qWait(20);

    QVERIFY(tabs->property(ThemeColors::SelfStyledProperty).toBool());
    QVERIFY(tabs->styleSheet().contains(ThemeColors::css(ThemeColors::TabGradientTop, ThemeColors::Dark)));
    QVERIFY(tabs->styleSheet().contains(ThemeColors::css(ThemeColors::TabChecked, ThemeColors::Dark)));
    QVERIFY(page->property(ThemeColors::SelfStyledProperty).toBool());
    QVERIFY(page->styleSheet().contains(ThemeColors::css(ThemeColors::TabPage, ThemeColors::Dark)));
    switchRoot(root.data(), "light");
    QVERIFY(tabs->styleSheet().contains(ThemeColors::css(ThemeColors::TabGradientTop, ThemeColors::Light)));
    QVERIFY(page->styleSheet().contains(ThemeColors::css(ThemeColors::TabPage, ThemeColors::Light)));

    // an authored sheet survives the rebuild of the vertical buttons
    tabs->setProperty("styleSheet", "QPushButton { color: red; }");
    tabs->setItemsVer("1;2;3");
    QCoreApplication::processEvents();
    QCOMPARE(tabs->styleSheet(), QString("QPushButton { color: red; }"));
    QVERIFY(!tabs->property(ThemeColors::SelfStyledProperty).toBool());
    switchRoot(root.data(), "dark");
    QCOMPARE(tabs->styleSheet(), QString("QPushButton { color: red; }"));
}
