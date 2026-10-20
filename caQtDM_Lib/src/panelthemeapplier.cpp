#include "panelthemeapplier.h"

#include <QApplication>
#include <QFile>
#include <QLabel>
#include <QMainWindow>
#include <QMetaProperty>
#include <QPalette>
#include <QMetaEnum>
#include <QXmlStreamReader>
#include <QWidget>

#include "caQtDM_Lib_global.h"
#include "cawavetable.h"
#include "themecolors.h"

Q_LOGGING_CATEGORY(panelThemeLog, "caqtdm.lib.paneltheme")
Q_LOGGING_CATEGORY(panelThemeAdaptLog, "caqtdm.lib.panelthemeadapt")

namespace {
const char ThemeModeProperty[] = "caqtdmThemeMode";
const char ThemeEnvironment[] = "CAQTDM_THEME";
const char ThemeModeOverrideEnvironment[] = "CAQTDM_PANEL_THEME_MODE";  // alias
const char ThemeRootProperty[] = "caqtdmAppliedThemeRoot";
const char LegacyLightProperty[] = "caqtdmAppliedLegacyLight";
const char AuthoredStyleSheetProperty[] = "caqtdmAuthoredStyleSheet";
const char AuthoredAutoFillProperty[] = "caqtdmAuthoredAutoFill";
const char ThemeAdaptProperty[] = "caqtdmThemeAdapt";          // panel opt-out
const char ThemeAdaptEnvironment[] = "CAQTDM_THEME_ADAPT";      // global off switch
const char AuthoredPrefix[] = "caqtdmAuthored.";              // saved original values

struct PanelStyleSheet {
    QString contents;
    QString sourceFile;
    QString darkContents;
    QString darkSourceFile;
    bool reloadable = false;
};

bool reloadStyleSheetFile(const QString &sourceFile, QString &contents)
{
    QFile file(sourceFile);
    if (!file.open(QFile::ReadOnly)) return false;
    contents = QLatin1String(file.readAll());
    qCInfo(panelThemeLog) << "caQtDM -- reloaded panel stylesheet" << sourceFile;
    return true;
}

PanelStyleSheet &panelStyleSheet()
{
    static PanelStyleSheet styleSheet;
    return styleSheet;
}

QString resolvedPanelStyleSheet(ThemeColors::Scheme scheme)
{
    PanelStyleSheet &styleSheet = panelStyleSheet();
    const QString reloadMode = QString::fromLocal8Bit(qgetenv("CAQTDM_STYLESHEET_RELOAD"));
    if (styleSheet.reloadable && reloadMode.contains("file", Qt::CaseInsensitive)) {
        if (!styleSheet.sourceFile.isEmpty())
            reloadStyleSheetFile(styleSheet.sourceFile, styleSheet.contents);
        if (!styleSheet.darkSourceFile.isEmpty())
            reloadStyleSheetFile(styleSheet.darkSourceFile, styleSheet.darkContents);
    }
    // the dark variant only exists when the author provided it (see resolveMode)
    const bool useDark = (scheme == ThemeColors::Dark) && !styleSheet.darkContents.isEmpty();
    const QString &contents = useDark ? styleSheet.darkContents : styleSheet.contents;
    if (reloadMode.contains("print", Qt::CaseInsensitive) && !contents.isEmpty())
        qCInfo(panelThemeLog) << "caQtDM -- panel stylesheet data:" << contents;
    return contents;
}

void applyPanelStyleSheet(QWidget *root, ThemeColors::Scheme scheme, bool adapt)
{
    if (!root) return;
    if (!root->property(AuthoredStyleSheetProperty).isValid())
        root->setProperty(AuthoredStyleSheetProperty, root->styleSheet());

    QString authored = root->property(AuthoredStyleSheetProperty).toString();
    if (adapt) authored = ThemeColors::adaptStyleSheet(authored, scheme);
    const QString external = resolvedPanelStyleSheet(scheme);
    root->setStyleSheet(authored.isEmpty() ? external :
                        external.isEmpty() ? authored : authored + '\n' + external);
}

const QPalette &legacyLightPalette()
{
    static const QPalette palette = ThemeColors::palette(ThemeColors::Light);
    return palette;
}

const QPalette &darkPalette()
{
    static const QPalette palette = ThemeColors::palette(ThemeColors::Dark);
    return palette;
}

QPalette resolvedApplicationPalette()
{
    const QPalette applicationPalette = QApplication::palette();
    QPalette palette;
    const QPalette::ColorGroup groups[] = {
        QPalette::Active, QPalette::Inactive, QPalette::Disabled
    };
    for (QPalette::ColorGroup colorGroup : groups) {
        for (int role = QPalette::WindowText; role < QPalette::NColorRoles; ++role) {
            if (role == QPalette::NoRole) continue;
            const QPalette::ColorRole colorRole = static_cast<QPalette::ColorRole>(role);
            palette.setColor(colorGroup, colorRole,
                             applicationPalette.color(colorGroup, colorRole));
        }
    }
    return palette;
}

QPalette paletteFor(PanelThemeApplier::Mode mode)
{
    switch (mode) {
    case PanelThemeApplier::Dark: return darkPalette();
    case PanelThemeApplier::System: return resolvedApplicationPalette();
    default: return legacyLightPalette();
    }
}

ThemeColors::Scheme schemeFor(PanelThemeApplier::Mode mode)
{
    switch (mode) {
    case PanelThemeApplier::Dark: return ThemeColors::Dark;
    case PanelThemeApplier::System: return ThemeColors::schemeOf(QApplication::palette());
    default: return ThemeColors::Light;
    }
}

QString environmentMode()
{
    QString mode = QString::fromLocal8Bit(qgetenv(ThemeEnvironment)).trimmed();
    if (mode.isEmpty())
        mode = QString::fromLocal8Bit(qgetenv(ThemeModeOverrideEnvironment)).trimmed();
    return mode;
}

QWidget *contentRoot(QWidget *root)
{
    if (QMainWindow *window = qobject_cast<QMainWindow *>(root))
        return window->centralWidget();
    return root;
}

void logPalette(const char *label, const QWidget *widget)
{
    if (!widget) return;
    const QPalette palette = widget->palette();
    qCDebug(panelThemeLog).nospace() << label << ' ' << widget
        << " parent=" << widget->parentWidget()
        << " paletteSet=" << widget->testAttribute(Qt::WA_SetPalette)
        << " styleSheet=" << !widget->styleSheet().isEmpty()
        << " text=" << palette.color(QPalette::Active, QPalette::Text).name()
        << " windowText=" << palette.color(QPalette::Active, QPalette::WindowText).name()
        << " base=" << palette.color(QPalette::Active, QPalette::Base).name()
        << " buttonText=" << palette.color(QPalette::Active, QPalette::ButtonText).name();
}
}

