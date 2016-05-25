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

#ifndef CAWIDGET_INTERFACE_H
#define CAWIDGET_INTERFACE_H

#include <QWidget>
#include <QString>
#include <QMenu>
#include <QMap>
#include "caqtdm_lib_interface.h"

class caWidgetInterface
{
public:
    virtual ~caWidgetInterface(){}
    // publush ca data
    virtual void caDataUpdate(const QString& units, const QString& String, const knobData& data) = 0;
    // subscribe ca data
    virtual void caActivate(CaQtDM_Lib_Interface* lib_interface, QMap<QString, QString> map, knobData* kData, int* specData, QWidget* parent) = 0;
    // info for context menu
    virtual void getWidgetInfo(QString* pv, int& nbPV, int& limitsDefault, int& precMode, int& limitsMode,
                               int& Precision, char* colMode, double& limitsMax, double& limitsMin) = 0;
    // create context menu
    virtual void createContextMenu(QMenu& menu) = 0;
    virtual QString getDragText() = 0;
};

#endif // CAWIDGET_INTERFACE_H
