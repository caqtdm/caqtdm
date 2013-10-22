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

#ifndef CAGRAPHICS_H
#define CAGRAPHICS_H

#include <QWidget>
#include <qtcontrols_global.h>

class  QTCON_EXPORT caGraphics : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QColor foreground READ getForeground WRITE setForeground)
    Q_PROPERTY(LineStyle linestyle READ getLineStyle WRITE setLineStyle)
    Q_PROPERTY(FillStyle fillstyle READ getFillStyle WRITE setFillStyle)
    Q_PROPERTY(int lineSize READ getLineSize WRITE setLineSize )
    Q_PROPERTY(QColor lineColor READ getLineColor WRITE setLineColor )
    Q_PROPERTY(colMode colorMode READ getColorMode WRITE setColorMode)

#include "caVisib.h"

    Q_PROPERTY(Form form READ getForm WRITE setForm)

    Q_PROPERTY( int arrowSize READ getArrowSize WRITE setArrowSize DESIGNABLE true)
    Q_PROPERTY( ArrowMode arrowMode READ getArrowMode WRITE setArrowMode DESIGNABLE true)

    Q_PROPERTY( int tiltAngle READ getTiltAngle WRITE setTiltAngle DESIGNABLE true)
    Q_PROPERTY( int startAngle READ getStartAngle WRITE setStartAngle DESIGNABLE true)
    Q_PROPERTY( int spanAngle READ getSpanAngle WRITE setSpanAngle DESIGNABLE true)

    Q_ENUMS(Form)
    Q_ENUMS(colMode)
    Q_ENUMS(LineStyle)
    Q_ENUMS(FillStyle)
    Q_ENUMS(ArrowMode)

public:

#include "caVisibPublic.h"

    enum LineStyle {Solid = 0, Dash, BigDash};
    enum FillStyle {Filled = 0, Outline};
    enum Form {Rectangle = 0, Circle, Arc, Triangle, Line, Arrow};

    enum ArrowMode { Single, Double};

    enum colMode {Static=0, Alarm};

    caGraphics( QWidget *parent = 0 );

    QColor getForeground() const {return thisForeColor;}
    void setForeground(QColor c);

    colMode getColorMode() const { return thisColorMode; }
    void setColorMode(colMode colormode);

    int value() const;

    void setLineSize( int size );
    int getLineSize() {
        return thisLineSize;
    }

    void setLineColor( QColor c );
    QColor getLineColor() {
        return thisLineColor;
    }

    Form getForm() const { return thisForm; }
    void setForm(Form form);

    FillStyle getFillStyle() const { return thisFillStyle; }
    void setFillStyle(FillStyle fillstyle);

    LineStyle getLineStyle() const { return thisLineStyle; }
    void setLineStyle(LineStyle linestyle);

    void setArrowSize( int size );
    int getArrowSize() {return thisArrowSize;}

    void setStartAngle( int angle );
    int getStartAngle() {return thisStartAngle;}

    void setTiltAngle( int angle );
    int getTiltAngle() {return thisTiltAngle;}

    void setSpanAngle( int angle );
    int getSpanAngle() {return thisSpanAngle;}

    void setArrowMode( ArrowMode mode );
    ArrowMode getArrowMode() {return thisArrowMode;}

    void setAlarmColors(short status);

    bool setPropertyEditorItems(QWidget *pTheWidget, QStringList QStringListToChange, bool bShowProperties);

    void setHidden(bool hide);

public slots:


signals:


protected:

    void paintEvent( QPaintEvent* );

private:

#include "caVisibPrivate.h"

    QPolygonF getHead( QPointF, QPointF );
    int thisLineSize;

    Form thisForm;
    QColor thisForeColor, thisForeColorOld;
    QColor thisLineColor, thisLineColorOld;
    colMode thisColorMode;
    FillStyle thisFillStyle;
    LineStyle thisLineStyle;
    QPoint lastPoint;
    int thisArrowSize;
    ArrowMode thisArrowMode;
    int thisStartAngle, thisSpanAngle, thisTiltAngle;

    QPolygonF drawCircle(int x1, int x2, int y1, int y2);
    QPolygonF rotateObject(int degrees, int w, int h, int linesize, const QPolygonF& object);

    bool thisHide;
};

#endif // CIRCLEBAR_H
