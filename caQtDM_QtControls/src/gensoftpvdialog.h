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
 *  Copyright (c) 2010 - 2026
 *
 *  Author:
 *    Helge Brands
 *  Contact details:
 *    helge.brands@psi.ch
 */

#ifndef GENSOFTPVDIALOG_H
#define GENSOFTPVDIALOG_H

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QSpinBox>

#include <qtcontrols_global.h>

class genSoftPV;

// designer dialog ("Edit soft PV...") writing the values back into the genSoftPV properties
class QTCON_EXPORT GenSoftPVDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GenSoftPVDialog(QWidget *widget = 0, QWidget *parent = 0);

private slots:
    void saveState();

private:
    QWidget *entry;

    QLineEdit *variableLine;
    QComboBox *dataTypeCombo, *modeCombo;
    QLineEdit *valueLine;
    QDoubleSpinBox *stepValue;
    QSpinBox *periodValue;
    QLineEdit *drvlLine, *drvhLine, *hoprLine, *loprLine, *lowLine, *loloLine, *highLine, *hihiLine;
    QCheckBox *overflowCheckBox, *persistentCheckBox;
    QSpinBox *nelmValue, *nordValue, *matrixRowsValue, *matrixColumnsValue;
    QLineEdit *unitsLine;
    QSpinBox *precisionValue;
    QLineEdit *enumStringsLine, *regexLine;

    QDialogButtonBox *buttonBox;
};

#endif // GENSOFTPVDIALOG_H
