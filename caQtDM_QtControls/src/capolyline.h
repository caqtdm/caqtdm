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

#ifndef CAPOLYLINE_H
#define CAPOLYLINE_H

#include <QWidget>
#include <qtcontrols_global.h>

QT_BEGIN_NAMESPACE
class QRect;
class QSize;
QT_END_NAMESPACE

class QTCON_EXPORT caPolyLine : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QColor foreground READ getForeground WRITE setForeground)
    Q_PROPERTY(LineStyle linestyle READ getLineStyle WRITE setLineStyle)
    Q_PROPERTY(int lineSize READ getLineSize WRITE setLineSize )
    Q_PROPERTY(QColor lineColor READ getLineColor WRITE setLineColor )
    Q_PROPERTY(colMode colorMode READ getColorMode WRITE setColorMode)
    Q_PROPERTY(QString xyPairs   READ getXYpairs WRITE setXYpairs)

    Q_PROPERTY(FillStyle fillstyle READ getFillStyle WRITE setFillStyle)
    Q_PROPERTY(PolyStyle polystyle READ getPolyStyle WRITE setPolyStyle)

#include "caVisibProps.h"
#include "caVisibDefs.h"

    Q_ENUMS(colMode)
    Q_ENUMS(LineStyle)
    Q_ENUMS(FillStyle)
    Q_ENUMS(PolyStyle)

public:

    enum LineStyle {Solid = 0, Dash, BigDash};
    enum colMode {Static=0, Alarm};

    enum FillStyle {Filled = 0, Outline};
    enum PolyStyle {Polyline = 0, Polygon};

    QString getXYpairs() const {return thisXYpairs;}
    void setXYpairs(QString const &pairs) {thisXYpairs = XYpairs = pairs; update();}

    QColor getForeground() const {return thisForeColor;}
    void setForeground(QColor c);

    colMode getColorMode() const { return thisColorMode; }
    void setColorMode(colMode colormode);

    void setAlarmColors(short status);

    void setLineSize( int size );
    int getLineSize() {
        return thisLineSize;
    }

    void setLineColor( QColor c );
    QColor getLineColor() {
        return thisLineColor;
    }

    LineStyle getLineStyle() const { return thisLineStyle; }
    void setLineStyle(LineStyle style);

    FillStyle getFillStyle() const { return thisFillStyle; }
    void setFillStyle(FillStyle style);

    PolyStyle getPolyStyle() const { return thisPolyStyle; }
    void setPolyStyle(PolyStyle style);

    caPolyLine(QWidget *parent = 0);

    void setPairs(const QString &newPairs);
    QString pairs() const;
    void clearPairs();

    void setInEditor(bool in);
    void setEditSize(int x, int y, int w, int h);

    void setHide(bool hide);

    void setActualSize(QSize size);

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

private:

    QString thisXYpairs;
    QString XYpairs;
    QColor thisLineColor, oldLineColor;
    int thisLineSize;
    QColor thisForeColor, oldForeColor;
    colMode thisColorMode;
    LineStyle thisLineStyle;
    FillStyle thisFillStyle;
    PolyStyle thisPolyStyle;

    int actualWidth, actualHeight;
    bool initialize;
    bool inEditor;
    bool inDesigner;

    QPointF actualPosition;
    QPointF lastPosition;
    bool mouseMove;
    QRect editSize;

    bool thisHide;

};

#endif
