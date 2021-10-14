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

#ifndef PVDIALOG_H
#define PVDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <qtcontrols_global.h>
#include <QtControls>
#include "JSON.h"
#include "JSONValue.h"

#define MAXPVLEN 120

QT_BEGIN_NAMESPACE
class QDialogButtonBox;
QT_END_NAMESPACE

//! [0]
class QTCON_EXPORT PVDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PVDialog(QWidget *plugin = 0, QWidget *parent = 0);

    QSize sizeHint() const;

private slots:
    void saveState();

private:

    void print_out(const wchar_t *output);
    wchar_t* converToWChar_t(QString text);

    QWidget *thisWidget;
    QCheckBox *dbndCheckBox, *arrayCheckBox, *syncCheckBox, *rateCheckBox, *tsCheckBox, *decCheckBox;
    QLabel    *pvLabel, *dbndLabel, *arrayLabel, *syncLabel, *rateLabel, *decLabel;
    QLabel    *arrayLabel_s, *arrayLabel_i, *arrayLabel_e, *tsLabel, *prefixLabel;
    QLabel    *remarkLabel;
    QSpinBox *arrayIntValue_s, *arrayIntValue_i, *arrayIntValue_e;
    QLineEdit *pvLine, *syncLine, *msgLine;
    QComboBox *dbndComboBox, *syncComboBox, *prefixComboBox;
    QDoubleSpinBox *dbndDoubleValue;
    QSpinBox *rateIntValue, *decIntValue;

    QDialogButtonBox *buttonBox;

    QWidget *entry;

};
//! [0]

#endif
