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


#ifndef INLINES_H
#define INLINES_H

#include "caslider.h"
#include "calineedit.h"
#include "cathermo.h"
#include "catextentry.h"
#include "canumeric.h"
#include "caapplynumeric.h"
#include "caspinbox.h"
#include "cagauge.h"
#include "catable.h"

// these functions are used in order to simplifie mainly caNumeric, caSpinBox and caApplyNumeric code of the limitsdialog

// max, min values
inline void setMaxValueA(QWidget *w, double &v) {
    if(caApplyNumeric* widget = qobject_cast<caApplyNumeric *>(w)) widget->setMaxValue(v);
    else if(caNumeric* widget = qobject_cast<caNumeric *>(w)) widget->setMaxValue(v);
    else if(caSpinbox* widget = qobject_cast<caSpinbox *>(w)) widget->setMaxValue(v);
    else if(caSlider* widget = qobject_cast<caSlider *>(w)) widget->setMaxValue(v);
    else if(caThermo* widget = qobject_cast<caThermo *>(w)) widget->setMaxValue(v);
    else if(caLineEdit* widget = qobject_cast<caLineEdit *>(w)) widget->setMaxValue(v);
    else if(EAbstractGauge* widget = qobject_cast<EAbstractGauge *>(w)) widget->setMaxValue(v);
    else if(caTable* widget = qobject_cast<caTable *>(w)) widget->setMaxValue(v);
    else printf("caQtDM -- setMaxValueA not implemented\n");
}

inline void setMinValueA(QWidget *w, double &v) {
    if(caApplyNumeric* widget = qobject_cast<caApplyNumeric *>(w)) widget->setMinValue(v);
    else if(caNumeric* widget = qobject_cast<caNumeric *>(w)) widget->setMinValue(v);
    else if(caSpinbox* widget = qobject_cast<caSpinbox *>(w)) widget->setMinValue(v);
    else if(caSlider* widget = qobject_cast<caSlider *>(w)) widget->setMinValue(v);
    else if(caThermo* widget = qobject_cast<caThermo *>(w)) widget->setMinValue(v);
    else if(caLineEdit* widget = qobject_cast<caLineEdit *>(w)) widget->setMinValue(v);
    else if(EAbstractGauge* widget = qobject_cast<EAbstractGauge *>(w)) widget->setMinValue(v);
    else if(caTable* widget = qobject_cast<caTable *>(w)) widget->setMinValue(v);
    else printf("caQtDM -- setMinValueA not implemented\n");
}

// limitsmode
inline void setLimitsModeChannel(QWidget *w) {
    if(caApplyNumeric* widget = qobject_cast<caApplyNumeric *>(w))  widget->setLimitsMode(caApplyNumeric::Channel);
    else if(caNumeric* widget = qobject_cast<caNumeric *>(w))  widget->setLimitsMode(caNumeric::Channel);
    else if(caSpinbox* widget = qobject_cast<caSpinbox *>(w)) widget->setLimitsMode(caSpinbox::Channel);
    else if(caSlider* widget = qobject_cast<caSlider *>(w)) widget->setLimitsMode(caSlider::Channel);
    else if(caThermo* widget = qobject_cast<caThermo *>(w)) widget->setLimitsMode(caThermo::Channel);
    else if(caLineEdit* widget = qobject_cast<caLineEdit *>(w)) widget->setLimitsMode(caLineEdit::Channel);
    else if(caTable* widget = qobject_cast<caTable *>(w)) widget->setLimitsMode(caTable::Channel);
    else printf("caQtDM -- setLimitsModeChannel not implemented\n");
}
inline void setLimitsModeUser(QWidget *w) {
    if(caApplyNumeric* widget = qobject_cast<caApplyNumeric *>(w))  widget->setLimitsMode(caApplyNumeric::User);
    else if(caNumeric* widget = qobject_cast<caNumeric *>(w))  widget->setLimitsMode(caNumeric::User);
    else if(caSpinbox* widget = qobject_cast<caSpinbox *>(w)) widget->setLimitsMode(caSpinbox::User);
    else if(caSlider* widget = qobject_cast<caSlider *>(w)) widget->setLimitsMode(caSlider::User);
    else if(caThermo* widget = qobject_cast<caThermo *>(w)) widget->setLimitsMode(caThermo::User);
    else if(caLineEdit* widget = qobject_cast<caLineEdit *>(w)) widget->setLimitsMode(caLineEdit::User);
    else if(caTable* widget = qobject_cast<caTable *>(w)) widget->setLimitsMode(caTable::User);
    else printf("caQtDM -- setLimitsModeUser not implemented\n");
}

