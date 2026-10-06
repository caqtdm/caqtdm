#include "panelthemeapplier.h"

#include <QApplication>
#include <QFile>
#include <QMainWindow>
#include <QPalette>
#include <QWidget>

#include "caQtDM_Lib_global.h"
#include "cawavetable.h"
#include "alarmdefs.h"

Q_LOGGING_CATEGORY(panelThemeLog, "caqtdm.lib.paneltheme")

namespace {
const char ThemeModeProperty[] = "caqtdmThemeMode";
const char ThemeModeOverrideEnvironment[] = "CAQTDM_PANEL_THEME_MODE";
const char ThemeRootProperty[] = "caqtdmAppliedThemeRoot";
const char LegacyLightProperty[] = "caqtdmAppliedLegacyLight";
const char AuthoredStyleSheetProperty[] = "caqtdmAuthoredStyleSheet";

struct PanelStyleSheet {
    QString contents;
    QString sourceFile;
    bool reloadable = false;
};

PanelStyleSheet &panelStyleSheet()
{
    static PanelStyleSheet styleSheet;
    return styleSheet;
}

QString resolvedPanelStyleSheet()
{
    PanelStyleSheet &styleSheet = panelStyleSheet();
    const QString reloadMode = QString::fromLocal8Bit(qgetenv("CAQTDM_STYLESHEET_RELOAD"));
    if (styleSheet.reloadable && reloadMode.contains("file", Qt::CaseInsensitive) &&
        !styleSheet.sourceFile.isEmpty()) {
        QFile file(styleSheet.sourceFile);
        if (file.open(QFile::ReadOnly)) {
            styleSheet.contents = QLatin1String(file.readAll());
            qCInfo(panelThemeLog) << "caQtDM -- reloaded panel stylesheet" << styleSheet.sourceFile;
        }
    }
    if (reloadMode.contains("print", Qt::CaseInsensitive) && !styleSheet.contents.isEmpty())
        qCInfo(panelThemeLog) << "caQtDM -- panel stylesheet data:" << styleSheet.contents;
    return styleSheet.contents;
}

void applyPanelStyleSheet(QWidget *root)
{
    if (!root) return;
    if (!root->property(AuthoredStyleSheetProperty).isValid())
        root->setProperty(AuthoredStyleSheetProperty, root->styleSheet());

    const QString authored = root->property(AuthoredStyleSheetProperty).toString();
    const QString external = resolvedPanelStyleSheet();
    root->setStyleSheet(authored.isEmpty() ? external :
                        external.isEmpty() ? authored : authored + '\n' + external);
}

QPalette createLegacyLightPalette()
{
    QPalette palette;
    const QPalette::ColorGroup groups[] = {
        QPalette::Active, QPalette::Inactive, QPalette::Disabled
    };
    for (QPalette::ColorGroup colorGroup : groups) {
        palette.setColor(colorGroup, QPalette::Window, PANEL_THEME_LEGACY_WHITE);
        palette.setColor(colorGroup, QPalette::WindowText, PANEL_THEME_LEGACY_BLACK);
        palette.setColor(colorGroup, QPalette::Base, PANEL_THEME_LEGACY_WHITE);
        palette.setColor(colorGroup, QPalette::AlternateBase, PANEL_THEME_LEGACY_ALTERNATE_BASE);
        palette.setColor(colorGroup, QPalette::Text, PANEL_THEME_LEGACY_BLACK);
        palette.setColor(colorGroup, QPalette::Button, PANEL_THEME_LEGACY_BUTTON);
        palette.setColor(colorGroup, QPalette::ButtonText, PANEL_THEME_LEGACY_BLACK);
        palette.setColor(colorGroup, QPalette::Light, PANEL_THEME_LEGACY_WHITE);
        palette.setColor(colorGroup, QPalette::Midlight, PANEL_THEME_LEGACY_MIDLIGHT);
        palette.setColor(colorGroup, QPalette::Mid, PANEL_THEME_LEGACY_MID);
        palette.setColor(colorGroup, QPalette::Dark, PANEL_THEME_LEGACY_DARK);
        palette.setColor(colorGroup, QPalette::Shadow, PANEL_THEME_LEGACY_SHADOW);
        palette.setColor(colorGroup, QPalette::ToolTipBase, PANEL_THEME_LEGACY_WHITE);
        palette.setColor(colorGroup, QPalette::ToolTipText, PANEL_THEME_LEGACY_BLACK);
        palette.setColor(colorGroup, QPalette::Highlight, PANEL_THEME_LEGACY_HIGHLIGHT);
        palette.setColor(colorGroup, QPalette::HighlightedText, PANEL_THEME_LEGACY_WHITE);
        palette.setColor(colorGroup, QPalette::Link, PANEL_THEME_LEGACY_LINK);
    }
    return palette;
}

const QPalette &legacyLightPalette()
{
    static const QPalette palette = createLegacyLightPalette();
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
            const QPalette::ColorRole colorRole = static_cast<QPalette::ColorRole>(role);
            palette.setColor(colorGroup, colorRole,
                             applicationPalette.color(colorGroup, colorRole));
        }
    }
    return palette;
}

QPalette paletteFor(bool legacyLight)
{
    return legacyLight ? legacyLightPalette() : resolvedApplicationPalette();
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
}

bool PanelThemeApplier::usesLegacyLightTheme(const QWidget *root)
{
    if (!root) return false;
    return resolveLegacyLightTheme(
        root->property(ThemeModeProperty).toString().compare("System", Qt::CaseInsensitive) != 0);
}

bool PanelThemeApplier::resolveLegacyLightTheme(bool panelLegacyLight)
{
    const QString overrideMode =
        QString::fromLocal8Bit(qgetenv(ThemeModeOverrideEnvironment)).trimmed();
    if (overrideMode.compare("System", Qt::CaseInsensitive) == 0) return false;
    if (overrideMode.compare("LegacyLight", Qt::CaseInsensitive) == 0) return true;
    return panelLegacyLight;
}

void PanelThemeApplier::apply(QWidget *root)
{
    if (!root) return;
    applyPanelStyleSheet(root);
    QWidget *const target = contentRoot(root);
    if (!target) return;
    const bool legacyLight = usesLegacyLightTheme(root);
    seed(target, legacyLight);
    target->setProperty(LegacyLightProperty, legacyLight);
    target->setProperty(ThemeRootProperty, true);
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
    qCDebug(panelThemeLog) << (legacyLight
                                 ? "applied legacy-light palette to content root"
                                 : "applied system palette to content root")
                            << target;
    if (!panelThemeLog().isDebugEnabled()) return;

    logPalette("panel root", root);
    logPalette("panel content root", target);
}

void PanelThemeApplier::seed(QWidget *contentRoot, const QWidget *themeRoot)
{
    if (!contentRoot || !themeRoot) return;
    seed(contentRoot, usesLegacyLightTheme(themeRoot));
}

void PanelThemeApplier::seed(QWidget *contentRoot, bool legacyLight)
{
    if (!contentRoot) return;
    contentRoot->setPalette(paletteFor(legacyLight));
}
