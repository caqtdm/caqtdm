#include "earrow.h"
//#include "elettracolors.h"
#include <QPainter>
#include <QRadialGradient>
#include <QtDebug>

EArrow::EArrow( QWidget *parent ) : QWidget( parent ) {
    setLineSize( 2 );
    setArrowSize(10);
    setLineColor(Qt::black);
    setBrushColor(Qt::gray);
    setArrowMode(RIGTH);
    setArrowDirection(HORIZONTAL);
}

void EArrow::setLineSize(int size ) {
    if (size > 0) {
        m_lineSize = size;
        update();
    }
}

void EArrow::setLineColor( QColor c ) {
    m_lineColor = c;
    update();
}

void EArrow::setBrushColor( QColor c ) {
    m_brushColor = c;
    update();
}

void EArrow::setArrowSize( int size ) {
    if (size >= 0) {
        m_arrowSize = size;
        update();
    }
}

void EArrow::setArrowMode( ArrowMode m ) {
    m_arrowMode = m;
    update();
};

void EArrow::setArrowDirection( ArrowDirection d ) {
    m_arrowDirection = d;
    update();
};

QPolygonF EArrow::getHead( QPointF p1, QPointF p2 ) {
    qreal Pi = 3.14;
    QPolygonF arrowHead;
    QLineF m_line = QLineF( p1, p2 );
    double angle = ::acos( m_line.dx() / m_line.length() );

    if ( m_line.dy() >= 0 )  angle = ( Pi * 2 ) - angle;

    QPointF arrowP1 = m_line.p1() + QPointF( sin( angle + Pi / 3 ) * getArrowSize(),
                      cos( angle + Pi / 3 ) * getArrowSize() );
    QPointF arrowP2 = m_line.p1() + QPointF( sin( angle + Pi - Pi / 3 ) * getArrowSize(),
                      cos( angle + Pi - Pi / 3 ) * getArrowSize() );

    arrowHead.clear();
    arrowHead << m_line.p1() << arrowP1 << arrowP2 ;

    return arrowHead;
}

void EArrow::paintEvent( QPaintEvent * ) {
    QPointF p1,p2;
    QLineF polyLine;
    QPainter painter( this );

    painter.setRenderHint( QPainter::Antialiasing );
    painter.setPen( QPen( getLineColor(), getLineSize(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ) );
    painter.setBrush( getBrushColor() );

    int m_margin = 3;

    if (VERTICAL == getArrowDirection()) {
        p1 = QPointF( width()/2, m_margin );
        p2 = QPointF( width()/2, height()-m_margin );
    } else if (RIGTHBOTTOM == getArrowDirection()) {
        p1 = QPointF( m_margin, m_margin );
        p2 = QPointF( width()-m_margin, height()-m_margin );
    } else if (RIGTHTOP == getArrowDirection()) {
        p1 = QPointF( m_margin, height()-m_margin );
        p2 = QPointF( width()-m_margin, m_margin );
    } else { // HORIZONTAL
        p1 = QPointF( m_margin, height()/2 );
        p2 = QPointF( width()-m_margin, height()/2 );
    }

    polyLine = QLineF( p1, p2 );
    painter.drawLine(polyLine);

    if (LEFT == getArrowMode()) {
        painter.drawPolygon( getHead(p1,p2) );
    } else if (RIGTH == getArrowMode()) {
        painter.drawPolygon(getHead(p2,p1));
    } else if (DOUBLE == getArrowMode()) {
        painter.drawPolygon(getHead(p1,p2));
        painter.drawPolygon(getHead(p2,p1));
    }
}

