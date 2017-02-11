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

#include "replacemacro.h"
#include "alarmdefs.h"
#include <QApplication>
#include <QMouseEvent>
#include <QToolTip>

replaceMacro::replaceMacro(QWidget *parent) : QWidget(parent)
{
    setStyleSheet("");

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    isShown = false;
    defaultPalette = palette();

    thisBackColor = QColor(230,230,230);
    thisForeColor = Qt::black;
    oldBackColor = Qt::black;
    oldForeColor = QColor(230,230,230);
    thisColorMode=Default;
    oldColorMode =Default;
    setColorMode(Default);
    thisStyle = oldStyle="";

    installEventFilter(this);
    this->setAcceptDrops(false);

    setElevation(on_top);
    setReloadOnChange(false);
    setDefineMacro(false);
    thisPV = "";
    localText = "";
    thisMacroValuesListCount = 0;

    macroKeyListCombo = new QComboBox(this);
    valueToChange = new caLineEdit(this);
    macroValueListCombo = new QComboBox(this);
    macroValueListCombo->setMinimumSize(2,2); //important for resizing as small as possible
    macroKeyListCombo->setMinimumSize(2,2);
    valueToChange->setMinimumSize(2,2);

    connect(macroValueListCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(reloadIt(int)));

    mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);
    setLayout(mainLayout);

    mainLayout->addWidget(macroKeyListCombo);
    mainLayout->addWidget(valueToChange);
    mainLayout->addWidget(macroValueListCombo);

    setForm(Value);
}

QString replaceMacro::getKey() const {
    if(thisForm == Value) {
        return comboKeys.at(macroKeyListCombo->currentIndex());
    } else if(thisForm == List || thisForm == Channel) {
        return getMacroKey();
    } else {
        return "";
    }
}

QString replaceMacro::getNewValue() const {
    if(thisForm == Value) {
        return localText;
    } else if(thisForm == List || thisForm == Channel) {
        return macroValueListCombo->currentText();
    } else {
        return "";
    }
}

bool replaceMacro::isPropertyVisible(Properties property)
{
    return designerVisible[property];
}

void replaceMacro::setPropertyVisible(Properties property, bool visible)
{
    designerVisible[property] = visible;
}


// slots
void replaceMacro::setIndex(double value) {
    if(thisForm == List || thisForm == Channel) {
        if(((int) value < macroValueListCombo->count()) && ((int) value >= 0)) macroValueListCombo->setCurrentIndex((int) value);
    } else if(thisForm == Value) {
        if(((int) value < macroKeyListCombo->count()) && ((int) value >= 0)) macroKeyListCombo->setCurrentIndex((int) value);
    }
}

void replaceMacro::setIndex(int value) {
    if(thisForm == List) {
        if(((int) value < macroValueListCombo->count()) && ((int) value >= 0)) macroValueListCombo->setCurrentIndex((int) value);
    } else if(thisForm == Value) {
        if(((int) value < macroKeyListCombo->count()) && ((int) value >= 0)) macroKeyListCombo->setCurrentIndex((int) value);
    }
}

void replaceMacro::setMacroname(QString const &newMacro) {
    setMacroKey(newMacro);
}

void replaceMacro::setMacrovalue(QString const &newValue) {
    setMacroValue(newValue);
    if(thisMacroValues.indexOf(newValue) == -1) {
       //printf("counts %d %d\n", thisMacroValues.count(), thisMacroValuesListCount);
       if(thisMacroValues.count() == thisMacroValuesListCount) {
            //printf("add an item\n");
            macroValueListCombo->addItem(newValue);
            thisMacroValues.append(newValue);

       } else {
           //printf("replace item at %d\n", thisMacroValuesListCount);
           thisMacroValues.replace(thisMacroValuesListCount, newValue);
           macroValueListCombo->setItemText(thisMacroValuesListCount, newValue);
           macroValueListCombo->setCurrentIndex(thisMacroValuesListCount);
           thisMacroValues.replace(thisMacroValuesListCount, newValue);
       }
    }
}

void replaceMacro::setMacroKey(QString const &MacroKey) {
    int indx;
    thisMacroKey = MacroKey;
    //for(int i=0; i< comboKeys.count(); i++) printf("comboKey=%s\n", qasc(comboKeys.at(i)));
    //for(int i=0; i< comboValues.count(); i++) printf("value=%s\n",  qasc(comboValues.at(i)));
    if((indx = comboKeys.indexOf(MacroKey)) != -1) {
        macroValueListCombo->setToolTip("<p style=\"color:#000000; background-color:#ffff00; white-space:pre;\">macro=" + MacroKey + " will get value from this combobox</p>");
    } else if(getDefineMacro()) {
        macroValueListCombo->setToolTip("<p style=\"color:#000000; background-color:#ffff00; white-space:pre;\" macro=" + MacroKey + " will be defined with the value from this combobox</p>");
    } else {
        macroValueListCombo->setToolTip("<p style=\"color:#000000; background-color:#ffff00; white-space:pre;\" macro=" + MacroKey + " will not be defined while not specified</p>");
    }
}

