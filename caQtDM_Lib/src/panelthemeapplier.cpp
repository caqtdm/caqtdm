#include "panelthemeapplier.h"

#include <QApplication>
#include <QMenu>
#include <QMenuBar>
#include <QPalette>
#include <QStatusBar>
#include <QToolBar>
#include <QWidget>

#include "caQtDM_Lib_global.h"

Q_LOGGING_CATEGORY(panelThemeLog, "caqtdm.lib.paneltheme")

namespace {
const char ThemeModeProperty[] = "caqtdmThemeMode";
const char ThemeRootProperty[] = "caqtdm_panel_theme_root";

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

bool isWindowChrome(const QWidget *widget)
{
    return qobject_cast<const QMenuBar *>(widget) ||
        qobject_cast<const QMenu *>(widget) ||
        qobject_cast<const QStatusBar *>(widget) ||
        qobject_cast<const QToolBar *>(widget);
}

bool belongsToRoot(const QWidget *root, const QWidget *widget)
{
    if (widget != root && !root->isAncestorOf(widget)) return false;
    for (const QWidget *parent = widget; parent && parent != root; parent = parent->parentWidget()) {
        if (isWindowChrome(parent)) return false;
        if (parent->property(ThemeRootProperty).toBool()) return false;
    }
    return true;
}
}

bool PanelThemeApplier::usesLegacyLightTheme(const QWidget *root)
{
    if (!root) return false;
    return root->property(ThemeModeProperty).toString().compare("System", Qt::CaseInsensitive) != 0;
}

void PanelThemeApplier::apply(QWidget *root)
{
    if (!root) return;
    root->setProperty(ThemeRootProperty, true);
    const bool legacyLight = usesLegacyLightTheme(root);
    const QPalette palette = legacyLight ? legacyLightPalette()
                                         : resolvedApplicationPalette();
    const bool updatesEnabled = root->updatesEnabled();
    root->setUpdatesEnabled(false);
    root->setPalette(palette);

    const QList<QWidget *> widgets = root->findChildren<QWidget *>();
    QList<QWidget *> chrome;
    for (QWidget *widget : widgets) {
        if (isWindowChrome(widget)) {
            chrome.append(widget);
            continue;
        }
        if (!belongsToRoot(root, widget)) continue;
        // QWidget::palette() is an *effective* palette.  Resolving that
        // value keeps the dark application roles which were inherited when
        // the panel was loaded, so a compatibility palette never reaches
        // custom-painted and stylesheet-backed descendants.  Apply the
        // selected panel palette as one deterministic subtree operation.
        // Explicit QSS and widget painting remain authoritative.
        widget->setPalette(palette);
    }
    for (QWidget *widget : chrome)
        widget->setPalette(QApplication::palette());

    root->setUpdatesEnabled(updatesEnabled);
    root->update();
    qCDebug(panelThemeLog) << (legacyLight ? "applied legacy-light palette to"
                                           : "applied system palette to")
                            << root << "widgets" << widgets.size();
}
