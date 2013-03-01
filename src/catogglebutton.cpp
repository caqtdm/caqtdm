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

#include "catogglebutton.h"
#include "alarmdefs.h"
#include <QtDebug>
#include <QMessageBox>
#include <QApplication>

caToggleButton::caToggleButton(QWidget *parent) : QCheckBox(parent)
{
    setCheckable(true);
    setTristate(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    thisPalette = palette();
    thisBackColor = defaultBackColor = QColor(200,200,200,0);
    thisForeColor = defaultForeColor = Qt::black;
    thisColorMode=Static;
    setBackground(defaultBackColor);
    setForeground(defaultForeColor);

    installEventFilter(this);
    connect(this, SIGNAL(clicked()), this, SLOT(buttonToggled()) );

    setAccessW(true);
}

void  caToggleButton::buttonToggled()
{
  //QMessageBox::information( this, "Toggled!", QString("The button is %1!").arg(isChecked()?"pressed":"released") );
  emit toggleButtonSignal(isChecked());
}

QString caToggleButton::getPV() const
{
    return thisPV;
}

void caToggleButton::setPV(QString const &newPV)
{
    thisPV = newPV;
}

void caToggleButton::setState(bool check)
{
    if(check) {
        setCheckState( Qt::Checked );
    } else {
        setCheckState( Qt::Unchecked );
    }
}

void caToggleButton::setColors(QColor bg, QColor fg)
{
    if((oldBackColor == bg) && (oldForeColor == fg)) return;
    QPalette thisPalette = palette();
    thisPalette.setColor(QPalette::WindowText, fg);
    thisPalette.setColor(QPalette::Button, bg);
    setPalette(thisPalette);
    oldBackColor = bg;
    oldForeColor = fg;
}

void caToggleButton::setBackground(QColor c)
{
    QColor color = c;
    if(thisColorMode == Default) {
        thisBackColor = defaultBackColor;
    } else {
      thisBackColor = color;
    }
    setColors(thisBackColor, thisForeColor);
}

void caToggleButton::setForeground(QColor c)
{
    QColor color = c;
    if(thisColorMode == Default) {
       thisForeColor= defaultForeColor;
    } else {
      thisForeColor = color;
    }
    setColors(thisBackColor, thisForeColor);
}

void caToggleButton::setAlarmColors(short status)
{
    QColor c;
    switch (status) {

    case NO_ALARM:
        c=AL_GREEN;
        break;
    case MINOR_ALARM:
        c=AL_YELLOW;
        break;
    case MAJOR_ALARM:
        c=AL_RED;
        break;
    case ALARM_INVALID:
    case NOTCONNECTED:
        c=AL_WHITE;
        break;
    default:
        c=AL_DEFAULT;
        break;
    }
    if(status == NOTCONNECTED) {
       setColors(c, c);
    } else {
       setColors(thisBackColor, c);
    }
}

void caToggleButton::setNormalColors()
{
    setColors(thisBackColor, thisForeColor);
}

void caToggleButton::setAccessW(int access)
{
     _AccessW = access;
}

bool caToggleButton::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Enter) {
        if(!_AccessW) {
            QApplication::setOverrideCursor(QCursor(Qt::ForbiddenCursor));
            setEnabled(false);
        } else {
            QApplication::restoreOverrideCursor();
        }
    } else if(event->type() == QEvent::Leave) {
        QApplication::restoreOverrideCursor();
        setEnabled(true);
    }
    return QObject::eventFilter(obj, event);
}






