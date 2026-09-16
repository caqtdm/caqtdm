#ifndef PANELTHEMEAPPLIER_H
#define PANELTHEMEAPPLIER_H

class QWidget;

class PanelThemeApplier
{
public:
    static bool resolveLegacyLightTheme(bool panelLegacyLight);
    static void seed(QWidget *contentRoot, bool legacyLight);
    static void seed(QWidget *contentRoot, const QWidget *themeRoot);
    static void apply(QWidget *root);
    static bool usesLegacyLightTheme(const QWidget *root);
};

#endif // PANELTHEMEAPPLIER_H
