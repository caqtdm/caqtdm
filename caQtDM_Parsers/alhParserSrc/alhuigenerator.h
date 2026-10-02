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

#ifndef ALHUIGENERATOR_H
#define ALHUIGENERATOR_H

#include <QByteArray>
#include <QString>
#include "alhparserdefs.h"

// .ui document for an .alhConfig: a caAlarmTree (parses the config itself) above a caAlarmLog
class ALHPARSER_EXPORT AlhUiGenerator
{
public:
    struct Params {
        QString configFile;       // absolute path, property of caAlarmTree
        QString includeDir;
        QString macros;           // "A=x,B=y"
        QString logTarget;        // objectName of the caAlarmLog
        bool alarmMode;
        bool withLog;
        QString title;
        int width;
        int height;
        Params() : logTarget(QStringLiteral("alarmLog")), alarmMode(true), withLog(true), width(900), height(700) {}
    };

    static QByteArray generate(const Params &params);
};

#endif // ALHUIGENERATOR_H