void PanelThemeApplier::setPanelStyleSheet(const QString &styleSheet, const QString &sourceFile,
                                           bool reloadable)
{
    PanelStyleSheet &configured = panelStyleSheet();
    configured.contents = styleSheet;
    configured.sourceFile = sourceFile;
    configured.reloadable = reloadable;
    configured.darkContents.clear();
    configured.darkSourceFile.clear();
}

void PanelThemeApplier::setPanelDarkStyleSheet(const QString &styleSheet, const QString &sourceFile)
{
    PanelStyleSheet &configured = panelStyleSheet();
    configured.darkContents = styleSheet;
    configured.darkSourceFile = sourceFile;
}

bool PanelThemeApplier::hasPanelStyleSheet()
{
    return !panelStyleSheet().contents.isEmpty();
}

bool PanelThemeApplier::hasPanelDarkStyleSheet()
{
    return !panelStyleSheet().darkContents.isEmpty();
}

PanelThemeApplier::Mode PanelThemeApplier::parseMode(const QString &text, bool *ok)
{
    const QString mode = text.trimmed();
    if (ok) *ok = true;
    if (mode.compare(QLatin1String("legacy"), Qt::CaseInsensitive) == 0 ||
        mode.compare(QLatin1String("legacylight"), Qt::CaseInsensitive) == 0) return Legacy;
    if (mode.compare(QLatin1String("system"), Qt::CaseInsensitive) == 0) return System;
    if (mode.compare(QLatin1String("dark"), Qt::CaseInsensitive) == 0) return Dark;
    if (ok) *ok = false;
    return Legacy;
}

QString PanelThemeApplier::modeName(Mode mode)
{
    switch (mode) {
    case System: return QLatin1String("system");
    case Dark: return QLatin1String("dark");
    default: return QLatin1String("legacy");
    }
}

PanelThemeApplier::Mode PanelThemeApplier::resolveMode(Mode panelMode)
{
    Mode mode = panelMode;
    const QString overrideMode = environmentMode();
    if (!overrideMode.isEmpty()) {
        bool ok = false;
        const Mode envMode = parseMode(overrideMode, &ok);
        if (ok) {
            mode = envMode;
        } else {
            static bool warned = false;
            if (!warned) qCWarning(panelThemeLog) << "caQtDM -- unknown theme mode" << overrideMode
                                                  << "(use legacy, system or dark)";
            warned = true;
        }
    }
    // a stylesheet file without a dark variant is written for the light palette and forces it
    if (mode != Legacy && hasPanelStyleSheet() && !hasPanelDarkStyleSheet()) {
        static bool informed = false;
        if (!informed) qCInfo(panelThemeLog) << "caQtDM -- panel stylesheet without dark variant"
                                             << "configured, theme mode" << modeName(mode)
                                             << "replaced by legacy";
        informed = true;
        mode = Legacy;
    }
    return mode;
}

PanelThemeApplier::Mode PanelThemeApplier::modeOf(const QWidget *root)
{
    if (!root) return Legacy;
    return resolveMode(parseMode(root->property(ThemeModeProperty).toString()));
}

