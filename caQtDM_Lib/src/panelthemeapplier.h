#ifndef PANELTHEMEAPPLIER_H
#define PANELTHEMEAPPLIER_H

#include <QString>

#include "caQtDM_Lib_global.h"

class QWidget;

// Applies the panel theme (palette + external stylesheet) to a panel root.
// Modes: legacy (fixed light palette, default), system (application
// palette), dark (own dark palette). A configured stylesheet file forces
// legacy unless a dark variant (<name>_dark.qss) is configured as well; the
// variant matching the panel scheme is then applied.
class CAQTDM_LIBSHARED_EXPORT PanelThemeApplier
{
public:
    enum Mode { Legacy = 0, System, Dark };

    // setPanelStyleSheet() also clears a dark variant set before
    static void setPanelStyleSheet(const QString &styleSheet, const QString &sourceFile = QString(),
                                   bool reloadable = false);
    static void setPanelDarkStyleSheet(const QString &styleSheet, const QString &sourceFile = QString());
    static bool hasPanelStyleSheet();
    static bool hasPanelDarkStyleSheet();

    static Mode parseMode(const QString &text, bool *ok = Q_NULLPTR);
    static QString modeName(Mode mode);
    static Mode resolveMode(Mode panelMode);       // environment override + stylesheet rule
    static Mode modeOf(const QWidget *root);       // panel property, then resolveMode()

    // compatibility wrappers (legacy == LegacyLight, !legacy == System)
    static bool usesLegacyLightTheme(const QWidget *root);

    static void seed(QWidget *contentRoot, Mode mode);

    static void propagateThroughOwnPalettes(QWidget *contentRoot);  // see .cpp
    static void apply(QWidget *root);

    // substitution matrix for authored colours (QColor properties and
    // stylesheets of plain Qt widgets); active in system/dark with a dark
    // scheme, restores the originals otherwise. Call before scanWidgets().
    static bool adaptActive(const QWidget *root);
    static void adaptAuthoredColors(QWidget *root);
};

#endif
