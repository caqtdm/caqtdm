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

#ifndef CATABLE_H
#define CATABLE_H

#include <QTableWidget>
#include <QAction>
#include <QFont>
#include <qtcontrols_global.h>

typedef char string40[40];

class QTCON_EXPORT caTable : public QTableWidget
{

    Q_OBJECT

    Q_PROPERTY(QString channels READ getPVS WRITE setPVS)
    Q_PROPERTY(QString columnSizes READ getColumnSizes WRITE setColumnSizes)

    Q_PROPERTY(colMode colorMode READ getColorMode WRITE setColorMode)
    Q_ENUMS(colMode)

    Q_PROPERTY(int precision READ getPrecision WRITE setPrecision)
    Q_PROPERTY(SourceMode precisionMode READ getPrecisionMode WRITE setPrecisionMode)

    Q_PROPERTY(SourceMode limitsMode READ getLimitsMode WRITE setLimitsMode)
    Q_ENUMS(SourceMode)

    Q_PROPERTY(double maxValue READ getMaxValue WRITE setMaxValue)
    Q_PROPERTY(double minValue READ getMinValue WRITE setMinValue)

public:

    caTable(QWidget *parent);

    enum colMode {Static=0, Alarm};
    colMode getColorMode() const { return thisColorMode; }
    void setColorMode(colMode colormode) {thisColorMode = colormode;}

    enum SourceMode {Channel = 0, User};
    SourceMode getPrecisionMode() const { return thisPrecMode; }
    void setPrecisionMode(SourceMode precmode) {thisPrecMode = precmode;}
    SourceMode getLimitsMode() const { return thisLimitsMode; }
    void setLimitsMode(SourceMode limitsmode) { thisLimitsMode = limitsmode;}

    int getPrecision() const {return thisPrecision;}
    void setPrecision(int prec) {thisPrecision = prec;}

    QString getPVS() const {return thisPV.join(";");}
    void setPVS(QString const &newPV) {thisPV = newPV.split(";");}

    QString getColumnSizes() const {return thisColumnSizes.join(";");}
    void setColumnSizes(QString const &newSizes);

    double getMaxValue()  const {return thisMaximum;}
    void setMaxValue(double const &maxim) {thisMaximum = maxim;}

    double getMinValue()  const {return thisMinimum;}
    void setMinValue(double const &minim) {thisMinimum = minim;}

    void setFormat(int row, int prec);
    void setValue(int row, int col, short severity, double value, QString const &unit);
    void displayText(int row, int col, short severity, QString const &text);

    void createActions();

    void setValueFont(QFont font);

private slots:

    void copy();

protected:

private:

    enum { MaxRows = 50 };
    enum { MaxCols = 5  };
    QStringList	thisPV;
    QStringList	thisColumnSizes;
    double thisMaximum, thisMinimum;
    QString thisStyle;
    QString oldStyle;
    QString keepText[MaxRows][MaxCols];
    string40 thisFormat[MaxRows];

    colMode thisColorMode;
    int thisPrecision;
    SourceMode thisPrecMode;
    SourceMode thisLimitsMode;

    QColor defaultBackColor;
    QColor defaultForeColor;

    QFont thisItemFont;

    QAction *copyAct;
};

#endif