bool PanelThemeApplier::usesLegacyLightTheme(const QWidget *root)
{
    if (!root) return false;
    return modeOf(root) == Legacy;
}

namespace { bool hasThemeRootAncestor(const QWidget *root); }

void PanelThemeApplier::apply(QWidget *root)
{
    if (!root) return;
    const Mode mode = modeOf(root);
    const ThemeColors::Scheme scheme = schemeFor(mode);
    adaptAuthoredColors(root);
    applyPanelStyleSheet(root, scheme, adaptActive(root));
    QWidget *const target = contentRoot(root);
    if (!target) return;
    const bool legacyLight = (mode == Legacy);
    // properties first: children read the scheme while handling PaletteChange
    target->setProperty(LegacyLightProperty, legacyLight);
    target->setProperty(ThemeRootProperty, true);
    target->setProperty(ThemeColors::ThemeSchemeProperty, scheme == ThemeColors::Dark ? "dark" : "light");
    seed(target, mode);
    propagateThroughOwnPalettes(target);
    // a panel palette that differs from the application palette must paint
    // the panel itself: dark panels on a light desktop, legacy panels on a dark one
    if (!target->property(AuthoredAutoFillProperty).isValid())
        target->setProperty(AuthoredAutoFillProperty, target->autoFillBackground());
    const bool applicationIsDark = ThemeColors::schemeOf(QApplication::palette()) == ThemeColors::Dark;
    // only the top-level panel paints itself; an include root stays transparent
    // and shows the surface of the panel it sits in
    const bool paintPanel = !hasThemeRootAncestor(root)
        && ((mode == Dark && !applicationIsDark) || (mode == Legacy && applicationIsDark));
    target->setAutoFillBackground(paintPanel || target->property(AuthoredAutoFillProperty).toBool());
    // includes are applied before their panel: take their auto-fill back here
    for (QWidget *nested : target->findChildren<QWidget *>()) {
        if (nested == target || !nested->property(ThemeRootProperty).toBool()) continue;
        nested->setAutoFillBackground(nested->property(AuthoredAutoFillProperty).toBool());
    }
    for (caWaveTable *table : target->findChildren<caWaveTable *>()) {
        bool belongsToNestedPanel = false;
        for (QWidget *parent = table->parentWidget(); parent && parent != target;
             parent = parent->parentWidget()) {
            if (parent->property(ThemeRootProperty).toBool()) {
                belongsToNestedPanel = true;
                break;
            }
        }
        if (!belongsToNestedPanel)
            table->setPanelThemePalette(legacyLight);
    }
    if (qobject_cast<QMainWindow *>(root))
        root->setPalette(QPalette());
    target->update();
    qCDebug(panelThemeLog) << "applied" << modeName(mode) << "palette to content root" << target;
    if (!panelThemeLog().isDebugEnabled()) return;

    logPalette("panel root", root);
    logPalette("panel content root", target);
    if (QWidget *sample = target->findChild<QLabel *>()) logPalette("sample QLabel", sample);
}

// Qt propagates a palette change only through widgets whose own palette changed
// as well: a child with an own, unchanged palette (authored, stylesheet or widget
// code) keeps its not yet resolved children on the application palette. Toggling
// the own palette of every such widget resolves everything below it.
void PanelThemeApplier::propagateThroughOwnPalettes(QWidget *contentRoot)
{
    if (!contentRoot) return;
    int toggled = 0;
    for (QWidget *w : contentRoot->findChildren<QWidget *>()) {
        if (!w->testAttribute(Qt::WA_SetPalette)) continue;
        const QPalette own = w->palette();
        w->setPalette(QPalette());
        w->setPalette(own);
        ++toggled;
    }
    qCDebug(panelThemeLog) << "propagated the panel palette through" << toggled << "widgets with an own palette";
}

void PanelThemeApplier::seed(QWidget *contentRoot, Mode mode)
{
    if (!contentRoot) return;
    // with stylesheet propagation Qt resolves the palette of children created after
    // an earlier seed only when the parent's palette changes: reset first, so that
    // apply() after loading reaches them even with the same palette
    const bool reset = contentRoot->testAttribute(Qt::WA_SetPalette);
    const QPalette panel = paletteFor(mode);
    if (reset) {
        // the intermediate palette must differ from the application palette as well,
        // otherwise Qt stops propagating where a child already shows that value
        QPalette bump = panel;
        const QColor text = panel.color(QPalette::WindowText);
        bump.setColor(QPalette::WindowText, QColor(text.red() ^ 1, text.green() ^ 1, text.blue() ^ 1));
        contentRoot->setPalette(bump);
    }
    contentRoot->setPalette(panel);
    qCDebug(panelThemeLog) << "seeded" << modeName(mode) << "palette on" << contentRoot << "reset" << reset
                           << "children" << contentRoot->findChildren<QWidget *>().size();
}

