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

#include "tst_textwidgets_theme.h"
#include "tst_theme_helpers.h"

#include <QApplication>
#include <QComboBox>
#include <QTest>

#include "alarmdefs.h"
#include "calineedit.h"
#include "camultilinestring.h"
#include "camenu.h"
#include "replacemacro.h"
#include "caslider.h"
#include "calinedraw.h"

using namespace ThemeTestHelpers;

namespace {
QString rgba(const QColor &c)
{
    return QString("rgba(%1, %2, %3, %4)").arg(c.red()).arg(c.green()).arg(c.blue()).arg(c.alpha());
}
const QColor darkBase = ThemeColors::color(ThemeColors::PaletteBase, ThemeColors::Dark);
const QColor darkText = ThemeColors::color(ThemeColors::PaletteText, ThemeColors::Dark);
const QColor lightBase = ThemeColors::color(ThemeColors::PaletteBase, ThemeColors::Light);
const QColor lightText = ThemeColors::color(ThemeColors::PaletteText, ThemeColors::Light);
}

void TestTextWidgetsTheme::lineEditDefaultFollowsPalette()
{
    // without a panel root: the application palette, as in the designer
    QWidget plain;
    caLineEdit *loose = new caLineEdit(&plain);
    plain.show();
    QCoreApplication::processEvents();
    const QPalette app = QApplication::palette();
    QVERIFY(loose->styleSheet().contains("background-color: " + rgba(app.color(QPalette::Base))));
    QVERIFY(loose->styleSheet().contains("color: " + rgba(app.color(QPalette::Text))));

    QScopedPointer<QWidget> root(darkRoot());
    caLineEdit *edit = new caLineEdit(root.data());
    QCOMPARE(edit->getColorMode(), caLineEdit::Default);
    settleRoot(root.data());
    root->show();
    QCoreApplication::processEvents();
    QVERIFY(edit->styleSheet().contains("background-color: " + rgba(darkBase)));
    QVERIFY(edit->styleSheet().contains("color: " + rgba(darkText)));

    // runtime switch of the panel palette, no setColorMode involved
    switchRoot(root.data(), "light");
    QVERIFY(edit->styleSheet().contains("background-color: " + rgba(lightBase)));
    QVERIFY(edit->styleSheet().contains("color: " + rgba(lightText)));
    switchRoot(root.data(), "dark");
    QVERIFY(edit->styleSheet().contains("background-color: " + rgba(darkBase)));

    // the focus frame literal is unchanged
    QVERIFY(edit->styleSheet().contains("2px groove rgba(255, 0, 0, 255)"));
}

void TestTextWidgetsTheme::lineEditStaticAndAlarmKeepColours()
{
    QScopedPointer<QWidget> root(darkRoot());
    const QColor wheat(245, 222, 179);
    caLineEdit *fixed = new caLineEdit(root.data());
    fixed->setColorMode(caLineEdit::Static);
    fixed->setBackground(wheat);
    fixed->setForeground(Qt::black);
    caLineEdit *alarm = new caLineEdit(root.data());
    alarm->setColorMode(caLineEdit::Alarm_Default);
    alarm->setAlarmHandling(caLineEdit::onBackground);
    settleRoot(root.data());
    root->show();
    QCoreApplication::processEvents();

    QVERIFY(fixed->styleSheet().contains("background-color: " + rgba(wheat)));
    QVERIFY(fixed->styleSheet().contains("color: " + rgba(QColor(Qt::black))));
    switchRoot(root.data(), "light");
    QVERIFY(fixed->styleSheet().contains("background-color: " + rgba(wheat)));

    switchRoot(root.data(), "dark");
    alarm->setAlarmColors(MAJOR_ALARM, 0.0, wheat, Qt::black);
    QVERIFY(alarm->styleSheet().contains("background-color: rgba(255, 0, 0, 255)"));
    QVERIFY(alarm->styleSheet().contains("color: " + rgba(darkBase)));     // text takes the panel base
    alarm->setAlarmColors(NO_ALARM, 0.0, wheat, Qt::black);                 // light palette text on green: flipped
    QVERIFY(alarm->styleSheet().contains("background-color: rgba(0, 205, 0, 255)"));
    QVERIFY(!alarm->styleSheet().contains("color: " + rgba(darkText)));
    alarm->setAlarmColors(NOTCONNECTED, 0.0, wheat, Qt::black);
    QVERIFY(alarm->styleSheet().contains("background-color: rgba(255, 255, 255, 255)"));

    // alarm on the background: a pair-adapted light text must stay readable on it (dark only)
    const QColor lightText(232, 232, 232);
    caLineEdit *onAlarm = new caLineEdit(root.data());
    onAlarm->setColorMode(caLineEdit::Alarm_Static);
    onAlarm->setAlarmHandling(caLineEdit::onBackground);
    settleRoot(root.data());
    onAlarm->setAlarmColors(NO_ALARM, 0.0, wheat, lightText);
    QVERIFY(onAlarm->styleSheet().contains("background-color: rgba(0, 205, 0, 255)"));
    QVERIFY(!onAlarm->styleSheet().contains("color: " + rgba(lightText)));
    switchRoot(root.data(), "light");
    onAlarm->setAlarmColors(NO_ALARM, 0.0, wheat, lightText);
    QVERIFY(onAlarm->styleSheet().contains("color: " + rgba(lightText)));  // light scheme: untouched
}

