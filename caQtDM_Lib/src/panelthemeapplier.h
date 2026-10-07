#ifndef PANELTHEMEAPPLIER_H
#define PANELTHEMEAPPLIER_H

#include <QString>

#include "caQtDM_Lib_global.h"

class QWidget;

class CAQTDM_LIBSHARED_EXPORT PanelThemeApplier
{
public:
    static void setPanelStyleSheet(const QString &styleSheet, const QString &sourceFile = QString(),
                                   bool reloadable = false);
    static bool resolveLegacyLightTheme(bool panelLegacyLight);
    static void seed(QWidget *contentRoot, bool legacyLight);
    static void seed(QWidget *contentRoot, const QWidget *themeRoot);
    static void apply(QWidget *root);
    static bool usesLegacyLightTheme(const QWidget *root);
};

#endif // PANELTHEMEAPPLIER_H
