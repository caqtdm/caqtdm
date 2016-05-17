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

#ifndef EFLAG_H
#define EFLAG_H

#include <QLabel>
#include <QVariant>
#include <QString>
#include <QColor>
#include <QList>
#include <qtcontrols_global.h>
#include "elabel.h"

class QTCON_EXPORT QGridLayout;

/**
 * \brief A widget composed by a matrix of ELabel
 *
 * this object displays a matrix of boolean values. you can configure the color and the
 * text of each cell.
 * The input value is a vector of boolean values or an integer which is interpreted as
 * a bitmask. the function setDisplayMask allows you to decide which bits must be displayed
 * and in which cell
 */
class QTCON_EXPORT EFlag : public QWidget
{
Q_OBJECT

Q_PROPERTY(int numRows READ readNumRows WRITE setNumRows)
Q_PROPERTY(int numColumns READ readNumColumns WRITE setNumColumns)
Q_PROPERTY(ESimpleLabel::ScaleMode fontScaleMode READ fontScaleMode WRITE setFontScaleMode)

Q_ENUMS(ScaleMode)
Q_ENUMS(alignmentHor)
Q_PROPERTY(QString trueColors   READ trueColors     WRITE setTrueColors   DESIGNABLE true)
Q_PROPERTY(QString falseColors  READ falseColors    WRITE setFalseColors  DESIGNABLE true)
Q_PROPERTY(QString trueStrings  READ trueStrings    WRITE setTrueStrings  DESIGNABLE true)
Q_PROPERTY(QString falseStrings READ falseStrings   WRITE setFalseStrings DESIGNABLE true)
Q_PROPERTY(QString displayMask  READ getDisplayMask WRITE setDisplayMask  DESIGNABLE true)

public:

    enum alignmentHor {left, right, center};

	EFlag(QWidget *parent);

	/**
	 * sets the value of the widget
	 * @param	v	the value to be displayed
	 * @param       ref  whether to refresh the widget or not
	 * The parameter `v' can be a list of boolean values or
	 * an unsigned int to which the display mask will be applied
	 * in order to extract the wanted fields.
	 */
	void setValue(QVariant v, bool ref=true);

	/**
	 * sets the value of the widget
	 * @param	v	the value to be displayed
	 * @param       ref  whether to refresh the widget or not
	 */
//	void setValue(unsigned int v, bool ref = true);

	/**
	 * returns the value of the widget
	 * @return	the displayed value
	 */
        QList<QVariant> readValue() const { return data; }

	/**
	 *	This one is used on behalf of the designer. A string is splitted and
	 *	then each element is put in an unsigned int vector finally passed
	 *	to setDisplayMask(QList<unsigned int>).
	 *	The format of the string must be a sequence of unsigned int separated
	 *	by a comma.
	 */
	void setDisplayMask(QString var);

	/** Returns the generic mask expressed as QList<QVariant> */
        const QString getDisplayMask() { return string_mask; }

	/**
	 * configures which bits have to be displayed and where
	 * In this version, which accepts a list of unsigned,
	 * a subsequent call to setValue() will mask its value
	 * with the bitmask specified here.
	 * For instance, if we write a piece of code like this:
	 *
	 * 	QList<unsigned int >  uintlist;
	 *     	uintlist << 2; uintlist << 0; uintlist << 1;
	 *      f->setDisplayMask(uintlist);
	 *
	 * a call to setValue(5) will produce:
	 *
	 * true, true, false
	 *
	 * since the value 5, which is 101 binary coded, is first masked
	 * with 100 (the element 2 in the list sets the 3rd bit to 1 leaving
	 * the others to 0), then 001 (the element 0 in the list sets to 1
	 * the first bit), then 010 (the last element in the list, 1).
	 *
	 * @param m a vector, which i-th value contains the position of the bit to be displayed in the i-th cell
	 */
	void setDisplayMask(QList<unsigned int> m);

