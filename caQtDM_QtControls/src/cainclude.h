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

#ifndef CAINCLUDE_H
#define CAINCLUDE_H

#include <QWidget>
//#include <QtUiTools/QUiLoader>
#include <QVBoxLayout>
#include <qtcontrols_global.h>

#define PRC 1

#ifdef PRC
   #include "parsepepfile.h"
#endif

class  QTCON_EXPORT caInclude : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QString macro READ getMacro WRITE setMacro)
    Q_PROPERTY(QString filename READ getFileName WRITE setFileName)

#include "caVisibProps.h"
#include "caVisibDefs.h"

public:

    caInclude( QWidget *parent = 0 );
    ~caInclude();

    QString getFileName() const {return newFileName;}
    void setFileName(QString const &filename);

    QString getMacro() const {return thisMacro.join(";");}
    void setMacro(QString const &newMacro) {thisMacro = newMacro.split(";");}

    QColor getBackground() const {return thisBackColor;}
    void setBackground(QColor c);

    void setLineSize( int size );

    void removeIncludedWidget();

public slots:

signals:

protected:

      void paintEvent( QPaintEvent* );

private:

    QString newFileName;
    QString prvFileName;
    QStringList thisMacro;
    QColor thisBackColor;
    QWidget *thisParent;
    int thisLineSize;
    QVBoxLayout *layout;
    QWidget *thisLoadedWidget;
    bool loadIncludes;

#ifdef PRC
    ParsePepFile *pepfile;
#endif
};

#endif
