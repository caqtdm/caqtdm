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

#if defined(_WIN32) || defined(_WIN64)
#define QWT_DLL
#endif

#include "qwt_thermo_marker_61.h"
#include "qwt_scale_engine.h"
#include "qwt_scale_draw.h"
#include "qwt_scale_map.h"
#include "qwt_color_map.h"
#include <qpainter.h>
#include <qevent.h>
#include <qdrawutil.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qmath.h>
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
#include <functional>
#endif





static inline void qwtDrawLine( QPainter *painter, int pos,
    const QColor &color, const QRect &pipeRect, const QRect &liquidRect,
    Qt::Orientation orientation )
{
    painter->setPen( color );
    if ( orientation == Qt::Horizontal )
    {
        if ( pos >= liquidRect.left() && pos < liquidRect.right() )
            painter->drawLine( pos, pipeRect.top(), pos, pipeRect.bottom() );
    }
    else
    {
        if ( pos >= liquidRect.top() && pos < liquidRect.bottom() )
            painter->drawLine( pipeRect.left(), pos, pipeRect.right(), pos );
    }
}

QVector<double> qwtTickList( const QwtScaleDiv &scaleDiv )
{
    QVector<double> values;

    double lowerLimit = scaleDiv.interval().minValue();
    double upperLimit = scaleDiv.interval().maxValue();

    if ( upperLimit < lowerLimit )
        qSwap( lowerLimit, upperLimit );

    values += lowerLimit;

    for ( int tickType = QwtScaleDiv::MinorTick;
        tickType < QwtScaleDiv::NTickTypes; tickType++ )
    {
        const QList<double> ticks = scaleDiv.ticks( tickType );

        for ( int i = 0; i < ticks.count(); i++ )
        {
            const double v = ticks[i];
            if ( v > lowerLimit && v < upperLimit )
                values += v;
        }
    }

    values += upperLimit;

    return values;
}

class QwtThermoMarker::PrivateData
{
public:
    PrivateData():
        orientation( Qt::Vertical ),
        scalePosition(QwtThermoMarker::LeftScale),
        scalePos( QwtThermoMarker::TrailingScale ),
        spacing( 3 ),
        borderWidth( 2 ),
        pipeWidth( 10 ),
        alarmLevel( 0.0 ),
        alarmEnabled( false ),
        autoFillPipe( true ),
        originMode( QwtThermoMarker::OriginMinimum ),
        origin( 0.0 ),
        colorMap( NULL ),
        minValue( 0.0 ),
        maxValue( 100.0 ),
        value( 0.0 ),
        prvValue (0.0),
        decayingValue (0.0)
    {
        rangeFlags = QwtInterval::IncludeBorders;
    }

    ~PrivateData()
    {
        delete colorMap;
    }

    Qt::Orientation orientation;
    QwtThermoMarker::ScalePosition scalePosition;
    QwtThermoMarker::ScalePos scalePos;

    int spacing;
    int borderWidth;
    int pipeWidth;

    QwtInterval::BorderFlags rangeFlags;
    double alarmLevel;
    bool alarmEnabled;
    bool autoFillPipe;
    QwtThermoMarker::OriginMode originMode;
    double origin;

    QwtColorMap *colorMap;

    double minValue;
    double maxValue;

    double value, prvValue, decayingValue;
};

/*!
  Constructor
  \param parent Parent widget
*/
QwtThermoMarker::QwtThermoMarker( QWidget *parent ): QwtAbstractScale( parent )
{
    d_data = new PrivateData;

    QSizePolicy policy( QSizePolicy::MinimumExpanding, QSizePolicy::Fixed );
    if ( d_data->orientation == Qt::Vertical )
        policy.transpose();

    setSizePolicy( policy );

    setAttribute( Qt::WA_WState_OwnSizePolicy, false );
    layoutThermo( true );
    thisType = Pipe;
    thisMarkerSize = 2;
    thisMarkerLineOption = false;

    redrawTimer = new QTimer(this);
    connect(redrawTimer, SIGNAL(timeout()), this, SLOT(redrawTimerExpired()));
    redrawTimer->start(RedrawInterval);

    setDecayOption(false);
    setDecayTime(1.0);
}

