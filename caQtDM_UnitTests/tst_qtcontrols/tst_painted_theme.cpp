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

#include "tst_painted_theme.h"
#include "tst_theme_helpers.h"

#include <QImage>
#include <QTest>

#include "alarmdefs.h"
#include "caclock.h"
#include "cagauge.h"
#include "cagraphics.h"
#include "calabelvertical.h"
#include "cabytecontroller.h"
#include "cameter.h"
#include "epushbutton.h"

using namespace ThemeTestHelpers;

namespace {
// true when the image has a pixel at least as light as the dark text colour
bool hasLightPixel(const QImage &image)
{
    for (int y = 0; y < image.height(); ++y)
        for (int x = 0; x < image.width(); ++x)
            if (image.pixelColor(x, y).lightness() >= 200) return true;
    return false;
}
}

void TestPaintedTheme::clockDefaultFollowsScheme()
{
    QScopedPointer<QWidget> root(darkRoot());
    caClock *clock = new caClock(root.data());
    settleRoot(root.data());
    root->show();
    QCoreApplication::processEvents();

    const QColor darkBase = ThemeColors::color(ThemeColors::ClockBase, ThemeColors::Dark).darker(150);
    const QColor lightBase = ThemeColors::color(ThemeColors::ClockBase, ThemeColors::Light).darker(150);
    QCOMPARE(clock->getColorMode(), caClock::Default);
    QCOMPARE(clock->palette().color(QPalette::Base), darkBase);
    QCOMPARE(clock->palette().color(QPalette::Text), ThemeColors::color(ThemeColors::ClockScaleText, ThemeColors::Dark));
    QCOMPARE(clock->getBaseColor(), QColor(Qt::gray));                 // property keeps the light default
    switchRoot(root.data(), "light");
    QCOMPARE(clock->palette().color(QPalette::Base), lightBase);       // today's look
    switchRoot(root.data(), "dark");
    QCOMPARE(clock->palette().color(QPalette::Base), darkBase);

    // Static keeps the author colour, Alarm the alarm colour
    clock->setColorMode(caClock::Static);
    QCOMPARE(clock->palette().color(QPalette::Base), lightBase);
    clock->setBaseColor(Qt::blue);
    QCOMPARE(clock->palette().color(QPalette::Base), QColor(Qt::blue).darker(150));
    clock->setColorMode(caClock::Alarm);
    clock->setAlarmColors(MAJOR_ALARM);
    QCOMPARE(clock->palette().color(QPalette::Base), ThemeColors::color(ThemeColors::AlarmMajor));
}

void TestPaintedTheme::gaugesPaintTextWithPalette()
{
    QScopedPointer<QWidget> root(darkRoot());
    caCircularGauge *circular = new caCircularGauge(root.data());
    circular->resize(200, 160);
    caLinearGauge *linear = new caLinearGauge(root.data());
    linear->resize(80, 200);
    linear->move(220, 0);
    settleRoot(root.data());
    root->show();
    QCoreApplication::processEvents();
    QCOMPARE(circular->palette().color(QPalette::WindowText),
             ThemeColors::color(ThemeColors::PaletteWindowText, ThemeColors::Dark));
    // scale digits are painted with the palette text: light pixels exist on the dark gauges
    QVERIFY(hasLightPixel(circular->grab().toImage()));
    QVERIFY(hasLightPixel(linear->grab().toImage()));
}

void TestPaintedTheme::graphicsDefaultFollowsScheme()
{
    QScopedPointer<QWidget> root(darkRoot());
    caGraphics *graphics = new caGraphics(root.data());
    graphics->resize(60, 40);
    graphics->setForm(caGraphics::Rectangle);
    graphics->setFillStyle(caGraphics::Filled);
    settleRoot(root.data());
    root->show();
    QCoreApplication::processEvents();
    QCOMPARE(graphics->getColorMode(), caGraphics::Static);          // constructor default stays Static
    QVERIFY(!hasLightPixel(graphics->grab().toImage()));             // black fill, author colour
    graphics->setColorMode(caGraphics::Default);
    QVERIFY(hasLightPixel(graphics->grab().toImage()));              // default black follows the dark scheme
    QCOMPARE(graphics->getForeground(), QColor(Qt::black));          // properties keep the light default
    graphics->setForeground(Qt::red);                                // author colours stay
    graphics->setLineColor(Qt::red);
    QVERIFY(!hasLightPixel(graphics->grab().toImage()));
}

