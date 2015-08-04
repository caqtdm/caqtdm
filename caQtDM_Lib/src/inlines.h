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
    if(caApplyNumeric* caapplynumericWidget = qobject_cast<caApplyNumeric *>(w)) caapplynumericWidget->setMaxValue(v);
    else if(caNumeric* canumericWidget = qobject_cast<caNumeric *>(w)) canumericWidget->setMaxValue(v);
    else if(caSpinbox* caspinboxWidget = qobject_cast<caSpinbox *>(w)) caspinboxWidget->setMaxValue(v);
    else if(caSlider* casliderWidget = qobject_cast<caSlider *>(w)) casliderWidget->setMaxValue(v);
    else if(caThermo* cathermoWidget = qobject_cast<caThermo *>(w)) cathermoWidget->setMaxValue(v);
    else if(caLineEdit* calineeditWidget = qobject_cast<caLineEdit *>(w)) calineeditWidget->setMaxValue(v);
    else if(EAbstractGauge* abstractgaugeWidget = qobject_cast<EAbstractGauge *>(w)) abstractgaugeWidget->setMaxValue(v);
    else if(caTable* tableWidget = qobject_cast<caTable *>(w)) tableWidget->setMaxValue(v);
    else printf("caQtDM -- setMaxValueA not implemented\n");
}

inline void setMinValueA(QWidget *w, double &v) {
    if(caApplyNumeric* caapplynumericWidget = qobject_cast<caApplyNumeric *>(w)) caapplynumericWidget->setMinValue(v);
    else if(caNumeric* canumericWidget = qobject_cast<caNumeric *>(w)) canumericWidget->setMinValue(v);
    else if(caSpinbox* caspinboxWidget = qobject_cast<caSpinbox *>(w)) caspinboxWidget->setMinValue(v);
    else if(caSlider* casliderWidget = qobject_cast<caSlider *>(w)) casliderWidget->setMinValue(v);
    else if(caThermo* cathermoWidget = qobject_cast<caThermo *>(w)) cathermoWidget->setMinValue(v);
    else if(caLineEdit* calineeditWidget = qobject_cast<caLineEdit *>(w)) calineeditWidget->setMinValue(v);
    else if(EAbstractGauge* abstractgaugeWidget = qobject_cast<EAbstractGauge *>(w)) abstractgaugeWidget->setMinValue(v);
    else if(caTable* tableWidget = qobject_cast<caTable *>(w)) tableWidget->setMinValue(v);
    else printf("caQtDM -- setMinValueA not implemented\n");
}

// limitsmode
inline void setLimitsModeChannel(QWidget *w) {
    if(caApplyNumeric* caapplynumericWidget = qobject_cast<caApplyNumeric *>(w))  caapplynumericWidget->setLimitsMode(caApplyNumeric::Channel);
    else if(caNumeric* canumericWidget = qobject_cast<caNumeric *>(w))  canumericWidget->setLimitsMode(caNumeric::Channel);
    else if(caSpinbox* caspinboxWidget = qobject_cast<caSpinbox *>(w)) caspinboxWidget->setLimitsMode(caSpinbox::Channel);
    else if(caSlider* casliderWidget = qobject_cast<caSlider *>(w)) casliderWidget->setLimitsMode(caSlider::Channel);
    else if(caThermo* cathermoWidget = qobject_cast<caThermo *>(w)) cathermoWidget->setLimitsMode(caThermo::Channel);
    else if(caLineEdit* calineeditWidget = qobject_cast<caLineEdit *>(w)) calineeditWidget->setLimitsMode(caLineEdit::Channel);
    else if(caTable* tableWidget = qobject_cast<caTable *>(w)) tableWidget->setLimitsMode(caTable::Channel);
    else printf("caQtDM -- setLimitsModeChannel not implemented\n");
}
inline void setLimitsModeUser(QWidget *w) {
    if(caApplyNumeric* caapplynumericWidget = qobject_cast<caApplyNumeric *>(w))  caapplynumericWidget->setLimitsMode(caApplyNumeric::User);
    else if(caNumeric* canumericWidget = qobject_cast<caNumeric *>(w))  canumericWidget->setLimitsMode(caNumeric::User);
    else if(caSpinbox* caspinboxWidget = qobject_cast<caSpinbox *>(w)) caspinboxWidget->setLimitsMode(caSpinbox::User);
    else if(caSlider* casliderWidget = qobject_cast<caSlider *>(w)) casliderWidget->setLimitsMode(caSlider::User);
    else if(caThermo* cathermoWidget = qobject_cast<caThermo *>(w)) cathermoWidget->setLimitsMode(caThermo::User);
    else if(caLineEdit* calineeditWidget = qobject_cast<caLineEdit *>(w)) calineeditWidget->setLimitsMode(caLineEdit::User);
    else if(caTable* catableWidget = qobject_cast<caTable *>(w)) catableWidget->setLimitsMode(caTable::User);
    else printf("caQtDM -- setLimitsModeUser not implemented\n");
}

