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

#ifndef EAPPLYBUTTON_H
#define EAPPLYBUTTON_H

#include <qtcontrols_global.h>
#include <econstants.h>
#include "epushbutton.h"
#include <QPushButton>

/**
 * \brief a button to apply a setting
 *
 * this is a simple button used to apply a setting, it has to slots 
 * that allows the developer to change the color of the displayed text:
 * valueModified sets the color to red and clearModified reverts to
 * the original setting
 */
class QTCON_EXPORT EApplyButton : public EPushButton
{
	Q_OBJECT
public:
	EApplyButton(QWidget *parent);
    ~EApplyButton(){}
	
	bool isModified() { return d_modified; }

public slots:
	/**
	 * tell the widget that the value to which it's associated
	 * has changed, so the color of the font changes to red
	 */
	void valueModified(double);
	/**
	 * resets font color to black
	 */
    void clearModified();

private:
	QColor defaultTextColor;
    bool d_modified;
};

#endif
