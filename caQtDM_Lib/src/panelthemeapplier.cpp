#include "panelthemeapplier.h"

#include <QApplication>
#include <QMainWindow>
#include <QPalette>
#include <QWidget>

#include "caQtDM_Lib_global.h"
#include "cawavetable.h"

Q_LOGGING_CATEGORY(panelThemeLog, "caqtdm.lib.paneltheme")

namespace {
const char ThemeModeProperty[] = "caqtdmThemeMode";
const char ThemeModeOverrideEnvironment[] = "CAQTDM_PANEL_THEME_MODE";
const char ThemeRootProperty[] = "caqtdmAppliedThemeRoot";

QPalette createLegacyLightPalette()
{
    QPalette palette;
    const QPalette::ColorGroup groups[] = {
        QPalette::Active, QPalette::Inactive, QPalette::Disabled
    };
    for (QPalette::ColorGroup colorGroup : groups) {
        palette.setColor(colorGroup, QPalette::Window, Qt::white);
        palette.setColor(colorGroup, QPalette::WindowText, Qt::black);
        palette.setColor(colorGroup, QPalette::Base, Qt::white);
        palette.setColor(colorGroup, QPalette::AlternateBase, QColor(245, 245, 245));
        palette.setColor(colorGroup, QPalette::Text, Qt::black);
        palette.setColor(colorGroup, QPalette::Button, QColor(239, 239, 239));
        palette.setColor(colorGroup, QPalette::ButtonText, Qt::black);
        palette.setColor(colorGroup, QPalette::Light, Qt::white);
        palette.setColor(colorGroup, QPalette::Midlight, QColor(247, 247, 247));
        palette.setColor(colorGroup, QPalette::Mid, QColor(184, 184, 184));
        palette.setColor(colorGroup, QPalette::Dark, QColor(160, 160, 160));
        palette.setColor(colorGroup, QPalette::Shadow, QColor(105, 105, 105));
        palette.setColor(colorGroup, QPalette::ToolTipBase, Qt::white);
        palette.setColor(colorGroup, QPalette::ToolTipText, Qt::black);
        palette.setColor(colorGroup, QPalette::Highlight, QColor(53, 132, 228));
        palette.setColor(colorGroup, QPalette::HighlightedText, Qt::white);
        palette.setColor(colorGroup, QPalette::Link, QColor(0, 70, 180));
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
    QWidget *const target = contentRoot(root);
    if (!target) return;
    const bool legacyLight = usesLegacyLightTheme(root);
    seed(target, legacyLight);
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
