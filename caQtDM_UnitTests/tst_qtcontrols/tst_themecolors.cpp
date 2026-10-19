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

#include "tst_themecolors.h"
#include "tst_theme_helpers.h"

#include <QApplication>
#include <QTest>
#include <QWidget>
#include <cmath>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "themecolors.h"
#include "alarmdefs.h"
#include "cathermo.h"
#include "cacartesianplot.h"
#include "castripplot.h"
#include <qwt_scale_widget.h>

namespace {

using ThemeTestHelpers::darkRoot;
using ThemeTestHelpers::switchRoot;
using ThemeTestHelpers::settleRoot;

double channel(int v)
{
    const double c = v / 255.0;
    return c <= 0.03928 ? c / 12.92 : std::pow((c + 0.055) / 1.055, 2.4);
}

double luminance(const QColor &c)
{
    return 0.2126 * channel(c.red()) + 0.7152 * channel(c.green()) + 0.0722 * channel(c.blue());
}

double contrast(const QColor &a, const QColor &b)
{
    const double la = luminance(a), lb = luminance(b);
    return (qMax(la, lb) + 0.05) / (qMin(la, lb) + 0.05);
}

}

void TestThemeColors::alarmColoursAreFixedAndAliased()
{
    const ThemeColor *alarms[] = { &ThemeColors::AlarmNoAlarm, &ThemeColors::AlarmMinor,
                                   &ThemeColors::AlarmMajor, &ThemeColors::AlarmInvalid,
                                   &ThemeColors::AlarmDefault };
    for (const ThemeColor *c : alarms)
        QCOMPARE(ThemeColors::color(*c, ThemeColors::Dark), ThemeColors::color(*c, ThemeColors::Light));

    // the historic alarmdefs.h values
    QCOMPARE(AL_GREEN, QColor(0x00, 0xcd, 0x00));
    QCOMPARE(AL_YELLOW, QColor(0xff, 0xff, 0x00));
    QCOMPARE(AL_RED, QColor(0xff, 0x00, 0x00));
    QCOMPARE(AL_WHITE, QColor(0xff, 0xff, 0xff));
    QCOMPARE(AL_GREY, QColor(0x88, 0x88, 0x88));
    QCOMPARE(AL_DEFAULT, AL_GREY);
    QCOMPARE(AL_LIGHTBLUE, QColor(0xad, 0xd8, 0xe6));
}

void TestThemeColors::cssFormat()
{
    QCOMPARE(ThemeColors::css(ThemeColors::PaletteBase, ThemeColors::Light), QString("rgb(255, 255, 255)"));
    QCOMPARE(ThemeColors::css(ThemeColors::PalettePlaceholderText, ThemeColors::Light),
             QString("rgba(0, 0, 0, 128)"));
}

void TestThemeColors::schemeOfPalette()
{
    QCOMPARE(ThemeColors::schemeOf(ThemeColors::palette(ThemeColors::Light)), ThemeColors::Light);
    QCOMPARE(ThemeColors::schemeOf(ThemeColors::palette(ThemeColors::Dark)), ThemeColors::Dark);
    QPalette p;
    p.setColor(QPalette::Window, QColor(127, 127, 127));
    QCOMPARE(ThemeColors::schemeOf(p), ThemeColors::Dark);
    p.setColor(QPalette::Window, QColor(128, 128, 128));
    QCOMPARE(ThemeColors::schemeOf(p), ThemeColors::Light);
}