	/** configures which bits are to be displayed
	 *  @param qlql	a list of a list containing the desired bit mask.
	 *  For instance, qlql could be a list containing the values
	 *  <3,4>, <15,16>, <8,11,12> if we are interested in reading the bits 3 and 4
	 *  and the bits 15 and 16 and the bit 8 together with 11 and 12.
	 *  The bits suggested in each list will mask the value (e.g. `val') passed as argument to
	 *  setValue(val) and extract the value represented by them inside `val'.
	 *
	 *  For instance, a mask of <0, 1> and <0, 2> applied to the value 5 (binary: 101)
	 *  will produce:
	 *
	 *  <0, 1> 5 becomes 1 (101 is ANDed with 011)
	 *  <0, 2>
	 *
	 */
	void setDisplayMask(QList <QList<unsigned int> > qlql);

	void initDisplayMask(void);

	/** Returns the nested lists which made up the display mask */
        QList< QList<unsigned int> >  displayMask() const { return mask; }

	/**
	 * configures the number of rows of the widget
	 * @param rows	the number of rows
	 */
        void setNumRows(int rows) { if (rows < 1) return; numRows = rows; arrangeCells(); }

	/**
	 * returns the current number of rows of the widget
	 * @return number of rows
	 */
        int readNumRows() const { return numRows; }

	/**
	 * configures the number of columns of the widget
	 * @param columns	the number of columns
	 */
        void setNumColumns(int columns) { if (columns < 1) return; numColumns = columns; arrangeCells(); }

	/**
	 * returns the current number of columns of the widget
	 * @return number of columns
	 */
        int readNumColumns() const { return numColumns; }

	/**
	 * configure colors and labels to be displayed in each cell
	 * @param i	cell to be configured
	 * @param falseString	string to be displayed when te value is false
	 * @param trueString	string to be displayed when te value is true
	 * @param falseColor	color to be displayed when te value is false
	 * @param trueColor	color to be displayed when te value is true
	 */
	void setBooleanDisplay(int i, QString falseString, QString trueString, QColor falseColor, QColor trueColor);

	/**
	 * The extended version associates a color and a string to a value.
	 * @param cell	the cell to be configured
	 * @param value	the value to which one associates a color and a string on the cell `cell'
	 * @param corresponding_string	the string to be shown when the value is `value'
	 * @param corresponding_color	the color that characterizes the value `value'
	 */
	void setEnumDisplay(int cell, unsigned int value, QString corresponding_str, QColor corresponding_color);

	void setTrueStrings(QString s);
	QString trueStrings();

	void setFalseStrings(QString s);
	QString falseStrings();

	void setTrueColors(QString c);
	QString trueColors();

	void setFalseColors(QString c);
	QString falseColors();

	/** \brief set the list of true colours specifying a QList of QColor elements
	 *
	 * Used to auto configure the TTable. You may use it with the designer too.
	 */
	void setTrueColorList(const QList<QColor> &cl);

	/** \brief returns the list of true colours, made up of QColor elements.
	 *
	 * @see setTrueColorList
	 * @see setFalseColorList
	 */
	QList<QColor> trueColorList() const;

	/** \brief set the list of false colours specifying a QList of QColor elements
	 *
	 * Used to auto configure the TTable. You may use it with the designer too.
	 */
	void setFalseColorList(const QList<QColor> &cl);

	/** \brief returns the list of false colours, made up of QColor elements.
	 *
	 * @see setFalseColorList
	 * @see setTrueColorList
	 */
	QList<QColor> falseColorList() const;

	void setMouseTracking(bool en);

	QList<ELabel*> cells;

	void setFontScaleMode(ESimpleLabel::ScaleMode m);

	ESimpleLabel::ScaleMode fontScaleMode();

    void setTextAlignment(alignmentHor m);
    void setFont(const QFont & font);

protected:

	void arrangeCells();
	void configureCells();

private:
    int	                numRows;
    int 	            numColumns;

	QList<QVariant>		m_falseColors;
	QList<QVariant>		m_trueColors;
    QVariant 		    last_val;
    QStringList		    m_falseStrings;
    QStringList		    m_trueStrings;

	QList<QVariant>		data;
	QList<QList <unsigned int> > mask;
    QString 		    string_mask; /* For the designer plugin */
	QGridLayout 		*grid;
};

#endif  /* EFLAG_H */
