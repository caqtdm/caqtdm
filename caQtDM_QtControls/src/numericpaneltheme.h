#ifndef NUMERICPANELTHEME_H
#define NUMERICPANELTHEME_H

#include <QCoreApplication>
#include <QEvent>
#include <QLabel>
#include <QPalette>
#include <QPointer>
#include <QWidget>

class NumericPanelTheme : public QObject
{
public:
    NumericPanelTheme(QWidget *host, QWidget *numeric)
        : QObject(host), host(host), numeric(numeric)
    {
        host->installEventFilter(this);
        if (numeric != host) numeric->installEventFilter(this);
    }

    void setDefaultMode(bool enabled)
    {
        defaultMode = enabled;
        refresh();
    }

private:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (watched == host && (event->type() == QEvent::Show ||
                                event->type() == QEvent::ParentChange ||
                                event->type() == QEvent::StyleChange)) {
            scheduleRefresh();
        } else if (watched == numeric &&
                   (event->type() == QEvent::ChildAdded ||
                    event->type() == QEvent::PaletteChange ||
                    event->type() == QEvent::StyleChange)) {
            scheduleRefresh();
        } else if (watched == themeRoot &&
                   (event->type() == QEvent::PaletteChange ||
                    event->type() == QEvent::StyleChange)) {
            scheduleRefresh();
        } else if (defaultMode && watched != host && watched != numeric &&
                   (event->type() == QEvent::PaletteChange ||
                    event->type() == QEvent::StyleChange)) {
            scheduleRefresh();
        }
        return false;
    }

    bool event(QEvent *event) override
    {
        if (event->type() == QEvent::User) {
            pending = false;
            refresh();
            return true;
        }
        return QObject::event(event);
    }

    void scheduleRefresh()
    {
        if (pending) return;
        pending = true;
        QCoreApplication::postEvent(this, new QEvent(QEvent::User));
    }

    void refresh()
    {
        QWidget *nearestRoot = nullptr;
        for (QWidget *parent = host; parent; parent = parent->parentWidget()) {
            if (parent->property("caqtdmAppliedThemeRoot").toBool()) {
                nearestRoot = parent;
                break;
            }
        }
        if (themeRoot != nearestRoot) {
            if (themeRoot) themeRoot->removeEventFilter(this);
            themeRoot = nearestRoot;
            if (themeRoot) themeRoot->installEventFilter(this);
        }

        if (!defaultMode || !themeRoot) {
            if (applied) {
                numeric->setPalette(QPalette());
                for (QLabel *label : numeric->findChildren<QLabel *>())
                    label->setPalette(QPalette());
                applied = false;
            }
            return;
        }

        const QPalette panelPalette = themeRoot->palette();
        if (numeric->palette() != panelPalette)
            numeric->setPalette(panelPalette);
        for (QLabel *label : numeric->findChildren<QLabel *>()) {
            label->installEventFilter(this);
            if (label->palette() != panelPalette)
                label->setPalette(panelPalette);
        }
        applied = true;
    }

    QWidget *host;
    QWidget *numeric;
    QPointer<QWidget> themeRoot;
    bool defaultMode = false;
    bool applied = false;
    bool pending = false;
};

#endif
