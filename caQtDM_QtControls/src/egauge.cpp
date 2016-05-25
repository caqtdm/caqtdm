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

#include "egauge.h"
#include <QPainter>
#include <QPaintEvent>
#include <QConicalGradient>
#include <QRadialGradient>
#include <QPolygonF>
#include <QApplication>
#include <eng_notation.h>

#include <QtDebug>

#define ORANGE QColor(243, 186, 7)

EAbstractGauge::EAbstractGauge(QWidget *parent) : QWidget(parent),
    m_colorMode(COLORBAR),
    m_referenceEnabled(true), m_scaleEnabled(true),
	m_numMajorTicks(LINEAR_MAJOR_TICKS), longestLabelIndex(-1),
        m_normalColor(QColor(Qt::green)), m_warningColor(ORANGE), m_errorColor(QColor(Qt::red)),
	d_logarithmicScale(false)
{
	m_minValue = -15;
	m_maxValue = 15;
	m_lowError = -10;
	m_highError = 10;
	m_lowWarning = -5;
	m_highWarning = 5;
	m_reference = 3;
	m_value = 0;
    m_numMajorTicks = 2;
    m_numMinorTicks = 1;
	d_naturalMinV = m_minValue;
	d_naturalMaxV = m_maxValue;
	d_naturalLowWarn = m_lowWarning;
	d_naturalHighWarn = m_highWarning;

	d_naturalLowErr = m_lowError;
	d_naturalHighErr = m_highError;
	d_naturalValue = m_value;
	d_naturalReference = m_reference;

	m_valueFormat = "%.1f";

    m_displayLimits = Channel_Limits;
    m_alarmLimits = Channel_Alarms;
    m_connected = true;
    setConnected(true);

    configure();
    update();
}

double EAbstractGauge::logarithm(double v)
{
  double logar;
  if(v > 0)
	logar = log10(v);
  else
  {
    //printf("Gauge logarithmic scale but value %.2f <= 0", v);
	logar = 0;
  }
  return logar;
}

void EAbstractGauge::setLogarithmicScale(bool en)
{
  d_logarithmicScale = en;
  /* high and low warn */
  d_logarithmicScale ?  m_lowWarning = logarithm(d_naturalLowWarn) : m_lowWarning = d_naturalLowWarn;
  d_logarithmicScale ?  m_highWarning = logarithm(d_naturalHighWarn) : m_highWarning = d_naturalHighWarn;
  /* errors */
  d_logarithmicScale ?  m_lowError = logarithm(d_naturalLowErr) : m_lowError = d_naturalLowErr;
  d_logarithmicScale ?  m_highError = logarithm(d_naturalHighErr) : m_highError = d_naturalHighErr;
  /* min and max value */
  d_logarithmicScale ?  m_minValue = logarithm(d_naturalMinV) : m_minValue = d_naturalMinV;
  d_logarithmicScale ?  m_maxValue = logarithm(d_naturalMaxV) : m_maxValue = d_naturalMaxV;
  /* value */
  d_logarithmicScale ? m_value = logarithm(d_naturalValue) : m_value = d_naturalValue;
  /* reference */
  d_logarithmicScale ? m_reference = logarithm(d_naturalReference) : m_reference = d_naturalReference;
  configure();
  update();
}

void EAbstractGauge::setLowWarning(double w)
{
	d_naturalLowWarn = w;
	if(d_logarithmicScale)
	  m_lowWarning = logarithm(w);
	else
	  m_lowWarning = w;
	configure();
	update();
}

void EAbstractGauge::setHighWarning(double w)
{
	d_naturalHighWarn = w;
	if(d_logarithmicScale)
	  m_highWarning = logarithm(w);
	else
	  m_highWarning = w;

	configure();
	update();
}

void EAbstractGauge::setLowError(double e)
{
	d_naturalLowErr = e;
	if(d_logarithmicScale)
	  m_lowError = logarithm(e);
	else
	  m_lowError = e;

	configure();
	update();
}

void EAbstractGauge::setHighError(double e)
{
	d_naturalHighErr = e;
	if(d_logarithmicScale)
	  m_highError = logarithm(e);
	else
	  m_highError = e;

	configure();
	update();
}

void EAbstractGauge::setMinValue(double v)
{
  d_naturalMinV = v;
  if(d_logarithmicScale)
	m_minValue = logarithm(v);
  else
	m_minValue = v;
  configure();
  update();
}

void EAbstractGauge::setMaxValue(double v)
{
  d_naturalMaxV = v;
   if(d_logarithmicScale)
	m_maxValue = logarithm(v);
  else
	m_maxValue = v;
  configure();
  update();
}

void EAbstractGauge::setReference(double r)
{
	d_naturalReference = r;
	if(d_logarithmicScale)
	  m_reference = logarithm(r);
	else
	  m_reference = r;
	update();
}

