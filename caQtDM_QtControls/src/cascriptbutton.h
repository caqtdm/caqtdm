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

#ifndef caScriptButton_H
#define caScriptButton_H

#include <QString>
#include <QPushButton>
#include <QEvent>
#include <qtcontrols_global.h>
#include <epushbutton.h>
#include <QHBoxLayout>
#include <QCheckBox>

class QTCON_EXPORT caScriptButton : public QWidget
{

    Q_OBJECT

    Q_ENUMS(defaultDisplay)

    Q_PROPERTY(QString label READ getLabel WRITE setLabel)
    Q_PROPERTY(QColor foreground READ getForeground WRITE setForeground)
    Q_PROPERTY(QColor background READ getBackground WRITE setBackground)
    Q_PROPERTY(EPushButton::ScaleMode fontScaleMode READ fontScaleMode WRITE setFontScaleModeL)

    Q_PROPERTY(defaultDisplay scriptDisplay READ getScriptDisplay WRITE setScriptDisplay)

    Q_PROPERTY(QString scriptCommand READ getScriptCommand WRITE setScriptCommand)
    Q_PROPERTY(QString scriptParameter READ getScriptParam WRITE setScriptParam)

#include "caElevation.h"

public:

     enum defaultDisplay {Invisible, Visible};

    caScriptButton(QWidget *parent);

    void setColors(QColor bg, QColor fg,  QColor hover, QColor border);

    QColor getForeground() const {return thisForeColor;}
    void setForeground(QColor c);

    QColor getBackground() const {return thisBackColor;}
    void setBackground(QColor c);

    QString getLabel() const {return thisLabel;}
    void setLabel(QString const &label);

    QString getScriptCommand() const {return thisScriptCommand;}
    QString getScriptParam() const {return thisScriptParam;}

    void setScriptCommand(QString const &m) {thisScriptCommand = m;}
    void setScriptParam(QString const &m) {thisScriptParam = m;}

    bool getDisplayShowExecution() const { return thisShowExecution;}

    void setFontScaleModeL(EPushButton::ScaleMode m);
    EPushButton::ScaleMode fontScaleMode();

    void setScriptDisplay(defaultDisplay m);
    defaultDisplay getScriptDisplay() {return  thisDefaultDisplay;}

    bool getAccessW() const {return _AccessW;}
    void setAccessW(bool access) {_AccessW = access;}

    void *getProcess() const {return thisProcess;}
    void setProcess(void *t) {thisProcess = t;}


private slots:

    void buttonToggled();
    void scriptButtonClicked();

protected:
       void changeEvent(QEvent *e);

signals:

   void scriptButtonSignal();

private:

    bool eventFilter(QObject *obj, QEvent *event);

    QString thisLabel;
    QColor thisForeColor, oldForeColor;
    QColor thisBackColor, oldBackColor;
    QColor thisHoverColor, oldHoverColor;
    QColor thisBorderColor;
    QPalette thisPalette;

    QString thisScriptCommand, thisScriptParam;
    bool thisShowExecution;

    QCheckBox *displayScript;
    EPushButton *buttonScript;
    EPushButton::ScaleMode thisScaleMode;

    defaultDisplay thisDefaultDisplay;

     bool _AccessW;
     void *thisProcess;
};

#endif
