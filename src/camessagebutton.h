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

#ifndef caMessage_H
#define caMessageMenu_H

#include <QString>
#include <QPushButton>
#include <QEvent>
#include <qtcontrols_global.h>
#include <epushbutton.h>

class QTCON_EXPORT caMessageButton : public EPushButton
{

    Q_OBJECT

    Q_ENUMS(Stacking)
    Q_PROPERTY(QString channel READ getPV WRITE setPV)
    Q_PROPERTY(QString label READ getLabel WRITE setLabel)
    Q_PROPERTY(QColor foreground READ getForeground WRITE setForeground)
    Q_PROPERTY(QColor background READ getBackground WRITE setBackground)
    Q_PROPERTY(QString releaseMessage READ getReleaseMessage WRITE setReleaseMessage)
    Q_PROPERTY(QString pressMessage READ getPressMessage WRITE setPressMessage)

    Q_PROPERTY(colMode colorMode READ getColorMode WRITE setColorMode)
    Q_ENUMS(colMode)

public:

    caMessageButton(QWidget *parent);

    enum colMode {Static=0, Alarm};

    void setColors(QColor bg, QColor fg,  QColor hover, QColor border);
    void setNormalColors();
    void setAlarmColors(short status);

    colMode getColorMode() const { return thisColorMode; }

    void setColorMode(colMode colormode) {thisColorMode = colormode;
                                          setBackground(thisBackColor);
                                          setForeground(thisForeColor);
                                          oldColorMode = thisColorMode;
                                           }

    QColor getForeground() const {return thisForeColor;}
    void setForeground(QColor c);

    QColor getBackground() const {return thisBackColor;}
    void setBackground(QColor c);

    QString getLabel() const {return thisLabel;}
    void setLabel(QString const &label);

    QString getPressMessage() const {return thisPressMessage;}
    QString getReleaseMessage() const {return thisReleaseMessage;}
    void setReleaseMessage(QString const &m) {thisReleaseMessage = m;}
    void setPressMessage(QString const &m) {thisPressMessage = m;}

    QString getPV() const {return thisPV;}
    void setPV(QString const &newPV){ thisPV = newPV;}

    int getAccessW() const {return _AccessW;}
    void setAccessW(int access);

    void buttonhandle(int type);


private slots:


protected:


signals:

    void messageButtonSignal(int type);

private:

    QString thisPV;
    QString thisLabel;
    QColor thisForeColor, oldForeColor;
    QColor thisBackColor, oldBackColor;
    QColor thisHoverColor, oldHoverColor;
    QColor thisBorderColor;
    QPalette thisPalette;

    colMode thisColorMode;
    colMode oldColorMode;

    QString thisReleaseMessage, thisPressMessage;
    bool eventFilter(QObject *obj, QEvent *event);
    bool _AccessW;
};

#endif
