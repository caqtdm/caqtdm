#include "panelcontrastcontroller.h"

#include <QAbstractButton>
#include <QApplication>
#include <QComboBox>
#include <QEvent>
#include <QImage>
#include <QLineEdit>
#include <QMetaProperty>
#include <QTabBar>
#include <QTextEdit>
#include <QTimer>
#include <QWidget>

#include "caQtDM_Lib_global.h"

namespace {
const qreal ContrastThreshold = 4.5;
const char OverrideMarker[] = "/* caqtdm-panelcontrast */";

qreal luminance(const QColor &color)
{
    const auto component = [](int value) {
        const qreal channel = value / 255.0;
        return channel <= 0.03928 ? channel / 12.92 : qPow((channel + 0.055) / 1.055, 2.4);
    };
    return 0.2126 * component(color.red()) + 0.7152 * component(color.green()) + 0.0722 * component(color.blue());
}

QPalette lightPalette()
{
    QPalette palette;
    for (int group = QPalette::Active; group <= QPalette::Inactive; ++group) {
        const QPalette::ColorGroup colorGroup = static_cast<QPalette::ColorGroup>(group);
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
}

PanelContrastController::PanelContrastController(QWidget *root)
    : QObject(root), m_root(root), m_hasOriginalPalette(false), m_lightFallback(false),
      m_evaluationPending(false)
{
    m_root->setProperty("caqtdm_panel_contrast_root", true);
    qApp->installEventFilter(this);
    scheduleEvaluation();
}

PanelContrastController::~PanelContrastController()
{
    if (qApp) qApp->removeEventFilter(this);
}

qreal PanelContrastController::contrastRatio(const QColor &foreground, const QColor &background)
{
    const qreal first = luminance(foreground);
    const qreal second = luminance(background);
    return (qMax(first, second) + 0.05) / (qMin(first, second) + 0.05);
}

bool PanelContrastController::needsLightFallback(int failingWidgets, int eligibleWidgets)
{
    return eligibleWidgets > 0 && failingWidgets * 2 > eligibleWidgets;
}

void PanelContrastController::scheduleEvaluation()
{
    if (m_evaluationPending || !m_root) return;
    m_evaluationPending = true;
    // Display updates often repaint several child widgets together.  Delay one
    // event-loop turn so they produce a single image sample for this panel.
    QTimer::singleShot(50, this, &PanelContrastController::evaluate);
}

bool PanelContrastController::eventFilter(QObject *watched, QEvent *event)
{
    QWidget *widget = qobject_cast<QWidget *>(watched);
    if (!widget || !belongsToThisPanel(widget)) return false;

    switch (event->type()) {
    case QEvent::Show:
    case QEvent::Paint:
    case QEvent::Resize:
    case QEvent::StyleChange:
    case QEvent::PaletteChange:
    case QEvent::ApplicationPaletteChange:
    case QEvent::DynamicPropertyChange:
        scheduleEvaluation();
        break;
    default:
        break;
    }
    return false;
}

bool PanelContrastController::belongsToThisPanel(QWidget *widget) const
{
    if (widget != m_root && !m_root->isAncestorOf(widget)) return false;
    for (QWidget *parent = widget->parentWidget(); parent && parent != m_root; parent = parent->parentWidget()) {
        if (parent->property("caqtdm_panel_contrast_root").toBool()) return false;
    }
    return true;
}

bool PanelContrastController::isCandidate(QWidget *widget) const
{
    if (!widget->isVisible() || widget == m_root) return false;
    if (qobject_cast<QAbstractButton *>(widget) || qobject_cast<QLineEdit *>(widget) ||
        qobject_cast<QComboBox *>(widget) || qobject_cast<QTextEdit *>(widget) ||
        qobject_cast<QTabBar *>(widget)) return true;
    const int textProperty = widget->metaObject()->indexOfProperty("text");
    return textProperty >= 0 && !widget->property("text").toString().isEmpty();
}

bool PanelContrastController::isAlarmControlled(QWidget *widget) const
{
    const int propertyIndex = widget->metaObject()->indexOfProperty("colorMode");
    if (propertyIndex < 0) return false;

    const QMetaProperty property = widget->metaObject()->property(propertyIndex);
    const QByteArray propertyType = property.typeName();
    const int value = widget->property("colorMode").toInt();
    const QMetaObject *metaObject = widget->metaObject();
    for (int index = 0; index < metaObject->enumeratorCount(); ++index) {
        const QMetaEnum enumerator = metaObject->enumerator(index);
        const QByteArray enumName = enumerator.name();
        if (propertyType != enumName && !propertyType.endsWith("::" + enumName)) continue;
        const char *key = enumerator.valueToKey(value);
        return key && QByteArray(key).contains("Alarm");
    }
    return false;
}

QColor PanelContrastController::foregroundFor(QWidget *widget) const
{
    if (widget->property("caqtdm_panel_contrast_palette_override").toBool()) {
        const QColor override = QColor(widget->property("caqtdm_panel_contrast_override").toString());
        if (override.isValid()) return override;
    }

    QPalette::ColorRole role = QPalette::WindowText;
    if (qobject_cast<QAbstractButton *>(widget) || qobject_cast<QTabBar *>(widget))
        role = QPalette::ButtonText;
    else if (qobject_cast<QLineEdit *>(widget) || qobject_cast<QTextEdit *>(widget) || qobject_cast<QComboBox *>(widget))
        role = QPalette::Text;

    const QColor paletteForeground = widget->palette().color(role);
    const QColor inheritedForeground = widget->parentWidget()
        ? widget->parentWidget()->palette().color(role)
        : qApp->palette().color(role);
    // A local palette role is how QSS-aware widgets express an alarm or
    // runtime color. It is more current than their configured foreground
    // property, which deliberately remains the designer value.
    if (paletteForeground.isValid() && paletteForeground.alpha() > 0 &&
        paletteForeground != inheritedForeground)
        return paletteForeground;

    const int foregroundProperty = widget->metaObject()->indexOfProperty("foreground");
    if (foregroundProperty >= 0) {
        const QColor foreground = widget->property("foreground").value<QColor>();
        if (foreground.isValid() && foreground.alpha() > 0) return foreground;
    }
    return paletteForeground;
}

QColor PanelContrastController::backgroundFor(QWidget *widget, const QImage &image) const
{
    QRect rect(widget->mapTo(m_root, QPoint(0, 0)), widget->size());
    rect = rect.intersected(image.rect());
    if (rect.isEmpty()) return QColor();

    const QColor foreground = foregroundFor(widget);
    struct Bucket { int count = 0; qint64 red = 0; qint64 green = 0; qint64 blue = 0; };
    Bucket buckets[512];
    for (int y = rect.top(); y <= rect.bottom(); y += 2) {
        for (int x = rect.left(); x <= rect.right(); x += 2) {
            const QColor color = QColor::fromRgb(image.pixel(x, y));
            const int distance = qAbs(color.red() - foreground.red()) + qAbs(color.green() - foreground.green()) + qAbs(color.blue() - foreground.blue());
            if (distance < 48) continue; // text and anti-aliased edges
            const int index = (color.red() / 32) * 64 + (color.green() / 32) * 8 + color.blue() / 32;
            Bucket &bucket = buckets[index];
            ++bucket.count;
            bucket.red += color.red();
            bucket.green += color.green();
            bucket.blue += color.blue();
        }
    }
    Bucket *dominant = nullptr;
    for (Bucket &bucket : buckets) {
        if (!dominant || bucket.count > dominant->count) dominant = &bucket;
    }
    if (!dominant || dominant->count == 0) return widget->palette().color(QPalette::Window);
    return QColor(dominant->red / dominant->count, dominant->green / dominant->count, dominant->blue / dominant->count);
}

bool PanelContrastController::systemPaletteIsDark() const
{
    return luminance(qApp->palette().color(QPalette::Window)) < 0.5;
}

void PanelContrastController::setLightFallback(bool enabled)
{
    if (enabled == m_lightFallback) return;
    if (enabled) {
        m_originalPalette = m_root->palette();
        m_hasOriginalPalette = true;
        const QPalette compatibilityPalette = lightPalette();
        const QList<QWidget *> widgets = m_root->findChildren<QWidget *>();
        for (QWidget *widget : widgets) {
            if (!belongsToThisPanel(widget)) continue;
            m_originalChildPalettes.insert(widget, widget->palette());
            connect(widget, &QObject::destroyed, this, [this, widget] {
                m_originalChildPalettes.remove(widget);
            });
        }
        m_root->setPalette(compatibilityPalette);
        for (auto it = m_originalChildPalettes.cbegin(); it != m_originalChildPalettes.cend(); ++it)
            it.key()->setPalette(it.value().resolve(compatibilityPalette));
    } else if (m_hasOriginalPalette) {
        m_root->setPalette(m_originalPalette);
        for (auto it = m_originalChildPalettes.cbegin(); it != m_originalChildPalettes.cend(); ++it)
            it.key()->setPalette(it.value());
        m_originalChildPalettes.clear();
    }
    m_lightFallback = enabled;
    qCDebug(panelContrastLog) << (enabled ? "enabled light fallback for" : "restored system palette for") << m_root;
}

void PanelContrastController::applyOverride(QWidget *widget, const QColor &color)
{
    // An inline stylesheet containing only "color" breaks inherited QSS
    // background rules (notably the tab-page wheat background in tests.ui).
    // Use a local palette foreground instead; it leaves the stylesheet
    // cascade and every authored background untouched.
    QString stylesheet = widget->styleSheet();
    const int marker = stylesheet.indexOf(QLatin1String(OverrideMarker));
    if (marker >= 0) {
        stylesheet.truncate(marker);
        widget->setStyleSheet(stylesheet);
    }

    if (!widget->property("caqtdm_panel_contrast_palette_override").toBool())
        widget->setProperty("caqtdm_panel_contrast_original_palette", widget->palette());

    QPalette palette = widget->palette();
    palette.setColor(QPalette::WindowText, color);
    palette.setColor(QPalette::Text, color);
    palette.setColor(QPalette::ButtonText, color);
    widget->setPalette(palette);
    widget->setProperty("caqtdm_panel_contrast_palette_override", true);
    widget->setProperty("caqtdm_panel_contrast_override", color.name());
    qCDebug(panelContrastLog) << "applied contrast override" << color << "to" << widget;
}

void PanelContrastController::removeOverride(QWidget *widget)
{
    if (widget->property("caqtdm_panel_contrast_palette_override").toBool()) {
        const QPalette palette = widget->property("caqtdm_panel_contrast_original_palette").value<QPalette>();
        widget->setPalette(palette);
        widget->setProperty("caqtdm_panel_contrast_palette_override", QVariant());
        widget->setProperty("caqtdm_panel_contrast_original_palette", QVariant());
        widget->setProperty("caqtdm_panel_contrast_override", QVariant());
        qCDebug(panelContrastLog) << "removed palette contrast override from" << widget;
    }

    QString stylesheet = widget->styleSheet();
    const int marker = stylesheet.indexOf(QLatin1String(OverrideMarker));
    if (marker >= 0) {
        stylesheet.truncate(marker);
        widget->setStyleSheet(stylesheet);
        widget->setProperty("caqtdm_panel_contrast_override", QVariant());
        qCDebug(panelContrastLog) << "removed legacy stylesheet contrast override from" << widget;
    }
}

void PanelContrastController::evaluate()
{
    m_evaluationPending = false;
    if (!m_root || !m_root->isVisible()) return;

    const QList<QWidget *> widgets = m_root->findChildren<QWidget *>();
    QList<QWidget *> candidates;
    for (QWidget *widget : widgets)
        if (belongsToThisPanel(widget) && isCandidate(widget)) candidates.append(widget);
    if (candidates.isEmpty()) return;

    QImage image = m_root->grab().toImage();
    int failing = 0;
    int eligible = 0;
    for (QWidget *widget : candidates) {
        if (isAlarmControlled(widget)) {
            removeOverride(widget);
            continue;
        }
        if (!widget->property("caqtdm_panel_contrast_palette_override").toBool())
            widget->setProperty("caqtdm_panel_contrast_override", QVariant());
        ++eligible;
        if (contrastRatio(foregroundFor(widget), backgroundFor(widget, image)) < ContrastThreshold) ++failing;
    }
    const bool useFallback = systemPaletteIsDark() && needsLightFallback(failing, eligible);
    setLightFallback(useFallback);
    if (useFallback) image = m_root->grab().toImage();

    for (QWidget *widget : candidates) {
        if (isAlarmControlled(widget)) continue;
        const bool hasOverride = widget->property("caqtdm_panel_contrast_palette_override").toBool();
        const QColor foreground = foregroundFor(widget);
        const QColor background = backgroundFor(widget, image);
        const qreal ratio = contrastRatio(foreground, background);
        // Keep a successful override until the widget itself replaces its stylesheet.
        // Otherwise the next paint would see the corrected color, remove it, and
        // immediately reapply it on the following paint.
        if (hasOverride && ratio >= ContrastThreshold) continue;
        if (ratio < ContrastThreshold) {
            const QColor black(Qt::black);
            const QColor white(Qt::white);
            applyOverride(widget, contrastRatio(black, background) >= contrastRatio(white, background) ? black : white);
        } else {
            removeOverride(widget);
        }
    }
    qCDebug(panelContrastLog) << "panel contrast" << m_root << "failing" << failing << "of" << eligible
                              << "light fallback" << useFallback;
}
