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

#include "eapplybutton.h"
#include "leftclick_with_modifiers_eater.h"

EApplyButton::EApplyButton(QWidget *parent) : EPushButton(parent)
{
	this->setFocusPolicy(Qt::StrongFocus);
	this->setText("APPLY");
    d_modified = false;
    //setMinimumWidth(MIN_BUTTON_SIZE);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	defaultTextColor = palette().color(QPalette::ButtonText);	
	connect(this, SIGNAL(clicked()), this, SLOT(clearModified()), Qt::QueuedConnection);
	defaultTextColor = palette().color(QPalette::ButtonText);
    LeftClickWithModifiersEater *leftClickWithModifiersEater = new LeftClickWithModifiersEater(this);
    installEventFilter(leftClickWithModifiersEater);
}

void EApplyButton::valueModified(double)
{
	if (isEnabled())
	{
		QPalette p = palette();
		p.setColor(QPalette::ButtonText, Qt::red);
		setPalette(p);
		d_modified = true;
	}
}

void EApplyButton::clearModified()
{
	if (isEnabled())
	{
		QPalette p = palette();
		p.setColor(QPalette::ButtonText, defaultTextColor);
		setPalette(p);
	}
	d_modified = false;
}
