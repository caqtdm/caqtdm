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

#include "caVisib.h"

    Q_ENUMS(colMode)
    Q_ENUMS(LineStyle)
    Q_ENUMS(FillStyle)
    Q_ENUMS(PolyStyle)

public:

#include "caVisibPublic.h"

    enum LineStyle {Solid = 0, Dash, BigDash};
    enum colMode {Static=0, Alarm};

    enum FillStyle {Filled = 0, Outline};
    enum PolyStyle {Polyline = 0, Polygon};

    QString getXYpairs() const {return thisXYpairs;}
    void setXYpairs(QString const &pairs) {thisXYpairs = pairs; update();}

    QColor getForeground() const {return thisForeColor;}
    void setForeground(QColor c);

    colMode getColorMode() const { return thisColorMode; }
    void setColorMode(colMode colormode);

    void setAlarmColors(short status);

    int value() const;

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

    void setHidden(bool hide);

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

private:

#include "caVisibPrivate.h"

    QString thisXYpairs;
    QColor thisLineColor;
    int thisLineSize;
    QColor thisForeColor;
    QColor thisBackColor;
    colMode thisColorMode;
    LineStyle thisLineStyle;
    FillStyle thisFillStyle;
    PolyStyle thisPolyStyle;

    int actualWidth, actualHeight;
    bool initialize;
    bool inEditor;

    QPointF actualPosition;
    QPointF lastPosition;
    bool mouseMove;
    QRect editSize;

    bool thisHide;

};

#endif
