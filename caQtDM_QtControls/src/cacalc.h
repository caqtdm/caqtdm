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

#ifndef CACALC_H
#define CACALC_H

#include <QtCore>
#include <QWidget>
#include <QLabel>
#include <limits>
#include "esimplelabel.h"
#include <qtcontrols_global.h>
#include "caPropHandleDefs.h"

#define MAX_QRECT_PARAMS 4

class  QTCON_EXPORT caCalc : public  ESimpleLabel
{
    Q_OBJECT

    Q_PROPERTY(QString variable READ getVariable WRITE setVariable)

    Q_PROPERTY(varType variableType READ getVariableType WRITE setVariableType)

    Q_PROPERTY(QColor foreground READ getForeground WRITE setForeground)
    Q_PROPERTY(QColor background READ getBackground WRITE setBackground)
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    Q_PROPERTY(QStringList channelList READ getPVList WRITE setPVList DESIGNABLE isPropertyVisible(pvlist) STORED false )
#else
    Q_PROPERTY(QStringList channelList READ getPVList WRITE setPVList DESIGNABLE true STORED false )
#endif


    Q_PROPERTY(QString channels READ getPV WRITE setPV DESIGNABLE false)
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    Q_PROPERTY(QString calc READ getCalc WRITE setCalc  DESIGNABLE isPropertyVisible(calcabcd))
    Q_PROPERTY(QString channel READ getChannelA WRITE setChannelA  DESIGNABLE isPropertyVisible(channela))
    Q_PROPERTY(QString channelB READ getChannelB WRITE setChannelB DESIGNABLE isPropertyVisible(channelb))
    Q_PROPERTY(QString channelC READ getChannelC WRITE setChannelC DESIGNABLE isPropertyVisible(channelc))
    Q_PROPERTY(QString channelD READ getChannelD WRITE setChannelD DESIGNABLE isPropertyVisible(channeld))
    Q_PROPERTY(double initialValue READ getInitialValue WRITE setInitialValue DESIGNABLE isPropertyVisible(initialvalue))
#else
    Q_PROPERTY(QString calc READ getCalc WRITE setCalc  DESIGNABLE true)
    Q_PROPERTY(QString channel READ getChannelA WRITE setChannelA  DESIGNABLE true)
    Q_PROPERTY(QString channelB READ getChannelB WRITE setChannelB DESIGNABLE true)
    Q_PROPERTY(QString channelC READ getChannelC WRITE setChannelC DESIGNABLE true)
    Q_PROPERTY(QString channelD READ getChannelD WRITE setChannelD DESIGNABLE true)
    Q_PROPERTY(double initialValue READ getInitialValue WRITE setInitialValue DESIGNABLE true)

#endif
    Q_PROPERTY(int precision READ getPrecision WRITE setPrecision)

    // this will prevent user interference
    Q_PROPERTY(QString styleSheet READ styleSheet WRITE noStyle DESIGNABLE false)

    Q_ENUMS(varType)

#include "addevent.h"

public:
#include "caPropHandle.h"
    enum varType { scalar = 0, vector};
    enum Properties { calcabcd = 0, channela, channelb, channelc, channeld, initialvalue, pvlist};

    QString getPV() const {return thisPV.join(";");}
    void setPV(QString const &newPV)  {thisPV = newPV.simplified().split(";");}
    QStringList getPVList() const {return thisPV;}
    void setPVList(QStringList list) {thisPV = list; updatePropertyEditorItem(this, "channels");}

    void noStyle(QString style) {Q_UNUSED(style);}

    varType getVariableType() const { return thisVarType; }
    void setVariableType(varType vartype);

    QString getVariable() const {return thisVariable;}
    void setVariable(QString const &var) {thisVariable = var;}

    QString getCalc() const {return thisCalc;}
    void setCalc(QString const &calc) {thisCalc = calc;}

    QString getChannelA() const {return thisChannelA;}
    void setChannelA(QString const &pv) {thisChannelA = pv;}

    QString getChannelB() const {return thisChannelB;}
    void setChannelB(QString const &pv) {thisChannelB = pv;}

    QString getChannelC() const {return thisChannelC;}
    void setChannelC(QString const &pv) {thisChannelC = pv;}

    QString getChannelD() const {return thisChannelD;}
    void setChannelD(QString const &pv) {thisChannelD = pv;}

    double getInitialValue() const {return thisValue;}
    void setInitialValue(double const &value) {thisValue = value;}

    caCalc( QWidget *parent = 0 );

    void setTextLine(const QString &txt);
    QString getTextLine(){return keepText;}
    void setForeAndBackground(QColor fg, QColor bg);

    int getPrecision() const {return thisPrecision;}
    void setPrecision(int prec) {thisPrecision = prec;}

    void setDataCount(int const &count) {thisDataCount = count;}
    int getDataCount() const {return thisDataCount;}
    void setValue(QString value);

    bool isPropertyVisible(Properties property);
    void setPropertyVisible(Properties property, bool visible);

    double getValue() { return thisValue;}

    QColor getForeground() const {return m_ForeColor;}
    void setForeground(QColor c);

    QColor getBackground() const {return m_BackColor;}
    void setBackground(QColor c);

    void setQRectParam(int x,double param);

public slots:
    void animation(QRect p) {
#include "animationcode.h"
    }

    void hideObject(bool hideit) {
#include "hideobjectcode.h"
    }

public slots:
    void setValue(double value);
    void setValue(QRect value);
    void setValue(QRectF value);
    void setValue(int value);
    void setValue(bool value);
signals:
    void changeValue(double value);
    void reduced_emitSignal(double value);
private:
    QString thisChannelA, thisChannelB, thisChannelC, thisChannelD, thisVariable;
    QString thisCalc;
    QString keepText;
    double thisValue;
    int thisPrecision;
    int thisDataCount;
    QStringList	thisPV;
    bool designerVisible[10];
    varType thisVarType;
    QColor m_ForeColor;
    QColor m_BackColor;
    bool checkSignal_value_bool;
    int  checkSignal_value_int;
    double checkSignal_value_double;
    double changeValue_value_double;
    QRect checkSignal_value_QRect;
    QRectF checkSignal_value_QRectF;

    // Array to store parameters of QRect or QRectF as doubles.
    double value_QRect_F_const[MAX_QRECT_PARAMS];

    // Array to store whether a specific parameter of a QRect or a QRectF is const.
    bool value_QRect_F_is_const[MAX_QRECT_PARAMS];
    bool is_a_pure_constant;

};

#endif // CACALC_H