void TestPaintedTheme::labelVerticalDefaultFollowsScheme()
{
    QScopedPointer<QWidget> root(darkRoot());
    caLabelVertical *label = new caLabelVertical(root.data());
    label->resize(40, 120);
    label->setText("test");
    settleRoot(root.data());
    root->show();
    QCoreApplication::processEvents();
    QVERIFY(!hasLightPixel(label->grab().toImage()));                // Static: black text
    label->setColorMode(caLabelVertical::Default);
    QVERIFY(hasLightPixel(label->grab().toImage()));                 // Default: palette text
    label->setAlarmColors(MAJOR_ALARM);                              // ignored in Default
    QCOMPARE(label->getForeground(), QColor(Qt::black));
}

void TestPaintedTheme::byteControllerDefaultFollowsScheme()
{
    QScopedPointer<QWidget> root(darkRoot());
    caByteController *bytes = new caByteController(root.data());
    settleRoot(root.data());
    root->show();
    QCoreApplication::processEvents();
    const QList<EPushButton *> cells = bytes->findChildren<EPushButton *>();
    QVERIFY(cells.size() > 0);
    for (EPushButton *cell : cells) QVERIFY(cell->property(ThemeColors::SelfStyledProperty).toBool());
    bytes->setColorMode(caByteController::Default);
    bytes->setValue(0);
    const QColor clear = ThemeColors::color(ThemeColors::ByteClearBit, ThemeColors::Dark);
    const QColor text = ThemeColors::color(ThemeColors::TextPrimary, ThemeColors::Dark);
    QVERIFY(cells.first()->styleSheet().contains(QString("rgba(%1, %2, %3, 255)").arg(clear.red()).arg(clear.green()).arg(clear.blue())));
    QVERIFY(cells.first()->styleSheet().contains(QString("color: rgba(%1, %2, %3, 255)").arg(text.red()).arg(text.green()).arg(text.blue())));
    switchRoot(root.data(), "light");
    QVERIFY(cells.first()->styleSheet().contains("rgba(160, 160, 164, 255)"));
}

void TestPaintedTheme::meterDefaultFollowsScheme()
{
    QScopedPointer<QWidget> root(darkRoot());
    caMeter *meter = new caMeter(root.data());
    settleRoot(root.data());
    root->show();
    QCoreApplication::processEvents();
    const QColor darkBase = ThemeColors::color(ThemeColors::ClockBase, ThemeColors::Dark).darker(150);
    const QColor lightBase = ThemeColors::color(ThemeColors::ClockBase, ThemeColors::Light).darker(150);
    QCOMPARE(meter->getColorMode(), caMeter::Default);
    QCOMPARE(meter->palette().color(QPalette::Base), darkBase);
    QCOMPARE(meter->palette().color(QPalette::Text), ThemeColors::color(ThemeColors::ClockScaleText, ThemeColors::Dark));
    switchRoot(root.data(), "light");
    QCOMPARE(meter->palette().color(QPalette::Base), lightBase);
    meter->setColorMode(caMeter::Static);
    meter->setBaseColor(Qt::blue);
    QCOMPARE(meter->palette().color(QPalette::Base), QColor(Qt::blue).darker(150));
    meter->setColorMode(caMeter::Alarm);
    meter->setAlarmColors(MAJOR_ALARM);
    QCOMPARE(meter->palette().color(QPalette::Base), ThemeColors::color(ThemeColors::AlarmMajor));
}
