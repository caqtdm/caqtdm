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

// --- text widgets ----------------------------------------------------------
THEME_FIXED(FocusFrame,     255,   0,   0);                   // focus border of entry widgets

// --- containers ------------------------------------------------------------
THEME_FIXED(FrameLine,      160, 160, 164);                   // Qt::gray, caFrame/caInclude default

// --- buttons ---------------------------------------------------------------
THEME_FIXED(StaticButton,   232, 232, 232);                   // caMessageButton default (Static)
THEME_FIXED(DisabledText,   128, 128, 128);                   // "grey" text of disabled buttons

// --- caDoubleTabWidget (generated stylesheet) ------------------------------
THEME_COLOR(TabPage,           255, 255, 200,    50,  50,  36);  // page background
THEME_COLOR(TabButtonBorder,   143, 143, 145,   110, 110, 112);  // #8f8f91
THEME_COLOR(TabGradientTop,    246, 247, 250,    52,  52,  56);  // #f6f7fa
THEME_COLOR(TabGradientBottom, 218, 219, 222,    46,  46,  50);  // #dadbde
THEME_FIXED(TabChecked,        255,   0, 255);                   // selected page marker
THEME_FIXED(TabDefaultBorder,    0,   0, 128);                   // "navy"

// --- gauges / clock ----------------------------------------------------------
THEME_FIXED(GaugeMarker,      0,   0, 128);                   // value marker (Qt::darkBlue)
THEME_COLOR(ByteClearBit,    160, 160, 164,    64,  64,  68);  // caByteController cleared bit (Qt::gray)
THEME_COLOR(ClockBase,       160, 160, 164,    64,  64,  68);  // dial base (Qt::gray)
THEME_COLOR(ClockScaleText,  253, 253, 255,   232, 232, 232);  // digits (light = today's value)
THEME_COLOR(StripFrame,      133, 190, 232,    36,  52,  66);  // caStripPlot frame default

// --- tables (built-in Qt6 header style) ------------------------------------
THEME_COLOR(TableAlternateBase,233, 231, 227,    48,  48,  48);
THEME_COLOR(TableHeader,       240, 240, 240,    47,  47,  47);  // #f0f0f0
THEME_COLOR(TableGrid,         216, 216, 216,    58,  58,  58);  // #d8d8d8 / #3a3a3a

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
THEME_FIXED(CurveCyan,        0, 255, 255);                   // caStripPlot curve 4

// --- panel palette roles (light = legacy palette of PR #170) ---------------
THEME_COLOR(PaletteWindow,          239, 239, 239,    48,  48,  48);   // light: desktop grey, painted on dark desktops
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
constexpr ThemeColor PaletteDisabledText = {{0, 0, 0, 96}, {255, 255, 255, 112}};  // translucent: reads on any surface
THEME_COLOR(PaletteDisabledHighlight,145,145, 145,    80,  80,  80);

// dynamic properties set by the panel theme applier on a panel root
extern QTCON_EXPORT const char *const ThemeRootProperty;    // "caqtdmAppliedThemeRoot"
extern QTCON_EXPORT const char *const ThemeSchemeProperty;  // "caqtdmAppliedThemeScheme"
// set by widgets on themselves or on internal children whose colours they
// regenerate: the substitution matrix leaves such a widget alone
extern QTCON_EXPORT const char *const SelfStyledProperty;   // "caqtdmThemeSelfStyled"
QTCON_EXPORT void markSelfStyled(QWidget *widget, bool on = true);

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
QTCON_EXPORT Scheme schemeFor(const QColor &background);     // lightness < 128 -> Dark
// Base/Text as Qt resolves them for the widget without its own stylesheet (ancestor
// sheets propagate into the palette); clears the own sheet, caller regenerates it.
// Fallbacks: SurfaceSunken / TextPrimary
QTCON_EXPORT void defaultBaseAndText(QWidget *widget, QColor &base, QColor &text);
QTCON_EXPORT QPalette palette(Scheme s);                     // complete panel palette
// copy of a (possibly inherited) palette with every role resolved, e.g. for popup windows
QTCON_EXPORT QPalette resolvedPalette(const QPalette &source);

// --- substitution matrix for authored panel colours (themeadapt.json) -----
// Light scheme: identity. Dark scheme: neutral colours follow a lightness ramp,
// saturated ones a curated table or the ramp with reduced saturation; the
// alarm colours red/green/yellow are never changed.
enum AdaptRole { AnyRole = 0, BackgroundRole, ForegroundRole };
QTCON_EXPORT bool adaptAvailable();                                   // matrix loaded
QTCON_EXPORT bool isSemanticColor(const QColor &c);
QTCON_EXPORT QColor adapt(const QColor &authored, Scheme s, AdaptRole r = AnyRole);
QTCON_EXPORT void adaptPair(QColor &foreground, QColor &background, Scheme s);
// light or dark version of a foreground, whichever reads on the background (>= 3:1)
QTCON_EXPORT void ensureContrast(QColor &foreground, const QColor &background, Scheme s);
QTCON_EXPORT QString adaptStyleSheet(const QString &styleSheet, Scheme s);
QTCON_EXPORT AdaptRole adaptRoleForProperty(const QString &propertyName);
QTCON_EXPORT bool adaptExcluded(const QObject *widget, const QString &propertyName);
QTCON_EXPORT double contrastRatio(const QColor &a, const QColor &b);  // WCAG
QTCON_EXPORT double relativeLuminance(const QColor &c);               // WCAG relative luminance 0..1
QTCON_EXPORT double darkSurfaceLuminance();                           // matrix: below this a surface is dark-built
QTCON_EXPORT double signalChroma();                                   // matrix: from this chroma a surface is a signal colour

}

#endif
