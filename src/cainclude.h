//******************************************************************************
// Copyright (c) 2012 Paul Scherrer Institut PSI), Villigen, Switzerland
// Disclaimer: neither  PSI, nor any of their employees makes any warranty
// or assumes any legal liability or responsibility for the use of this software
//******************************************************************************
//******************************************************************************
//
//     Author : Anton Chr. Mezger
//
//******************************************************************************

#ifndef CAINCLUDE_H
#define CAINCLUDE_H

#include <QWidget>
#include <QtUiTools/QUiLoader>
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

#include "caVisib.h"

public:

#include "caVisibPublic.h"

    caInclude( QWidget *parent = 0 );
    ~caInclude();

    QString getFileName() const {return newFileName;}
    void setFileName(QString const &filename);

    QString getMacro() const {return thisMacro.join(";");}
    void setMacro(QString const &newMacro) {thisMacro = newMacro.split(";");}

    QColor getBackground() const {return thisBackColor;}
    void setBackground(QColor c);

    void setLineSize( int size );
    int getLineSize() {
        return thisLineSize;
    }

    void removeIncludedWidget();

public slots:

signals:

protected:

      void paintEvent( QPaintEvent* );

private:

#include "caVisibPrivate.h"

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
