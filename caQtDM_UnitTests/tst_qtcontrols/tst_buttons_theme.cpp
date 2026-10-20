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

#include "tst_buttons_theme.h"
#include "tst_theme_helpers.h"

#include <QMenu>
#include <QTest>

#include "alarmdefs.h"
#include "camessagebutton.h"
#include "catogglebutton.h"
#include "cachoice.h"
#include "carowcolmenu.h"
#include "cascriptbutton.h"
#include "epushbutton.h"
#include "imagepushbutton.h"

using namespace ThemeTestHelpers;

namespace {
// ImagePushButton has no Q_OBJECT: collect them through the EPushButton children
QList<ImagePushButton *> imageButtons(const QWidget *parent)
{
    QList<ImagePushButton *> result;
    for (EPushButton *button : parent->findChildren<EPushButton *>()) {
        if (ImagePushButton *image = dynamic_cast<ImagePushButton *>(button)) result.append(image);
    }
    return result;
}
QString rgba(const QColor &c)
{
    return QString("rgba(%1, %2, %3, %4)").arg(c.red()).arg(c.green()).arg(c.blue()).arg(c.alpha());
}
const QColor darkWindow = ThemeColors::color(ThemeColors::PaletteWindow, ThemeColors::Dark);
const QColor lightWindow = ThemeColors::color(ThemeColors::PaletteWindow, ThemeColors::Light);
const QColor darkText = ThemeColors::color(ThemeColors::PaletteWindowText, ThemeColors::Dark);
const QColor lightText = ThemeColors::color(ThemeColors::PaletteWindowText, ThemeColors::Light);
}

void TestButtonsTheme::messageButtonModes()
{
    QScopedPointer<QWidget> root(darkRoot());
    caMessageButton *button = new caMessageButton(root.data());
    settleRoot(root.data());
    root->show();
    QCoreApplication::processEvents();

    // the constructor default is Static with the fixed grey, in both schemes
    QCOMPARE(button->getColorMode(), caMessageButton::Static);
    QVERIFY(button->styleSheet().contains(rgba(ThemeColors::color(ThemeColors::StaticButton))));
    switchRoot(root.data(), "light");
    QVERIFY(button->styleSheet().contains(rgba(ThemeColors::color(ThemeColors::StaticButton))));

    // Default: no stylesheet, the palette paints the button
    button->setColorMode(caMessageButton::Default);
    QVERIFY(button->styleSheet().isEmpty());

    // Alarm: the alarm colour is the background
    button->setColorMode(caMessageButton::Alarm);
    button->setAlarmColors(MAJOR_ALARM);
    QVERIFY(button->styleSheet().contains(rgba(ThemeColors::color(ThemeColors::AlarmMajor))));
}

void TestButtonsTheme::toggleButtonDefaultFollowsPalette()
{
    QScopedPointer<QWidget> root(darkRoot());
    caToggleButton *toggle = new caToggleButton(root.data());
    settleRoot(root.data());
    root->show();
    QCoreApplication::processEvents();
    QCOMPARE(toggle->getColorMode(), caToggleButton::Default);
    QVERIFY(toggle->styleSheet().isEmpty());
    QCOMPARE(toggle->palette().color(QPalette::WindowText), darkText);
    switchRoot(root.data(), "light");
    QVERIFY(toggle->styleSheet().isEmpty());
    QCOMPARE(toggle->palette().color(QPalette::WindowText), lightText);

    // Static keeps the author colours under any root
    toggle->setColorMode(caToggleButton::Static);
    toggle->setBackground(QColor(255, 255, 224));
    switchRoot(root.data(), "dark");
    QVERIFY(toggle->styleSheet().contains("rgba(255, 255, 224, 255)"));
}