//! Destructor
QwtThermoMarker::~QwtThermoMarker()
{
    delete d_data;
}

/*!
  \brief Exclude/Include min/max values

  According to the flags minValue() and maxValue()
  are included/excluded from the pipe. In case of an
  excluded value the corresponding tick is painted
  1 pixel off of the pipeRect().

  F.e. when a minimum
  of 0.0 has to be displayed as an empty pipe the minValue()
  needs to be excluded.

  \param flags Range flags
  \sa rangeFlags()
*/
void QwtThermoMarker::setRangeFlags( QwtInterval::BorderFlags flags )
{
    if ( d_data->rangeFlags != flags )
    {
        d_data->rangeFlags = flags;
        update();
    }
}

/*!
  \return Range flags
  \sa setRangeFlags()
*/
QwtInterval::BorderFlags QwtThermoMarker::rangeFlags() const
{
    return d_data->rangeFlags;
}

/*!
  Set the maximum value.

*/
void QwtThermoMarker::setMaxValue( double maxValue )
{
    setUpperBound(maxValue);
    setLowerBound(d_data->minValue);
    d_data->maxValue = maxValue;
    layoutThermo( true );
}

//! Return the maximum value.
double QwtThermoMarker::maxValue() const
{
    return d_data->maxValue;
}

/*!
  Set the minimum value.
*/
void QwtThermoMarker::setMinValue( double minValue )
{
    setLowerBound(minValue);
    setUpperBound( d_data->maxValue);
    d_data->minValue = minValue;
    layoutThermo( true );
}

//! Return the minimum value.
double QwtThermoMarker::minValue() const
{
    return d_data->minValue;
}

/*!
  Set the current value.

  \param value New Value
  \sa value()
*/
void QwtThermoMarker::setValue( double value )
{
    if (d_data->value != value) {
        d_data->value = value;

        if(d_data->orientation == Qt::Vertical && thisType == Pipe && getDecayOption()) {
            //printf("%f %f\n ", prvValue, d_data->decayingValue);
            if(prvValue > d_data->decayingValue) {
                d_data->prvValue = prvValue;
                peakLevelChanged.start();
                if(!redrawTimer->isActive() && value < prvValue) redrawTimer->start(RedrawInterval);
            }
        }
        update();
    }
    prvValue = value;
}

//! Return the value.
double QwtThermoMarker::value() const
{
    return d_data->value;
}

/*!
  \brief Set a scale draw

  For changing the labels of the scales, it
  is necessary to derive from QwtScaleDraw and
  overload QwtScaleDraw::label().

  \param scaleDraw ScaleDraw object, that has to be created with
                   new and will be deleted in ~QwtThermo() or the next
                   call of setScaleDraw().
*/
void QwtThermoMarker::setScaleDraw( QwtScaleDraw *scaleDraw )
{
    setAbstractScaleDraw( scaleDraw );
}

/*!
   \return the scale draw of the thermo
   \sa setScaleDraw()
*/
const QwtScaleDraw *QwtThermoMarker::scaleDraw() const
{
    return static_cast<const QwtScaleDraw *>( abstractScaleDraw() );
}

/*!
   \return the scale draw of the thermo
   \sa setScaleDraw()
*/
QwtScaleDraw *QwtThermoMarker::scaleDraw()
{
    return static_cast<QwtScaleDraw *>( abstractScaleDraw() );
}

