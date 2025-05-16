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

#include <QtGui>
#include "limitsDialog.h"

int limitsDialog::extractPrecisionFromFormat(const QString &fmt)
{
    int prec = -1;
    bool ok = false;

    QString format = fmt.trimmed();
    if(format.right(1).contains("d)")) return 0;   // d format
    if(!format.left(1).contains("%")) return -1;    // not a valid format

    if(format.right(1).contains("f")) {            // f format
        int len = format.size();
        if(len-3 > 0) {
            QString rightPart = format.mid(len-3, len);
            if(rightPart.left(1).contains(".")) {
                QString precision = rightPart.mid(1,1);
                prec = precision.toInt(&ok);
                if(ok) return prec;
            } else {
                return -1;
            }
        } else {
            return -1;
        }
    }
    return -1;
}


QString limitsDialog::getFormatFromPrecision(int prec)
{
    QString format = "\\%.%1f";
    format = format.arg(prec);
    return format;
}

limitsDialog::limitsDialog(QWidget *w, MutexKnobData *data, const QString &title, QWidget *parent) : QWidget(parent)
{
    int thisWidth = 650;
    int thisHeight = 250;
    bool showMax = false;

    thisWidget = w;

    monData = data;

    Qt::WindowFlags flags = Qt::Dialog;
    setWindowFlags(flags);
    QGridLayout *Layout = new QGridLayout;
    setWindowModality (Qt::WindowModal);

#if defined(MOBILE_IOS)
    if(qApp->primaryScreen()->size().height() < 500) {
        thisWidth=430;  // normal for iphone
        thisHeight=200;
    }
    Specials special;
    special.setNewStyleSheet(this, qApp->primaryScreen()->size(), 16, 10);
    QPalette palette;
    palette.setBrush(QPalette::Window, QColor(255,255,224,255));
    setPalette(palette);
    setAutoFillBackground(true);
    setGeometry(QStyle::alignedRect(Qt::LeftToRight,Qt::AlignCenter, QSize(thisWidth,thisHeight), qApp->primaryScreen()->availableGeometry()));
#elif defined(MOBILE_ANDROID)
    QPalette palette;
    palette.setBrush(QPalette::Window, QColor(255,255,224,255));
    setPalette(palette);
    setAutoFillBackground(true);
    showMax = true;
#else
    move(parent->x() + parent->width() / 2 - thisWidth / 2 , parent->y() + parent->height() /2 -thisHeight/2);
#endif

    QVBoxLayout *mainLayout = new QVBoxLayout;
    QGroupBox *groupBox = new QGroupBox("Limits and precision change");

    QLabel *highLimitLabel = new QLabel("high limit");
    QLabel *lowLimitLabel = new QLabel("low limit");

    highLimitComboBox = new QComboBox();
    highLimitComboBox->addItem("channel");
    highLimitComboBox->addItem("user");

    lowLimitComboBox = new QComboBox();
    lowLimitComboBox->addItem("channel");
    lowLimitComboBox->addItem("user");

    // treat limits
    QLabel *limitsLabel = new QLabel("Limits ");

    // add combobox and set correct item
    limitsComboBox = new QComboBox();
    limitsComboBox->addItem("channel");
    limitsComboBox->addItem("user");

    minimumLineEdit = new QLineEdit("");
    maximumLineEdit = new QLineEdit("");

    int row = 1;
    QString className = w->metaObject()->className();

    if(className.contains("caSlider")) {

        Layout->addWidget(highLimitLabel, row, 0);
        Layout->addWidget(highLimitComboBox, row, 1);
        Layout->addWidget(maximumLineEdit, row, 2);

        row++;
        Layout->addWidget(lowLimitLabel, row, 0);
        Layout->addWidget(lowLimitComboBox, row, 1);
        Layout->addWidget(minimumLineEdit, row, 2);

    }
    else
    {
        // add all to layout
        Layout->addWidget(limitsLabel, row, 0);
        Layout->addWidget(limitsComboBox, row, 1);
        Layout->addWidget(minimumLineEdit, row, 2);
        Layout->addWidget(maximumLineEdit, row, 3);
    }

    // precision
    QLabel *precisionLabel = new QLabel("Precision ");

    // add combobox and set correct item
    precisionComboBox = new QComboBox();
    precisionComboBox->addItem("channel");
    precisionComboBox->addItem("user");

    precisionLineEdit = new QSpinBox();
    precisionLineEdit->setMinimum(0);
    precisionLineEdit->setMaximum(17);
    precisionLineEdit->setSingleStep(1);

    // add all to layout
    row++;
    Layout->addWidget(precisionLabel,    row, 0);
    Layout->addWidget(precisionComboBox, row, 1);
    Layout->addWidget(precisionLineEdit, row, 2);

    // box with buttons
    buttonBox = new QDialogButtonBox( Qt::Horizontal );
    QPushButton *button = new QPushButton( "Return" );
    connect( button, SIGNAL(clicked()), this, SLOT(cancelClicked()) );
    buttonBox->addButton(button, QDialogButtonBox::RejectRole );

    button = new QPushButton( "Apply" );
    connect( button, SIGNAL(clicked()), this, SLOT(applyClicked()) );
    buttonBox->addButton(button, QDialogButtonBox::ApplyRole );

    if(className.contains("caNumeric") || className.contains("caApplyNumeric") || className.contains("caSpinbox")) {
        // precisionLineEdit->setMaximum(PREC_LIMIT_NUMERIC);
        Layout->addWidget(buttonBox, 5, 0, 1, -1);
    }
    else if(className.contains("caSlider"))
    {
        Layout->addWidget(buttonBox, 4, 0, 1, -1);
    }
    else {
         Layout->addWidget(buttonBox, 3, 0, 1, -1);
    }

    groupBox->setLayout(Layout);
    mainLayout->addWidget(groupBox);

    setLayout(mainLayout);

    if(caSlider* sliderWidget = qobject_cast<caSlider *>(w)) {
        thisPV = sliderWidget->getPV();
    } else if(caLineEdit* lineeditWidget  = qobject_cast<caLineEdit *>(w)) {
        thisPV = lineeditWidget->getPV();
    } else if(caTextEntry* textentryWidget  = qobject_cast<caTextEntry *>(w)) {
        thisPV = textentryWidget->getPV();
    } else if(caThermo* thermoWidget  = qobject_cast<caThermo *>(w)) {
        thisPV = thermoWidget->getPV();
    } else if(caNumeric* numericWidget = qobject_cast<caNumeric *>(w)) {
        thisPV = numericWidget->getPV();
    } else if(caApplyNumeric* applynumericWidget = qobject_cast<caApplyNumeric *>(w)) {
        thisPV = applynumericWidget->getPV();
    } else if(caSpinbox* spinboxWidget = qobject_cast<caSpinbox *>(w)) {
        thisPV = spinboxWidget->getPV();
    } else if(caLinearGauge* lineargaugeWidget = qobject_cast<caLinearGauge *>(w)) {
        thisPV = lineargaugeWidget->getPV();
    } else if(caCircularGauge* circulargaugeWidget = qobject_cast<caCircularGauge *>(w)) {
        thisPV = circulargaugeWidget->getPV();
    } else if(caMeter* meterWidget = qobject_cast<caMeter *>(w)) {
        thisPV = meterWidget->getPV();
    }

    if(className.contains("Gauge")) {
        QLabel *Title = new QLabel(thisPV + " / " + w->objectName() + " (warning/alarm limits can not yet be changed)");
        Layout->addWidget(Title,0,0,1,-1);
    } else {
        QLabel *Title = new QLabel(thisPV + " / " + w->objectName());
        Layout->addWidget(Title,0,0,1,-1);
    }

    // get channel limits if needed later
    knobData *kPtr = monData->getMutexKnobDataPV(w, thisPV);
    if(kPtr != (knobData*) Q_NULLPTR) {
        channelLowerLimit = kPtr->edata.lower_disp_limit;
        channelUpperLimit = kPtr->edata.upper_disp_limit;
        channelPrecision = kPtr->edata.precision;
        if(kPtr->edata.lower_disp_limit != kPtr->edata.upper_disp_limit) doNothing = false; else doNothing = true;
    } else {
        doNothing = true;
    }

    // fill fields
    if(caSlider* sliderWidget = qobject_cast<caSlider *>(w)) {
        caSlider::SourceMode high = sliderWidget->getHighLimitMode();
        caSlider::SourceMode low = sliderWidget->getLowLimitMode();

        if(high == caSlider::Channel)
            highLimitComboBox->setCurrentIndex(0);
        else
            highLimitComboBox->setCurrentIndex(1);

        if(low == caSlider::Channel)
            lowLimitComboBox->setCurrentIndex(0);
        else
            lowLimitComboBox->setCurrentIndex(1);

        initMin = sliderWidget->getMinValue();
        initMax = sliderWidget->getMaxValue();

        minimumLineEdit->setText(QString::number(initMin, 'g'));
        maximumLineEdit->setText(QString::number(initMax, 'g'));

        if(sliderWidget->getPrecisionMode() == caSlider::Channel) {
            initPrecision =  channelPrecision;
            precisionComboBox->setCurrentIndex(0);
        } else {
            initPrecision = sliderWidget->getPrecision();
            precisionComboBox->setCurrentIndex(1);
        }
        if(initPrecision >=0) precisionLineEdit->setValue(initPrecision);

        // fill fields
     } else if(caMeter* meterWidget = qobject_cast<caMeter *>(w)) {
            caMeter::SourceMode mode = meterWidget->getLimitsMode();
            if(mode == caMeter::Channel) limitsComboBox->setCurrentIndex(0); else limitsComboBox->setCurrentIndex(1);
            initMin = meterWidget->getMinValue();
            initMax = meterWidget->getMaxValue();
            minimumLineEdit->setText(QString::number(initMin, 'g'));
            maximumLineEdit->setText(QString::number(initMax, 'g'));
            if(meterWidget->getPrecisionMode() == caMeter::Channel) {
                initPrecision =  channelPrecision;
                precisionComboBox->setCurrentIndex(0);
            } else {
                initPrecision = meterWidget->getPrecision();
                precisionComboBox->setCurrentIndex(1);
            }
            if(initPrecision >=0) precisionLineEdit->setValue(initPrecision);

    } else if(EAbstractGauge* abstractgaugeWidget = qobject_cast<EAbstractGauge *>(w)) {
            EAbstractGauge::displayLims mode = abstractgaugeWidget->getDisplayLimits();
            if(mode == EAbstractGauge::Channel_Limits) limitsComboBox->setCurrentIndex(0); else limitsComboBox->setCurrentIndex(1);
            initMin = abstractgaugeWidget->minValue();
            initMax = abstractgaugeWidget->maxValue();
            minimumLineEdit->setText(QString::number(initMin, 'g'));
            maximumLineEdit->setText(QString::number(initMax, 'g'));
            precisionComboBox->setCurrentIndex(1);
            precisionComboBox->setDisabled(true);
            initPrecision = extractPrecisionFromFormat(abstractgaugeWidget->valueFormat());
            if(initPrecision >=0) precisionLineEdit->setValue(initPrecision);

    } else  if(className.contains("caNumeric") || className.contains("caApplyNumeric")  || className.contains("caSpinbox")) {
        int decDigits=2, intDigits=4;
        bool fixedFormat = false;

        if(caNumeric* numericWidget = qobject_cast<caNumeric *>(w)) {
            if(numericWidget->getLimitsMode()== caNumeric::Channel) limitsComboBox->setCurrentIndex(0); else limitsComboBox->setCurrentIndex(1);
            if(numericWidget->getPrecisionMode() == caNumeric::Channel) {
                initPrecision =  channelPrecision;
                precisionComboBox->setCurrentIndex(0);
            } else {
                initPrecision = numericWidget->decDigits();
                precisionComboBox->setCurrentIndex(1);
            }

            decDigits = numericWidget->decDigits();
            intDigits = numericWidget->intDigits();
            initMin = numericWidget->getMinValue();
            initMax = numericWidget->getMaxValue();
            fixedFormat = numericWidget->getFixedFormat();
        } else if(caApplyNumeric* applynumericWidget = qobject_cast<caApplyNumeric *>(w)) {
            if(applynumericWidget->getLimitsMode() == caApplyNumeric::Channel) limitsComboBox->setCurrentIndex(0); else limitsComboBox->setCurrentIndex(1);
            if(applynumericWidget->getPrecisionMode() == caApplyNumeric::Channel) initPrecision =  channelPrecision; else initPrecision = applynumericWidget->decDigits();
            decDigits = applynumericWidget->decDigits();
            intDigits = applynumericWidget->intDigits();
            initMin = applynumericWidget->getMinValue();
            initMax = applynumericWidget->getMaxValue();
            fixedFormat = applynumericWidget->getFixedFormat();
        } else if(caSpinbox* spinboxWidget = qobject_cast<caSpinbox *>(w)) {
            if(spinboxWidget->getLimitsMode() == caSpinbox::Channel) limitsComboBox->setCurrentIndex(0); else limitsComboBox->setCurrentIndex(1);
            if(spinboxWidget->getPrecisionMode() == caSpinbox::Channel) initPrecision =  channelPrecision; else initPrecision = spinboxWidget->decDigits();
            decDigits = spinboxWidget->decDigits();
            intDigits = spinboxWidget->intDigits();
            initMin = spinboxWidget->getMinValue();
            initMax = spinboxWidget->getMaxValue();
            fixedFormat = spinboxWidget->getFixedFormat();
        }

        minimumLineEdit->setText(QString::number(initMin, 'g'));
        maximumLineEdit->setText(QString::number(initMax, 'g'));
        precisionLineEdit->setValue(initPrecision);

        // special addition for fixed format
        row++;
        QLabel *integerLabel = new QLabel("integer digits");
        integerLineEdit = new QSpinBox();
        integerLineEdit->setMinimum(0);
        integerLineEdit->setMaximum(7);
        integerLineEdit->setSingleStep(1);
        Layout->addWidget(integerLabel, row, 2);
        Layout->addWidget(integerLineEdit, row, 3);
        integerLineEdit->setValue(intDigits);

        QLabel *decimalLabel = new QLabel("decimal digits");
        decimalLineEdit = new QSpinBox();
        decimalLineEdit->setMinimum(0);
        decimalLineEdit->setMaximum(5);
        decimalLineEdit->setSingleStep(1);
        Layout->addWidget(decimalLabel, row+1, 2);
        Layout->addWidget(decimalLineEdit, row+1, 3);
        decimalLineEdit->setValue(decDigits);

        QLabel *formatLabel = new QLabel("Fixed format ");
        formatComboBox = new QComboBox();
        formatComboBox->addItem("no");
        formatComboBox->addItem("yes");
        Layout->addWidget(formatLabel, row, 0);
        Layout->addWidget(formatComboBox, row, 1);
        if(fixedFormat) {
            formatComboBox->setCurrentIndex(1);
            integerLineEdit->setDisabled(false);
            decimalLineEdit->setDisabled(false);
            precisionLineEdit->setDisabled(true);
        } else {
            formatComboBox->setCurrentIndex(0);
            integerLineEdit->setDisabled(true);
            decimalLineEdit->setDisabled(true);
            precisionLineEdit->setDisabled(false);
        }

        connect(formatComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(indexChanged(int)));

    } else if(caThermo* thermoWidget = qobject_cast<caThermo *>(w)) {
        caThermo::SourceMode mode = thermoWidget->getLimitsMode();
        if(mode == caThermo::Channel) limitsComboBox->setCurrentIndex(0); else limitsComboBox->setCurrentIndex(1);
        initMin = thermoWidget->minValue();
        initMax = thermoWidget->maxValue();
        minimumLineEdit->setText(QString::number(initMin, 'g'));
        maximumLineEdit->setText(QString::number(initMax, 'g'));

    } else if(caLineEdit* lineeditWidget  = qobject_cast<caLineEdit *>(w)) {
        caLineEdit::SourceMode mode = lineeditWidget->getLimitsMode();
        if(mode == caLineEdit::Channel) {
            limitsComboBox->setCurrentIndex(0);
            initMin = channelLowerLimit;
            initMax = channelUpperLimit;
        } else {
            limitsComboBox->setCurrentIndex(1);
            initMin = lineeditWidget->getMinValue();
            initMax = lineeditWidget->getMaxValue();
        }
        minimumLineEdit->setText(QString::number(initMin, 'g'));
        maximumLineEdit->setText(QString::number(initMax, 'g'));

        mode = lineeditWidget->getPrecisionMode();
        if(mode == caLineEdit::Channel) {
            initPrecision =  channelPrecision;
            precisionComboBox->setCurrentIndex(0);
        } else {
            initPrecision = lineeditWidget->getPrecision();
            precisionComboBox->setCurrentIndex(1);
        }
        precisionLineEdit->setValue(initPrecision);
    }

    setWindowTitle(title);

    if(!showMax) showNormal();
    else showMaximized();
}