// setform
void replaceMacro::setForm(Form form)
{
    thisForm = form;

    //printf("list %s\n", qasc(thisMacroValues.join(";")));

    setPropertyVisible(macroValuesList, false);
    setPropertyVisible(macroValue, false);
    setPropertyVisible(macroKey, true);
    setPropertyVisible(channel, false);

    switch (form) {
    case Value:
        setPropertyVisible(macroValue, true);
        macroKeyListCombo->setVisible(true);
        valueToChange->setVisible(true);
        macroValueListCombo->setVisible(false);

        valueToChange->setBaseSize(100,12);
        valueToChange->clearFocus();
        valueToChange->setReadOnly(false);
        valueToChange->setEnabled(true);
        valueToChange->setFocusPolicy(Qt::StrongFocus);
        break;

    case List:
        setPropertyVisible(macroValuesList, true);
        macroKeyListCombo->setVisible(false);
        valueToChange->setVisible(false);
        macroValueListCombo->setVisible(true);

        if(thisMacroValues.count() > 0) {
            if(thisMacroValues.at(0).size() != 0) thisMacroValues.prepend("");
        }
        macroValueListCombo->clear();
        macroValueListCombo->addItems(thisMacroValues);
        break;

    case Channel:
        setPropertyVisible(channel, true);
        macroKeyListCombo->setVisible(false);
        valueToChange->setVisible(false);
        macroValueListCombo->setVisible(true);
        break;
    }

    thisMacroValuesListCount = macroValueListCombo->count();
    update();
}

void replaceMacro::updateValueList(QStringList values) {
    int indx;
    disconnect(macroValueListCombo, SIGNAL(currentIndexChanged(int)), 0, 0);
    thisMacroValues = values;
    if(thisMacroValues.count() > 0) {
        if(thisMacroValues.at(0).size() != 0) thisMacroValues.prepend("");
    }
    macroValueListCombo->clear();
    macroValueListCombo->addItems(thisMacroValues);

    //set index correctly
    for(int i=0; i<qMin(comboKeys.count(), comboValues.count()); ++i) {
        if((indx = thisMacroValues.indexOf(comboValues.at(i))) != 0 && comboKeys.at(i) == getMacroKey()) {
            if(indx < macroValueListCombo->count()) macroValueListCombo->setCurrentIndex(indx);
        }
    }
    connect(macroValueListCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(reloadIt(int)));
}

QStringList replaceMacro::getValueList() const {
    QStringList values = thisMacroValues;
    values.removeAt(0); // take of blanc line
    return values;
}

void replaceMacro::updateCombo(QStringList keys, QStringList values)
{
    int indx;
    int index = -1;
    comboKeys = keys;
    comboValues = values;
    macroKeyListCombo->clear();

    // fill keylist and values list
    for(int i=0; i<qMin(keys.count(), values.count()); ++i) {
        QString item = "";
        if(getMacroKey() == keys.at(i)) {
            index = i;
            if(getMacroValue().length() > 0) {
                item = keys.at(i) + ": " + getMacroValue();
            } else {
                item = keys.at(i) + ": " + values.at(i);
            }
        } else {
            item = keys.at(i) + ": " + values.at(i);
        }
        macroKeyListCombo->addItem(item);

        // when not in values list given at designer time, then add it anyway
        if((indx = thisMacroValues.indexOf(values.at(i))) == -1 && keys.at(i) == getMacroKey()) {
            //printf("for %s value %s not in list of macrovalues\n", qasc(keys.at(i)), qasc(values.at(i)));
            macroValueListCombo->addItem(values.at(i));
            macroValueListCombo->setCurrentIndex(macroValueListCombo->count()-1);
            thisMacroValues.append(values.at(i));
            thisMacroValuesListCount = macroValueListCombo->count();
        } else if((indx = thisMacroValues.indexOf(values.at(i))) != 0 && keys.at(i) == getMacroKey()) {
            if(indx < macroValueListCombo->count()) macroValueListCombo->setCurrentIndex(indx);
        }

    }

    // found key, set index to it
    if(index != -1) {
        macroKeyListCombo->setCurrentIndex(index);

    // key not found, add it when it was specified and set index
    } else if(getMacroKey().length() > 0 && getDefineMacro()) {
        macroKeyListCombo->addItem(getMacroKey() + ":" + getMacroValue());
        macroKeyListCombo->setCurrentIndex(macroKeyListCombo->count()-1);
        comboKeys.append(getMacroKey());
    }

    setMacroKey(thisMacroKey);
}