/*!
  Paint event handler
  \param event Paint event
*/
void QwtThermoMarker::paintEvent( QPaintEvent *event )
{
    QPainter painter( this );
    painter.setClipRegion( event->region() );

    QStyleOption opt;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    opt.init(this);
#else
    opt.initFrom(this);
#endif

    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    const QRect tRect = pipeRect();

    if ( !tRect.contains( event->rect() ) )
    {
        if ( d_data->scalePos != QwtThermoMarker::NoScales )
            scaleDraw()->draw( &painter, palette() );
    }

    const int bw = d_data->borderWidth;

    const QBrush brush = palette().brush( QPalette::Base );
    qDrawShadePanel( &painter,
        tRect.adjusted( -bw, -bw, bw, bw ),
        palette(), true, bw,
        d_data->autoFillPipe ? &brush : NULL );

    drawLiquid( &painter, tRect );
}

/*!
  Resize event handler
  \param event Resize event
*/
void QwtThermoMarker::resizeEvent( QResizeEvent *event )
{
    Q_UNUSED( event );
    layoutThermo( false );
}

/*!
  Qt change event handler
  \param event Event
*/
void QwtThermoMarker::changeEvent( QEvent *event )
{
    switch( event->type() )
    {
        case QEvent::StyleChange:
        case QEvent::FontChange:
        {
            layoutThermo( true );
            break;
        }
        default:
            break;
    }
}

/*!
  Recalculate the QwtThermo geometry and layout based on
  pipeRect() and the fonts.

  \param update_geometry notify the layout system and call update
         to redraw the scale
*/
void QwtThermoMarker::layoutThermo( bool update_geometry )
{
    const QRect tRect = pipeRect();
    const int bw = d_data->borderWidth + d_data->spacing;
    const bool inverted = ( upperBound() < lowerBound() );

    int from, to;

    if ( d_data->orientation == Qt::Horizontal )
    {
        from = tRect.left();
        to = tRect.right();

        if ( d_data->rangeFlags & QwtInterval::ExcludeMinimum )
        {
            if ( inverted )
                to++;
            else
                from--;
        }
        if ( d_data->rangeFlags & QwtInterval::ExcludeMaximum )
        {
            if ( inverted )
                from--;
            else
                to++;
        }

        if ( d_data->scalePos == QwtThermoMarker::TrailingScale )
        {
            scaleDraw()->setAlignment( QwtScaleDraw::TopScale );
            scaleDraw()->move( from, tRect.top() - bw );
        }
        else
        {
            scaleDraw()->setAlignment( QwtScaleDraw::BottomScale );
            scaleDraw()->move( from, tRect.bottom() + bw );
        }

        scaleDraw()->setLength( to - from );
    }
    else // Qt::Vertical
    {
        from = tRect.top();
        to = tRect.bottom();

        if ( d_data->rangeFlags & QwtInterval::ExcludeMinimum )
        {
            if ( inverted )
                from--;
            else
                to++;
        }
        if ( d_data->rangeFlags & QwtInterval::ExcludeMaximum )
        {
            if ( inverted )
                to++;
            else
                from--;
        }

        if ( d_data->scalePos == QwtThermoMarker::LeadingScale )
        {
            scaleDraw()->setAlignment( QwtScaleDraw::RightScale );
            scaleDraw()->move( tRect.right() + bw, from );
        }
        else
        {
            scaleDraw()->setAlignment( QwtScaleDraw::LeftScale );
            scaleDraw()->move( tRect.left() - bw, from );
        }

        scaleDraw()->setLength( to - from );
    }

    if ( update_geometry )
    {
        updateGeometry();
        update();
    }
}