void limitsDialog::indexChanged(int) {

    if(formatComboBox->currentIndex() == 1) {
        formatComboBox->setCurrentIndex(1);
        integerLineEdit->setDisabled(false);
        decimalLineEdit->setDisabled(false);
        precisionLineEdit->setDisabled(true);
    } else {
        formatComboBox->setCurrentIndex(0);
        integerLineEdit->setDisabled(true);
        decimalLineEdit->setDisabled(true);
        precisionLineEdit->setDisabled(false);
    }
}

void limitsDialog::cancelClicked()
{
    close();
}

void limitsDialog::applyClicked()
{
    bool ok1, ok2, fixedFormat = false;
    QString minimum, maximum;
    double min, max;
    int prec, intDigits=4, decDigits=2;

    int limitsMode = limitsComboBox->currentIndex();
    int precisionMode = precisionComboBox->currentIndex();

    QString className = thisWidget->metaObject()->className();
    if(className.contains("caNumeric") || className.contains("caApplyNumeric") || className.contains("caSpinbox")) {
        if(formatComboBox->currentIndex() == 0) fixedFormat = false; else fixedFormat = true;
        intDigits = integerLineEdit->value();
        decDigits = decimalLineEdit->value();
    }

    minimum = minimumLineEdit->text().trimmed();
    min = minimum.toDouble(&ok1);
    maximum = maximumLineEdit->text().trimmed();
    max = maximum.toDouble(&ok2);

    if(!ok1) min = initMin;
    if(!ok2) max = initMax;
    if(min == max) {
        min = initMin;
        max = initMax;
    }

    prec= precisionLineEdit->value();

    // set already something to the dialog
    QString highS;
    QString lowS;
    if(className.contains("caSlider"))
    {
        highS = highLimitComboBox->currentText();
        lowS = lowLimitComboBox->currentText();

        if(highS == "channel")
        {
            maximumLineEdit->setText(QString::number(channelUpperLimit, 'g'));
        }
        else if(highS == "user" && ok2)
        {
            maximumLineEdit->setText(QString::number(max, 'g'));
        }
        else
        {
            maximumLineEdit->setText(QString::number(initMax, 'g'));
        }

        if(lowS == "channel")
        {
            minimumLineEdit->setText(QString::number(channelLowerLimit, 'g'));
        }
        else  if(lowS == "user" && ok1)
        {
            minimumLineEdit->setText(QString::number(min, 'g'));
        }
        else
        {
            minimumLineEdit->setText(QString::number(initMin, 'g'));
        }
    }
    else if(limitsMode == Channel && !doNothing) {
        minimumLineEdit->setText(QString::number(channelLowerLimit, 'g'));
        maximumLineEdit->setText(QString::number(channelUpperLimit, 'g'));
    } else if(limitsMode == User && ok1 && ok2) {
        minimumLineEdit->setText(QString::number(min, 'g'));
        maximumLineEdit->setText(QString::number(max, 'g'));
    } else if(limitsMode == User) {
        minimumLineEdit->setText(QString::number(initMin, 'g'));
        maximumLineEdit->setText(QString::number(initMax, 'g'));
    }

    if(precisionMode == Channel ) {
        precisionLineEdit->setValue(channelPrecision);
    } else if(precisionMode == User) {
        precisionLineEdit->setValue(prec);
    } else if(precisionMode == User) {
        precisionLineEdit->setValue((int) initPrecision);
    }

    // ************* we have a slider
    if(caSlider* sliderWidget = qobject_cast<caSlider *>(thisWidget)) {

        // do this to prevent qwtslider to set the value when changing bounds
        // however we could miss a value coming from the monitor (is this really true?)
        sliderWidget->blockSignals(true);

        if(highS == "channel") {
            sliderWidget->setHighLimitMode(caSlider::Channel);
            if(!doNothing) {
                sliderWidget->setMaxValue(channelUpperLimit);
            }
        } else if(highS == "user"){
            sliderWidget->setHighLimitMode(caSlider::User);
            sliderWidget->setMaxValue(max);
        }

        if(lowS == "channel") {
            sliderWidget->setLowLimitMode(caSlider::Channel);
            if(!doNothing) {
                sliderWidget->setMinValue(channelLowerLimit);
            }
        } else if(lowS == "user"){
            sliderWidget->setLowLimitMode(caSlider::User);
            sliderWidget->setMinValue(min);
        }

        if(precisionMode == Channel) {
            sliderWidget->setPrecisionMode(caSlider::Channel);
            sliderWidget->setPrecision(channelPrecision);
        } else if(precisionMode == User){
            sliderWidget->setPrecisionMode(caSlider::User);
            sliderWidget->setPrecision(prec);
        }

        sliderWidget->blockSignals(false);
        // set eventual missed value
        knobData *kPtr = monData->getMutexKnobDataPV(sliderWidget, thisPV);
        if(kPtr != (knobData*) Q_NULLPTR) sliderWidget->setSliderValue(kPtr->edata.rvalue);

    } else if(caMeter* meterWidget = qobject_cast<caMeter *>(thisWidget)) {
        if(limitsMode == Channel) {
            meterWidget->setLimitsMode(caMeter::Channel);
            if(!doNothing) {
                meterWidget->setMaxValue(channelUpperLimit);
                meterWidget->setMinValue(channelLowerLimit);
            }
        } else if(limitsMode == User){
            meterWidget->setLimitsMode(caMeter::User);
            meterWidget->setMaxValue(max);
            meterWidget->setMinValue(min);
        }

        if(precisionMode == Channel) {
            meterWidget->setPrecisionMode(caMeter::Channel);
            meterWidget->setPrecision(channelPrecision);
        } else if(precisionMode == User){
            meterWidget->setPrecisionMode(caMeter::User);
            meterWidget->setPrecision(prec);
        }
        meterWidget->updateMeter();
        meterWidget->invalidate();

        // ************* we have a thermometer
    } else if(caThermo* thermoWidget = qobject_cast<caThermo *>(thisWidget)) {

        if(limitsMode == Channel) {
            thermoWidget->setLimitsMode(caThermo::Channel);
            if(!doNothing) {
                if(thermoWidget->getDirection() == caThermo::Down  || thermoWidget->getDirection() == caThermo::Left) {
                    thermoWidget->setMinValue(channelUpperLimit);
                    thermoWidget->setMaxValue(channelLowerLimit);
                } else {
                    thermoWidget->setMaxValue(channelUpperLimit);
                    thermoWidget->setMinValue(channelLowerLimit);
                }
            }
        } else if(limitsMode == User){
            thermoWidget->setLimitsMode(caThermo::User);
            if(thermoWidget->getDirection() == caThermo::Down  || thermoWidget->getDirection() == caThermo::Left) {
                thermoWidget->setMinValue(max);
                thermoWidget->setMaxValue(min);
            } else {
                thermoWidget->setMaxValue(max);
                thermoWidget->setMinValue(min);
            }
        }

        if(precisionMode == Channel) {
            thermoWidget->setPrecisionMode(caThermo::Channel);
            thermoWidget->setPrecision(channelPrecision);
        } else if(precisionMode == User){
            thermoWidget->setPrecisionMode(caThermo::User);
            thermoWidget->setPrecision(prec);
        }

        // ************* we have a calineedit or catextentry
    } else if(caLineEdit* lineeditWidget = qobject_cast<caLineEdit *>(thisWidget)) {

        if(limitsMode == Channel) {
            lineeditWidget->setLimitsMode(caLineEdit::Channel);
            if(!doNothing) {
                lineeditWidget->setMaxValue(channelUpperLimit);
                lineeditWidget->setMinValue(channelLowerLimit);
            }
        } else if(limitsMode == User){
            lineeditWidget->setLimitsMode(caLineEdit::User);
            lineeditWidget->setMaxValue(max);
            lineeditWidget->setMinValue(min);
        }

        if(precisionMode == Channel) {
            lineeditWidget->setPrecisionMode(caLineEdit::Channel);
            lineeditWidget->setFormat(channelPrecision);
        } else if(precisionMode == User){
            lineeditWidget->setPrecisionMode(caLineEdit::User);
            lineeditWidget->setPrecision(prec);
            lineeditWidget->setFormat(prec);
        }
        lineeditWidget->updateAlarmColors();

        // ************* we have a caNumeric, caApplyNumeric or caSpinbox
    } else if(className.contains("caNumeric") || className.contains("caApplyNumeric")  || className.contains("caSpinbox")) {
        if(prec > PREC_LIMIT_NUMERIC) prec = PREC_LIMIT_NUMERIC;
        // qDebug() << "set precision to:"  << prec;

        if(limitsMode == Channel) {
            setLimitsModeChannel(thisWidget);
            if(!doNothing) {
                setMaxValueA(thisWidget, channelUpperLimit);
                setMinValueA(thisWidget, channelLowerLimit);
            }
        } else if(limitsMode == User){
            setLimitsModeUser(thisWidget);
            setMaxValueA(thisWidget, max);
            setMinValueA(thisWidget, min);
        }

        if(fixedFormat) {
           setFixedFormatA(thisWidget, true);
           setDecDigitsA(thisWidget, decDigits);
           setIntDigitsA(thisWidget, intDigits);
        } else {
           setFixedFormatA(thisWidget, false);
           if(precisionMode == Channel) {
               setPrecisionModeChannel(thisWidget);
               setDecDigitsA(thisWidget, (int) channelPrecision);
           } else if(precisionMode == User){
               setPrecisionModeUser(thisWidget);
               setDecDigitsA(thisWidget, prec);
           }
        }

        knobData *kPtr = monData->getMutexKnobDataPV(thisWidget, thisPV);
        CaQtDM_Lib *compute = (CaQtDM_Lib *) thisWidget;
        if(kPtr != (knobData*) Q_NULLPTR) {
            kPtr->edata.initialize = true;
            compute->ComputeNumericMaxMinPrec(thisWidget, *kPtr);
            kPtr->edata.initialize = false;
        }

    } else if(EAbstractGauge* abstractgaugeWidget = qobject_cast<EAbstractGauge *>(thisWidget)) {

        if(limitsMode == Channel) {
            abstractgaugeWidget->setDisplayLimits(EAbstractGauge::Channel_Limits);
        } else if(limitsMode == User){
            abstractgaugeWidget->setDisplayLimits(EAbstractGauge::User_Limits);
            abstractgaugeWidget->setMaxValue(max);
            abstractgaugeWidget->setMinValue(min);
            abstractgaugeWidget->setLowWarning(min);
            abstractgaugeWidget->setHighWarning(max);
            abstractgaugeWidget->setLowError(min);
            abstractgaugeWidget->setHighError(max);
        }
        knobData *kPtr = monData->getMutexKnobDataPV(abstractgaugeWidget, thisPV);
        CaQtDM_Lib *compute = (CaQtDM_Lib *) abstractgaugeWidget;
        if(kPtr != (knobData*) Q_NULLPTR) {
            kPtr->edata.initialize = true;
            compute->UpdateGauge(abstractgaugeWidget, *kPtr);
            kPtr->edata.initialize = false;
        }
        abstractgaugeWidget->setValueFormat(getFormatFromPrecision(prec));
    }
}

void limitsDialog::exec()
{
    connect(buttonBox, SIGNAL(rejected()), &loop, SLOT(quit()) );
    connect(buttonBox, SIGNAL(accepted()), &loop, SLOT(quit()) );
    loop.exec();
    close();
    deleteLater();
}

void limitsDialog::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    loop.quit();
}

void limitsDialog::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    QPen pen(Qt::black, 3, Qt::SolidLine, Qt::FlatCap, Qt::RoundJoin);
    painter.setPen(pen);
    painter.drawRoundedRect(5, 5, width()-7, height()-7, 3, 3);

    QWidget::paintEvent(e);
}

