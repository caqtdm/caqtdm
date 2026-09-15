#ifndef PANELTHEMEAPPLIER_H
#define PANELTHEMEAPPLIER_H

class QWidget;

class PanelThemeApplier
{
public:
    static void apply(QWidget *root);
    static bool usesLegacyLightTheme(const QWidget *root);
};

#endif // PANELTHEMEAPPLIER_H