/*!
  \return Bounding rectangle of the pipe ( without borders )
          in widget coordinates
*/
QRect QwtThermoMarker::pipeRect() const
{
    int mbd = 0;
    if ( d_data->scalePos != QwtThermoMarker::NoScales )
    {
        int d1, d2;
        scaleDraw()->getBorderDistHint( font(), d1, d2 );
        mbd = qMax( d1, d2 );
    }
    const int bw = d_data->borderWidth;
    const int scaleOff = bw + mbd;

    const QRect cr = contentsRect();

    QRect pipeRect = cr;
    if ( d_data->orientation == Qt::Horizontal )
    {
        pipeRect.adjust( scaleOff, 0, -scaleOff, 0 );

        if ( d_data->scalePos == QwtThermoMarker::TrailingScale )
            pipeRect.setTop( cr.top() + cr.height() - bw - d_data->pipeWidth );
        else
            pipeRect.setTop( bw );

        pipeRect.setHeight( d_data->pipeWidth );
    }
    else // Qt::Vertical
    {
        pipeRect.adjust( 0, scaleOff, 0, -scaleOff );

        if ( d_data->scalePos == QwtThermoMarker::LeadingScale )
            pipeRect.setLeft( bw );
        else
            pipeRect.setLeft( cr.left() + cr.width() - bw - d_data->pipeWidth );

        pipeRect.setWidth( d_data->pipeWidth );
    }

    return pipeRect;
}

/*!
  \brief Set the orientation.
  \param orientation Allowed values are Qt::Horizontal and Qt::Vertical.

  \sa orientation(), scalePosition()
*/
void QwtThermoMarker::setOrientation( Qt::Orientation orientation )
{
    if ( orientation == d_data->orientation )
        return;

    d_data->orientation = orientation;

    if ( !testAttribute( Qt::WA_WState_OwnSizePolicy ) )
    {
        QSizePolicy sp = sizePolicy();
        sp.transpose();
        setSizePolicy( sp );

        setAttribute( Qt::WA_WState_OwnSizePolicy, false );
    }

    layoutThermo( true );
}

/*!
  \return Orientation
  \sa setOrientation()
*/
Qt::Orientation QwtThermoMarker::orientation() const
{
    return d_data->orientation;
}

/*!
  \brief Change how the origin is determined.
  \sa originMode(), serOrigin(), origin()
 */
void QwtThermoMarker::setOriginMode( OriginMode m )
{
    if ( m == d_data->originMode )
        return;

    d_data->originMode = m;
    update();
}

/*!
  \return Mode, how the origin is determined.
  \sa setOriginMode(), serOrigin(), origin()
 */
QwtThermoMarker::OriginMode QwtThermoMarker::originMode() const
{
    return d_data->originMode;
}

/*!
  \brief Specifies the custom origin.

  If originMode is set to OriginCustom this property controls where the
  liquid starts.

  \param origin New origin level
  \sa setOriginMode(), originMode(), origin()
 */
void QwtThermoMarker::setOrigin( double origin )
{
    if ( origin == d_data->origin )
        return;

    d_data->origin = origin;
    update();
}

/*!
  \return Origin of the thermo, when OriginCustom is enabled
  \sa setOrigin(), setOriginMode(), originMode()
 */
double QwtThermoMarker::origin() const
{
    return d_data->origin;
}

void QwtThermoMarker::setScalePosition( ScalePosition scalePosition )
{
    d_data->scalePosition = scalePosition;
    if (  scalePosition == BottomScale )
        setScalePos(LeadingScale);
    else if( scalePosition == TopScale )
        setScalePos(TrailingScale);
    else if (  scalePosition == LeftScale )
         setScalePos(TrailingScale);
    else if ( scalePosition == RightScale )
        setScalePos(LeadingScale);
    else
        setScalePos(NoScales);

}

/*!
   Return the scale position.
   \sa setScalePosition()
*/
QwtThermoMarker::ScalePosition QwtThermoMarker::scalePosition() const
{
    return d_data->scalePosition;
}

/*!
  \brief Change the position of the scale
  \param scalePosition Position of the scale.

  \sa ScalePosition, scalePosition()
*/
void QwtThermoMarker::setScalePos( ScalePos scalePos )
{
    if ( d_data->scalePos == scalePos )
        return;

    d_data->scalePos = scalePos;

    if ( testAttribute( Qt::WA_WState_Polished ) )
        layoutThermo( true );
}