// ---------------------------------------------------------------------------
// substitution matrix for authored colours

namespace {

// the content root of this panel carries ThemeRootProperty itself; only roots
// below it belong to nested includes
bool insideNestedRoot(const QWidget *widget, const QWidget *root, const QWidget *content)
{
    for (const QWidget *w = widget->parentWidget(); w && w != root && w != content; w = w->parentWidget())
        if (w->property(ThemeRootProperty).toBool()) return true;
    return false;
}

// a nested include root (its content carries ThemeRootProperty) is applied on its
// own: the enclosing panel must not record its adapted stylesheet as authored
bool isNestedRoot(QWidget *widget)
{
    const QWidget *content = contentRoot(widget);
    return content && content->property(ThemeRootProperty).toBool();
}

bool hasColorProperty(const QWidget *widget)
{
    const QMetaObject *meta = widget->metaObject();
    for (int i = 0; i < meta->propertyCount(); ++i)
        if (meta->property(i).userType() == QMetaType::QColor) return true;
    return false;
}

// colour mode Default/Alarm_Default: background, foreground and text follow
// the theme through the widget code, the matrix leaves them alone
QString colorModeKey(const QWidget *widget)
{
    const QMetaObject *meta = widget->metaObject();
    const int index = meta->indexOfProperty("colorMode");
    if (index < 0) return QString();
    const QMetaProperty mode = meta->property(index);
    if (!mode.isEnumType()) return QString();
    const char *key = mode.enumerator().valueToKey(mode.read(widget).toInt());
    return key ? QString::fromLatin1(key) : QString();
}

bool followsThemeByMode(const QWidget *widget)
{
    return colorModeKey(widget).contains("Default");
}

// a dial in colour mode Alarm shows the alarm colour, not a themed surface: its
// authored scale colour must keep its contrast to that (caMeter, caClock)
bool scaleOnAlarmDial(const QWidget *widget, const QString &name)
{
    return name == "scaleColor" && colorModeKey(widget) == QLatin1String("Alarm");
}

bool themeDrivenProperty(const QString &name)
{
    return name == "background" || name == "foreground" || name == "textColor" || name == "baseColor"
        || name == "lineColor";
}

// curve colours of a plot carry meaning: no ramp, only contrast to the canvas
bool isCurveColor(const QWidget *widget, const QString &name)
{
    return widget->inherits("QwtPlot") && name.startsWith(QLatin1String("color_"));
}

// caGraphics/caPolyLine: "foreground" is the fill, a surface when fillstyle is Filled
bool hasFillStyle(const QWidget *widget)
{
    return widget->metaObject()->indexOfProperty("fillstyle") >= 0;
}

bool isFilledGraphic(const QWidget *widget)
{
    const QMetaObject *meta = widget->metaObject();
    const int index = meta->indexOfProperty("fillstyle");
    if (index < 0 || !meta->property(index).isEnumType()) return false;
    const QMetaProperty fill = meta->property(index);
    const char *key = fill.enumerator().valueToKey(fill.read(widget).toInt());
    return key && qstrcmp(key, "Filled") == 0;
}

// the surface the author gave a widget: its background, for a filled graphic its
// fill; a surface that is already dark marks a widget built for a dark panel
QColor authoredSurface(const QWidget *widget)
{
    QString name = QLatin1String("background");
    if (hasFillStyle(widget)) {
        if (!isFilledGraphic(widget)) return QColor();
        name = QLatin1String("foreground");
    }
    const QMetaObject *meta = widget->metaObject();
    const int index = meta->indexOfProperty(qPrintable(name));
    if (index < 0) return QColor();
    const QVariant original = widget->property(qPrintable(QString::fromLatin1(AuthoredPrefix) + name));
    return original.isValid() ? original.value<QColor>() : meta->property(index).read(widget).value<QColor>();
}

QString colorModeKey(const QWidget *widget);

// dark (low luminance) or a saturated signal colour: the author chose it for
// effect, the widget keeps its colours (thresholds from themeadapt.json)
bool darkBuiltSurface(const QColor &surface)
{
    if (!surface.isValid() || surface.alpha() != 255) return false;
    const int hi = qMax(surface.red(), qMax(surface.green(), surface.blue()));
    const int lo = qMin(surface.red(), qMin(surface.green(), surface.blue()));
    return ThemeColors::relativeLuminance(surface) < ThemeColors::darkSurfaceLuminance()
        || (hi - lo) / 255.0 >= ThemeColors::signalChroma();
}

bool isDarkBuilt(const QWidget *widget)
{
    return darkBuiltSurface(authoredSurface(widget));
}

// in colour mode Alarm these properties carry the alarm colour written by the
// dispatcher: the matrix must not write authored values over them
bool alarmOwnedProperty(const QWidget *widget, const QString &name)
{
    if (colorModeKey(widget) != QLatin1String("Alarm")) return false;
    if (hasFillStyle(widget))
        return name == QLatin1String("foreground") || name == QLatin1String("lineColor");
    return widget->inherits("caLabelVertical") && name == QLatin1String("foreground");
}

// a root below another theme root is an include: it is painted by its panel
bool hasThemeRootAncestor(const QWidget *root)
{
    for (const QWidget *w = root->parentWidget(); w; w = w->parentWidget())
        if (w->property(ThemeRootProperty).toBool()) return true;
    return false;
}

bool surfaceOrText(const QString &name)
{
    return name == QLatin1String("background") || name == QLatin1String("foreground")
        || name == QLatin1String("lineColor");
}

// an Outline frame uses "background" as its line colour; the "foreground" of a
// plot is its frame surface (the canvas is "background" and stays excluded); the
// "foreground" of a graphic is its fill
ThemeColors::AdaptRole roleFor(const QWidget *widget, const QString &name)
{
    if (name == QLatin1String("foreground") && (widget->inherits("QwtPlot") || hasFillStyle(widget)))
        return ThemeColors::BackgroundRole;
    if (name == QLatin1String("background")) {
        const QMetaObject *meta = widget->metaObject();
        const int index = meta->indexOfProperty("backgroundMode");
        if (index >= 0 && meta->property(index).isEnumType()) {
            const QMetaProperty mode = meta->property(index);
            const char *key = mode.enumerator().valueToKey(mode.read(widget).toInt());
            if (key && qstrcmp(key, "Outline") == 0) return ThemeColors::ForegroundRole;
        }
    }
    return ThemeColors::adaptRoleForProperty(name);
}

// the dispatcher keeps load-time copies of these colours for reconnects and
// alarm updates (HandleWidget, caqtdm_lib.cpp); keep them on the current scheme
void syncDispatcherSnapshot(QWidget *widget, const QString &name, const QColor &target)
{
    const char *key = name == QLatin1String("background") ? "BColor"
                    : name == QLatin1String("foreground") ? "FColor"
                    : name == QLatin1String("lineColor") ? "LColor" : Q_NULLPTR;
    if (!key || !widget->property(key).isValid()) return;
    if (widget->property(key).value<QColor>() != target) widget->setProperty(key, target);
}

bool alarmOwnedProperty(const QWidget *widget, const QString &name);

// one widget: every writable QColor property, the pair foreground/background,
// and the stylesheet of plain Qt widgets (ca widgets regenerate theirs and
// mark themselves or their internal children as self-styled); recordOnly just
// stores the authored values
// an authored <palette> block (the palette channel is adapted even for
// self-styled widgets, whose flag protects their stylesheet): every role set is
// adapted like a colour property, a surface role without its text role gets a
// readable text role, the original palette comes back in the light scheme
ThemeColors::AdaptRole paletteRoleClass(QPalette::ColorRole role)
{
    switch (role) {
    case QPalette::WindowText: case QPalette::Text: case QPalette::ButtonText: case QPalette::BrightText:
    case QPalette::ToolTipText: case QPalette::HighlightedText:
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    case QPalette::PlaceholderText:
#endif
        return ThemeColors::ForegroundRole;
    case QPalette::Window: case QPalette::Base: case QPalette::Button: case QPalette::AlternateBase:
    case QPalette::ToolTipBase: case QPalette::Light: case QPalette::Midlight: case QPalette::Mid:
    case QPalette::Dark: case QPalette::Shadow:
        return ThemeColors::BackgroundRole;
    default:
        return ThemeColors::AnyRole;
    }
}

const QPalette::ColorGroup PaletteGroups[] = { QPalette::Active, QPalette::Inactive, QPalette::Disabled };

// a role the widget's own code keeps up to date: never recorded or written
bool widgetOwnedRole(const QWidget *widget, QPalette::ColorRole role)
{
    return role == QPalette::AlternateBase && (widget->inherits("caTable") || widget->inherits("caWaveTable"));
}

QPalette withoutOwnedRoles(const QWidget *widget, const QPalette &authored)
{
    QPalette palette;
    for (int r = 0; r < QPalette::NColorRoles; ++r) {
        const QPalette::ColorRole role = static_cast<QPalette::ColorRole>(r);
        if (widgetOwnedRole(widget, role)) continue;
        for (QPalette::ColorGroup group : PaletteGroups)
            if (authored.isBrushSet(group, role)) palette.setBrush(group, role, authored.brush(group, role));
    }
    return palette;
}

bool hasOwnRoles(const QPalette &palette)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return palette.resolveMask() != 0;
#else
    return palette.resolve() != 0;
#endif
}

