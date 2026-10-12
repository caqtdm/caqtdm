#include "panelthemeapplier.h"

#include <QApplication>
#include <QFile>
#include <QMainWindow>
#include <QPalette>
#include <QWidget>

#include "caQtDM_Lib_global.h"
#include "cawavetable.h"
#include "themecolors.h"

Q_LOGGING_CATEGORY(panelThemeLog, "caqtdm.lib.paneltheme")

namespace {
const char ThemeModeProperty[] = "caqtdmThemeMode";
const char ThemeEnvironment[] = "CAQTDM_THEME";
const char ThemeModeOverrideEnvironment[] = "CAQTDM_PANEL_THEME_MODE";  // alias
const char ThemeRootProperty[] = "caqtdmAppliedThemeRoot";
const char LegacyLightProperty[] = "caqtdmAppliedLegacyLight";
const char AuthoredStyleSheetProperty[] = "caqtdmAuthoredStyleSheet";
const char AuthoredAutoFillProperty[] = "caqtdmAuthoredAutoFill";

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

void applyPanelStyleSheet(QWidget *root, ThemeColors::Scheme scheme)
{
    if (!root) return;
    if (!root->property(AuthoredStyleSheetProperty).isValid())
        root->setProperty(AuthoredStyleSheetProperty, root->styleSheet());

    const QString authored = root->property(AuthoredStyleSheetProperty).toString();
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

bool PanelThemeApplier::resolveLegacyLightTheme(bool panelLegacyLight)
{
    return resolveMode(panelLegacyLight ? Legacy : System) == Legacy;
}

void PanelThemeApplier::apply(QWidget *root)
{
    if (!root) return;
    const Mode mode = modeOf(root);
    const ThemeColors::Scheme scheme = schemeFor(mode);
    applyPanelStyleSheet(root, scheme);
    QWidget *const target = contentRoot(root);
    if (!target) return;
    const bool legacyLight = (mode == Legacy);
    // properties first: children read the scheme while handling PaletteChange
    target->setProperty(LegacyLightProperty, legacyLight);
    target->setProperty(ThemeRootProperty, true);
    target->setProperty(ThemeColors::ThemeSchemeProperty, scheme == ThemeColors::Dark ? "dark" : "light");
    seed(target, mode);
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
}

void PanelThemeApplier::seed(QWidget *contentRoot, const QWidget *themeRoot)
{
    if (!contentRoot || !themeRoot) return;
    seed(contentRoot, modeOf(themeRoot));
}

void PanelThemeApplier::seed(QWidget *contentRoot, Mode mode)
{
    if (!contentRoot) return;
    contentRoot->setPalette(paletteFor(mode));
}