/*!
   \return Scale position.
   \sa setScalePosition()
*/
QwtThermoMarker::ScalePos QwtThermoMarker::scalePos() const
{
    return d_data->scalePos;
}

//! Notify a scale change.
void QwtThermoMarker::scaleChange()
{
    layoutThermo( true );
}

void QwtThermoMarker::redrawTimerExpired()
{
    if(d_data->orientation == Qt::Vertical && thisType == Pipe && getDecayOption()) {
        qreal decayTime = getDecayTime();  // 1-2 seconds
        // Decay the peak signal
        const int elapsedMs = (int)peakLevelChanged.elapsed();
        const qreal decayAmount = qAbs(d_data->maxValue - d_data->minValue) * elapsedMs / 9000.0 / decayTime;   // 1s for 1/10 of bar
        //printf("value=%f elapsedMs=%d decayAmount=%lf prvValue=%lf\n", d_data->value, elapsedMs, decayAmount, d_data->prvValue);
        if (decayAmount < d_data->prvValue) {
            d_data->decayingValue = d_data->prvValue - decayAmount;
        } else {
            d_data->decayingValue = 0.0;
        }
        if(d_data->decayingValue <= d_data->value){
            d_data->decayingValue = 0.0;
            redrawTimer->stop();
        }
        update();
    }
}

/*!
   Redraw the liquid in thermometer pipe.
   \param painter Painter
   \param pipeRect Bounding rectangle of the pipe without borders
*/
void QwtThermoMarker::drawLiquid(QPainter *painter, const QRect &pipeRect ) const
{
    QRect markerRect;
    painter->save();
    painter->setClipRect( pipeRect, Qt::IntersectClip );
    painter->setPen( Qt::NoPen );

    const QwtScaleMap scaleMap = scaleDraw()->scaleMap();

    QRect liquidRect = fillRect( pipeRect, thisMarkerSize);
    if(thisMarkerLineOption) markerRect = fillRect (pipeRect, 1);

    // decaying value
    QRect decayingRect = pipeRect;
    int from = qRound( scaleMap.transform( d_data->decayingValue) );
    int to = qRound( scaleMap.transform( d_data->origin ) );

    if ( to < from ) qSwap( from, to );

    if(d_data->orientation == Qt::Vertical && thisType == Pipe && getDecayOption()) {
        decayingRect.setBottom( from + 1);
        decayingRect.setTop( from - 1);
        //printf("drawliquid value=%lf d_data->decayingPrvValue=%lf y=%d\n", d_data->value, d_data->decayingValue, from);
        QBrush brush = palette().brush( QPalette::ButtonText );
        QColor color = brush.color();
        color = color.lighter();
        painter->fillRect(decayingRect, color);
    }

    if ( d_data->colorMap != Q_NULLPTR ) {
        const QwtInterval interval = scaleDiv().interval().normalized();

        // Because the positions of the ticks are rounded
        // we calculate the colors for the rounded tick values

        QVector<double> values = qwtTickList( scaleDraw()->scaleDiv() );

        if ( scaleMap.isInverting() ) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            qSort( values.begin(), values.end(), qGreater<double>() );
#else
            std::sort( values.begin(), values.end(), std::greater<double>() );
#endif
        } else {

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            qSort( values.begin(), values.end(), qLess<double>() );
#else
            std::sort( values.begin(), values.end(), std::less<double>() );
#endif

        }

        int from = 0;
        if ( !values.isEmpty() ) {
            from = qRound( scaleMap.transform( values[0] ) );
            qwtDrawLine( painter, from, d_data->colorMap->color( interval, values[0] ), pipeRect, liquidRect, d_data->orientation );
        }

        for ( int i = 1; i < values.size(); i++ ) {
            const int to = qRound( scaleMap.transform( values[i] ) );

            for ( int pos = from + 1; pos < to; pos++ ) {
                const double v = scaleMap.invTransform( pos );
                qwtDrawLine( painter, pos, d_data->colorMap->color( interval, v ), pipeRect, liquidRect, d_data->orientation );
            }

            qwtDrawLine( painter, to, d_data->colorMap->color( interval, values[i] ), pipeRect, liquidRect, d_data->orientation );
            from = to;
        }

    } else {

        if ( !liquidRect.isEmpty() && d_data->alarmEnabled ) {
            const QRect r = alarmRect( liquidRect );
            if ( !r.isEmpty() ) {
                painter->fillRect( r, palette().brush( QPalette::Highlight ) );
                liquidRect = QRegion( liquidRect ).subtracted( r ).boundingRect();
            }
        }

        painter->fillRect( liquidRect, palette().brush( QPalette::ButtonText ) );

        if(thisType == Marker && thisMarkerLineOption) {
            QBrush brush = palette().brush( QPalette::ButtonText );
            QColor color = brush.color();
            color = color.darker();
            painter->fillRect(markerRect, color);
        }
    }

    painter->restore();
}