void TestTextWidgetsTheme::multiLineStringDefaultFollowsPalette()
{
    QScopedPointer<QWidget> root(darkRoot());
    caMultiLineString *text = new caMultiLineString(root.data());
    settleRoot(root.data());
    root->show();
    QCoreApplication::processEvents();
    QVERIFY(text->styleSheet().contains("background-color: " + rgba(darkBase)));
    QVERIFY(text->styleSheet().contains("color: " + rgba(darkText)));
    switchRoot(root.data(), "light");
    QVERIFY(text->styleSheet().contains("background-color: " + rgba(lightBase)));
}

// a transparent Base (panel stylesheet "background: rgba(.., 0)") must not reach
// the generated stylesheet: field and popup list would be see-through
void TestTextWidgetsTheme::defaultColoursStayOpaque()
{
    QScopedPointer<QWidget> root(darkRoot());
    QPalette seeThrough = root->palette();
    seeThrough.setColor(QPalette::Base, QColor(105, 105, 105, 0));
    seeThrough.setColor(QPalette::Text, QColor(200, 200, 200, 0));
    root->setPalette(seeThrough);
    caLineEdit *edit = new caLineEdit(root.data());
    caMenu *menu = new caMenu(root.data());
    settleRoot(root.data());
    root->show();
    QCoreApplication::processEvents();
    QVERIFY2(edit->styleSheet().contains("background-color: " + rgba(darkBase)), qPrintable(edit->styleSheet()));
    QVERIFY2(edit->styleSheet().contains("color: " + rgba(darkText)), qPrintable(edit->styleSheet()));
    QVERIFY2(menu->styleSheet().contains("background-color: " + rgba(darkBase)), qPrintable(menu->styleSheet()));
    QVERIFY(!menu->styleSheet().contains(", 0)"));
}

void TestTextWidgetsTheme::menuDefaultFollowsPalette()
{
    QScopedPointer<QWidget> root(darkRoot());
    caMenu *menu = new caMenu(root.data());
    settleRoot(root.data());
    root->show();
    QCoreApplication::processEvents();
    QVERIFY(menu->styleSheet().contains("background-color: " + rgba(darkBase)));
    QVERIFY(menu->styleSheet().contains("color: " + rgba(darkText)));
    switchRoot(root.data(), "light");
    QVERIFY(menu->styleSheet().contains("background-color: " + rgba(lightBase)));
    menu->setAlarmColors(MAJOR_ALARM);                       // ignored in Default mode
    QVERIFY(menu->styleSheet().contains("background-color: " + rgba(lightBase)));
    menu->setColorMode(caMenu::Alarm);
    menu->setAlarmColors(MAJOR_ALARM);
    QVERIFY(menu->styleSheet().contains("background-color: rgba(255, 0, 0, 255)"));
}

