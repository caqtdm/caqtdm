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

#ifndef _EGAUGE_H
#define _EGAUGE_H

#include <math.h>
#include <QWidget>
#include <qtcontrols_global.h>
#include <QtDebug>

#ifdef __MINGW32__
inline static double exp10(double x)
{
  return pow(10.0, x);
}
#endif

#define LINEAR_MAJOR_TICKS 6
#define CIRCULAR_MAJOR_TICKS 11
#define NUM_MINOR_TICKS 5

class QRect;
class QLineF;

/**
 * \brief an abstract class used to implement widgets with a color scale
 *
 * you can configure the graphical behaviour by setting warnings, errors and colors
 */
class QTCON_EXPORT EAbstractGauge : public QWidget
{
Q_OBJECT

Q_ENUMS(ColorMode)
Q_ENUMS(displayLims)
Q_ENUMS(alarmLims)

Q_PROPERTY(double minValue READ minValue WRITE setMinValue)
Q_PROPERTY(double maxValue READ maxValue WRITE setMaxValue)

Q_PROPERTY(displayLims displayLimits READ getDisplayLimits WRITE setDisplayLimits)

Q_PROPERTY(double highError READ highError WRITE setHighError)
Q_PROPERTY(double lowError READ lowError WRITE setLowError)

Q_PROPERTY(double lowWarning READ lowWarning WRITE setLowWarning)
Q_PROPERTY(double highWarning READ highWarning WRITE setHighWarning)

Q_PROPERTY(alarmLims alarmLimits READ getAlarmLimits WRITE setAlarmLimits)

Q_PROPERTY(double value READ value WRITE setValue)
Q_PROPERTY(double reference READ reference WRITE setReference)
Q_PROPERTY(bool scaleEnabled READ isScaleEnabled WRITE setScaleEnabled)
Q_PROPERTY(bool referenceEnabled READ isReferenceEnabled WRITE setReferenceEnabled)

Q_PROPERTY(ColorMode colorMode READ colorMode WRITE setColorMode)
Q_PROPERTY(bool externalScale READ externalScale WRITE setExternalScale)
Q_PROPERTY(bool logarithmicScale READ logarithmicScale WRITE setLogarithmicScale)
Q_PROPERTY(QString valueFormat READ valueFormat WRITE setValueFormat)
Q_PROPERTY(int numMajorTicks READ numMajorTicks WRITE setNumMajorTicks)
Q_PROPERTY(int numMinorTicks READ numMinorTicks WRITE setNumMinorTicks)


public:

    enum displayLims {Channel_Limits = 0 , User_Limits};
    enum alarmLims  {Channel_Alarms = 0 , User_Alarms, None};

	/**
     * Color Mode to be used to paint the widget
	 */
	enum ColorMode 
	{
		GRADIENT, /**< show a bar with a gradient of colors */
		COLORBAR, /**< show a bar with different colors */
		SINGLECOLOR /**< show a bar whose color represents the state */
	};

	EAbstractGauge(QWidget * = NULL);
    ~EAbstractGauge(){}
	
	bool logarithmicScale() { return d_logarithmicScale; }
	void setLogarithmicScale(bool l);
	
	void setLowWarning(double w);
    double lowWarning(){ return d_naturalLowWarn; }

	void setHighWarning(double w);
    double highWarning(){ return d_naturalHighWarn; }

	void setLowError(double e);
    double lowError(){ return d_naturalLowErr; }

	void setHighError(double e);
    double highError(){ return d_naturalHighErr; }

	void setMinValue(double v);
    double minValue(){ return d_naturalMinV; }

	void setMaxValue(double v);
    double maxValue(){ return d_naturalMaxV; }
	
    double value(){ return d_naturalValue; }
	
	void setReference(double r);
    double reference(){ return d_naturalReference; }
	
	void setScaleEnabled(bool b);
    bool isScaleEnabled(){ return m_scaleEnabled; }

    void setDisplayLimits(displayLims lims) {m_displayLimits = lims; }
    displayLims getDisplayLimits(){ return m_displayLimits; }

    void setAlarmLimits(alarmLims lims) {m_alarmLimits = lims;}
    alarmLims getAlarmLimits(){return m_alarmLimits; }
	
	void setReferenceEnabled(bool b);
    bool isReferenceEnabled(){ return m_referenceEnabled; }
	
	void setColorMode(ColorMode m);
    ColorMode colorMode(){ return m_colorMode; }

	void setExternalScale(bool b);
    bool externalScale(){ return m_externalScale; }

	void setNumMajorTicks(int t);
	int numMajorTicks(){ return m_numMajorTicks; }
	
	void setNumMinorTicks(int t);
	int numMinorTicks(){ return m_numMinorTicks; }
	
    void setValueFormat(QString s){ m_valueFormat = s; configure(); update(); }
    QString valueFormat(){ return m_valueFormat; }

    void setConnected(bool c);
    bool isConnected() { return m_connected;}
	
public slots:
	void setValue(double);
	void setValue(int);

protected:
    /* these might store the values in natural or as logarithmic */
	double m_minValue, m_maxValue;
	double m_lowWarning, m_highWarning;
	double m_lowError, m_highError;
	double m_value,  m_reference;
	/* the following always store the natural values, never transformed into logarithmic */
	double d_naturalMinV, d_naturalMaxV, d_naturalLowWarn, d_naturalHighWarn,
           d_naturalLowErr, d_naturalHighErr, d_naturalValue, d_naturalReference;
	
	ColorMode	m_colorMode;
	bool 		m_referenceEnabled;

    alarmLims  m_alarmLimits;
    displayLims m_displayLimits;
	bool 		m_scaleEnabled;

    bool        m_connected;
    ColorMode   m_colorModeSaved;
    int		    m_numMajorTicks, m_numMinorTicks;
	QStringList	labels;
    int		    longestLabelIndex;
	bool		m_externalScale;
	
	QVector<qreal>	v_p;
	QVector<QColor>	v_c;
	QColor		m_normalColor;
	QColor		m_warningColor;
	QColor		m_errorColor;
	QString 	m_valueFormat;
	
	virtual void configure();
	
  private:
	bool d_logarithmicScale;
	double logarithm(double);
};

#endif
