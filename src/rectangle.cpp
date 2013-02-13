//******************************************************************************
// Copyright (c) 2012 Paul Scherrer Institut PSI), Villigen, Switzerland
// Disclaimer: neither  PSI, nor any of their employees makes any warranty
// or assumes any legal liability or responsibility for the use of this software
//******************************************************************************
//******************************************************************************
//
//     Author : Anton Chr. Mezger
//
//******************************************************************************

#include <QPaintEvent>

#include <QPainter>
#include <QDebug>

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