void TestTextWidgetsTheme::replaceMacroDefaultFollowsPalette()
{
    QScopedPointer<QWidget> root(darkRoot());
    replaceMacro *macro = new replaceMacro(root.data());
    settleRoot(root.data());
    root->show();
    QCoreApplication::processEvents();
    const QList<QComboBox *> combos = macro->findChildren<QComboBox *>();
    QVERIFY(combos.size() >= 2);
    for (QComboBox *combo : combos)
        QVERIFY(combo->styleSheet().contains("background-color: " + rgba(darkBase)));
    switchRoot(root.data(), "light");
    for (QComboBox *combo : combos)
        QVERIFY(combo->styleSheet().contains("background-color: " + rgba(lightBase)));
}

void TestTextWidgetsTheme::sliderDefaultFollowsPalette()
{
    QScopedPointer<QWidget> root(darkRoot());
    caSlider *slider = new caSlider(root.data());
    slider->setColorMode(caSlider::Default);
    settleRoot(root.data());
    root->show();
    QCoreApplication::processEvents();
    QVERIFY(slider->styleSheet().contains("background-color: " + rgba(darkBase)));
    QVERIFY(slider->styleSheet().contains("border: 1px solid rgb(255, 0, 0)"));
    switchRoot(root.data(), "light");
    QVERIFY(slider->styleSheet().contains("background-color: " + rgba(lightBase)));

    // static keeps the author colour under a dark root
    caSlider *fixed = new caSlider(root.data());
    fixed->setBackground(QColor(255, 255, 224));
    switchRoot(root.data(), "dark");
    QVERIFY(fixed->styleSheet().contains("background-color: rgba(255, 255, 224, 255)"));
}

void TestTextWidgetsTheme::lineDrawDefaultFollowsPalette()
{
    QScopedPointer<QWidget> root(darkRoot());
    caLineDraw *draw = new caLineDraw(root.data());
    QCOMPARE(draw->getColorMode(), caLineDraw::Default);
    draw->resize(120, 30);
    settleRoot(root.data());
    root->show();
    QCoreApplication::processEvents();
    // default mode paints the palette colours; the properties keep the author values
    QCOMPARE(draw->getBackground(), QColor(255, 255, 255, 0));
    QCOMPARE(draw->grab().toImage().pixelColor(draw->width() / 2, draw->height() / 2), darkBase);
    switchRoot(root.data(), "light");
    QCOMPARE(draw->grab().toImage().pixelColor(draw->width() / 2, draw->height() / 2), lightBase);
}

// a palette change must not undo the colours the dispatcher set last:
// not connected (white on white) and alarm colours survive the deferred refresh
void TestTextWidgetsTheme::lineEditKeepsStateAcrossPaletteChange()
{
    QScopedPointer<QWidget> root(darkRoot());
    caLineEdit *edit = new caLineEdit(root.data());
    settleRoot(root.data());
    root->show();
    QCoreApplication::processEvents();
    const QString white = rgba(ThemeColors::color(ThemeColors::AlarmInvalid));
    edit->setAlarmColors(NOTCONNECTED, 0.0, edit->getBackground(), edit->getForeground());
    QVERIFY(edit->styleSheet().contains("background-color: " + white));
    switchRoot(root.data(), "light");
    switchRoot(root.data(), "dark");                         // dark base != white: must stay white
    QVERIFY2(edit->styleSheet().contains("background-color: " + white), qPrintable(edit->styleSheet()));
    // reconnect: the author colours (here the palette defaults) come back
    edit->setAlarmColors(NO_ALARM, 0.0, edit->getBackground(), edit->getForeground());
    QVERIFY(edit->styleSheet().contains("background-color: " + rgba(darkBase)));

    caLineEdit *alarm = new caLineEdit(root.data());
    alarm->setColorMode(caLineEdit::Alarm_Default);
    alarm->setAlarmHandling(caLineEdit::onBackground);
    settleRoot(root.data());
    alarm->setAlarmColors(MAJOR_ALARM, 0.0, alarm->getBackground(), alarm->getForeground());
    const QString red = rgba(ThemeColors::color(ThemeColors::AlarmMajor));
    QVERIFY(alarm->styleSheet().contains("background-color: " + red));
    switchRoot(root.data(), "dark");
    QVERIFY2(alarm->styleSheet().contains("background-color: " + red), qPrintable(alarm->styleSheet()));
}

