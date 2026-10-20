/*
 *  This file is part of the caQtDM Framework, developed at the Paul Scherrer Institut,
 *  Villigen, Switzerland
 *
 *  The caQtDM Framework is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  The caQtDM Framework is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the caQtDM Framework.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright (c) 2010 - 2014
 *
 *  Author:
 *    Anton Mezger
 *  Contact details:
 *    anton.mezger@psi.ch
 */

#include "camenu.h"
#include "alarmdefs.h"
#include "themecolors.h"
#include <QTimer>
#include <QPainter>
#include <QTimer>
#include <QLineEdit>
#include <QMouseEvent>
#include <QApplication>

Q_LOGGING_CATEGORY(caMenuLog, "caqtdm.widgets.camenu")

// colour set of caMenu (level 2), values come from themecolors.h
namespace caMenuColor {
    constexpr const ThemeColor &AlarmNoAlarm = ThemeColors::AlarmNoAlarm;
    constexpr const ThemeColor &AlarmMinor   = ThemeColors::AlarmMinor;
    constexpr const ThemeColor &AlarmMajor   = ThemeColors::AlarmMajor;
    constexpr const ThemeColor &AlarmInvalid = ThemeColors::AlarmInvalid;
    constexpr const ThemeColor &AlarmDefault = ThemeColors::AlarmDefault;
}

caMenu::caMenu(QWidget *parent) : QComboBox(parent)
{
    // to start with, clear the stylesheet, so that playing around
    // is not possible.
    setStyleSheet("");

    setAttribute(Qt::WA_StyleSheet);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    applyingColors = false;
    refreshPending = false;
    defaultsShown = false;
    refreshDefaultColors();
    defaultPalette = palette();
    setLabelDisplay(false);

    thisPV = "";
    thisMaskPV = "";
    thisMaskValue = 0xFFFF;  // bit 0=do not display enum field; bit 1=display enum field
    nonMaskedStrings.clear();
    maskedStrings.clear();
    lastIndex = 0;
    alarmstatus= -1;

    /* certainly not, while disturbs color handling
    defBackColor = QColor(255, 248, 220, 255);
    defForeColor = Qt::black;
    */

    oldStyle="DEADBEEF";
    thisStyle="";
    thisBackColor = QColor(230,230,230);
    thisForeColor = Qt::black;
    oldBackColor = QColor(230,230,230);
    oldForeColor = Qt::black;
    thisColorMode=Default;
    oldColorMode =Alarm;
    setColorMode(Default);
    updateAlarmStatus_once_Later=false;

    setAccessW(true);
    installEventFilter(this);

    setElevation(on_top);
}

QString caMenu::getPV() const
{
    return thisPV;
}

void caMenu::setPV(QString const &newPV)
{
    thisPV = newPV;
    if(thisLabelDisplay) {
        setLabelDisplay(true);
    }
}

void caMenu::setMaskPV(QString const &newPV)
{
    thisMaskPV = newPV;
}

QString caMenu::getMaskPV() const
{
    return thisMaskPV;
}

void caMenu::setBackground(QColor c)
{
    thisBackColor = c;
    setColors(thisBackColor, thisForeColor);
}

void caMenu::setForeground(QColor c)
{
    thisForeColor = c;
    setColors(thisBackColor, thisForeColor);
}

// default colours: Base/Text as the panel palette and external stylesheets define
// them for this widget (own stylesheet cleared while reading, see themecolors.h)
void caMenu::refreshDefaultColors()
{
    applyingColors = true;            // clearing the sheet re-enters changeEvent
    ThemeColors::defaultBaseAndText(this, defBackColor, defForeColor);
    applyingColors = false;
    oldDefBackColor = oldDefForeColor = QColor();         // force a new stylesheet
    oldStyle.clear();
}

// deferred after an external palette/style change (not from inside the event)
void caMenu::applyDefaultColors()
{
    refreshPending = false;
    // re-apply what was set last (author colours, alarm or not connected) in the mode of
    // that call: a palette change must not undo the dispatcher's state
    const colMode lastMode = oldColorMode;
    const colMode mode = thisColorMode;
    refreshDefaultColors();
    thisColorMode = lastMode;
    setColors(oldBackColor, oldForeColor);
    thisColorMode = mode;
}

