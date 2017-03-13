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

#ifndef ReplaceMacro_H
#define ReplaceMacro_H

#include <QVBoxLayout>
#include <QComboBox>
#include <QFrame>
#include <QEvent>
#include <QDebug>
#include <caLineEdit>

#include <qtcontrols_global.h>
#include "caPropHandleDefs.h"


class QTCON_EXPORT replaceMacro : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool reloadOnChange READ getReloadOnChange WRITE setReloadOnChange)
    Q_PROPERTY(bool defineInexistentMacro READ getDefineMacro WRITE setDefineMacro)
    Q_PROPERTY(colMode colorMode READ getColorMode WRITE setColorMode)
    Q_PROPERTY(QColor foreground READ getForeground WRITE setForeground)
    Q_PROPERTY(QColor background READ getBackground WRITE setBackground)

    Q_PROPERTY(Form displayMode READ getForm WRITE setForm)

    Q_PROPERTY(QString macroKey READ getMacroKey WRITE setMacroKey)

    Q_PROPERTY(QStringList macroValuesList READ getMacroValuesList WRITE setMacroValuesList DESIGNABLE isPropertyVisible(macroValuesList)  STORED false)
    Q_PROPERTY(QString macroValues READ getMacroValues WRITE setMacroValues  DESIGNABLE inactiveButVisible())

    Q_PROPERTY(QString enumChannel READ getPV WRITE setPV DESIGNABLE isPropertyVisible(channel))

    Q_PROPERTY(QString macroValue READ getMacroValue WRITE setMacroValue DESIGNABLE isPropertyVisible(macroValue))
    Q_ENUMS(Form)
    Q_ENUMS(colMode)

#include "caElevation.h"

public:
#include "caPropHandle.h"

    enum Form {Value = 0, List, Channel};
    enum Properties { macroValue = 0, macroValuesList, macroKey, channel};
    enum colMode {Default, Static};

    replaceMacro( QWidget *parent = 0 );

    colMode getColorMode() const { return thisColorMode; }
    void setColorMode(colMode colormode) {thisColorMode = colormode; setColors(thisBackColor, thisForeColor);}

    QColor getForeground() const {return thisForeColor;}
    void setForeground(QColor c);

    QColor getBackground() const {return thisBackColor;}
    void setBackground(QColor c);

    bool getReloadOnChange() const { return thisReload;}
    void setReloadOnChange(bool reload) {thisReload = reload;}

    bool getDefineMacro() const { return thisDefineMacro;}
    void setDefineMacro(bool define) {thisDefineMacro = define;}

    Form getForm() const { return thisForm; }
    void setForm(Form mode);

    QString getPV() const {return thisPV;}
    void setPV(QString const &newPV) {thisPV = newPV;}

    QString getMacroValues() const {return thisMacroValues.join(";");}
    void setMacroValues(QString const &newMacro) {thisMacroValues = newMacro.split(";");
                                                  thisMacroValuesListCount = thisMacroValues.count();
                                                  setForm(thisForm); }

    QStringList getMacroValuesList() const {return thisMacroValues;}
    void setMacroValuesList(QStringList list) {thisMacroValues = list; updatePropertyEditorItem(this, "macroValues");}

    QString getMacroValue() const {return thisMacroValue;}
    void setMacroValue(QString const &MacroValue) {thisMacroValue = MacroValue; valueToChange->setText(MacroValue);
                                                   localText=MacroValue;}

    QString getMacroKey() const {return thisMacroKey;}
    void setMacroKey(QString const &MacroKey);

    void updateCombo(QStringList keys, QStringList values);

    QString getKey() const;
    QString getNewValue() const;

    bool isPropertyVisible(Properties property);
    void setPropertyVisible(Properties property, bool visible);

    void updateValueList(QStringList values);
    QStringList getValueList() const;

    void setColors(QColor bg, QColor fg);
    void setAlarmColors(short status);
    void setNormalColors();

public slots:
    void animation(QRect p) {
#include "animationcode.h"
    }

public slots:
    void setIndex(double value);
    void setIndex(int value);
    void setMacroname(QString const &newMacro);
    void setMacrovalue(QString const &newValue);

signals:
    void reloadDisplay();

private slots:
    void reloadIt(int value) {
        Q_UNUSED(value);
        if((macroValueListCombo->currentIndex() > 0) && thisReload) {
            emit reloadDisplay();
        }
    }

protected:
     virtual bool event(QEvent *);

private:
    bool eventFilter(QObject *obj, QEvent *event);
    void reload() {emit reloadDisplay();}

    QVBoxLayout  *mainLayout;
    bool thisReload;
    bool thisDefineMacro;
    Form thisForm;
    QStringList thisMacroValues;
    QStringList comboKeys;
    QStringList comboValues;
    QString thisMacroValue;
    QString thisMacroKey;
    QComboBox *macroKeyListCombo;
    QComboBox *macroValueListCombo;
    int thisMacroValuesListCount;

    QColor thisForeColor, oldForeColor;
    QColor thisBackColor, oldBackColor;
    QColor defBackColor, defForeColor, defSelectColor;
    QPalette thisPalette;
    bool thisLabelDisplay;
    int thisAccessW;
    colMode  thisColorMode, oldColorMode;
    QPalette defaultPalette;
    QString thisStyle, oldStyle;

    caLineEdit *valueToChange;
    QString localText;
    bool designerVisible[10];
    QString thisPV;
    bool isShown;
};
#endif
