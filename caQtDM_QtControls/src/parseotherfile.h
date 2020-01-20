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

#ifndef PARSEOTHERFILE_H
#define PARSEOTHERFILE_H

#include <qtcontrols_global.h>
#include <QMap>
#include <QBuffer>
#include <QDataStream>
#include <QSharedMemory>
#include <QWidget>
#include <QCoreApplication>
#include <QFile>
#include <QtUiTools/QUiLoader>
#include "adlParserMain.h"
#include "edlParserMain.h"

#define PRINT(x)

class QTCON_EXPORT ParseOtherFile

{
    Q_DECLARE_TR_FUNCTIONS(ParseOtherFile)

public:
    ParseOtherFile(QString filename, bool &ok, QString &errorString);
    QWidget *load(QWidget *parent);

    virtual ~ParseOtherFile() {delete buffer;}

protected:

private slots:

private:

    QBuffer *buffer;
    bool fileExists;
};

#endif