QStringList ownRoleNames(const QPalette &palette)
{
    QStringList names;
    const QMetaEnum roles = QMetaEnum::fromType<QPalette::ColorRole>();
    for (int r = 0; r < QPalette::NColorRoles; ++r)
        if (palette.isBrushSet(QPalette::Active, static_cast<QPalette::ColorRole>(r))) names << QString::fromLatin1(roles.valueToKey(r));
    return names;
}

bool sameRoles(const QPalette &current, const QPalette &target)      // the roles set in target only
{
    for (int r = 0; r < QPalette::NColorRoles; ++r) {
        for (QPalette::ColorGroup group : PaletteGroups) {
            const QPalette::ColorRole role = static_cast<QPalette::ColorRole>(r);
            if (target.isBrushSet(group, role) && current.brush(group, role) != target.brush(group, role)) return false;
        }
    }
    return true;
}

// a widget whose own palette can only come from the panel author: not a Qwt
// widget, not a ca widget that builds its palette in code (caTable/caWaveTable
// only add AlternateBase and keep the authored roles) and not inside a ca widget
// (their internals set palettes in code); the containers caFrame, caInclude and
// caDoubleTabWidget hold authored widgets
bool authoredPaletteWidget(const QWidget *widget)
{
    static const char *const codePalettes[] = { "caAlarmTree", "caChoice", "caClock", "caCartesianPlot", "caFrame",
                                                "caInclude", "caMeter", "caRowColMenu", "caSlider", "caStripPlot", "caThermo" };
    if (QString::fromLatin1(widget->metaObject()->className()).startsWith(QLatin1String("Qwt"))) return false;
    for (const char *cls : codePalettes)
        if (widget->inherits(cls)) return false;
    for (const QWidget *p = widget->parentWidget(); p; p = p->parentWidget()) {
        if (p->property(ThemeRootProperty).toBool()) break;
        if (!QString::fromLatin1(p->metaObject()->className()).startsWith(QLatin1String("ca"))) continue;
        if (!p->inherits("caFrame") && !p->inherits("caInclude") && !p->inherits("caDoubleTabWidget")) return false;
    }
    return true;
}

