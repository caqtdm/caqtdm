#ifndef _EARROW_H
#define _EARROW_H

#include <QWidget>
#include <math.h>
#include <QtGui>
#include <QGraphicsLineItem>
#include <qtcontrols_global.h>

/**
 * \brief An arrow
 */
class QTCON_EXPORT EArrow : public QWidget {
    Q_OBJECT
    Q_ENUMS(ArrowMode)
    Q_ENUMS(ArrowDirection)
    Q_PROPERTY( int lineSize READ getLineSize WRITE setLineSize )
    Q_PROPERTY( QColor lineColor READ getLineColor WRITE setLineColor )
    Q_PROPERTY( QColor brushColor READ getBrushColor WRITE setBrushColor )
    Q_PROPERTY( int arrowSize READ getArrowSize WRITE setArrowSize )
    Q_PROPERTY( ArrowMode arrowMode READ getArrowMode WRITE setArrowMode )
    Q_PROPERTY( ArrowDirection arrowDirection READ getArrowDirection WRITE setArrowDirection )

public:
    EArrow( QWidget* );
    ~EArrow() {};

    enum ArrowMode { LEFT, RIGTH, DOUBLE, NONE};

    enum ArrowDirection { VERTICAL, HORIZONTAL, RIGTHBOTTOM, RIGTHTOP };

    void setLineSize( int size );
    int getLineSize() {
        return m_lineSize;
    }

    void setLineColor( QColor c );
    QColor getLineColor() {
        return m_lineColor;
    }

    void setBrushColor( QColor c );
    QColor getBrushColor() {
        return m_brushColor;
    }

    void setArrowSize( int size );
    int getArrowSize() {
        return m_arrowSize;
    }

    void setArrowMode( ArrowMode m );
    ArrowMode getArrowMode() {
        return m_arrowMode;
    };

    void setArrowDirection( ArrowDirection m );
    ArrowDirection getArrowDirection() {
        return m_arrowDirection;
    };

protected:
    virtual void paintEvent( QPaintEvent* );
    QColor m_lineColor, m_brushColor;
    int m_lineSize, m_arrowSize;
    ArrowMode m_arrowMode;
    ArrowDirection m_arrowDirection;

private:
    QPolygonF getHead( QPointF, QPointF );
};

#endif