// precisionmode
inline void setPrecisionModeChannel(QWidget *w) {
    if(caApplyNumeric* caapplynumericWidget = qobject_cast<caApplyNumeric *>(w))  caapplynumericWidget->setPrecisionMode(caApplyNumeric::Channel);
    else if(caNumeric* canumericWidget = qobject_cast<caNumeric *>(w))  canumericWidget->setPrecisionMode(caNumeric::Channel);
    else if(caSpinbox* caspinboxWidget = qobject_cast<caSpinbox *>(w)) caspinboxWidget->setPrecisionMode(caSpinbox::Channel);
    else if(caLineEdit* calineeditWidget = qobject_cast<caLineEdit *>(w)) calineeditWidget->setPrecisionMode(caLineEdit::Channel);
    else if(caTable* catableWidget = qobject_cast<caTable *>(w)) catableWidget->setPrecisionMode(caTable::Channel);
    else printf("caQtDM -- setPrecisionModeChannel not implemented\n");
}
inline void setPrecisionModeUser(QWidget *w) {
    if(caApplyNumeric* caapplynumericWidget = qobject_cast<caApplyNumeric *>(w))  caapplynumericWidget->setPrecisionMode(caApplyNumeric::User);
    else if(caNumeric* canumericWidget = qobject_cast<caNumeric *>(w))  canumericWidget->setPrecisionMode(caNumeric::User);
    else if(caSpinbox* caspinboxWidget = qobject_cast<caSpinbox *>(w)) caspinboxWidget->setPrecisionMode(caSpinbox::User);
    else if(caLineEdit* calineeditWidget = qobject_cast<caLineEdit *>(w)) calineeditWidget->setPrecisionMode(caLineEdit::User);
    else if(caTable* catableWidget = qobject_cast<caTable *>(w)) catableWidget->setPrecisionMode(caTable::User);
    else printf("caQtDM -- setPrecisionModeUser not implemented\n");
}

// format and digits for spinbox and numeric
inline void setDecDigitsA(QWidget *w, int v) {
    if(caApplyNumeric* caapplynumericWidget = qobject_cast<caApplyNumeric *>(w))  caapplynumericWidget->setDecDigits(v);
    else if(caNumeric* canumericWidget = qobject_cast<caNumeric *>(w))  canumericWidget->setDecDigits(v);
    else if(caSpinbox* caspinboxWidget = qobject_cast<caSpinbox *>(w)) caspinboxWidget->setDecDigits(v);
    else printf("caQtDM -- setDecDigitsA not implemented\n");
}

inline void setIntDigitsA(QWidget *w, int &v) {
    if(caApplyNumeric* caapplynumericWidget = qobject_cast<caApplyNumeric *>(w))  caapplynumericWidget->setIntDigits(v);
    else if(caNumeric* canumericWidget = qobject_cast<caNumeric *>(w))  canumericWidget->setIntDigits(v);
    else if(caSpinbox* caspinboxWidget = qobject_cast<caSpinbox *>(w)) caspinboxWidget->setIntDigits(v);
    else printf("caQtDM -- setIntDigitsA not implemented\n");
}

inline void setFixedFormatA(QWidget *w, bool v) {
    if(caApplyNumeric* caapplynumericWidget = qobject_cast<caApplyNumeric *>(w))  caapplynumericWidget->setFixedFormat(v);
    else if(caNumeric* canumericWidget = qobject_cast<caNumeric *>(w))  canumericWidget->setFixedFormat(v);
    else if(caSpinbox* caspinboxWidget = qobject_cast<caSpinbox *>(w)) caspinboxWidget->setFixedFormat(v);
    else printf("caQtDM -- setFixedFormatA not implemented\n");
}


#endif // INLINES_H