void TestButtonsTheme::choiceCellsSelfStyledAndDefaultStructural()
{
    QScopedPointer<QWidget> root(darkRoot());
    caChoice *choice = new caChoice(root.data());
    choice->populateCells(QStringList() << "OFF" << "ON" << "ERROR", 0);
    settleRoot(root.data());
    root->show();
    QCoreApplication::processEvents();

    const QList<EPushButton *> cells = choice->findChildren<EPushButton *>();
    QCOMPARE(cells.size(), 3);
    for (EPushButton *cell : cells)
        QVERIFY(cell->property(ThemeColors::SelfStyledProperty).toBool());

    // Default: structural sheet without any colour
    QCOMPARE(choice->getColorMode(), caChoice::Default);
    QVERIFY(!choice->styleSheet().contains("rgb"));
    QVERIFY(choice->styleSheet().contains("border-style: outset"));

    // Static: the author colours form the gradient, new cells are marked as well
    choice->setColorMode(caChoice::Static);
    QVERIFY(choice->styleSheet().contains("qlineargradient"));
    QVERIFY(choice->styleSheet().contains(ThemeColors::css(ThemeColors::DisabledText, ThemeColors::Light)));
    choice->populateCells(QStringList() << "A" << "B", 1);
    QCoreApplication::processEvents();
    for (EPushButton *cell : choice->findChildren<EPushButton *>())
        QVERIFY(cell->property(ThemeColors::SelfStyledProperty).toBool());
}

void TestButtonsTheme::rowColMenuMenuFollowsRootPalette()
{
    QScopedPointer<QWidget> root(darkRoot());
    caRowColMenu *menu = new caRowColMenu(root.data());
    menu->setStacking(caRowColMenu::Menu);              // two default entries -> popup menu
    settleRoot(root.data());
    root->show();
    QCoreApplication::processEvents();

    QList<ImagePushButton *> buttons = imageButtons(menu);
    QCOMPARE(buttons.size(), 1);
    QVERIFY(buttons.first()->property(ThemeColors::SelfStyledProperty).toBool());
    QVERIFY(buttons.first()->menu() != Q_NULLPTR);
    QCOMPARE(buttons.first()->menu()->palette().color(QPalette::Window), darkWindow);
    switchRoot(root.data(), "light");
    QCOMPARE(buttons.first()->menu()->palette().color(QPalette::Window), lightWindow);

    // repopulating replaces the button instead of hiding the old one
    menu->setLabels("a;b;c");
    QTest::qWait(20);                                   // deleteLater of the old button
    buttons = imageButtons(menu);
    QCOMPARE(buttons.size(), 1);
    QCOMPARE(buttons.first()->menu()->palette().color(QPalette::Window), lightWindow);
    for (EPushButton *cell : menu->findChildren<EPushButton *>())
        QVERIFY(cell->property(ThemeColors::SelfStyledProperty).toBool());

    // a transparent Window colour (stylesheet "background: rgba(.., 0)" on the panel)
    // must not reach the popup: it stays opaque in the colours of the scheme
    QPalette seeThrough = root->palette();
    seeThrough.setColor(QPalette::Window, QColor(105, 105, 105, 0));
    root->setPalette(seeThrough);
    QCoreApplication::processEvents();
    const QColor popupWindow = buttons.first()->menu()->palette().color(QPalette::Window);
    QCOMPARE(popupWindow.alpha(), 255);
    QCOMPARE(popupWindow, ThemeColors::palette(ThemeColors::Light).color(QPalette::Window));
}

void TestButtonsTheme::scriptButtonStaysNativeUntilColoured()
{
    QScopedPointer<QWidget> root(darkRoot());
    caScriptButton *script = new caScriptButton(root.data());
    settleRoot(root.data());
    root->show();
    QCoreApplication::processEvents();
    QVERIFY(script->styleSheet().isEmpty());            // native button, as in legacy panels
    switchRoot(root.data(), "light");
    QVERIFY(script->styleSheet().isEmpty());

    // an author colour brings the generated sheet with valid hover/border colours
    script->setBackground(Qt::red);
    QVERIFY(script->styleSheet().contains(rgba(QColor(Qt::red).darker(150))));
    QVERIFY(script->styleSheet().contains(rgba(QColor(Qt::red).lighter(120))));

    // a font change still rescales the child (changeEvent calls the base class)
    QFont font = script->font();
    font.setPointSize(font.pointSize() + 4);
    script->setFont(font);
    QCoreApplication::processEvents();
    EPushButton *child = script->findChild<EPushButton *>();
    QVERIFY(child != Q_NULLPTR);
    QVERIFY(child->isVisible());
}
