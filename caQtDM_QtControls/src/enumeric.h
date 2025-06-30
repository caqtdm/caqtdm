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

#ifndef ENUMERIC_H
#define ENUMERIC_H

#include <math.h>
#include <QFrame>
#include <QEvent>
#include <qtcontrols_global.h>
#include <float_delegate.h>
#include <fontscalingwidget.h>

class QLabel;
class QButtonGroup;
class QLineEdit;
class QPushButton;
class QGridLayout;
class QAbstractButton;

/**
 * \brief a widget to set a value
 *
 * this widget is used to set a value, whenever the displayed 
 * value changes the signal valueChanged(double) is emitted
 */
class QTCON_EXPORT ENumeric : public QFrame, public FloatDelegate
{
    Q_OBJECT
    Q_PROPERTY(int integerDigits READ intDigits WRITE setIntDigits)
    Q_PROPERTY(int decimalDigits READ decDigits WRITE setDecDigits)
    Q_PROPERTY(double value READ value WRITE setValue)
    Q_PROPERTY(double maxValue READ maximum WRITE setMaximum)
    Q_PROPERTY(double minValue READ minimum WRITE setMinimum)

    /* scalable fonts */
    Q_PROPERTY(bool digitsFontScaleEnabled READ digitsFontScaleEnabled WRITE setDigitsFontScaleEnabled)

public:
    ENumeric(QWidget *parent, int intDigits = 2, int decDigits = 1);
    ~ENumeric(){}

    bool readAccessW() const {return _AccessW;}
    void writeAccessW(bool access);
    void silentSetValue(double v);
    double value() const { return data*pow(10.0, -decDig); }
    void setMaximum(double v);
    double maximum() const { return d_maxAsDouble; }
    void setMinimum(double v);
    double minimum() const { return d_minAsDouble; }
    void setIntDigits(int i);
    int intDigits() const { return intDig; }
    void setDecDigits(int d);
    int decDigits() const { return decDig; }
    bool digitsFontScaleEnabled() { return d_fontScaleEnabled; }
    void setDigitsFontScaleEnabled(bool en);

public slots:
    virtual void setEnabled(bool);
    virtual void setDisabled(bool);
    void setValue(double v);

private slots:
    void dataInput();
    void upData(QAbstractButton*);
    void downData(QAbstractButton*);
    void valueUpdated();

protected slots:
/* ENumeric uses ESimpleLabel::calculateFontPointSizeF method to calculate the point size of
 * one of its numeric labels. Then applies the same point size to all the labels it is made up of.
 */
    void resizeEvent(QResizeEvent *);

signals:
    void valueChanged(double);

protected:
    virtual void showEvent(QShowEvent *);
    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

private:
    void mouseDoubleClickEvent(QMouseEvent*);
    bool eventFilter(QObject *obj, QEvent *event);
    void init();
    void clearContainers();
    void showData();
    void formatDigit(QPushButton*, QLabel*, QPushButton*);
    void reconstructGeometry();
    void downDataIndex(int id);
    void upDataIndex(int id);
    void updateRoundColors(int i);
    void triggerRoundColorUpdate();
    double transformNumberSpace(long long value, int dig);
    long long transformNumberSpace(double value, int dig);

    int idUpVuoto, idDownVuoto;
    int idUpPoint, idDownPoint;
    int intDig;
    int decDig;
    int digits;
    long long data;
    long long minVal;
    long long maxVal;
    double d_minAsDouble, d_maxAsDouble;
    QButtonGroup *bup;
    QButtonGroup *bdown;
    QGridLayout *box;
    QLabel *signLabel, *pointLabel;
    QLineEdit *text;
    QVector<QLabel*> labels;
    bool d_fontScaleEnabled;
    bool _AccessW;
    int lastLabel, lastLabelOnTab;
    double csValue;
};
#endif // EDIGIT_H
