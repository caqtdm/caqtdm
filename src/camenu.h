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

#ifndef CAMENU_H
#define CAMENU_H
#include <QComboBox>
#include <qtcontrols_global.h>

class QTCON_EXPORT caMenu : public QComboBox
{
    Q_ENUMS(colMode)
    Q_PROPERTY(QString channel READ getPV WRITE setPV)
    Q_PROPERTY(colMode colorMode READ getColorMode WRITE setColorMode)
    Q_PROPERTY(QColor foreground READ getForeground WRITE setForeground)
    Q_PROPERTY(QColor background READ getBackground WRITE setBackground)
    Q_PROPERTY(bool labelDisplay READ getLabelDisplay WRITE setLabelDisplay)
    Q_OBJECT

public:

    enum colMode {Default, Static, Alarm};

    QString getPV() const;
    void setPV(QString const &newPV);

    QColor getForeground() const {return thisForeColor;}
    void setForeground(QColor c);

    QColor getBackground() const {return thisBackColor;}
    void setBackground(QColor c);

    bool getLabelDisplay() const { return thisLabelDisplay; }
    void setLabelDisplay(bool thisLabelDisplay);

    colMode getColorMode() const { return thisColorMode; }
    void setColorMode(colMode colormode) {thisColorMode = colormode; setColors(thisBackColor, thisForeColor, true);}

    QString getLabel() const;

     void populateCells(QStringList list);

    caMenu(QWidget *parent);

    int getAccessW() const {return thisAccessW;}
    void setAccessW(int access);

    void setColors(QColor bg, QColor fg, bool force);
    void setAlarmColors(short status);
    void setNormalColors();

private slots:


protected:

private:
    QString thisPV;
    QColor thisForeColor, oldForeColor;
    QColor thisBackColor, oldBackColor;
    QPalette thisPalette;
    bool thisLabelDisplay;
    bool thisAccessW;
    colMode  thisColorMode, oldColorMode;
    QPalette defaultPalette;

    bool eventFilter(QObject *obj, QEvent *event);
};

#endif
