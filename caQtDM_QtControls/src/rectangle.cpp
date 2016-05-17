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

#include <QPaintEvent>
#include <QPainter>
#include "rectangle.h"

rectangle::rectangle( QWidget *parent ) : QWidget( parent )
{
    m_value = 0;

    QSizePolicy policy( QSizePolicy::Expanding, QSizePolicy::Expanding);
    policy.setHeightForWidth( true );
    setSizePolicy( policy );
    setLineSize(1);
    setLineColor(Qt::black);
    writeFG(Qt::red);
    writeBG(Qt::gray);
}

void rectangle::setLineSize(int size ) {
    if (size > 0) {
        m_lineSize = size;
        update();
    }
}

void rectangle::setLineColor( QColor c ) {
    m_lineColor = c;
    update();
}

void rectangle::writeFG(QColor c)
{
    thisForeColor = c;
    update();
}

void rectangle::writeBG(QColor c)
{
    thisBackColor = c;
    update();
}

int rectangle::value() const
{
    return m_value;
}

void rectangle::setValue( int value )
{
    if( value < 0 )   value = 0;
    if( value > 100 ) value = 100;
    if( m_value == value ) return;

    m_value = value;

    update();

    emit valueChanged( m_value );
}


void rectangle::paintEvent( QPaintEvent *event )
{
    Q_UNUSED(event);
    QPainter p( this );
    p.setPen( QPen( getLineColor(), getLineSize(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ) );
    p.setBrush(readFG());
    p.drawRect(0, 0, width()-1, height()-1 );
}