/*!
  \brief Change the spacing between pipe and scale

  A spacing of 0 means, that the backbone of the scale is below
  the pipe.

  The default setting is 3 pixels.

  \param spacing Number of pixels
  \sa spacing();
*/
void QwtThermoMarker::setSpacing( int spacing )
{
    if ( spacing <= 0 )
        spacing = 0;

    if ( spacing != d_data->spacing  )
    {
        d_data->spacing = spacing;
        layoutThermo( true );
    }
}

/*!
  \return Number of pixels between pipe and scale
  \sa setSpacing()
*/
int QwtThermoMarker::spacing() const
{
    return d_data->spacing;
}

/*!
   Set the border width of the pipe.
   \param width Border width
   \sa borderWidth()
*/
void QwtThermoMarker::setBorderWidth( int width )
{
    if ( width <= 0 )
        width = 0;

    if ( width != d_data->borderWidth  )
    {
        d_data->borderWidth = width;
        layoutThermo( true );
    }
}

/*!
   \return Border width of the thermometer pipe.
   \sa setBorderWidth()
*/
int QwtThermoMarker::borderWidth() const
{
    return d_data->borderWidth;
}

/*!
  \brief Assign a color map for the fill color

  \param colorMap Color map
  \warning The alarm threshold has no effect, when
           a color map has been assigned
*/
void QwtThermoMarker::setColorMap( QwtColorMap *colorMap )
{
    if ( colorMap != d_data->colorMap )
    {
        delete d_data->colorMap;
        d_data->colorMap = colorMap;
    }
}

/*!
  \return Color map for the fill color
  \warning The alarm threshold has no effect, when
           a color map has been assigned
*/
QwtColorMap *QwtThermoMarker::colorMap()
{
    return d_data->colorMap;
}

/*!
  \return Color map for the fill color
  \warning The alarm threshold has no effect, when
           a color map has been assigned
*/
const QwtColorMap *QwtThermoMarker::colorMap() const
{
    return d_data->colorMap;
}

/*!
  \brief Change the brush of the liquid.

  Changes the QPalette::ButtonText brush of the palette.

  \param brush New brush.
  \sa fillBrush(), QWidget::setPalette()
*/
void QwtThermoMarker::setFillBrush( const QBrush& brush )
{
    QPalette pal = palette();
    pal.setBrush( QPalette::ButtonText, brush );
    setPalette( pal );
}

/*!
  \return Liquid ( QPalette::ButtonText ) brush.
  \sa setFillBrush(), QWidget::palette()
*/
QBrush QwtThermoMarker::fillBrush() const
{
    return palette().brush( QPalette::ButtonText );
}

/*!
  \brief Specify the liquid brush above the alarm threshold

  Changes the QPalette::Highlight brush of the palette.

  \param brush New brush.
  \sa alarmBrush(), QWidget::setPalette()

  \warning The alarm threshold has no effect, when
           a color map has been assigned
*/
void QwtThermoMarker::setAlarmBrush( const QBrush& brush )
{
    QPalette pal = palette();
    pal.setBrush( QPalette::Highlight, brush );
    setPalette( pal );
}

