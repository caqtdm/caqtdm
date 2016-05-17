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

#include <QDir>
#include <QDebug>
#include <QApplication>
#include <QPluginLoader>
#include <QLibraryInfo>
#include "caQtDM_Lib_global.h"
#include "MessageWindow.h"
#include "controlsinterface.h"

class CAQTDM_LIBSHARED_EXPORT loadPlugins
{

public:
   loadPlugins();
   ~loadPlugins() {}

   bool loadAll(QMap<QString, ControlsInterface*> &interfaces, MutexKnobData *mutexKnobData = 0, MessageWindow *messageWindow = 0,
                QMap<QString, QString> options = (QMap<QString, QString>()));
   void printPlugins(const QMap<QString,ControlsInterface*> interfaces);

private:
     ControlsInterface *controlsInterface;
};

