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

/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_THERMOMARKER_H
#define QWT_THERMOMARKER_H

#include <qtcontrols_global.h>
#include "qwt_global.h"
#include "qwt_abstract_scale.h"
#include "qwt_interval.h"

class QwtScaleDraw;
class QwtColorMap;

/*!
  \brief The Thermometer Widget

  QwtThermo is a widget which displays a value in an interval. It supports:
  - a horizontal or vertical layout;
  - a range;
  - a scale;
  - an alarm level.

  \image html sysinfo.png

  The fill colors might be calculated from an optional color map
  If no color map has been assigned QwtThermo uses the 
  following colors/brushes from the widget palette:

  - QPalette::Base
    Background of the pipe
  - QPalette::ButtonText
    Fill brush below the alarm level
  - QPalette::Highlight
    Fill brush for the values above the alarm level
  - QPalette::WindowText
    For the axis of the scale
  - QPalette::Text
    For the labels of the scale
*/
class QTCON_EXPORT QwtThermoMarker: public QwtAbstractScale
{
    Q_OBJECT

    Q_ENUMS( ScalePos )

    Q_ENUMS( ScalePosition )
    Q_ENUMS( OriginMode )
    Q_ENUMS( DisplayType )

    Q_PROPERTY( Qt::Orientation orientation READ orientation WRITE setOrientation DESIGNABLE false)
    Q_PROPERTY( ScalePosition scalePosition READ scalePosition WRITE setScalePosition )

    Q_PROPERTY( ScalePos scalePos READ scalePos WRITE setScalePos  DESIGNABLE false)

    Q_PROPERTY( OriginMode originMode READ originMode WRITE setOriginMode DESIGNABLE false)

    Q_PROPERTY( bool alarmEnabled READ alarmEnabled WRITE setAlarmEnabled DESIGNABLE false)
    Q_PROPERTY( double alarmLevel READ alarmLevel WRITE setAlarmLevel DESIGNABLE false)
    Q_PROPERTY( double origin READ origin WRITE setOrigin DESIGNABLE false)
    Q_PROPERTY( int spacing READ spacing WRITE setSpacing )
    Q_PROPERTY( int borderWidth READ borderWidth WRITE setBorderWidth )
    Q_PROPERTY( double maxValue READ maxValue WRITE setMaxValue )
    Q_PROPERTY( double minValue READ minValue WRITE setMinValue )
    Q_PROPERTY( int pipeWidth READ pipeWidth WRITE setPipeWidth )
    Q_PROPERTY( double value READ value WRITE setValue )
    Q_PROPERTY(DisplayType type READ getType WRITE setType)

public:

    enum DisplayType {Pipe, Marker, PipeFromCenter};
    DisplayType getType() const { return thisType; }
    void setType(DisplayType displaytype) {thisType = displaytype; layoutThermo(true);}

    enum ScalePosition
    {
        //! No scale
        NoScale,

        //! The scale is left of the pipe
        LeftScale,

        //! The scale is right of the pipe
        RightScale,

        //! The scale is above the pipe
        TopScale,

        //! The scale is below the pipe
        BottomScale
    };

    /*!
      Position of the scale
      \sa setScalePosition(), setOrientation()
     */
    enum ScalePos
    {
        //! The slider has no scale
        NoScales,

        //! The scale is right of a vertical or below of a horizontal slider
        LeadingScale,

        //! The scale is left of a vertical or above of a horizontal slider
        TrailingScale
    };

    /*!
      Origin mode. This property specifies where the beginning of the liquid
      is placed.

      \sa setOriginMode(), setOrigin()
    */
    enum OriginMode
    {
        //! The origin is the minimum of the scale
        OriginMinimum,

        //! The origin is the maximum of the scale
        OriginMaximum,

        //! The origin is specified using the origin() property
        OriginCustom
    };

    explicit QwtThermoMarker( QWidget *parent = NULL );
    virtual ~QwtThermoMarker();

    void setOrientation( Qt::Orientation );
    Qt::Orientation orientation() const;

    void setScalePosition( ScalePosition);
    ScalePosition scalePosition() const;

    void setScalePos( ScalePos s );
    ScalePos scalePos() const;

    void setMaxValue( double maxValue );
    double maxValue() const;
    void setMinValue( double minValue );
    double minValue() const;

    void setSpacing( int );
    int spacing() const;

    void setBorderWidth( int w );
    int borderWidth() const;

    void setOriginMode( OriginMode );
    OriginMode originMode() const;

    void setOrigin( double );
    double origin() const;

    void setFillBrush( const QBrush &b );
    QBrush fillBrush() const;

    void setAlarmBrush( const QBrush &b );
    QBrush alarmBrush() const;

    void setAlarmLevel( double v );
    double alarmLevel() const;

    void setAlarmEnabled( bool tf );
    bool alarmEnabled() const;

    void setColorMap( QwtColorMap * );
    QwtColorMap *colorMap();
    const QwtColorMap *colorMap() const;

    void setPipeWidth( int w );
    int pipeWidth() const;

    void setRangeFlags( QwtInterval::BorderFlags );
    QwtInterval::BorderFlags rangeFlags() const;

    double value() const;

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

    void setScaleDraw( QwtScaleDraw * );
    const QwtScaleDraw *scaleDraw() const;

public Q_SLOTS:
    virtual void setValue( double val );

protected:
    virtual void drawLiquid( QPainter *, const QRect & ) const;
    virtual void scaleChange();

    virtual void paintEvent( QPaintEvent * );
    virtual void resizeEvent( QResizeEvent * );
    virtual void changeEvent( QEvent * );

    QwtScaleDraw *scaleDraw();

    QRect pipeRect() const;
    QRect fillRect( const QRect & ) const;
    QRect alarmRect( const QRect & ) const;

private:
    void layoutThermo( bool );

    class PrivateData;
    PrivateData *d_data;
    DisplayType thisType;
};

#endif
