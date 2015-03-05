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

#ifndef specialFunctions_H
#define specialFunctions_H

#include <QtGui>
#include <QDesktopWidget>
#include <QApplication>
#include <QStandardPaths>

class Specials {

public:

#ifdef MOBILE_ANDROID
    QString stdpathdoc=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
#endif
#ifdef MOBILE_IOS
    QString stdpathdoc=QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
#endif

    void setNewStyleSheet(QWidget* w, QSize size, int bigPtSize=16, int smallPtSize=10, QString myStyle ="", int pointSizeCorrection = 0);

};
#endif
