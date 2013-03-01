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

#ifndef CATOGGLEBUTTON_H
#define CATOGGLEBUTTON_H

#include <QCheckBox>
#include <QString>
#include <QColor>
#include <QEvent>
#include <qtcontrols_global.h>

class QTCON_EXPORT caToggleButton : public QCheckBox
{
    Q_OBJECT

    Q_ENUMS(colMode)

    Q_PROPERTY(QString channel READ getPV WRITE setPV)

    Q_PROPERTY(QColor foreground READ getForeground WRITE setForeground)
    Q_PROPERTY(QColor background READ getBackground WRITE setBackground)

    Q_PROPERTY(colMode colorMode READ getColorMode WRITE setColorMode)

public:

    caToggleButton(QWidget *parent);

    enum colMode {Default, Static, Alarm};

    QColor getForeground() const {return thisForeColor;}
    void setForeground(QColor c);
    QColor getBackground() const {return thisBackColor;}
    void setBackground(QColor c);

    colMode getColorMode() const { return thisColorMode; }
    void setColorMode(colMode colormode) {thisColorMode = colormode;
                                          setBackground(thisBackColor);
                                          setForeground(thisForeColor);
                                           }
    void setAlarmColors(short status);
    void setUserAlarmColors(double val);
    void setColors(QColor bg, QColor fg);
    void setNormalColors();

    QString getPV() const;
    void setPV(QString const &newPV);

    void setState(bool state);
    bool bitState(long value);

    int getAccessW() const {return _AccessW;}
    void setAccessW(int access);

private slots:

    void buttonToggled();

signals:

    void toggleButtonSignal(bool checked);


protected:


private:

    QColor thisForeColor, oldForeColor;
    QColor thisBackColor, oldBackColor;
    QPalette thisPalette;
    colMode thisColorMode;
    QColor defaultBackColor;
    QColor defaultForeColor;

    QString     thisPV;
    bool eventFilter(QObject *obj, QEvent *event);
    bool _AccessW;
};

#endif  /* CATOGGLEBUTTON */
