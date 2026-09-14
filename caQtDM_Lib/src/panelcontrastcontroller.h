#ifndef PANELCONTRASTCONTROLLER_H
#define PANELCONTRASTCONTROLLER_H

#include <QObject>
#include <QColor>
#include <QHash>
#include <QPalette>

class QWidget;
class QEvent;
class QImage;

// Runtime-only compatibility aid for panels authored for a light palette.
class PanelContrastController : public QObject
{
    Q_OBJECT
public:
    explicit PanelContrastController(QWidget *root);
    ~PanelContrastController() override;

    static qreal contrastRatio(const QColor &foreground, const QColor &background);
    static bool needsLightFallback(int failingWidgets, int eligibleWidgets);

public slots:
    void scheduleEvaluation();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void evaluate();

private:
    QColor foregroundFor(QWidget *widget) const;
    QColor backgroundFor(QWidget *widget, const QImage &image) const;
    bool isCandidate(QWidget *widget) const;
    bool isAlarmControlled(QWidget *widget) const;
    bool belongsToThisPanel(QWidget *widget) const;
    bool renderedPanelLooksLight(const QImage &image) const;
    bool systemPaletteIsDark() const;
    void setLightFallback(bool enabled);
    void applyOverride(QWidget *widget, const QColor &color);
    void removeOverride(QWidget *widget);

    QWidget *m_root;
    QPalette m_originalPalette;
    QHash<QWidget *, QPalette> m_originalChildPalettes;
    bool m_hasOriginalPalette;
    bool m_lightFallback;
    bool m_evaluationPending;
};

#endif // PANELCONTRASTCONTROLLER_H