/*!
  \return Liquid brush ( QPalette::Highlight ) above the alarm threshold.
  \sa setAlarmBrush(), QWidget::palette()

  \warning The alarm threshold has no effect, when
           a color map has been assigned
*/
QBrush QwtThermoMarker::alarmBrush() const
{
    return palette().brush( QPalette::Highlight );
}

/*!
  Specify the alarm threshold.

  \param level Alarm threshold
  \sa alarmLevel()

  \warning The alarm threshold has no effect, when
           a color map has been assigned
*/
void QwtThermoMarker::setAlarmLevel( double level )
{
    d_data->alarmLevel = level;
    d_data->alarmEnabled = 1;
    update();
}

/*!
  \return Alarm threshold.
  \sa setAlarmLevel()

  \warning The alarm threshold has no effect, when
           a color map has been assigned
*/
double QwtThermoMarker::alarmLevel() const
{
    return d_data->alarmLevel;
}

/*!
  Change the width of the pipe.

  \param width Width of the pipe
  \sa pipeWidth()
*/
void QwtThermoMarker::setPipeWidth( int width )
{
    if ( width > 0 )
    {
        d_data->pipeWidth = width;
        layoutThermo( true );
    }
}

/*!
  \return Width of the pipe.
  \sa setPipeWidth()
*/
int QwtThermoMarker::pipeWidth() const
{
    return d_data->pipeWidth;
}

/*!
  \brief Enable or disable the alarm threshold
  \param on true (disabled) or false (enabled)

  \warning The alarm threshold has no effect, when
           a color map has been assigned
*/
void QwtThermoMarker::setAlarmEnabled( bool on )
{
    d_data->alarmEnabled = on;
    update();
}

/*!
  \return True, when the alarm threshold is enabled.

  \warning The alarm threshold has no effect, when
           a color map has been assigned
*/
bool QwtThermoMarker::alarmEnabled() const
{
    return d_data->alarmEnabled;
}

/*!
  \return the minimum size hint
  \sa minimumSizeHint()
*/
QSize QwtThermoMarker::sizeHint() const
{
    return minimumSizeHint();
}

/*!
  \return Minimum size hint
  \warning The return value depends on the font and the scale.
  \sa sizeHint()
*/
QSize QwtThermoMarker::minimumSizeHint() const
{
    int w = 0, h = 0;

    if ( d_data->scalePos != NoScales )
    {
        const int sdExtent = qCeil( scaleDraw()->extent( font() ) );
        const int sdLength = scaleDraw()->minLength( font() );

        w = sdLength;
        h = d_data->pipeWidth + sdExtent + d_data->spacing;

    }
    else // no scale
    {
        w = 200;
        h = d_data->pipeWidth;
    }

    if ( d_data->orientation == Qt::Vertical )
        qSwap( w, h );

    w += 2 * d_data->borderWidth;
    h += 2 * d_data->borderWidth;

    // finally add the margins

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    int left, right, top, bottom;
    getContentsMargins( &left, &top, &right, &bottom );
    w += left + right;
    h += top + bottom;

#else
    QMargins content=contentsMargins();
    w += content.left() + content.right();
    h += content.top() + content.bottom();
#endif


    return QSize( w, h );
}

/*!
  \brief Calculate the filled rectangle of the pipe

  \param pipeRect Rectangle of the pipe
  \return Rectangle to be filled ( fill and alarm brush )

  \sa pipeRect(), alarmRect()
 */