void TestThemeColors::schemeOfWidgetUsesNearestRoot()
{
    QWidget plain;
    QCOMPARE(ThemeColors::schemeOf(&plain), ThemeColors::Light);   // designer / no panel
    QCOMPARE(ThemeColors::schemeOf(static_cast<QWidget *>(Q_NULLPTR)), ThemeColors::Light);

    QScopedPointer<QWidget> root(darkRoot());
    QWidget *frame = new QWidget(root.data());
    QWidget *child = new QWidget(frame);
    QCOMPARE(ThemeColors::schemeOf(child), ThemeColors::Dark);

    // a nested include with its own root wins
    frame->setProperty(ThemeColors::ThemeRootProperty, true);
    frame->setProperty(ThemeColors::ThemeSchemeProperty, "light");
    QCOMPARE(ThemeColors::schemeOf(child), ThemeColors::Light);
    QCOMPARE(ThemeColors::schemeOf(root.data()), ThemeColors::Dark);

    // themed(): only the light default follows the scheme
    QCOMPARE(ThemeColors::themed(QColor(224, 224, 224), ThemeColors::Surface, ThemeColors::Dark),
             ThemeColors::color(ThemeColors::Surface, ThemeColors::Dark));
    QCOMPARE(ThemeColors::themed(Qt::red, ThemeColors::Surface, ThemeColors::Dark), QColor(Qt::red));
    QCOMPARE(ThemeColors::themed(QColor(224, 224, 224), ThemeColors::Surface, ThemeColors::Light),
             QColor(224, 224, 224));
}

void TestThemeColors::darkSurfacesKeepContrastToAlarmColours()
{
    const ThemeColor *surfaces[] = { &ThemeColors::Surface, &ThemeColors::SurfaceSunken,
                                     &ThemeColors::ArrowBackground, &ThemeColors::ThermoPipeDefault,
                                     &ThemeColors::PlotFrame, &ThemeColors::PaletteWindow,
                                     &ThemeColors::PaletteBase, &ThemeColors::PaletteAlternateBase,
                                     &ThemeColors::PaletteButton, &ThemeColors::TabPage,
                                     &ThemeColors::TabGradientTop, &ThemeColors::TabGradientBottom,
                                     &ThemeColors::TableAlternateBase, &ThemeColors::TableHeader };
    const ThemeColor *alarms[] = { &ThemeColors::AlarmNoAlarm, &ThemeColors::AlarmMinor,
                                   &ThemeColors::AlarmMajor, &ThemeColors::AlarmInvalid,
                                   &ThemeColors::AlarmDefault };
    for (const ThemeColor *s : surfaces) {
        for (const ThemeColor *a : alarms) {
            const double ratio = contrast(ThemeColors::color(*s, ThemeColors::Dark),
                                          ThemeColors::color(*a, ThemeColors::Dark));
            QVERIFY2(ratio >= 3.0, qPrintable(QString("contrast %1 for %2 on %3")
                                                  .arg(ratio)
                                                  .arg(ThemeColors::color(*a).name())
                                                  .arg(ThemeColors::color(*s, ThemeColors::Dark).name())));
        }
    }
    // the dark text must be readable on every dark surface as well
    for (const ThemeColor *s : surfaces)
        QVERIFY(contrast(ThemeColors::color(*s, ThemeColors::Dark),
                         ThemeColors::color(ThemeColors::TextPrimary, ThemeColors::Dark)) >= 4.5);
}

void TestThemeColors::thermoDefaultsUnchangedWithoutRoot()
{
    QWidget parent;
    caThermo *thermo = new caThermo(&parent);
    parent.show();
    QCoreApplication::processEvents();

    // the historic constructor defaults (Static mode)
    QCOMPARE(thermo->getBackground(), QColor(224, 224, 224));
    QCOMPARE(thermo->getForeground(), QColor(Qt::black));
    QCOMPARE(thermo->getTextColor(), QColor(Qt::black));
    QCOMPARE(thermo->palette().color(QPalette::Window), QColor(224, 224, 224));
    QCOMPARE(thermo->palette().color(QPalette::ButtonText), QColor(Qt::black));
    QCOMPARE(thermo->palette().color(QPalette::Base), QColor(224, 224, 224).darker(125));

    thermo->setColorMode(caThermo::Default);
    QCoreApplication::processEvents();
    QCOMPARE(thermo->palette().color(QPalette::Window), QColor(224, 224, 224));
    QCOMPARE(thermo->palette().color(QPalette::WindowText), QColor(Qt::black));
}

