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

#include <QtDesigner/QtDesigner>
#include <QFormLayout>
#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>

#include "gensoftpvdialog.h"
#include "gensoftpv.h"

GenSoftPVDialog::GenSoftPVDialog(QWidget *widget, QWidget *parent) : QDialog(parent)
{
    entry = widget;
    genSoftPV *softpv = qobject_cast<genSoftPV *>(widget);

    setWindowTitle(tr("edit generic soft PV for the internal plugin"));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // channel definition
    QGroupBox *channelGroup = new QGroupBox(tr("channel"), this);
    QFormLayout *channelForm = new QFormLayout(channelGroup);

    variableLine = new QLineEdit(this);
    variableLine->setToolTip(tr("channel name, referenced by other widgets as internal://<variable>"));
    channelForm->addRow(tr("variable:"), variableLine);

    dataTypeCombo = new QComboBox(this);
    // order must match genSoftPV::DataType, the mapping goes by index
    dataTypeCombo->addItems(QStringList() << "double" << "float" << "int" << "long"
                                          << "enum" << "string" << "char");
    channelForm->addRow(tr("data type:"), dataTypeCombo);

    modeCombo = new QComboBox(this);
    modeCombo->addItems(QStringList() << "constant" << "counter");
    channelForm->addRow(tr("mode:"), modeCombo);

    valueLine = new QLineEdit(this);
    valueLine->setToolTip(tr("initial value (VAL): scalar, text or ';' separated list for waveforms"));
    channelForm->addRow(tr("value:"), valueLine);

    stepValue = new QDoubleSpinBox(this);
    stepValue->setRange(-1.0e12, 1.0e12);
    stepValue->setDecimals(6);
    stepValue->setValue(1.0);
    channelForm->addRow(tr("step:"), stepValue);

    periodValue = new QSpinBox(this);
    periodValue->setRange(10, 3600000);
    periodValue->setValue(1000);
    periodValue->setSuffix(" ms");
    channelForm->addRow(tr("period:"), periodValue);

    overflowCheckBox = new QCheckBox(tr("overflow (wrap at the drive limits)"), this);
    overflowCheckBox->setChecked(true);
    channelForm->addRow(QString(), overflowCheckBox);

    persistentCheckBox = new QCheckBox(tr("persistent (keeps running without any monitor)"), this);
    channelForm->addRow(QString(), persistentCheckBox);

    mainLayout->addWidget(channelGroup);

    // limits, EPICS field names
    QGroupBox *limitsGroup = new QGroupBox(tr("limits (empty = not defined)"), this);
    QGridLayout *limitsGrid = new QGridLayout(limitsGroup);

    drvlLine = new QLineEdit(this);
    drvhLine = new QLineEdit(this);
    loprLine = new QLineEdit(this);
    hoprLine = new QLineEdit(this);
    lowLine = new QLineEdit(this);
    loloLine = new QLineEdit(this);
    highLine = new QLineEdit(this);
    hihiLine = new QLineEdit(this);

    QString oprTip = tr("operating range, clamped into the DRVL/DRVH range; limits widget input");
    loprLine->setToolTip(oprTip);
    hoprLine->setToolTip(oprTip);

    limitsGrid->addWidget(new QLabel(tr("DRVL:"), this), 0, 0);
    limitsGrid->addWidget(drvlLine, 0, 1);
    limitsGrid->addWidget(new QLabel(tr("DRVH:"), this), 0, 2);
    limitsGrid->addWidget(drvhLine, 0, 3);
    limitsGrid->addWidget(new QLabel(tr("LOPR:"), this), 1, 0);
    limitsGrid->addWidget(loprLine, 1, 1);
    limitsGrid->addWidget(new QLabel(tr("HOPR:"), this), 1, 2);
    limitsGrid->addWidget(hoprLine, 1, 3);
    limitsGrid->addWidget(new QLabel(tr("LOW:"), this), 2, 0);
    limitsGrid->addWidget(lowLine, 2, 1);
    limitsGrid->addWidget(new QLabel(tr("HIGH:"), this), 2, 2);
    limitsGrid->addWidget(highLine, 2, 3);
    limitsGrid->addWidget(new QLabel(tr("LOLO:"), this), 3, 0);
    limitsGrid->addWidget(loloLine, 3, 1);
    limitsGrid->addWidget(new QLabel(tr("HIHI:"), this), 3, 2);
    limitsGrid->addWidget(hihiLine, 3, 3);

    mainLayout->addWidget(limitsGroup);

    // array and presentation options
    QGroupBox *optionsGroup = new QGroupBox(tr("array / presentation"), this);
    QFormLayout *optionsForm = new QFormLayout(optionsGroup);

    nelmValue = new QSpinBox(this);
    nelmValue->setRange(1, 1000000);
    nelmValue->setToolTip(tr("NELM: maximum array size"));
    optionsForm->addRow(tr("nelm:"), nelmValue);

    nordValue = new QSpinBox(this);
    nordValue->setRange(-1, 1000000);
    nordValue->setSpecialValueText(tr("auto"));
    nordValue->setValue(-1);
    nordValue->setToolTip(tr("NORD: used elements, auto = full array or length of the value list"));
    optionsForm->addRow(tr("nord:"), nordValue);

    matrixRowsValue = new QSpinBox(this);
    matrixRowsValue->setRange(0, 1000);
    matrixRowsValue->setSpecialValueText(tr("none"));
    matrixRowsValue->setToolTip(tr("matrix rows; rows and columns > 0 make the channel a row-major matrix of the data type"));
    optionsForm->addRow(tr("matrix rows:"), matrixRowsValue);

    matrixColumnsValue = new QSpinBox(this);
    matrixColumnsValue->setRange(0, 1000);
    matrixColumnsValue->setSpecialValueText(tr("none"));
    matrixColumnsValue->setToolTip(tr("matrix columns; the value list is taken row by row"));
    optionsForm->addRow(tr("matrix columns:"), matrixColumnsValue);

    unitsLine = new QLineEdit(this);
    optionsForm->addRow(tr("units:"), unitsLine);

    precisionValue = new QSpinBox(this);
    precisionValue->setRange(-1, 17);
    precisionValue->setSpecialValueText(tr("default"));
    precisionValue->setValue(-1);
    optionsForm->addRow(tr("precision:"), precisionValue);

    enumStringsLine = new QLineEdit(this);
    enumStringsLine->setToolTip(tr("enum states, ';' separated (e.g. OFF;ON;ERROR)"));
    optionsForm->addRow(tr("enum strings:"), enumStringsLine);

    regexLine = new QLineEdit(this);
    regexLine->setToolTip(tr("string generator pattern, e.g. STATE-[0-9]{2} or (ON|OFF)-[a-c]"));
    optionsForm->addRow(tr("regex:"), regexLine);

    mainLayout->addWidget(optionsGroup);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(saveState()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    mainLayout->addWidget(buttonBox);

    // preset the form from the current widget properties
    if(softpv != (genSoftPV *) Q_NULLPTR) {
        variableLine->setText(softpv->getVariable());
        dataTypeCombo->setCurrentIndex((int) softpv->getDataType());
        modeCombo->setCurrentIndex((int) softpv->getMode());
        valueLine->setText(softpv->getValue());
        stepValue->setValue(softpv->getStep());
        periodValue->setValue(softpv->getPeriod());
        drvlLine->setText(softpv->getDrvl());
        drvhLine->setText(softpv->getDrvh());
        loprLine->setText(softpv->getLopr());
        hoprLine->setText(softpv->getHopr());
        lowLine->setText(softpv->getLow());
        loloLine->setText(softpv->getLolo());
        highLine->setText(softpv->getHigh());
        hihiLine->setText(softpv->getHihi());
        overflowCheckBox->setChecked(softpv->getoverflow());
        persistentCheckBox->setChecked(softpv->getPersistent());
        nelmValue->setValue(softpv->getNelm());
        nordValue->setValue(softpv->getNord());
        matrixRowsValue->setValue(softpv->getMatrixRows());
        matrixColumnsValue->setValue(softpv->getMatrixColumns());
        unitsLine->setText(softpv->getUnits());
        precisionValue->setValue(softpv->getPrecision());
        enumStringsLine->setText(softpv->getEnumStrings().join(";"));
        regexLine->setText(softpv->getRegex());
    }
}

void GenSoftPVDialog::saveState()
{
    if(QDesignerFormWindowInterface *formWindow = QDesignerFormWindowInterface::findFormWindow(entry)) {
        QDesignerFormWindowCursorInterface *cursor = formWindow->cursor();

        cursor->setProperty("variable", variableLine->text().trimmed());
        cursor->setProperty("dataType", dataTypeCombo->currentIndex());
        cursor->setProperty("mode", modeCombo->currentIndex());
        cursor->setProperty("value", valueLine->text());
        cursor->setProperty("step", stepValue->value());
        cursor->setProperty("period", periodValue->value());
        cursor->setProperty("drvl", drvlLine->text().trimmed());
        cursor->setProperty("drvh", drvhLine->text().trimmed());
        cursor->setProperty("lopr", loprLine->text().trimmed());
        cursor->setProperty("hopr", hoprLine->text().trimmed());
        cursor->setProperty("low", lowLine->text().trimmed());
        cursor->setProperty("lolo", loloLine->text().trimmed());
        cursor->setProperty("high", highLine->text().trimmed());
        cursor->setProperty("hihi", hihiLine->text().trimmed());
        cursor->setProperty("overflow", overflowCheckBox->isChecked());
        cursor->setProperty("persistent", persistentCheckBox->isChecked());
        cursor->setProperty("nelm", nelmValue->value());
        cursor->setProperty("nord", nordValue->value());
        cursor->setProperty("matrixRows", matrixRowsValue->value());
        cursor->setProperty("matrixColumns", matrixColumnsValue->value());
        cursor->setProperty("units", unitsLine->text().trimmed());
        cursor->setProperty("precision", precisionValue->value());
        QString enums = enumStringsLine->text().trimmed();
        cursor->setProperty("enumStrings", enums.isEmpty() ? QStringList() : enums.split(";"));
        cursor->setProperty("regex", regexLine->text().trimmed());
    }
    accept();
}
