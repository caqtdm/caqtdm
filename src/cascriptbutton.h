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
    Q_PROPERTY(EPushButton::ScaleMode fontScaleMode READ fontScaleMode WRITE setFontScaleMode)

    Q_PROPERTY(defaultDisplay scriptDisplay READ getScriptDisplay WRITE setScriptDisplay)

    Q_PROPERTY(QString scriptCommand READ getScriptCommand WRITE setScriptCommand)
    Q_PROPERTY(QString scriptParameter READ getScriptParam WRITE setScriptParam)


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

    void setFontScaleMode(EPushButton::ScaleMode m);
    EPushButton::ScaleMode fontScaleMode();

    void setScriptDisplay(defaultDisplay m);
    defaultDisplay getScriptDisplay() {return  thisDefaultDisplay;}

    int getAccessW() const {return _AccessW;}
    void setAccessW(int access) {_AccessW = access;}

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