void TestThemeColors::thermoDefaultModeFollowsDarkRoot()
{
    QScopedPointer<QWidget> root(darkRoot());
    caThermo *thermo = new caThermo(root.data());
    thermo->setColorMode(caThermo::Default);
    root->show();
    QCoreApplication::processEvents();

    QCOMPARE(thermo->palette().color(QPalette::Window),
             ThemeColors::color(ThemeColors::Surface, ThemeColors::Dark));
    QCOMPARE(thermo->palette().color(QPalette::WindowText),
             ThemeColors::color(ThemeColors::TextPrimary, ThemeColors::Dark));
    QCOMPARE(thermo->palette().color(QPalette::ButtonText),
             ThemeColors::color(ThemeColors::ThermoFillDefault, ThemeColors::Dark));
    QCOMPARE(thermo->palette().color(QPalette::Base),
             ThemeColors::color(ThemeColors::ThermoPipeDefault, ThemeColors::Dark).darker(125));
    // the author properties are untouched
    QCOMPARE(thermo->getBackground(), QColor(224, 224, 224));

    // runtime switch back to light and again to dark
    switchRoot(root.data(), "light");
    QCOMPARE(thermo->palette().color(QPalette::Window), QColor(224, 224, 224));
    QCOMPARE(thermo->palette().color(QPalette::WindowText), QColor(Qt::black));
    switchRoot(root.data(), "dark");
    QCOMPARE(thermo->palette().color(QPalette::Window),
             ThemeColors::color(ThemeColors::Surface, ThemeColors::Dark));

    // an authored background in Default mode stays
    thermo->setBackground(Qt::yellow);
    QCOMPARE(thermo->palette().color(QPalette::Window), QColor(Qt::yellow));
}

void TestThemeColors::thermoStaticModeIgnoresDarkRoot()
{
    QScopedPointer<QWidget> root(darkRoot());
    caThermo *thermo = new caThermo(root.data());
    root->show();
    QCoreApplication::processEvents();

    QCOMPARE(thermo->palette().color(QPalette::Window), QColor(224, 224, 224));
    QCOMPARE(thermo->palette().color(QPalette::ButtonText), QColor(Qt::black));
    QCOMPARE(thermo->palette().color(QPalette::WindowText), QColor(Qt::black));

    thermo->setColorMode(caThermo::Alarm_Static);
    thermo->setAlarmColors(MAJOR_ALARM);
    QCOMPARE(thermo->palette().color(QPalette::ButtonText), AL_RED);
    QCOMPARE(thermo->palette().color(QPalette::Window), QColor(224, 224, 224));
}

void TestThemeColors::cartesianPlotDefaultsFollowDarkRoot()
{
    QScopedPointer<QWidget> root(darkRoot());
    caCartesianPlot *plot = new caCartesianPlot(root.data());
    root->show();
    QCoreApplication::processEvents();

    const QColor darkFrame = ThemeColors::color(ThemeColors::PlotFrame, ThemeColors::Dark);
    const QColor darkScale = ThemeColors::color(ThemeColors::PlotScale, ThemeColors::Dark);
    QCOMPARE(plot->palette().color(QPalette::Window), darkFrame);
    QCOMPARE(plot->axisWidget(QwtPlot::xBottom)->palette().color(QPalette::Text), darkScale);
    // properties keep the light defaults, the canvas stays black
    QCOMPARE(plot->getForeground(), QColor(Qt::white));
    QCOMPARE(plot->getScaleColor(), QColor(Qt::black));
    QCOMPARE(plot->getBackground(), QColor(Qt::black));

    switchRoot(root.data(), "light");
    QCOMPARE(plot->palette().color(QPalette::Window), QColor(Qt::white));
    QCOMPARE(plot->axisWidget(QwtPlot::xBottom)->palette().color(QPalette::Text), QColor(Qt::black));

    // an authored scale colour is never themed
    switchRoot(root.data(), "dark");
    plot->setScaleColor(Qt::red);
    QCOMPARE(plot->axisWidget(QwtPlot::xBottom)->palette().color(QPalette::Text), QColor(Qt::red));

    // the default scale colour follows the own frame: an authored light frame keeps
    // dark scale text in a dark panel, a dark frame gets light text in a light panel
    plot->setScaleColor(Qt::black);
    plot->setForeground(QColor(218, 218, 218));
    QCOMPARE(plot->axisWidget(QwtPlot::xBottom)->palette().color(QPalette::Text), QColor(Qt::black));
    switchRoot(root.data(), "light");
    plot->setForeground(QColor(40, 40, 40));
    QCOMPARE(plot->axisWidget(QwtPlot::xBottom)->palette().color(QPalette::Text), darkScale);
    QVERIFY(!ThemeColors::adaptExcluded(plot, "background"));   // canvas and curves go through the applier's curve rule
    QVERIFY(!ThemeColors::adaptExcluded(plot, "foreground"));
    QColor black(Qt::black), red(Qt::red);
    const QColor canvas(42, 42, 42);
    ThemeColors::ensureContrast(black, canvas, ThemeColors::Dark);
    ThemeColors::ensureContrast(red, canvas, ThemeColors::Dark);
    QVERIFY(black.lightness() > 200);
    QCOMPARE(red, QColor(Qt::red));
    QCOMPARE(plot->palette().color(QPalette::Window), darkFrame);
}

