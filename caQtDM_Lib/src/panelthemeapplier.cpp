#include "panelthemeapplier.h"

#include <QApplication>
#include <QFile>
#include <QLabel>
#include <QMainWindow>
#include <QMetaProperty>
#include <QPalette>
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
    const bool paintPanel = (mode == Dark && !applicationIsDark) || (mode == Legacy && applicationIsDark);
    target->setAutoFillBackground(paintPanel || target->property(AuthoredAutoFillProperty).toBool());
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

bool isDarkBuilt(const QWidget *widget)
{
    const QColor surface = authoredSurface(widget);
    return surface.isValid() && surface.alpha() == 255 && surface.lightness() <= 128;
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

// one widget: every writable QColor property, the pair foreground/background,
// and the stylesheet of plain Qt widgets (ca widgets regenerate theirs and
// mark themselves or their internal children as self-styled); recordOnly just
// stores the authored values
int adaptWidget(QWidget *widget, bool active, ThemeColors::Scheme scheme, bool recordOnly)
{
    int changed = 0;
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
        if (recordOnly) continue;
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