int adaptAuthoredPalette(QWidget *widget, bool active, ThemeColors::Scheme scheme, bool recordOnly)
{
    if (!authoredPaletteWidget(widget)) return 0;
    const QString key = QString::fromLatin1(AuthoredPrefix) + "palette";
    const QVariant original = widget->property(qPrintable(key));   // from recordAuthoredPalettes()
    if (!original.isValid() || recordOnly) return 0;
    const QPalette authored = original.value<QPalette>();
    if (!hasOwnRoles(authored)) return 0;
    // a dark or signal-coloured authored surface keeps the whole palette
    const QColor surface = authored.isBrushSet(QPalette::Active, QPalette::Base) ? authored.color(QPalette::Active, QPalette::Base)
                         : authored.isBrushSet(QPalette::Active, QPalette::Window) ? authored.color(QPalette::Active, QPalette::Window) : QColor();
    QPalette target = authored;
    if (active && !darkBuiltSurface(surface)) {
        for (int r = 0; r < QPalette::NColorRoles; ++r) {
            const QPalette::ColorRole role = static_cast<QPalette::ColorRole>(r);
            for (QPalette::ColorGroup group : PaletteGroups) {
                if (!authored.isBrushSet(group, role)) continue;
                target.setColor(group, role, ThemeColors::adapt(authored.color(group, role), scheme, paletteRoleClass(role)));
            }
        }
        // text on an authored surface: the adapted pair, or a readable text from the scheme
        const QPalette::ColorRole pairs[][2] = { { QPalette::Text, QPalette::Base }, { QPalette::WindowText, QPalette::Window },
                                                 { QPalette::ButtonText, QPalette::Button } };
        const QPalette schemePalette = ThemeColors::palette(scheme);
        for (const QPalette::ColorRole *pair : pairs) {
            if (!authored.isBrushSet(QPalette::Active, pair[1])) continue;
            QColor text = authored.isBrushSet(QPalette::Active, pair[0]) ? target.color(QPalette::Active, pair[0])
                                                                          : schemePalette.color(pair[0]);
            ThemeColors::ensureContrast(text, target.color(QPalette::Active, pair[1]), scheme);
            target.setColor(QPalette::Active, pair[0], text);
            target.setColor(QPalette::Inactive, pair[0], text);
        }
    }
    if (sameRoles(widget->palette(), target)) return 0;
    widget->setPalette(target);
    qCDebug(panelThemeAdaptLog) << widget->metaObject()->className() << widget->objectName()
                                << (active ? "palette adapted" : "palette restored");
    return 1;
}

