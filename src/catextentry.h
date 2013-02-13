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

#ifndef caTextEntry_H
#define caTextEntry_H

#include <QGridLayout>
#include <QFrame>
#include <QEvent>

#include <qtcontrols_global.h>
#include <caLineEdit>


class QTCON_EXPORT caTextEntry : public caLineEdit
{
    Q_OBJECT
    Q_PROPERTY(bool unitsEnabled READ getUnitsEnabled WRITE setUnitsEnabled DESIGNABLE false)

public:

    caTextEntry( QWidget *parent = 0 );
    int getAccessW() const {return _AccessW;}
    void setAccessW(int access);
    void updateText(const QString &text);

private slots:
    void dataInput();

signals:

    void TextEntryChanged(QString);

private:
    bool eventFilter(QObject *obj, QEvent *event);
    bool _AccessW;
    QString startText;
    bool emitted;
};
#endif
