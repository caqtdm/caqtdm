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

#include <QApplication>
#include <QTest>
#include <QWidget>
#include <cmath>

#include "themecolors.h"
#include "alarmdefs.h"
#include "cathermo.h"
#include "cacartesianplot.h"
#include <qwt_scale_widget.h>

namespace {

QWidget *darkRoot()
{
    QWidget *root = new QWidget;
    root->setProperty(ThemeColors::ThemeRootProperty, true);
    root->setProperty(ThemeColors::ThemeSchemeProperty, "dark");
    root->setPalette(ThemeColors::palette(ThemeColors::Dark));
    return root;
}

void switchRoot(QWidget *root, const char *scheme)
{
    root->setProperty(ThemeColors::ThemeSchemeProperty, scheme);
    root->setPalette(ThemeColors::palette(QString(scheme) == "dark" ? ThemeColors::Dark
                                                                     : ThemeColors::Light));
    QCoreApplication::processEvents();
}

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
                                     &ThemeColors::PaletteButton };
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
    QCOMPARE(plot->palette().color(QPalette::Window), darkFrame);
}