// precisionmode
inline void setPrecisionModeChannel(QWidget *w) {
    if(caApplyNumeric* widget = qobject_cast<caApplyNumeric *>(w))  widget->setPrecisionMode(caApplyNumeric::Channel);
    else if(caNumeric* widget = qobject_cast<caNumeric *>(w))  widget->setPrecisionMode(caNumeric::Channel);
    else if(caSpinbox* widget = qobject_cast<caSpinbox *>(w)) widget->setPrecisionMode(caSpinbox::Channel);
    else if(caLineEdit* widget = qobject_cast<caLineEdit *>(w)) widget->setPrecisionMode(caLineEdit::Channel);
    else if(caTable* widget = qobject_cast<caTable *>(w)) widget->setPrecisionMode(caTable::Channel);
    else printf("caQtDM -- setPrecisionModeChannel not implemented\n");
}
inline void setPrecisionModeUser(QWidget *w) {
    if(caApplyNumeric* widget = qobject_cast<caApplyNumeric *>(w))  widget->setPrecisionMode(caApplyNumeric::User);
    else if(caNumeric* widget = qobject_cast<caNumeric *>(w))  widget->setPrecisionMode(caNumeric::User);
    else if(caSpinbox* widget = qobject_cast<caSpinbox *>(w)) widget->setPrecisionMode(caSpinbox::User);
    else if(caLineEdit* widget = qobject_cast<caLineEdit *>(w)) widget->setPrecisionMode(caLineEdit::User);
    else if(caTable* widget = qobject_cast<caTable *>(w)) widget->setPrecisionMode(caTable::User);
    else printf("caQtDM -- setPrecisionModeUser not implemented\n");
}

// format and digits for spinbox and numeric
inline void setDecDigitsA(QWidget *w, int v) {
    if(caApplyNumeric* widget = qobject_cast<caApplyNumeric *>(w))  widget->setDecDigits(v);
    else if(caNumeric* widget = qobject_cast<caNumeric *>(w))  widget->setDecDigits(v);
    else if(caSpinbox* widget = qobject_cast<caSpinbox *>(w)) widget->setDecDigits(v);
    else printf("caQtDM -- setDecDigitsA not implemented\n");
}

inline void setIntDigitsA(QWidget *w, int &v) {
    if(caApplyNumeric* widget = qobject_cast<caApplyNumeric *>(w))  widget->setIntDigits(v);
    else if(caNumeric* widget = qobject_cast<caNumeric *>(w))  widget->setIntDigits(v);
    else if(caSpinbox* widget = qobject_cast<caSpinbox *>(w)) widget->setIntDigits(v);
    else printf("caQtDM -- setIntDigitsA not implemented\n");
}

inline void setFixedFormatA(QWidget *w, bool v) {
    if(caApplyNumeric* widget = qobject_cast<caApplyNumeric *>(w))  widget->setFixedFormat(v);
    else if(caNumeric* widget = qobject_cast<caNumeric *>(w))  widget->setFixedFormat(v);
    else if(caSpinbox* widget = qobject_cast<caSpinbox *>(w)) widget->setFixedFormat(v);
    else printf("caQtDM -- setFixedFormatA not implemented\n");
}


#endif // INLINES_H
