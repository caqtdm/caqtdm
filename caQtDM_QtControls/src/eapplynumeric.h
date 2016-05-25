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

#ifndef EAPPLYNUMERIC_H
#define EAPPLYNUMERIC_H

#include <qtcontrols_global.h>
#include <enumeric.h>
#include <eapplybutton.h>
#include <QBoxLayout>


/**
 * \brief a widget to set a value and write it
 *
 * this widget includes a ENumeric and a EApplyButton, it connects the signal
 * valueChanged(double) of the former with the slot valueModified(double) of the latter
 * 
 * it emits the signal clicked(double) when the button is clicked
 */
class QTCON_EXPORT EApplyNumeric : public QWidget, public FloatDelegate
{

Q_OBJECT


Q_PROPERTY(double value READ value WRITE setValue)
Q_PROPERTY(int integerDigits READ intDigits WRITE setIntDigits)
Q_PROPERTY(int decimalDigits READ decDigits WRITE setDecDigits)
Q_PROPERTY(Qt::Orientation buttonPos READ buttonPosition WRITE setButtonPosition)
Q_PROPERTY(QString buttonText READ buttonText WRITE setButtonText)
Q_PROPERTY(QFont numericFont READ numericFont WRITE setNumericFont)
Q_PROPERTY(bool applyButtonActive READ applyButtonActive WRITE setApplyButtonActive)
/* scalable fonts */
Q_PROPERTY(bool digitsFontScaleEnabled READ digitsFontScaleEnabled WRITE setDigitsFontScaleEnabled DESIGNABLE true)

public:
	EApplyNumeric(QWidget *parent, int i = 2, int d = 1, Qt::Orientation pos = Qt::Horizontal);
        ~EApplyNumeric(){}
	
	/** \brief returns the behaviour of the Apply button after a change in the value of the numeric widget
	 *
	 * @return true if the Apply button becomes red after a value change on the numeric widget.
	 * @return false if the Apply button does not change after a value change on the numeric widget.
	 *
	 * @see setApplyButtonActive
	 */
	bool applyButtonActive() { return d_applyButtonActive; }
	
	bool digitsFontScaleEnabled();
	
	void setDigitsFontScaleEnabled(bool en);

private: 
	void 		init();
	int		intDig, decDig;
	ENumeric 	*data;
	EApplyButton 	*button;
	Qt::Orientation	buttonPos;
	QBoxLayout	*box;
	bool d_applyButtonActive;
	bool d_applyOnValueChanged;
	bool d_fontScaleEnabled;

public:
	/** 
	 * sets whether the button is on the right side of the numeric widget 
	 * or at the bottom
	 * @param	pos	Horizontal or Vertical
	 */
    void setButtonPosition(Qt::Orientation pos){ buttonPos = pos; init(); }
	/** 
	 * returns wheter the button is on the right side of the numeric widget
	 * or at the bottom
	 * @return Horizontal or Vertical
	 */
    Qt::Orientation buttonPosition() const { return buttonPos; }

	/**
	 * sets value displayed by the widget
	 * @param v	the value to be displayed
	 */
    void setValue(double v){ data->setValue(v); }
    void silentSetValue(double v);
	/**
	 * returns the value displayed by the widget
	 * @return the value displayed
	 */
    double value() const { return data->value(); }

	/**
	 * sets the maximum value of the widget
	 * @param v 	max val
	 */
        void setMaximum(double v){ data->setMaximum(v); }
	/**
	 * returns the maximum value of the widget
	 * @return max val
	 */
        double maximum() const { return data->maximum(); }

	/**
	 * sets the minimum value of the widget
	 * @param v 	min val
	 */
        void setMinimum(double v){ data->setMinimum(v); }
	/**
	 * returns the minimum value of the widget
	 * @return min val
	 */
        double minimum() const { return data->minimum(); }

	/**
	 * sets number of integer digits to be displayed
	 * @param i	number of digits
	 */
    void setIntDigits(int i){ data->setIntDigits(i); }
	/**
	 * returns number of integer digits displayed
	 * @return i	number of digits
	 */
    int intDigits() const { return data->intDigits(); }

	/**
	 * sets number of decimal digits to be displayed
	 * @param d	number of digits
	 */
    void setDecDigits(int d){ data->setDecDigits(d); }
	/**
	 * returns number of decimal digits displayed
	 * @return d	number of digits
	 */
    int decDigits() const { return data->decDigits(); }
	
	/**
	 * sets text displayed in the button
	 * @param s	text to be displayed
	 */
    void setButtonText(const QString &s){ button->setText(s); }
	/**
	 * returns the text displyed in the button
	 * @return text displayed
	 */
    QString buttonText() const { return button->text(); }
	
	/** \brief returns the state of the ApplyNumeric.
	 *
	 * @return true if the value was modified since last click
	 * @return false if the value has never been modified after last Apply click.
	 */
	bool isModified();
	
	/** \brief changes the behaviour of the Apply button.
	 * 
	 * Normally, when the value on the numeric widget changes, the button becomes <em>red</em>,
	 * to highlight that the value has changed since the last click on the <em>Apply</em> button itself.
	 * 
	 * @param en if true the button becomes red when the value changes
	 *           if false, the value change on the numeric widget does not make the apply button change its color.
	 * <p><strong>Observation</strong>: disabling this property does not mean that the Apply button is disabled too.
	 * If clicked, it will still apply the value displayed in the numeric widget.</p>
	 * <p><em>Note</em>: introduced in qtcontrols version <em>4.x</em>.</p>
	 */
	void setApplyButtonActive(bool en) { d_applyButtonActive = en; }
	
        QFont numericFont() const { return data->font(); }
        void setNumericFont(const QFont f) { data->setFont(f); }
	
signals:
	/**
	 * signal emitted when the button is clicked, the parameter
	 * is the value displayed by the widget
	 */
	void clicked(double);

        /** \brief signal emitted when the displayed value changes on the TApplyNumeric.
          *
          * @param value the new value
          */
        void valueChanged(double value);

public slots:
	virtual void applyValue();
    void clearModified(){ button->clearModified(); }
	virtual void setFont(const QFont &);
	void disable() { setDisabled(true); }
	
  private slots:
	void numericValueChanged(double);

protected:
    void resizeEvent ( QResizeEvent * event);

};

#endif