int adaptWidget(QWidget *widget, bool active, ThemeColors::Scheme scheme, bool recordOnly)
{
    int changed = adaptAuthoredPalette(widget, active, scheme, recordOnly);
    if (widget->property(ThemeColors::SelfStyledProperty).toBool()) return changed;
    const QMetaObject *meta = widget->metaObject();
    const bool themeByMode = followsThemeByMode(widget);
    // a widget the author already built dark keeps its surface and text colours
    const bool darkBuilt = !recordOnly && active && !themeByMode && isDarkBuilt(widget);
    if (darkBuilt)
        qCDebug(panelThemeAdaptLog) << widget->metaObject()->className() << widget->objectName()
                                    << "kept (dark surface)";
    QColor foreground, background, canvas;
    bool hasForeground = false, hasBackground = false;
    QList<QMetaProperty> curves;
    for (int i = 0; i < meta->propertyCount(); ++i) {
        const QMetaProperty property = meta->property(i);
        if (property.userType() != QMetaType::QColor || !property.isWritable() || !property.isReadable())
            continue;
        const QString name = QString::fromLatin1(property.name());
        if (ThemeColors::adaptExcluded(widget, name)) continue;
        if (themeByMode && themeDrivenProperty(name)) continue;
        if (scaleOnAlarmDial(widget, name)) continue;
        if (isCurveColor(widget, name)) { curves.append(property); continue; }
        const QString key = QString::fromLatin1(AuthoredPrefix) + name;
        QVariant original = widget->property(qPrintable(key));
        if (!original.isValid()) {
            original = property.read(widget);
            widget->setProperty(qPrintable(key), original);
        }
        if (recordOnly || alarmOwnedProperty(widget, name)) continue;
        const QColor authored = original.value<QColor>();
        QColor target = authored;
        if (active && !(darkBuilt && surfaceOrText(name))) target = ThemeColors::adapt(authored, scheme, roleFor(widget, name));
        if (name == "foreground") { foreground = authored; hasForeground = true; }
        if (name == "background") { background = authored; hasBackground = true; canvas = target; }
        if (property.read(widget).value<QColor>() != target) {
            property.write(widget, target);
            ++changed;
            qCDebug(panelThemeAdaptLog) << widget->metaObject()->className() << widget->objectName()
                                        << name << authored.name() << "->" << target.name();
        }
        syncDispatcherSnapshot(widget, name, target);
    }
    for (const QMetaProperty &property : curves) {
        const QString key = QString::fromLatin1(AuthoredPrefix) + QString::fromLatin1(property.name());
        QVariant original = widget->property(qPrintable(key));
        if (!original.isValid()) {
            original = property.read(widget);
            widget->setProperty(qPrintable(key), original);
        }
        if (recordOnly) continue;
        QColor target = original.value<QColor>();
        if (active && hasBackground) ThemeColors::ensureContrast(target, canvas, scheme);
        if (property.read(widget).value<QColor>() != target) {
            property.write(widget, target);
            ++changed;
            qCDebug(panelThemeAdaptLog) << widget->metaObject()->className() << widget->objectName()
                                        << property.name() << original.value<QColor>().name() << "->" << target.name();
        }
    }
    // pair rule: not for plots (frame), graphics (fill) or dark-built widgets
    if (active && hasForeground && hasBackground && !darkBuilt && !widget->inherits("QwtPlot") && !hasFillStyle(widget)) {
        QColor fg = foreground, bg = background;
        ThemeColors::adaptPair(fg, bg, scheme);
        if (widget->property("foreground").value<QColor>() != fg) {
            widget->setProperty("foreground", fg);
            ++changed;
            qCDebug(panelThemeAdaptLog) << widget->metaObject()->className() << widget->objectName()
                                        << "foreground (pair)" << foreground.name() << "->" << fg.name();
        }
        syncDispatcherSnapshot(widget, QLatin1String("foreground"), fg);
    }
    if (!hasColorProperty(widget) && !widget->testAttribute(Qt::WA_StyleSheet) &&
        !widget->property(qPrintable(QString::fromLatin1(AuthoredPrefix) + "styleSheet")).isValid() &&
        widget->styleSheet().isEmpty()) {
        return changed;                                  // nothing authored here
    }
    if (!hasColorProperty(widget)) {
        const QString key = QString::fromLatin1(AuthoredPrefix) + "styleSheet";
        QVariant original = widget->property(qPrintable(key));
        if (!original.isValid()) {
            original = widget->styleSheet();
            widget->setProperty(qPrintable(key), original);
        }
        if (recordOnly) return changed;
        const QString authored = original.toString();
        const QString target = active ? ThemeColors::adaptStyleSheet(authored, scheme) : authored;
        if (widget->styleSheet() != target) {
            widget->setStyleSheet(target);
            ++changed;
            qCDebug(panelThemeAdaptLog) << widget->metaObject()->className() << widget->objectName()
                                        << "styleSheet adapted";
        }
    }
    return changed;
}

}

bool PanelThemeApplier::adaptActive(const QWidget *root)
{
    if (!root || !ThemeColors::adaptAvailable()) return false;
    const QString off = QString::fromLocal8Bit(qgetenv(ThemeAdaptEnvironment)).trimmed();
    if (off == "0" || off.compare(QLatin1String("false"), Qt::CaseInsensitive) == 0) return false;
    const QVariant optOut = root->property(ThemeAdaptProperty);
    if (optOut.isValid() && !optOut.toBool()) return false;
    const Mode mode = modeOf(root);
    return mode != Legacy && schemeFor(mode) == ThemeColors::Dark;
}