void EAbstractGauge::setScaleEnabled(bool b)
{
	m_scaleEnabled = b;
	configure();
	update();
}

void EAbstractGauge::setReferenceEnabled(bool b)
{
	m_referenceEnabled = b;
	configure();
	update();
}

void EAbstractGauge::setColorMode(ColorMode m)
{
	m_colorMode = m;
	configure();
	update();
}

void EAbstractGauge::setExternalScale(bool b)
{
	m_externalScale = b;
	configure();
	update();
}

void EAbstractGauge::setNumMajorTicks(int t)
{
	m_numMajorTicks = t;
	configure();
}

void EAbstractGauge::setNumMinorTicks(int t)
{
	m_numMinorTicks = t;
	configure();
}

void EAbstractGauge::configure()
{
	qreal interval = m_maxValue - m_minValue;
	v_p.clear();
	v_c.clear();
	if (m_colorMode == GRADIENT)
	{
		v_p << m_minValue << (m_minValue+m_lowError)*.5;
		v_p << (m_lowError+m_lowWarning)*.5;
		v_p << (m_lowWarning+m_highWarning)*.5 << (m_highWarning+m_highError)*.5;
		v_p << (m_highError+m_maxValue)*.5 << m_maxValue;
		v_c << m_errorColor << m_errorColor << m_warningColor << m_normalColor;
		v_c << m_warningColor << m_errorColor << m_errorColor;
		if ((m_minValue == m_lowError) && (m_lowError == m_lowWarning))
			v_c[0] = v_c[1] = v_c[2] = m_normalColor;
		if ((m_maxValue == m_highError) && (m_highError == m_highWarning))
			v_c[4] = v_c[5] = v_c[6] = m_normalColor;
	}
	else if (m_colorMode == COLORBAR)
	{
		v_p << m_minValue << m_lowError << m_lowWarning;
		v_c << m_errorColor    << m_errorColor    << m_warningColor;
		v_p << m_highWarning << m_highError << m_maxValue;
		v_c << m_normalColor     << m_warningColor  << m_errorColor;
	}
	else
	{
        if(!m_connected)
            v_c << QColor(Qt::white);
        else if (m_value < m_lowError)
			v_c << m_errorColor;
		else if (m_value < m_lowWarning)
			v_c << m_warningColor;
		else if (m_value < m_highWarning)
			v_c << m_normalColor;
		else if (m_value < m_highError)
			v_c << m_warningColor;
		else
			v_c << m_errorColor;
	}

	for (int i = 0; i < v_p.size(); i++)
		v_p[i] = (v_p[i] - m_minValue) / interval;

	labels.clear();
	double val = m_minValue;
	double tick = (m_maxValue-m_minValue)/((m_numMajorTicks-1)*m_numMinorTicks);
	double step = qRound((m_maxValue-m_minValue)/(m_numMajorTicks-1)/tick)*tick;
	longestLabelIndex = -1;
	for (int i = 0; i < m_numMajorTicks; i++)
	{
        double representedValue;
#if defined(_MSC_VER)||defined(__APPLE__)||defined(__ANDROID__)
		d_logarithmicScale ? representedValue  = pow(10,val) : representedValue  = val;
#else
		d_logarithmicScale ? representedValue  = exp10(val) : representedValue  = val;
#endif

        EngString engString(QString().sprintf(m_valueFormat.toLatin1(), representedValue), m_valueFormat, representedValue);
// 		QString s = QString::number(representedValue);
		if ((longestLabelIndex == -1) || engString.length() > labels[longestLabelIndex].length())
			longestLabelIndex = i;
		labels << engString;
		val += step;
	}
}

void EAbstractGauge::setValue(double v)
{
  /* v is a natural value, not logarithmic */
	if(v == d_naturalValue)
		return;

	d_naturalValue = v;
	d_logarithmicScale ? m_value = logarithm(v) : m_value = v;

	m_value = qBound(m_minValue, m_value, m_maxValue);

	if (m_colorMode == SINGLECOLOR)
	{
		v_c.clear();

        if(!m_connected) {
            m_value=m_maxValue;
            v_c << QColor(Qt::white);
        } else if (m_value < m_lowError)
			v_c << m_errorColor;
		else if (m_value < m_lowWarning)
			v_c << m_warningColor;
		else if (m_value < m_highWarning)
			v_c << m_normalColor;
		else if (m_value < m_highError)
			v_c << m_warningColor;
		else
			v_c << m_errorColor;
	}
	update();
}

void EAbstractGauge::setValue(int v)
{
	setValue((double)v);
}


void EAbstractGauge::setConnected(bool c)
{
    if(m_connected) m_colorModeSaved = m_colorMode;
    m_connected = c;
    if(!c) {
        m_colorMode = SINGLECOLOR;
        configure();
        //setValue(m_maxValue);
    } else {
        m_colorMode =  m_colorModeSaved;
        configure();
    }
    update();
}

