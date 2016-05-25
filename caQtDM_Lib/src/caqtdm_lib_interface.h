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

#ifndef CAQTDM_LIB_INTERFACE_H
#define CAQTDM_LIB_INTERFACE_H

#include "dbrString.h"
#include "knobDefines.h"
#include "knobData.h"

#include <QWidget>

class CaQtDM_Lib_Interface
{
public:

#include "formattype.h"

    virtual int addMonitor(QWidget *thisW, knobData *data, QString pv, QWidget *w, int *specData, QMap<QString, QString> map, QString *pvRep) = 0;
    // write pv methods
    virtual void TreatRequestedValue(QString pv, QString text, FormatType fType, QWidget *w) = 0;
    // more .... if required

    // create new methods - to access MutexKnobData *mutexKnobDataP
    virtual knobData* GetMutexKnobDataPtr(int index) = 0;
    virtual knobData* GetMutexKnobDataPV(QWidget *widget, QString pv) = 0;
    // more .... if required
};

#endif // CAQTDM_LIB_INTERFACE_H


