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

void EApplyNumeric::silentSetValue(double v)
{
     data->silentSetValue(v);
}

void EApplyNumeric::init()
{
	if (box)
		delete box;
		
	if (buttonPos == Qt::Horizontal)
		box = new QHBoxLayout(this);
	else if (buttonPos == Qt::Vertical)
		box = new QVBoxLayout(this);
    else
        box = new QHBoxLayout(this);

	box->setMargin(0);
    box->setSpacing(0);

	if (!data)
		data = new ENumeric(this, intDig, decDig);
	if (!button)
        button = new EApplyButton(this);
	box->addWidget(data, 3);
    box->addWidget(button, 1);

	setMinimumWidth(data->minimumWidth() + button->minimumWidth());
	data->setDigitsFontScaleEnabled(d_fontScaleEnabled ? ESimpleLabel::Height : ESimpleLabel::None);
    button->setFontScaleMode(EPushButton::WidthAndHeight);
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
    button->setMaximumSize(size().width()/3, size().height()/3);
}



