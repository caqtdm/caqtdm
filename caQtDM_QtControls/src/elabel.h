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

#ifndef ELABEL_H
#define ELABEL_H

#include <QVariant>
#include <QString>
#include <QColor>
#include <QtDebug>
#include <qtcontrols_global.h>
#include <esimplelabel.h>

/**
 * \brief A label that can be used to display numeric, boolean or enum values
 */
class QTCON_EXPORT ELabel : public ESimpleLabel
{
Q_OBJECT
Q_PROPERTY(QColor trueColor    READ trueColor   WRITE setTrueColor   DESIGNABLE false)
Q_PROPERTY(QColor falseColor   READ falseColor  WRITE setFalseColor  DESIGNABLE false)
Q_PROPERTY(QString trueString  READ trueString  WRITE setTrueString  DESIGNABLE false)
Q_PROPERTY(QString falseString READ falseString  WRITE setFalseString DESIGNABLE false)

public:
	ELabel(QWidget *parent);

        virtual ~ELabel() {}

	void setValue(QVariant v, bool ref=true);
        QVariant value() const { return val; }

	void setBooleanDisplay(QString, QString, QColor, QColor);

	void setEnumDisplay(unsigned int value, QString label, QColor color);

	void setTrueString(QString s);
        QString trueString() const { return m_trueString; }

	void setFalseString(QString s);
        QString falseString() const { return m_falseString; }

	void setTrueColor(QColor c);
        QColor trueColor() const { return m_trueColor; }

	void setFalseColor(QColor c);
        QColor falseColor() const { return m_falseColor; }

	bool enumDisplayConfigured();

	void clearEnumDisplay();

	void display();

protected:

	QVariant val, last_val;

	/* to display boolean values*/
	QColor m_falseColor;
	QColor m_trueColor;
	QString m_falseString;
	QString m_trueString;
	QPalette pal;

	/* to display enum values */
	QVector<unsigned int> v_values;
	QVector<QColor> v_colors;
	QVector<QString> v_strings;
};

#endif  /* ELABEL_H */