void caMenu::setColors(QColor bg, QColor fg)
{
    if(!defBackColor.isValid() || !defForeColor.isValid()) return;
    if(applyingColors) return;        // setStyleSheet/setPalette re-enter through changeEvent
    applyingColors = true;
    if((bg != oldBackColor) || (fg != oldForeColor) || (thisColorMode != oldColorMode)
            || (defBackColor != oldDefBackColor) || (defForeColor != oldDefForeColor)) {
        if(thisColorMode == Default) {
            thisStyle = "caMenu {background-color: rgba(%1, %2, %3, %4); color: rgba(%5, %6, %7, %8);}";
            thisStyle = thisStyle.arg(defBackColor.red()).arg(defBackColor.green()).arg(defBackColor.blue()).arg(defBackColor.alpha()).
                    arg(defForeColor.red()).arg(defForeColor.green()).arg(defForeColor.blue()).arg(defForeColor.alpha());
        } else {
            thisStyle = "caMenu {background-color: rgba(%1, %2, %3, %4); color: rgba(%5, %6, %7, %8);}";
            thisStyle = thisStyle.arg(bg.red()).arg(bg.green()).arg(bg.blue()).arg(bg.alpha()).
                    arg(fg.red()).arg(fg.green()).arg(fg.blue()).arg(fg.alpha());
            oldBackColor = bg;
            oldForeColor = fg;

        }
    }

    if(thisStyle != oldStyle || thisColorMode != oldColorMode) {
        qCDebug(caMenuLog) << "caMenu style update" << (thisStyle != oldStyle) << (thisColorMode != oldColorMode) << "(" << alarmstatus << ")" << this->objectName();
        setStyleSheet(thisStyle);
        oldStyle = thisStyle;
    }

    oldColorMode = thisColorMode;
    oldDefBackColor = defBackColor;
    oldDefForeColor = defForeColor;
    applyingColors = false;
}

// the panel palette changed (theme switch): regenerate the colours
void caMenu::changeEvent(QEvent *e)
{
    QComboBox::changeEvent(e);
    if(applyingColors || refreshPending) return;
    if(e->type() == QEvent::PaletteChange || e->type() == QEvent::StyleChange) {
        refreshPending = true;
        QTimer::singleShot(0, this, &caMenu::applyDefaultColors);
    }
}

void caMenu::setAlarmColors(short status)
{
    QColor bg, fg;
    alarmstatus= status;
    fg = thisForeColor;
    switch (status) {

    case NO_ALARM:
        bg = ThemeColors::color(caMenuColor::AlarmNoAlarm);
        break;
    case MINOR_ALARM:
        bg = ThemeColors::color(caMenuColor::AlarmMinor);
        break;
    case MAJOR_ALARM:
        bg = ThemeColors::color(caMenuColor::AlarmMajor);
        break;
    case INVALID_ALARM:
    case NOTCONNECTED:
        bg = ThemeColors::color(caMenuColor::AlarmInvalid);
        fg = ThemeColors::color(caMenuColor::AlarmInvalid);
        break;
    default:
        bg = ThemeColors::color(caMenuColor::AlarmDefault);
        fg = thisForeColor;
        break;
    }
    setColors(bg, fg);
    // it looks that depending on the generation time of the Combobox widget and the application of the
    // stylesheet, the stylesheet is not active correctly. The timer trigger the colors once a second later.
    // This is only a workaround!
    if (!updateAlarmStatus_once_Later){
        updateAlarmStatus_bg=bg;
        updateAlarmStatus_fg=fg;
        QTimer::singleShot(1000, this, SLOT(alarmrewrite()));
        updateAlarmStatus_once_Later=true;
    }
}



void caMenu::setNormalColors()
{
    setColors(thisBackColor, thisForeColor);
}

void caMenu::alarmrewrite()
{
    setColors(updateAlarmStatus_bg, updateAlarmStatus_fg);

}