void PanelThemeApplier::recordAuthoredPalettes(QWidget *root, const QByteArray &ui)
{
    if (!root || ui.isEmpty()) return;
    QXmlStreamReader xml(ui);
    QStringList widgets;                                           // open <widget> names
    const QMetaEnum roles = QMetaEnum::fromType<QPalette::ColorRole>();
    int recorded = 0;
    while (!xml.atEnd()) {
        xml.readNext();
        const QString tag = xml.name().toString();
        if (xml.isStartElement() && tag == QLatin1String("widget")) {
            widgets.append(xml.attributes().value(QLatin1String("name")).toString());
        } else if (xml.isEndElement() && tag == QLatin1String("widget")) {
            if (!widgets.isEmpty()) widgets.removeLast();
        } else if (xml.isStartElement() && tag == QLatin1String("property") && !widgets.isEmpty()
                   && xml.attributes().value(QLatin1String("name")) == QLatin1String("palette")) {
            QPalette palette;
            QPalette::ColorGroup group = QPalette::Active;
            int role = -1;
            bool solid = true;
            while (!xml.atEnd() && !(xml.isEndElement() && xml.name() == QLatin1String("property"))) {
                xml.readNext();
                if (!xml.isStartElement()) continue;
                const QString inner = xml.name().toString();
                if (inner == QLatin1String("active")) group = QPalette::Active;
                else if (inner == QLatin1String("inactive")) group = QPalette::Inactive;
                else if (inner == QLatin1String("disabled")) group = QPalette::Disabled;
                else if (inner == QLatin1String("colorrole")) role = roles.keyToValue(qPrintable(xml.attributes().value(QLatin1String("role")).toString()));
                else if (inner == QLatin1String("brush")) {
                    const QString style = xml.attributes().value(QLatin1String("brushstyle")).toString();
                    solid = style.isEmpty() || style == QLatin1String("SolidPattern");
                } else if (inner == QLatin1String("color")) {
                    int rgb[3] = { 0, 0, 0 };
                    const int alpha = xml.attributes().hasAttribute(QLatin1String("alpha")) ? xml.attributes().value(QLatin1String("alpha")).toInt() : 255;
                    while (!xml.atEnd() && !(xml.isEndElement() && xml.name() == QLatin1String("color"))) {
                        xml.readNext();
                        if (!xml.isStartElement()) continue;
                        const QString channel = xml.name().toString();
                        const int index = channel == QLatin1String("red") ? 0 : channel == QLatin1String("green") ? 1 : channel == QLatin1String("blue") ? 2 : -1;
                        const int value = xml.readElementText().toInt();
                        if (index >= 0) rgb[index] = value;
                    }
                    if (role >= 0 && solid) palette.setColor(group, static_cast<QPalette::ColorRole>(role), QColor(rgb[0], rgb[1], rgb[2], alpha));
                }
            }
            const QString &name = widgets.last();
            QWidget *widget = root->objectName() == name ? root : root->findChild<QWidget *>(name);
            if (!widget) continue;
            palette = withoutOwnedRoles(widget, palette);
            if (!hasOwnRoles(palette)) continue;
            widget->setProperty(qPrintable(QString::fromLatin1(AuthoredPrefix) + "palette"), QVariant::fromValue(palette));
            ++recorded;
            qCDebug(panelThemeAdaptLog) << widget->metaObject()->className() << name << "authored palette" << ownRoleNames(palette);
        }
    }
    if (xml.hasError()) qCDebug(panelThemeAdaptLog) << "palette scan stopped:" << xml.errorString();
    qCDebug(panelThemeAdaptLog) << "recorded" << recorded << "authored palettes in panel" << root->objectName();
}

void PanelThemeApplier::adaptAuthoredColors(QWidget *root)
{
    if (!root) return;
    const bool active = adaptActive(root);
    const ThemeColors::Scheme scheme = active ? ThemeColors::Dark : ThemeColors::Light;
    // a panel never adapted needs no restore pass; it still records the authored
    // values at load time (later the properties may hold alarm colours)
    const bool recordOnly = !active && !root->property("caqtdmAdapted").toBool();
    if (recordOnly && modeOf(root) == Legacy) return;
    root->setProperty("caqtdmAdapted", active);
    int changed = 0;
    QWidget *const content = contentRoot(root);
    QList<QWidget *> widgets = root->findChildren<QWidget *>();
    for (QWidget *widget : widgets) {
        if (widget == content || isNestedRoot(widget) || insideNestedRoot(widget, root, content)) continue;
        changed += adaptWidget(widget, active, scheme, recordOnly);
    }
    if (recordOnly) {
        qCDebug(panelThemeAdaptLog) << "recorded the authored colours of panel" << root->objectName();
        return;
    }
    qCInfo(panelThemeAdaptLog) << "caQtDM --" << (active ? "adapted" : "restored") << changed
                               << "authored colours in panel" << root->objectName();
}
