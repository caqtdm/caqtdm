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

#ifndef CATABLE_H
#define CATABLE_H

#include <QTableWidget>
#include <QAction>
#include <QFont>
#include <qtcontrols_global.h>

#include "caPropHandleDefs.h"

typedef char string40[40];

class QTCON_EXPORT caTable : public QTableWidget
{
    Q_OBJECT

    Q_PROPERTY(QStringList channelsList READ getPVSList WRITE setPVSList STORED false)
    Q_PROPERTY(QString channels READ getPVS WRITE setPVS DESIGNABLE false)

    Q_PROPERTY(QString columnSizes READ getColumnSizes WRITE setColumnSizes)

    Q_PROPERTY(colMode colorMode READ getColorMode WRITE setColorMode)
    Q_ENUMS(colMode)

    Q_PROPERTY(int precision READ getPrecision WRITE setPrecision)
    Q_PROPERTY(SourceMode precisionMode READ getPrecisionMode WRITE setPrecisionMode)

    Q_PROPERTY(SourceMode limitsMode READ getLimitsMode WRITE setLimitsMode)
    Q_ENUMS(SourceMode)

    Q_PROPERTY(double maxValue READ getMaxValue WRITE setMaxValue)
    Q_PROPERTY(double minValue READ getMinValue WRITE setMinValue)

    Q_PROPERTY(QString scriptCommand READ getScriptCommand WRITE setScriptCommand)
    Q_PROPERTY(QString scriptParameter READ getScriptParam WRITE setScriptParam)

    // this will prevent user interference
    Q_PROPERTY(QString styleSheet READ styleSheet WRITE noStyle DESIGNABLE false)

public:
#include "caPropHandle.h"

    void noStyle(QString style) {Q_UNUSED(style);}

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

    QString getPVS() const {return thisPVS.join(";");}
    void setPVS(QString const &newPV) {thisPVS = newPV.split(";");}
    QStringList getPVSList() const {return thisPVS;}
    void setPVSList(QStringList list) {thisPVS = list; updatePropertyEditorItem(this, "channels");}

    QString getColumnSizes() const {return thisColumnSizes.join(";");}
    void setColumnSizes(QString const &newSizes);

    QString getScriptCommand() const {return thisScriptCommand;}
    QString getScriptParam() const {return thisScriptParam;}

    void setScriptCommand(QString const &m) {thisScriptCommand = m;}
    void setScriptParam(QString const &m) {thisScriptParam = m;}

    double getMaxValue()  const {return thisMaximum;}
    void setMaxValue(double const &maxim) {thisMaximum = maxim;}

    double getMinValue()  const {return thisMinimum;}
    void setMinValue(double const &minim) {thisMinimum = minim;}

    void setFormat(int row, int prec);
    void setValue(int row, int col, short severity, double value, QString const &unit);
    void displayText(int row, int col, short severity, QString const &text);

    void createActions();

    void setValueFont(QFont font);
    void copy();

public slots:
    void animation(QRect p) {
#include "animationcode.h"
    }

    void hideObject(bool hideit) {
#include "hideobjectcode.h"
    }

signals:
   void TableDoubleClickedSignal(QString pv);

private slots:
    void celldoubleclicked(int, int);
    void cellclicked(int, int);

private:
    enum { MaxRows = 500 };
    enum { MaxCols = 5  };
    QStringList	thisPVS;
    QStringList	thisColumnSizes;
    double thisMaximum, thisMinimum;
    QString thisStyle;
    QString oldStyle;
    string40 thisFormat[MaxRows];
    QTableWidgetItem* tableItem[MaxRows][MaxCols];

    colMode thisColorMode;
    int thisPrecision;
    SourceMode thisPrecMode;
    SourceMode thisLimitsMode;

    QColor defaultBackColor;
    QColor defaultForeColor;

    QString thisScriptCommand, thisScriptParam;

    QFont thisItemFont;

    QAction *copyAct;
};

#endif
