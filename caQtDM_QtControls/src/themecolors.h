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

#ifndef THEMECOLORS_H
#define THEMECOLORS_H

#include <QColor>
#include <QPalette>
#include <QString>
#include <QtGlobal>
#include "qtcontrols_export.h"

class QWidget;

// Central colour definitions (level 1). Every colour carries a light and a
// dark value; alarm colours have both equal (checked at compile time).
// Widgets never use these names directly but define their own colour set
// (level 2) that points here.

struct ThemeRgb {
    int r, g, b, a;
};

constexpr bool operator==(const ThemeRgb &x, const ThemeRgb &y)
{
    return x.r == y.r && x.g == y.g && x.b == y.b && x.a == y.a;
}

struct ThemeColor {
    ThemeRgb light;
    ThemeRgb dark;
};

#define THEME_COLOR(name, lr, lg, lb, dr, dg, db) \
    constexpr ThemeColor name = {{lr, lg, lb, 255}, {dr, dg, db, 255}}
#define THEME_FIXED(name, r, g, b) THEME_COLOR(name, r, g, b, r, g, b)
#define THEME_ASSERT_FIXED(name) \
    static_assert(name.light == name.dark, #name " must be identical in light and dark mode")

namespace ThemeColors {

enum Scheme { Light = 0, Dark = 1 };

// --- alarm colours: fixed, never redefined per scheme -------------------
THEME_FIXED(AlarmNoAlarm,   0x00, 0xcd, 0x00);
THEME_FIXED(AlarmMinor,     0xff, 0xff, 0x00);
THEME_FIXED(AlarmMajor,     0xff, 0x00, 0x00);
THEME_FIXED(AlarmInvalid,   0xff, 0xff, 0xff);
THEME_FIXED(AlarmDefault,   0x88, 0x88, 0x88);
THEME_ASSERT_FIXED(AlarmNoAlarm);
THEME_ASSERT_FIXED(AlarmMinor);
THEME_ASSERT_FIXED(AlarmMajor);
THEME_ASSERT_FIXED(AlarmInvalid);
THEME_ASSERT_FIXED(AlarmDefault);

// --- named colours used by the alarm handler (light/dark handled there) --
THEME_FIXED(White,          0xff, 0xff, 0xff);
THEME_FIXED(Black,          0x00, 0x00, 0x00);
THEME_FIXED(LightGrey,      0xd3, 0xd3, 0xd3);
THEME_FIXED(Blue,           0x00, 0x00, 0xff);
THEME_FIXED(LightBlue,      0xad, 0xd8, 0xe6);
THEME_FIXED(SteelBlue,      0x46, 0x82, 0xb4);
THEME_FIXED(RoyalBlue,      0x41, 0x69, 0xe1);

// --- neutral surfaces and text (dark values are a first proposal; every
//     surface keeps a contrast >= 3:1 to the alarm colours, see unit test) --
THEME_COLOR(Surface,        224, 224, 224,    48,  48,  48);  // caThermo/caSlider background
THEME_COLOR(SurfaceSunken,  255, 248, 220,    42,  42,  42);  // fallback base (cornsilk)
THEME_COLOR(TextPrimary,      0,   0,   0,   232, 232, 232);  // default text/fill
THEME_COLOR(ArrowBackground,224, 224, 224,    53,  53,  53);  // caNumeric arrow buttons

// --- caThermo -------------------------------------------------------------
THEME_COLOR(ThermoFillDefault, 128,   0,   0,   200,  60,  60);  // Qt::darkRed / lighter red
THEME_COLOR(ThermoPipeDefault, 192, 192, 192,    38,  38,  38);  // Qt::lightGray / dark pipe

// --- plots (canvas and curves stay in both schemes) ----------------------
THEME_FIXED(PlotCanvas,       0,   0,   0);
THEME_COLOR(PlotFrame,      255, 255, 255,    40,  40,  40);
THEME_COLOR(PlotScale,        0,   0,   0,   232, 232, 232);
THEME_COLOR(PlotGrid,       160, 160, 164,   160, 160, 164);  // Qt::gray, on the black canvas
THEME_COLOR(PlotCanvasFrame,133, 190, 232,   133, 190, 232);
THEME_COLOR(PlotTracker,    255, 255, 255,   255, 255, 255);  // tracker text background
THEME_FIXED(PlotZoomBand,   255,   0,   0);                   // zoom rubber band
THEME_FIXED(PlotGlCanvas,   240, 230, 140);                   // "khaki"
THEME_FIXED(PlotNotConnected,255,255, 255);
THEME_FIXED(Curve1,         255, 255, 255);
THEME_FIXED(Curve2,         255,   0,   0);
THEME_FIXED(Curve3,         255, 255,   0);
THEME_FIXED(Curve4,           0,   0, 255);
THEME_FIXED(Curve5,           0, 255,   0);
THEME_FIXED(Curve6,         255,   0, 255);

// --- panel palette roles (light = legacy palette of PR #170) ---------------
THEME_COLOR(PaletteWindow,          255, 255, 255,    48,  48,  48);
THEME_COLOR(PaletteWindowText,        0,   0,   0,   232, 232, 232);
THEME_COLOR(PaletteBase,            255, 255, 255,    42,  42,  42);
THEME_COLOR(PaletteAlternateBase,   245, 245, 245,    48,  48,  48);
THEME_COLOR(PaletteText,              0,   0,   0,   232, 232, 232);
THEME_COLOR(PaletteButton,          239, 239, 239,    53,  53,  53);
THEME_COLOR(PaletteButtonText,        0,   0,   0,   232, 232, 232);
THEME_COLOR(PaletteLight,           255, 255, 255,    80,  80,  80);
THEME_COLOR(PaletteMidlight,        247, 247, 247,    65,  65,  65);
THEME_COLOR(PaletteMid,             184, 184, 184,    40,  40,  40);
THEME_COLOR(PaletteDark,            160, 160, 160,    30,  30,  30);
THEME_COLOR(PaletteShadow,          105, 105, 105,    10,  10,  10);
THEME_COLOR(PaletteToolTipBase,     255, 255, 255,    48,  48,  48);
THEME_COLOR(PaletteToolTipText,       0,   0,   0,   232, 232, 232);
THEME_COLOR(PaletteHighlight,        53, 132, 228,    42, 130, 218);
THEME_COLOR(PaletteHighlightedText, 255, 255, 255,   255, 255, 255);
THEME_COLOR(PaletteLink,              0,  70, 180,   100, 160, 255);
THEME_COLOR(PaletteLinkVisited,     255,   0, 255,   200, 120, 255);
THEME_COLOR(PaletteBrightText,      255, 255, 255,   255,  96,  96);
constexpr ThemeColor PalettePlaceholderText = {{0, 0, 0, 128}, {232, 232, 232, 128}};
THEME_COLOR(PaletteDisabledText,    190, 190, 190,   128, 128, 128);
THEME_COLOR(PaletteDisabledHighlight,145,145, 145,    80,  80,  80);

// dynamic properties set by the panel theme applier on a panel root
extern QTCON_EXPORT const char *const ThemeRootProperty;    // "caqtdmAppliedThemeRoot"
extern QTCON_EXPORT const char *const ThemeSchemeProperty;  // "caqtdmAppliedThemeScheme"

// header-only so that alarmdefs.h can alias AL_* without a link dependency
inline QColor color(const ThemeColor &c, Scheme s = Light)
{
    const ThemeRgb &v = (s == Dark) ? c.dark : c.light;
    return QColor(v.r, v.g, v.b, v.a);
}

// an author value equal to the light default follows the scheme, anything else stays
inline QColor themed(const QColor &value, const ThemeColor &c, Scheme s)
{
    return (s != Light && value == color(c, Light)) ? color(c, s) : value;
}

QTCON_EXPORT QString css(const ThemeColor &c, Scheme s);     // "rgb(r, g, b)" for stylesheets
QTCON_EXPORT Scheme schemeOf(const QPalette &palette);       // window lightness < 128 -> Dark
QTCON_EXPORT Scheme schemeOf(const QWidget *widget);         // nearest panel root, else Light
QTCON_EXPORT QPalette palette(Scheme s);                     // complete panel palette

}

#endif
