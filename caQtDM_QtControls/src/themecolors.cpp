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
#include <QApplication>
#include <QVariant>
#include <QFile>
#include <QHash>
#include <QSet>
#include <QVector>
#include <QPair>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <algorithm>
#include <cmath>

Q_LOGGING_CATEGORY(themeColorsLog, "caqtdm.widgets.themecolors")

namespace ThemeColors {

const char *const ThemeRootProperty = "caqtdmAppliedThemeRoot";
const char *const ThemeSchemeProperty = "caqtdmAppliedThemeScheme";
const char *const SelfStyledProperty = "caqtdmThemeSelfStyled";

void markSelfStyled(QWidget *widget, bool on)
{
    if (widget) widget->setProperty(SelfStyledProperty, on);
}

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

Scheme schemeFor(const QColor &background)
{
    return background.lightness() < 128 ? Dark : Light;
}

void defaultBaseAndText(QWidget *widget, QColor &base, QColor &text)
{
    if (widget && !widget->styleSheet().isEmpty()) widget->setStyleSheet(QString());
    const QPalette p = widget ? widget->palette() : QApplication::palette();
    base = p.color(QPalette::Base);
    text = p.color(QPalette::Text);
    if (!base.isValid()) base = color(SurfaceSunken);
    if (!text.isValid()) text = color(TextPrimary);
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

QPalette resolvedPalette(const QPalette &source)
{
    QPalette p;
    const QPalette::ColorGroup groups[] = { QPalette::Active, QPalette::Inactive, QPalette::Disabled };
    for (QPalette::ColorGroup g : groups) {
        for (int r = 0; r < int(QPalette::NColorRoles); ++r) {
            const QPalette::ColorRole role = QPalette::ColorRole(r);
            if (role == QPalette::NoRole) continue;
            p.setBrush(g, role, source.brush(g, role));
        }
    }
    return p;
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


// ---------------------------------------------------------------------------
// substitution matrix (themeadapt.json)

namespace {

typedef QVector<QPair<double, double> > Ramp;   // (lightness in, lightness out), 0..255
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
typedef float HslReal;                            // QColor::getHslF takes float* in Qt 6
#else
typedef qreal HslReal;
#endif

struct AdaptMatrix {
    bool loaded;
    double neutralSaturation;
    double saturationFactor;
    double minContrast;
    Ramp rampBackground, rampForeground, rampAny;
    QSet<QRgb> semantic;
    QHash<QRgb, QRgb> curated;
    QVector<QPair<QString, QString> > exclude;   // (class, property)
    AdaptMatrix() : loaded(false), neutralSaturation(0.12), saturationFactor(0.85), minContrast(3.0) {}
};

QRgb rgbKey(const QColor &c)
{
    return qRgb(c.red(), c.green(), c.blue());
}

Ramp readRamp(const QJsonValue &value)
{
    Ramp ramp;
    const QJsonArray points = value.toArray();
    for (int i = 0; i < points.size(); ++i) {
        const QJsonArray p = points.at(i).toArray();
        if (p.size() == 2) ramp.append(qMakePair(p.at(0).toDouble(), p.at(1).toDouble()));
    }
    std::sort(ramp.begin(), ramp.end());
    return ramp;
}

AdaptMatrix loadMatrix()
{
    AdaptMatrix m;
    QString path = QString::fromLocal8Bit(qgetenv("CAQTDM_THEME_ADAPT_FILE")).trimmed();
    if (path.isEmpty()) path = QLatin1String(":/src/themeadapt.json");
    QFile file(path);
    if (!file.open(QFile::ReadOnly)) {
        qCWarning(themeColorsLog) << "caQtDM -- theme adapt matrix" << path << "not readable";
        return m;
    }
    QJsonParseError error;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        qCWarning(themeColorsLog) << "caQtDM -- theme adapt matrix" << path << "invalid:" << error.errorString();
        return m;
    }
    const QJsonObject o = doc.object();
    m.neutralSaturation = o.value("neutralSaturation").toDouble(0.12);
    m.saturationFactor = o.value("saturationFactor").toDouble(0.85);
    m.minContrast = o.value("minContrast").toDouble(3.0);
    const QJsonObject ramps = o.value("ramps").toObject();
    m.rampBackground = readRamp(ramps.value("background"));
    m.rampForeground = readRamp(ramps.value("foreground"));
    m.rampAny = readRamp(ramps.value("any"));
    const QJsonArray semantic = o.value("semantic").toArray();
    for (int i = 0; i < semantic.size(); ++i) {
        const QColor c(semantic.at(i).toString());
        if (c.isValid()) m.semantic.insert(rgbKey(c));
    }
    const QJsonObject curated = o.value("curated").toObject();
    for (QJsonObject::const_iterator it = curated.constBegin(); it != curated.constEnd(); ++it) {
        const QColor from(it.key()), to(it.value().toString());
        if (from.isValid() && to.isValid()) m.curated.insert(rgbKey(from), rgbKey(to));
    }
    const QJsonArray exclude = o.value("exclude").toArray();
    for (int i = 0; i < exclude.size(); ++i) {
        const QJsonObject e = exclude.at(i).toObject();
        m.exclude.append(qMakePair(e.value("class").toString(), e.value("property").toString()));
    }
    m.loaded = !m.rampAny.isEmpty() && !m.rampBackground.isEmpty() && !m.rampForeground.isEmpty();
    qCDebug(themeColorsLog) << "caQtDM -- theme adapt matrix loaded from" << path
                            << "curated" << m.curated.size() << "excluded" << m.exclude.size();
    return m;
}

const AdaptMatrix &matrix()
{
    static const AdaptMatrix m = loadMatrix();
    return m;
}

double interpolate(const Ramp &ramp, double x)
{
    if (ramp.isEmpty()) return x;
    if (x <= ramp.first().first) return ramp.first().second;
    for (int i = 1; i < ramp.size(); ++i) {
        if (x <= ramp.at(i).first) {
            const double x0 = ramp.at(i - 1).first, y0 = ramp.at(i - 1).second;
            const double x1 = ramp.at(i).first, y1 = ramp.at(i).second;
            return x1 > x0 ? y0 + (y1 - y0) * (x - x0) / (x1 - x0) : y1;
        }
    }
    return ramp.last().second;
}

double channelLuminance(int v)
{
    const double c = v / 255.0;
    return c <= 0.03928 ? c / 12.92 : std::pow((c + 0.055) / 1.055, 2.4);
}

double luminance(const QColor &c)
{
    return 0.2126 * channelLuminance(c.red()) + 0.7152 * channelLuminance(c.green())
           + 0.0722 * channelLuminance(c.blue());
}

QColor withLightness(const QColor &c, double lightness)
{
    HslReal h, s, l, a;
    c.getHslF(&h, &s, &l, &a);
    return QColor::fromHslF(h < 0 ? 0 : h, s, lightness, a);
}

QString cssColor(const QColor &c, bool rgbNotation, bool withAlpha)
{
    if (rgbNotation) {
        if (withAlpha)
            return QString("rgba(%1, %2, %3, %4)").arg(c.red()).arg(c.green()).arg(c.blue()).arg(c.alpha());
        return QString("rgb(%1, %2, %3)").arg(c.red()).arg(c.green()).arg(c.blue());
    }
    return withAlpha ? c.name(QColor::HexArgb) : c.name(QColor::HexRgb);
}

AdaptRole roleForCssProperty(const QString &property)
{
    const QString p = property.toLower();
    if (p.contains("background")) return BackgroundRole;
    if (p == "color" || p == "selection-color" || p == "placeholder-text-color") return ForegroundRole;
    return AnyRole;
}

}

bool adaptAvailable()
{
    return matrix().loaded;
}

bool isSemanticColor(const QColor &c)
{
    return c.isValid() && matrix().semantic.contains(rgbKey(c));
}

double contrastRatio(const QColor &a, const QColor &b)
{
    const double la = luminance(a), lb = luminance(b);
    return (qMax(la, lb) + 0.05) / (qMin(la, lb) + 0.05);
}

QColor adapt(const QColor &authored, Scheme s, AdaptRole role)
{
    const AdaptMatrix &m = matrix();
    if (s != Dark || !m.loaded || !authored.isValid() || authored.alpha() == 0) return authored;
    if (m.semantic.contains(rgbKey(authored))) return authored;
    if (role != ForegroundRole) {
        QHash<QRgb, QRgb>::const_iterator it = m.curated.constFind(rgbKey(authored));
        if (it != m.curated.constEnd()) {
            QColor curated(it.value());
            curated.setAlpha(authored.alpha());
            return curated;
        }
    }
    HslReal h, sat, l, a;
    authored.getHslF(&h, &sat, &l, &a);
    const Ramp &ramp = role == BackgroundRole ? m.rampBackground
                     : role == ForegroundRole ? m.rampForeground : m.rampAny;
    const double lightness = qBound(0.0, interpolate(ramp, l * 255.0) / 255.0, 1.0);
    double saturation = sat;
    if (role != ForegroundRole && sat >= m.neutralSaturation) {
        // surfaces: keep the chroma (max - min) rather than the HSL saturation,
        // which is near 1 for pastel tints and would turn them into deep colours
        const int hi = qMax(authored.red(), qMax(authored.green(), authored.blue()));
        const int lo = qMin(authored.red(), qMin(authored.green(), authored.blue()));
        const double chroma = (hi - lo) / 255.0 * m.saturationFactor;
        const double range = 1.0 - qAbs(2.0 * lightness - 1.0);
        saturation = qMin(sat * m.saturationFactor, range > 0.0 ? chroma / range : 0.0);
    }
    return QColor::fromHslF(h < 0 ? 0 : h, saturation, lightness, a);
}

void adaptPair(QColor &foreground, QColor &background, Scheme s)
{
    if (s != Dark || !matrix().loaded) return;
    foreground = adapt(foreground, s, ForegroundRole);
    background = adapt(background, s, BackgroundRole);
    // a transparent background shows the panel: read against the window colour
    const QColor against = background.alpha() == 0 ? color(PaletteWindow, s) : background;
    ensureContrast(foreground, against, s);
}

void ensureContrast(QColor &foreground, const QColor &background, Scheme s)
{
    if (s != Dark || !matrix().loaded) return;
    if (!foreground.isValid() || !background.isValid() || background.alpha() == 0) return;
    if (isSemanticColor(foreground)) return;                 // alarm colours are never changed
    if (contrastRatio(foreground, background) >= matrix().minContrast) return;
    // light or dark text, whichever reads better; hue and saturation stay
    const QColor light = withLightness(foreground, 232.0 / 255.0);
    const QColor dark = withLightness(foreground, 24.0 / 255.0);
    foreground = contrastRatio(light, background) >= contrastRatio(dark, background) ? light : dark;
}

QString adaptStyleSheet(const QString &styleSheet, Scheme s)
{
    if (s != Dark || !matrix().loaded || styleSheet.isEmpty()) return styleSheet;
    static const QRegularExpression declaration("([A-Za-z-]+)\\s*:\\s*([^;{}]*)");
    static const QRegularExpression token(
        "rgba?\\(\\s*\\d+\\s*,\\s*\\d+\\s*,\\s*\\d+(?:\\s*,\\s*\\d+)?\\s*\\)|#[0-9A-Fa-f]{8}\\b|#[0-9A-Fa-f]{6}\\b|#[0-9A-Fa-f]{3}\\b|\\b[A-Za-z]+\\b");
    QString result;
    int last = 0;
    QRegularExpressionMatchIterator declarations = declaration.globalMatch(styleSheet);
    while (declarations.hasNext()) {
        const QRegularExpressionMatch d = declarations.next();
        const AdaptRole role = roleForCssProperty(d.captured(1));
        const QString value = d.captured(2);
        result += styleSheet.mid(last, d.capturedStart(2) - last);
        QString adapted;
        int valueLast = 0;
        QRegularExpressionMatchIterator tokens = token.globalMatch(value);
        while (tokens.hasNext()) {
            const QRegularExpressionMatch t = tokens.next();
            const QString text = t.captured(0);
            QString replacement = text;
            if (text.startsWith("rgb")) {
                static const QRegularExpression numbers("\\d+");
                QVector<int> v;
                QRegularExpressionMatchIterator n = numbers.globalMatch(text);
                while (n.hasNext()) v.append(n.next().captured(0).toInt());
                if (v.size() >= 3) {
                    const QColor c(v.at(0), v.at(1), v.at(2), v.size() > 3 ? v.at(3) : 255);
                    replacement = cssColor(adapt(c, s, role), true, v.size() > 3);
                }
            } else if (text.startsWith('#')) {
                QColor c;
                if (text.size() == 9) {                       // #AARRGGBB (Qt stylesheet order)
                    c = QColor(text.mid(3));
                    c.setAlpha(text.mid(1, 2).toInt(Q_NULLPTR, 16));
                } else {
                    c = QColor(text);
                }
                if (c.isValid()) replacement = cssColor(adapt(c, s, role), false, text.size() == 9);
            } else {
                const QColor c(text);                          // named colour, e.g. wheat
                if (c.isValid() && c.alpha() != 0) replacement = cssColor(adapt(c, s, role), false, false);
            }
            adapted += value.mid(valueLast, t.capturedStart(0) - valueLast) + replacement;
            valueLast = t.capturedEnd(0);
        }
        adapted += value.mid(valueLast);
        result += adapted;
        last = d.capturedEnd(2);
    }
    result += styleSheet.mid(last);
    return result;
}

AdaptRole adaptRoleForProperty(const QString &propertyName)
{
    const QString p = propertyName.toLower();
    if (p.contains("background")) return BackgroundRole;
    if (p.contains("foreground") || p.contains("textcolor") || p.contains("scalevaluecolor")
        || p.contains("scalecolor") || p.contains("framecolor") || p == "color") return ForegroundRole;
    return AnyRole;
}

bool adaptExcluded(const QObject *widget, const QString &propertyName)
{
    if (!widget) return false;
    const AdaptMatrix &m = matrix();
    for (int i = 0; i < m.exclude.size(); ++i) {
        const QString &className = m.exclude.at(i).first;
        const QString &property = m.exclude.at(i).second;
        if ((className.isEmpty() || widget->inherits(className.toLatin1().constData())) &&
            (property.isEmpty() || property == propertyName)) return true;
    }
    return false;
}

}