QRect QwtThermoMarker::fillRect( const QRect &pipeRect, int markerSize) const
{
    double origin;
    if ( d_data->originMode == OriginMinimum ) {
        origin = qMin( lowerBound(), upperBound() );
    } else if ( d_data->originMode == OriginMaximum ) {
        origin = qMax( lowerBound(), upperBound() );
    } else { // OriginCustom
        origin = d_data->origin;
    }

    const QwtScaleMap scaleMap = scaleDraw()->scaleMap();

    int from = qRound( scaleMap.transform( d_data->value ) );
    int to = qRound( scaleMap.transform( origin ) );

    if ( to < from ) qSwap( from, to );

    QRect fillRect = pipeRect;
    if ( d_data->orientation == Qt::Horizontal ) {
        if(thisType == Marker) {
            if(lowerBound() < upperBound()) qSwap( from, to );
            fillRect.setLeft( from + markerSize );
            fillRect.setRight(from - markerSize );
        } else if(thisType == PipeFromCenter) {
            const float center = lowerBound() + (upperBound()-lowerBound())/2;
            const int cval = qRound(scaleMap.transform(center));
            if(lowerBound() > upperBound()) qSwap( from, to );
            fillRect.setLeft(cval);
            fillRect.setRight(to);
        } else {
            fillRect.setLeft( from );
            fillRect.setRight( to );
        }

    } else { // Qt::Vertical

        if(thisType == Marker) {
            if(lowerBound() > upperBound()) qSwap( from, to );
            fillRect.setBottom( from + markerSize);
            fillRect.setTop( from - markerSize);
        } else if(thisType == PipeFromCenter) {
            const float center = lowerBound() + (upperBound()-lowerBound())/2;
            const int cval = qRound(scaleMap.transform(center));
            if(lowerBound() > upperBound()) qSwap( from, to );
            fillRect.setTop(from);
            fillRect.setBottom(cval);
        } else {
            fillRect.setTop( from );
            fillRect.setBottom( to );
        }
    }

    return fillRect.normalized();
}

/*!
  \brief Calculate the alarm rectangle of the pipe

  \param fillRect Filled rectangle in the pipe
  \return Rectangle to be filled with the alarm brush

  \sa pipeRect(), fillRect(), alarmLevel(), alarmBrush()
 */
QRect QwtThermoMarker::alarmRect( const QRect &fillRect ) const
{
    QRect alarmRect( 0, 0, -1, -1); // something invalid

    if ( !d_data->alarmEnabled )
        return alarmRect;

    const bool inverted = ( upperBound() < lowerBound() );

    bool increasing;
    if ( d_data->originMode == OriginCustom )
    {
        increasing = d_data->value > d_data->origin;
    }
    else
    {
        increasing = d_data->originMode == OriginMinimum;
    }

    const QwtScaleMap map = scaleDraw()->scaleMap();
    const int alarmPos = qRound( map.transform( d_data->alarmLevel ) );
    const int valuePos = qRound( map.transform( d_data->value ) );

    if ( d_data->orientation == Qt::Horizontal )
    {
        int v1, v2;
        if ( inverted )
        {
            v1 = fillRect.left();

            v2 = alarmPos - 1;
            v2 = qMin( v2, increasing ? fillRect.right() : valuePos );
        }
        else
        {
            v1 = alarmPos + 1;
            v1 = qMax( v1, increasing ? fillRect.left() : valuePos );

            v2 = fillRect.right();

        }
        alarmRect.setRect( v1, fillRect.top(), v2 - v1 + 1, fillRect.height() );
    }
    else
    {
        int v1, v2;
        if ( inverted )
        {
            v1 = alarmPos + 1;
            v1 = qMax( v1, increasing ? fillRect.top() : valuePos );

            v2 = fillRect.bottom();
        }
        else
        {
            v1 = fillRect.top();

            v2 = alarmPos - 1;
            v2 = qMin( v2, increasing ? fillRect.bottom() : valuePos );
        }

        alarmRect.setRect( fillRect.left(), v1, fillRect.width(), v2 - v1 + 1 );
    }

    return alarmRect;
}


#include "moc_qwt_thermo_marker_61.cpp"