namespace {
bool closeTo(const QColor &a, const QColor &b, int tolerance = 2)
{
    return qAbs(a.red() - b.red()) <= tolerance && qAbs(a.green() - b.green()) <= tolerance
           && qAbs(a.blue() - b.blue()) <= tolerance && a.alpha() == b.alpha();
}
ThemeColors::AdaptRole roleNamed(const QString &name)
{
    if (name == "background") return ThemeColors::BackgroundRole;
    if (name == "foreground") return ThemeColors::ForegroundRole;
    return ThemeColors::AnyRole;
}
}

void TestThemeColors::adaptMatrixLoadsAndIsIdentityInLight()
{
    QVERIFY(ThemeColors::adaptAvailable());
    const QColor wheat(245, 222, 179);
    QCOMPARE(ThemeColors::adapt(wheat, ThemeColors::Light), wheat);
    QCOMPARE(ThemeColors::adapt(wheat, ThemeColors::Light, ThemeColors::ForegroundRole), wheat);
    QCOMPARE(ThemeColors::adaptStyleSheet("QLabel { color: wheat; }", ThemeColors::Light), QString("QLabel { color: wheat; }"));
    // curated entry and ramp anchors
    QCOMPARE(ThemeColors::adapt(wheat, ThemeColors::Dark, ThemeColors::BackgroundRole), QColor(0x3a, 0x32, 0x26));
    QVERIFY(closeTo(ThemeColors::adapt(Qt::white, ThemeColors::Dark, ThemeColors::BackgroundRole), QColor(42, 42, 42)));
    QVERIFY(closeTo(ThemeColors::adapt(Qt::black, ThemeColors::Dark, ThemeColors::ForegroundRole), QColor(232, 232, 232)));
    QVERIFY(closeTo(ThemeColors::adapt(Qt::black, ThemeColors::Dark, ThemeColors::BackgroundRole), QColor(0, 0, 0)));
    // contrast of the curated dark surfaces against the alarm colours
    const ThemeColor *alarms[] = { &ThemeColors::AlarmNoAlarm, &ThemeColors::AlarmMinor,
                                   &ThemeColors::AlarmMajor, &ThemeColors::AlarmInvalid,
                                   &ThemeColors::AlarmDefault };
    const QColor surfaces[] = { ThemeColors::adapt(wheat, ThemeColors::Dark, ThemeColors::BackgroundRole),
                                ThemeColors::adapt(QColor(255, 248, 220), ThemeColors::Dark, ThemeColors::BackgroundRole),
                                ThemeColors::adapt(QColor(255, 255, 224), ThemeColors::Dark, ThemeColors::BackgroundRole),
                                ThemeColors::adapt(QColor(155, 255, 255), ThemeColors::Dark, ThemeColors::BackgroundRole) };
    for (const QColor &s : surfaces)
        for (const ThemeColor *a : alarms)
            QVERIFY2(ThemeColors::contrastRatio(s, ThemeColors::color(*a)) >= 3.0, qPrintable(s.name()));
}

