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

#include <QResizeEvent>
#include <QPainter>
#include "elabel.h"

ELabel::ELabel(QWidget *parent) : ESimpleLabel(parent)
{
        m_falseColor = QColor(Qt::red);
        m_trueColor = QColor(Qt::green);
	m_falseString = QString("Zero");
	m_trueString = QString("One");

	val = QVariant("No Data");
	last_val = QVariant("Last value not initializeth");
    setAlignment(Qt::AlignLeft);
	pal = palette();
    if(pal.color(backgroundRole()) != QColor(Qt::white))
	{
        pal.setColor(backgroundRole(), QColor(Qt::white));
		setPalette(pal);
	}
	setFrameShape(QFrame::WinPanel);
	setFrameShadow(QFrame::Sunken);
	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	setAutoFillBackground(true);
}

void ELabel::setValue(QVariant v, bool ref)
{
	if ((v.type() == last_val.type()) && (last_val == v))
	{
		//last_val = v;
		qDebug() << "non aggiorno";
		return;
	}
	else
		last_val = val = v;

	if (ref) 
		display();
}

void ELabel::setTrueString(QString s) 
{
	m_trueString = s; 
	display(); 
}

void ELabel::setFalseString(QString s) 
{
	m_falseString = s;
	display(); 
}
		
void ELabel::setTrueColor(QColor c) 
{
	m_trueColor = c; 
	display(); 
}

void ELabel::setFalseColor(QColor c) 
{
	m_falseColor = c;
	display();
}

void ELabel::setBooleanDisplay(QString fS, QString tS, QColor fC, QColor tC)
{
	m_falseString = fS;
	m_trueString = tS;
	m_falseColor = fC;
	m_trueColor = tC;
	update();
}

/* The extended version has got the `SetEnumDisplay()' to configure
 * a string and a color for each desired value 
 */
void ELabel::setEnumDisplay(unsigned int value, QString label, QColor color)
{
	/* If a value has been already set, update it */
	for (int i = 0; i < v_values.size(); i++)
	{
		if (v_values[i] == value)
		{
			v_colors[i] = color;
			v_strings[i] = label;
	                display();
			return;
		}
	}
	/* Put the string and the color into their vectors */
	v_values.push_back(value);
	v_colors.push_back(color);
	v_strings.push_back(label);
	/* refresh the label */
	display();
	return;
}

void ELabel::clearEnumDisplay()
{
        v_values.clear();
        v_colors.clear();
        v_strings.clear();
}

bool ELabel::enumDisplayConfigured()
{
        return (v_values.size() > 0);
}

void ELabel::display()
{
	QPalette palette = this->palette();
	if (val.type() == QVariant::Bool)
	{
		if (val.toBool())
		{
			if (palette.color(backgroundRole()) != m_trueColor || text() != m_trueString)
			{
				palette.setColor(backgroundRole(), m_trueColor);
				setPalette(palette);
				setText(m_trueString);
			}
		}
		else
		{
//			qDebug() << "val.toBool() e` falso!";
			if (palette.color(backgroundRole()) != m_falseColor || m_falseString != text())
			{
				palette.setColor(backgroundRole(), m_falseColor);
				setPalette(palette);
				setText(m_falseString);
			}
		}
	}
	//else if (val.type() == QVariant::UInt)
	else if (val.canConvert(QVariant::UInt) && (v_colors.size()) && (!val.toString().contains("###")))
	{
		/* Look for the value `val' inside the v_values 
		 * vector, to see if a string and a color were 
		 * configured for that value.
		 */
		int index = v_values.indexOf(val.toUInt());
		if (index != -1)
		{
			if(palette.color(backgroundRole()) != v_colors[index])
			{
				palette.setColor(backgroundRole(), v_colors[index]);
				setPalette(palette);
			}
			setText(v_strings[index]);
		}
		else /* No string nor a colour for that value! */
		{
			if(palette.color(backgroundRole()) != QColor("white"))
			{
				palette.setColor(backgroundRole(), QColor("white") );
				setPalette(palette);
			}
			setText(QString("No match for value %1!").arg(val.toUInt() ) );
		}
//		setPalette(pal);
	}
	else
	{
		QString s = val.toString();
		if (s.contains("###"))
		{
			if(palette.color(backgroundRole()) != QColor("white"))
			{
				pal.setColor(backgroundRole(), Qt::white);
				setPalette(pal);
			}
		}
		setText(val.toString());
	}
	qDebug() << "text(): " << text() << "m_trueString: " << m_trueString << "m_falseString: " << m_falseString;
//	QLabel::paintEvent(e);
}




