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

#ifndef RECTANGLE_H
#define RECTANGLE_H

#include <QWidget>

class  rectangle : public QWidget
{
    Q_OBJECT

public:

    rectangle( QWidget *parent = 0 );

    QColor readFG() const {return thisForeColor;}
    void writeFG(QColor c);

    QColor readBG() const {return thisBackColor;}
    void writeBG(QColor c);

    int value() const;

    void setLineColor( QColor c );
    QColor getLineColor() { return m_lineColor;}

    void setLineSize( int size );
    int getLineSize() {return m_lineSize;}


public slots:

    void setValue( int );

signals:

    void valueChanged( int );

protected:

    void paintEvent( QPaintEvent* );

private:

    int m_value;
    QColor m_lineColor;
    int m_lineSize;

    QColor thisForeColor;
    QColor thisBackColor;
};

#endif // RECTANGLE