void TestThemeColors::adaptMatchesPythonVectors()
{
    QFile file(":/theme/theme_adapt_vectors.json");
    QVERIFY2(file.open(QFile::ReadOnly), "vectors resource missing");
    const QJsonObject root = QJsonDocument::fromJson(file.readAll()).object();
    const QJsonArray colors = root.value("colors").toArray();
    QVERIFY(colors.size() > 20);
    for (int i = 0; i < colors.size(); ++i) {
        const QJsonObject v = colors.at(i).toObject();
        const QColor in(v.value("color").toString()), expected(v.value("expected").toString());
        const QColor out = ThemeColors::adapt(in, ThemeColors::Dark, roleNamed(v.value("role").toString()));
        QVERIFY2(closeTo(out, expected), qPrintable(QString("%1 %2: %3 != %4").arg(in.name(), v.value("role").toString(), out.name(), expected.name())));
    }
    const QJsonArray pairs = root.value("pairs").toArray();
    for (int i = 0; i < pairs.size(); ++i) {
        const QJsonObject v = pairs.at(i).toObject();
        QColor fg(v.value("foreground").toString()), bg(v.value("background").toString());
        ThemeColors::adaptPair(fg, bg, ThemeColors::Dark);
        QVERIFY2(closeTo(fg, QColor(v.value("expectedForeground").toString())), qPrintable(fg.name()));
        QVERIFY2(closeTo(bg, QColor(v.value("expectedBackground").toString())), qPrintable(bg.name()));
    }
    const QJsonObject sheet = root.value("styleSheet").toObject();
    QCOMPARE(ThemeColors::adaptStyleSheet(sheet.value("input").toString(), ThemeColors::Dark),
             sheet.value("expected").toString());
}

void TestThemeColors::adaptKeepsAlarmColoursAndTransparency()
{
    QVERIFY(ThemeColors::isSemanticColor(QColor(255, 0, 0)));
    QVERIFY(!ThemeColors::isSemanticColor(QColor(254, 0, 0)));
    QCOMPARE(ThemeColors::adapt(QColor(255, 0, 0), ThemeColors::Dark, ThemeColors::BackgroundRole), QColor(255, 0, 0));
    QCOMPARE(ThemeColors::adapt(QColor(0, 205, 0), ThemeColors::Dark), QColor(0, 205, 0));
    QCOMPARE(ThemeColors::adapt(QColor(255, 255, 0), ThemeColors::Dark, ThemeColors::ForegroundRole), QColor(255, 255, 0));
    const QColor transparent(255, 255, 255, 0);
    QCOMPARE(ThemeColors::adapt(transparent, ThemeColors::Dark), transparent);
    const QColor half(255, 255, 255, 128);
    QCOMPARE(ThemeColors::adapt(half, ThemeColors::Dark, ThemeColors::BackgroundRole).alpha(), 128);
    QCOMPARE(ThemeColors::adapt(QColor(), ThemeColors::Dark), QColor());
    // the pair rule flips dark text on a dark surface
    QColor fg = Qt::black, bg(245, 222, 179);
    ThemeColors::adaptPair(fg, bg, ThemeColors::Dark);
    QVERIFY(ThemeColors::contrastRatio(fg, bg) >= 3.0);
    QVERIFY(fg.lightness() > 200);
}