bool replaceMacro::eventFilter(QObject *obj, QEvent *event)
{
    // repeat enter or return key are not really wanted
    if (event->type() == QEvent::KeyPress){
        QKeyEvent *ev = static_cast<QKeyEvent *>(event);
        if (ev != (QKeyEvent *)0) {
            // when return pressed, set text and reload
            if (ev->key() == Qt::Key_Return || ev->key() == Qt::Key_Enter) {
                if (ev->isAutoRepeat()) {
                    //printf("keyPressEvent ignore\n");
                    event->ignore();
                }
                else {
                    event->accept();
                    localText =valueToChange->text();
                    if(thisReload) reload();
                }
            }
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
    }

    // treat mouse enter and leave as well as focus out
    if (event->type() == QEvent::Enter) {
        this->activateWindow();  // I added this for ios while I could not get the focus
    } else if(event->type() == QEvent::Leave) {
        QApplication::restoreOverrideCursor();
        localText = valueToChange->text();
        clearFocus();
    } else if(event->type() == QEvent::FocusOut) {
        localText = valueToChange->text();
    } else if (event->type() == QEvent::FocusIn) {
    }
    return QObject::eventFilter(obj, event);
}

void replaceMacro::setBackground(QColor c)
{
    thisBackColor = c;
    setColors(thisBackColor, thisForeColor);
}

void replaceMacro::setForeground(QColor c)
{
    thisForeColor = c;
    setColors(thisBackColor, thisForeColor);
}

void replaceMacro::setColors(QColor bg, QColor fg)
{
    if(!defBackColor.isValid() || !defForeColor.isValid()) return;
    if((bg != oldBackColor) || (fg != oldForeColor) || (thisColorMode != oldColorMode)) {
        if(thisColorMode == Default) {
            thisStyle = "QComboBox {background-color: rgba(%1, %2, %3, %4); color: rgba(%5, %6, %7, %8);}";
            thisStyle = thisStyle.arg(defBackColor.red()).arg(defBackColor.green()).arg(defBackColor.blue()).arg(defBackColor.alpha()).
                    arg(defForeColor.red()).arg(defForeColor.green()).arg(defForeColor.blue()).arg(defForeColor.alpha());

        } else {
            thisStyle = "QComboBox {background-color: rgba(%1, %2, %3, %4); color: rgba(%5, %6, %7, %8);}";
            thisStyle = thisStyle.arg(bg.red()).arg(bg.green()).arg(bg.blue()).arg(bg.alpha()).
                    arg(fg.red()).arg(fg.green()).arg(fg.blue()).arg(fg.alpha());
            oldBackColor = bg;
            oldForeColor = fg;
        }
    }

    if(thisStyle != oldStyle || thisColorMode != oldColorMode) {
        macroValueListCombo->setStyleSheet(thisStyle);
        macroKeyListCombo->setStyleSheet(thisStyle);
        oldStyle = thisStyle;
        update();
    }
    oldColorMode = thisColorMode;
}

void replaceMacro::setAlarmColors(short status)
{
    QColor bg, fg;
    fg = thisForeColor;
    switch (status) {

    case NO_ALARM:
        bg = AL_GREEN;
        break;
    case MINOR_ALARM:
        bg = AL_YELLOW;
        break;
    case MAJOR_ALARM:
        bg = AL_RED;
        break;
    case INVALID_ALARM:
    case NOTCONNECTED:
        bg = AL_WHITE;
        fg = AL_WHITE;
        break;
    default:
        bg = AL_DEFAULT;
        fg = thisForeColor;
        break;
    }
    colMode aux = thisColorMode;
    thisColorMode = Static;
    setColors(bg, fg);
    thisColorMode = aux;
}

void replaceMacro::setNormalColors()
{
    setColors(thisBackColor, thisForeColor);
}

bool replaceMacro::event(QEvent *e)
{
    if(e->type() == QEvent::Resize || e->type() == QEvent::Show) {
        // we try to get the default color for the background set through the external stylesheets
        if(!isShown) {
          setStyleSheet("");
          QString c=  palette().color(QPalette::Base).name();
          defBackColor = QColor(c);
          //printf("default back color %s %s\n", qasc(c), qasc(this->objectName()));
          c=  palette().color(QPalette::Text).name();
          defForeColor = QColor(c);
          //printf("default fore color %s %s\n", qasc(c), qasc(this->objectName()));

          if(!defBackColor.isValid()) defBackColor = QColor(255, 248, 220, 255);
          if(!defForeColor.isValid()) defForeColor = Qt::black;

          setColors(thisBackColor, thisForeColor);
          isShown = true;
        }
    }
    return QWidget::event(e);
}
