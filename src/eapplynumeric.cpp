#include "eapplynumeric.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <ESimpleLabel>
#include <QResizeEvent>

EApplyNumeric::EApplyNumeric(QWidget *parent, int i, int d, Qt::Orientation pos) : QWidget(parent), FloatDelegate()
{
	box = NULL;
	data = NULL;
	button = NULL;
	buttonPos = pos;
	intDig = i;
	decDig = d;
	d_applyButtonActive = true;
    d_fontScaleEnabled = true;

	init();
}

bool EApplyNumeric::digitsFontScaleEnabled()
{
  return d_fontScaleEnabled;
}
	
void EApplyNumeric::setDigitsFontScaleEnabled(bool en)
{
  if(data)
  {
	d_fontScaleEnabled = en;
	data->setDigitsFontScaleEnabled(d_fontScaleEnabled ? ESimpleLabel::Height : ESimpleLabel::None);
  }
  else
        printf("EApplyNumeric ENumeric not initialized");
}

void EApplyNumeric::init()
{
	if (box)
		delete box;
		
	if (buttonPos == Qt::Horizontal)
		box = new QHBoxLayout(this);
	else if (buttonPos == Qt::Vertical)
		box = new QVBoxLayout(this);

	box->setMargin(0);
	box->setSpacing(3);

	if (!data)
		data = new ENumeric(this, intDig, decDig);
	if (!button)
		button = new EApplyButton(this);
	box->addWidget(data, 3);
	box->addWidget(button, 1);

	setMinimumWidth(data->minimumWidth() + button->minimumWidth());
	data->setDigitsFontScaleEnabled(d_fontScaleEnabled ? ESimpleLabel::Height : ESimpleLabel::None);
	connect(data, SIGNAL(valueChanged(double)), this, SLOT(numericValueChanged(double)));
        /* map ENumeric valueChanged() signal into EApplyNumeric omonimous signal */
        connect(data, SIGNAL(valueChanged(double)), this, SIGNAL(valueChanged(double)));
	connect(button, SIGNAL(clicked()), this, SLOT(applyValue()));
}

void EApplyNumeric::setFont(const QFont &f)
{
	QWidget::setFont(f);
	data->setFont(f);
	button->setFont(f);
}

void EApplyNumeric::applyValue()
{
  emit clicked(data->value());
}

bool EApplyNumeric::isModified()
{
  if(button)
	return button->isModified();
  return false;
}

void EApplyNumeric::numericValueChanged(double val)
{
  if(d_applyButtonActive && button)
        button->valueModified(val);
}

void EApplyNumeric::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    data->resize(size());
}



