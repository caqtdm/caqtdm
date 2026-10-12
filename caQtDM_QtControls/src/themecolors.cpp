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

#include "themecolors.h"
#include "qtcontrols_global.h"

#include <QWidget>
#include <QVariant>

Q_LOGGING_CATEGORY(themeColorsLog, "caqtdm.widgets.themecolors")

namespace ThemeColors {

const char *const ThemeRootProperty = "caqtdmAppliedThemeRoot";
const char *const ThemeSchemeProperty = "caqtdmAppliedThemeScheme";

QString css(const ThemeColor &c, Scheme s)
{
    const ThemeRgb &v = (s == Dark) ? c.dark : c.light;
    if (v.a == 255)
        return QString("rgb(%1, %2, %3)").arg(v.r).arg(v.g).arg(v.b);
    return QString("rgba(%1, %2, %3, %4)").arg(v.r).arg(v.g).arg(v.b).arg(v.a);
}

Scheme schemeOf(const QPalette &palette)
{
    return palette.color(QPalette::Window).lightness() < 128 ? Dark : Light;
}

Scheme schemeOf(const QWidget *widget)
{
    for (const QWidget *w = widget; w; w = w->parentWidget()) {
        if (!w->property(ThemeRootProperty).toBool()) continue;
        const QString scheme = w->property(ThemeSchemeProperty).toString();
        return scheme.compare(QLatin1String("dark"), Qt::CaseInsensitive) == 0 ? Dark : Light;
    }
    return Light;  // no panel root (designer, tests): today's colours
}

QPalette palette(Scheme s)
{
    QPalette p;
    const QPalette::ColorGroup groups[] = {
        QPalette::Active, QPalette::Inactive, QPalette::Disabled
    };
    for (QPalette::ColorGroup g : groups) {
        p.setColor(g, QPalette::Window, color(PaletteWindow, s));
        p.setColor(g, QPalette::WindowText, color(PaletteWindowText, s));
        p.setColor(g, QPalette::Base, color(PaletteBase, s));
        p.setColor(g, QPalette::AlternateBase, color(PaletteAlternateBase, s));
        p.setColor(g, QPalette::Text, color(PaletteText, s));
        p.setColor(g, QPalette::Button, color(PaletteButton, s));
        p.setColor(g, QPalette::ButtonText, color(PaletteButtonText, s));
        p.setColor(g, QPalette::Light, color(PaletteLight, s));
        p.setColor(g, QPalette::Midlight, color(PaletteMidlight, s));
        p.setColor(g, QPalette::Mid, color(PaletteMid, s));
        p.setColor(g, QPalette::Dark, color(PaletteDark, s));
        p.setColor(g, QPalette::Shadow, color(PaletteShadow, s));
        p.setColor(g, QPalette::ToolTipBase, color(PaletteToolTipBase, s));
        p.setColor(g, QPalette::ToolTipText, color(PaletteToolTipText, s));
        p.setColor(g, QPalette::Highlight, color(PaletteHighlight, s));
        p.setColor(g, QPalette::HighlightedText, color(PaletteHighlightedText, s));
        p.setColor(g, QPalette::Link, color(PaletteLink, s));
        p.setColor(g, QPalette::LinkVisited, color(PaletteLinkVisited, s));
        p.setColor(g, QPalette::BrightText, color(PaletteBrightText, s));
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
        p.setColor(g, QPalette::PlaceholderText, color(PalettePlaceholderText, s));
#endif
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
        p.setColor(g, QPalette::Accent, color(PaletteHighlight, s));
#endif
    }
    // disabled widgets stay greyed out as with the application palette
    p.setColor(QPalette::Disabled, QPalette::WindowText, color(PaletteDisabledText, s));
    p.setColor(QPalette::Disabled, QPalette::Text, color(PaletteDisabledText, s));
    p.setColor(QPalette::Disabled, QPalette::ButtonText, color(PaletteDisabledText, s));
    p.setColor(QPalette::Disabled, QPalette::Highlight, color(PaletteDisabledHighlight, s));
    qCDebug(themeColorsLog) << "panel palette for scheme" << (s == Dark ? "dark" : "light");
    return p;
}

}
