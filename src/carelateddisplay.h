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

#ifndef caRelatedDisplay_H
#define caRelatedDisplay_H

#include <qtcontrols_global.h>
#include <carowcolmenu.h>

class QTCON_EXPORT caRelatedDisplay : public caRowColMenu
{

    Q_OBJECT

    Q_PROPERTY(Stacking stackingMode READ getStacking WRITE setStacking)
    Q_PROPERTY(QString removeParent READ getReplaceModes WRITE setReplaceModes)

public:

    caRelatedDisplay(QWidget *parent);
    QString getReplaceModes() const {return replacemodes.join(";");}
    void setReplaceModes(QString const &newL) {replacemodes = newL.split(";");}


private:

    QStringList  replacemodes;

};

#endif