void TestThemeColors::adaptStyleSheetKeepsSyntax()
{
    const QString in = "QWidget#centralwidget {background: rgb(58, 94, 171);}\n"
                       "QTabBar::tab:selected { background: #fafafa; border: 2px solid #C4C4C3; }\n"
                       "caLabel { color: black; background-color: rgba(255, 255, 255, 0); font: bold; }";
    const QString out = ThemeColors::adaptStyleSheet(in, ThemeColors::Dark);
    QVERIFY(out.startsWith("QWidget#centralwidget {background: rgb("));   // selector untouched
    QVERIFY(out.contains("QTabBar::tab:selected { background: #2c2c2c; border: 2px solid #5a5a5a; }"));
    QVERIFY(out.contains("color: #e8e8e8; background-color: rgba(255, 255, 255, 0); font: bold;"));
    QCOMPARE(out.count('{'), in.count('{'));
    QCOMPARE(out.count(';'), in.count(';'));
    // role for widget properties
    QCOMPARE(ThemeColors::adaptRoleForProperty("background"), ThemeColors::BackgroundRole);
    QCOMPARE(ThemeColors::adaptRoleForProperty("foreground"), ThemeColors::ForegroundRole);
    QCOMPARE(ThemeColors::adaptRoleForProperty("scaleColor"), ThemeColors::ForegroundRole);
    QCOMPARE(ThemeColors::adaptRoleForProperty("lineColor"), ThemeColors::AnyRole);
    QCOMPARE(ThemeColors::adaptRoleForProperty("frameColor"), ThemeColors::ForegroundRole);
}

void TestThemeColors::thermoScaleTextFollowsOwnBackground()
{
    QScopedPointer<QWidget> root(darkRoot());
    caThermo *thermo = new caThermo(root.data());
    thermo->setColorMode(caThermo::Default);
    thermo->setBackground(QColor(245, 222, 179));          // authored light background
    root->show();
    QCoreApplication::processEvents();
    QCOMPARE(thermo->palette().color(QPalette::Window), QColor(245, 222, 179));
    QCOMPARE(thermo->palette().color(QPalette::WindowText), QColor(Qt::black));  // stays readable
    thermo->setBackground(QColor(224, 224, 224));          // light default -> themed dark
    QCOMPARE(thermo->palette().color(QPalette::Window), ThemeColors::color(ThemeColors::Surface, ThemeColors::Dark));
    QCOMPARE(thermo->palette().color(QPalette::WindowText), ThemeColors::color(ThemeColors::TextPrimary, ThemeColors::Dark));
}

void TestThemeColors::stripPlotScaleFollowsFrame()
{
    QScopedPointer<QWidget> root(darkRoot());
    caStripPlot *plot = new caStripPlot(root.data());
    settleRoot(root.data());
    root->show();
    QCoreApplication::processEvents();
    // defaults follow the scheme: dark frame, light scale text; properties keep the light values
    QCOMPARE(plot->palette().color(QPalette::Window), ThemeColors::color(ThemeColors::StripFrame, ThemeColors::Dark));
    QCOMPARE(plot->axisWidget(QwtPlot::xBottom)->palette().color(QPalette::Text),
             ThemeColors::color(ThemeColors::PlotScale, ThemeColors::Dark));
    QCOMPARE(plot->getForeground(), ThemeColors::color(ThemeColors::StripFrame));
    QCOMPARE(plot->getScaleColor(), QColor(Qt::black));
    QCOMPARE(plot->getBackground(), QColor(Qt::black));
    // an authored light frame keeps dark scale text in the dark panel
    plot->setForeground(QColor(218, 218, 218));
    QCOMPARE(plot->axisWidget(QwtPlot::xBottom)->palette().color(QPalette::Text), QColor(Qt::black));
    // an authored dark frame in a light panel gets light scale text
    switchRoot(root.data(), "light");
    plot->setForeground(QColor(40, 40, 40));
    QCOMPARE(plot->axisWidget(QwtPlot::xBottom)->palette().color(QPalette::Text),
             ThemeColors::color(ThemeColors::PlotScale, ThemeColors::Dark));
    // back to the defaults in the light panel: today's colours
    plot->setForeground(ThemeColors::color(ThemeColors::StripFrame));
    QCOMPARE(plot->palette().color(QPalette::Window), QColor(133, 190, 232));
    QCOMPARE(plot->axisWidget(QwtPlot::xBottom)->palette().color(QPalette::Text), QColor(Qt::black));
}