QString caMenu::getLabel() const
{
    if(thisLabelDisplay) {
        if(thisPV.contains(":")) {
            QStringList list = thisPV.split(":");
            return list[0];
        } else {
            return "";
        }
    } else {
        return "";
    }
}

// this is to add a label on the menu with the first part of a device name
void caMenu::setLabelDisplay(bool c)
{
    thisLabelDisplay= c;
    clear();
    if(thisLabelDisplay) {
        addItem(getLabel());
        setCurrentIndex(0);
    }
}

void caMenu::setAccessW(int access)
{
     thisAccessW = access;
}

void caMenu::setMaskValue(const int &mask)
{
    if((nonMaskedStrings.size() > 0) && (mask != thisMaskValue)) {
        thisMaskValue = mask;
        populateCells(nonMaskedStrings);

    }
    thisMaskValue = mask;
    setIndex(lastIndex);
}

void caMenu::populateCells(QStringList stringlist)
{
    nonMaskedStrings = stringlist;
    // remove first event filter for the actual items

    QList<QWidget*> widgets = this->findChildren<QWidget*>();
    foreach (QWidget* widget, widgets) widget->removeEventFilter(this);

    clear();
    if(getLabelDisplay()) {
        addItem(getLabel());
    }

    maskedStrings.clear();
    for (int i=0; i<stringlist.size(); ++i) {
        if(((thisMaskValue >> i) & 1) == 1) maskedStrings.append(stringlist.at(i));
    }

    if(maskedStrings.size() > 0) {
        addItems(maskedStrings);
        // add now the event filter for the new items
        widgets = this->findChildren<QWidget*>();
        foreach (QWidget* widget, widgets) {
            widget->installEventFilter(this);
        }
    }
}

void caMenu::setIndex(int const &indx)
{
    if(nonMaskedStrings.size() <= 0 || indx >= nonMaskedStrings.size()) {setCurrentIndex(-1); return;}
    bool found = false;
    lastIndex = indx;
    QString currentString = nonMaskedStrings.at(indx);
    for (int i=0; i<maskedStrings.size(); i++) {
        if(currentString == maskedStrings.at(i)) {
            setCurrentIndex(i);
            found = true;
            break;
        }
    }
    if(!found) setCurrentIndex(-1);

    if(getLabelDisplay()) {
        setCurrentIndex(0);
    }
}

bool caMenu::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Enter) {
        if(!thisAccessW) {
            // disable also menu items
            for (int i=0; i < this->count(); i++) {
              QModelIndex index = model()->index(i, 0);
              QVariant v(0);   // disable flag
              model()->setData(index, v, Qt::UserRole - 1);
            }
            QApplication::setOverrideCursor(QCursor(Qt::ForbiddenCursor));

        } else {
            QApplication::restoreOverrideCursor();
        }
    } else if(event->type() == QEvent::Leave) {
        QApplication::restoreOverrideCursor();
        // enable menu items again
        for (int i=0; i < this->count(); i++) {
          QModelIndex index = model()->index(i, 0);
          QVariant v(1 | 32);  // enable flag
          model()->setData(index, v, Qt::UserRole - 1);
        }
    // move cursor with tab focus
    } else if(event->type() == QEvent::KeyRelease) {
        QKeyEvent *ev = static_cast<QKeyEvent *>(event);
        if (ev != (QKeyEvent *)0) {
            if(ev->key() == Qt::Key_Tab) {
                QCursor *cur = new QCursor;
                QPoint p = QWidget::mapToGlobal(QPoint(this->width()/2, this->height()/2));
                cur->setPos( p.x(), p.y());
                setFocus();
            }
        }
    } else if(event->type() == QEvent::Wheel) {
        event->ignore();
        return true;
    }
    return QObject::eventFilter(obj, event);
}


bool caMenu::event(QEvent *e)
{
    if(e->type() == QEvent::Show && !defaultsShown) {
        defaultsShown = true;
        applyDefaultColors();
    }
    return QComboBox::event(e);
}


