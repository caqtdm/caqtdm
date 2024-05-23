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
 *  Copyright (c) 2010 - 2024
 *
 *  Author:
 *    Erik Schwarz
 *  Contact details:
 *    erik.schwarz@psi.ch
 */

#ifndef PLOTHELPERCLASSES_H
#define PLOTHELPERCLASSES_H

#include "qwt_plot.h"
#include "qwt_date_scale_draw.h"
#include "qwt_interval.h"
#include "qwt_plot_canvas.h"
#include "qwt_plot_picker.h"
#include "qwt_plot_zoomer.h"
#include "qwt_scale_engine.h"
#include "qwt_date_scale_engine.h"
#include "qwt_text.h"
#include <cmath>

class PlotDateScaleDraw: public QwtDateScaleDraw
{
public:

    PlotDateScaleDraw(const QDateTime &base): baseTime(base)
    {
    }
    virtual QwtText label(double v) const
    {
        QDateTime upTime = baseTime.addSecs((int) v);
        return upTime.toString("hh:mm:ss");
    }

private:
    QDateTime baseTime;

};

#if QWT_VERSION >= 0x060100
class PlotDateScaleEngine: public QwtDateScaleEngine
{
public:

    PlotDateScaleEngine(const int &nb, Qt::TimeSpec time): QwtDateScaleEngine(time)
    {
        nbTicks = nb;
    }

    virtual QwtScaleDiv divideScale( double x1, double x2, int , int , double) const
    {
        QList<double> Ticks[QwtScaleDiv::NTickTypes];
        const QwtInterval interval = QwtInterval( x1, x2 ).normalized();

        if (interval.width() <= 0 ) return QwtScaleDiv();

        QwtScaleDiv scaleDiv;

        for (int i=0; i<nbTicks+1; i++) {
            Ticks[QwtScaleDiv::MajorTick] << x1 + ((x2-x1)*i / nbTicks);
        }

        scaleDiv = QwtScaleDiv(interval, Ticks);
        if ( x1 > x2 ) scaleDiv.invert();

        return scaleDiv;
    }

private:
    int nbTicks;
};
#endif

#ifdef QWT_USE_OPENGL
class GLCanvas: public QwtPlotGLCanvas
{
public:
    GLCanvas( QwtPlot *parent = NULL ):
        QwtPlotGLCanvas( parent )
    {
        setContentsMargins( 1, 1, 1, 1 );
    }

protected:
    virtual void paintEvent( QPaintEvent *event )
    {
        QPainter painter( this );
        painter.setClipRegion( event->region() );

        QwtPlot *plot = qobject_cast< QwtPlot *>( parent() );
        if ( plot )
            plot->drawCanvas( &painter );

        painter.setPen( palette().foreground().color() );
        painter.drawRect( rect().adjusted( 0, 0, -1, -1 ) );
    }

};
#endif

class PlotScaleEngine: public QwtLinearScaleEngine
{
public:

    PlotScaleEngine(const int &nb): QwtLinearScaleEngine()
    {
        nbTicks = nb;
    }

    virtual QwtScaleDiv divideScale( double x1, double x2, int , int , double) const
    {
        double stepSize = (x2-x1) / nbTicks;
        int maxMinorSteps = 10;

        QwtInterval interval = QwtInterval(x1, x2 ).normalized();
        if ( interval.width() <= 0 ) return QwtScaleDiv();

        QwtScaleDiv scaleDiv;
        QList<double> Ticks[QwtScaleDiv::NTickTypes];
        buildTicks(interval, stepSize, maxMinorSteps, Ticks);
        scaleDiv = QwtScaleDiv( interval, Ticks );

        if ( x1 > x2 ) scaleDiv.invert();

        return scaleDiv;
    }

private:
    int nbTicks;
};

/* Class for creating a linear ScaleEngine that has a modified divideScale, such that the Labels to be drawn by the ScaleDraw
 * have the correct spacing.
 * */
class PlotLinearScaleEngine: public QwtLinearScaleEngine {
public:
    PlotLinearScaleEngine(double MinOld = 10, double MaxOld = 100, double MinNew = 10, double MaxNew = 100) : QwtLinearScaleEngine(), _MinOld(MinOld), _MaxOld(MaxOld), _MinNew(MinNew), _MaxNew(MaxNew)
    {
    }

    QwtScaleDiv divideScale(double x1, double x2, int numMajorSteps, int numMinorSteps, double stepSize = 0.0) const{
        x1 = ((_MaxNew - _MinNew) / (_MaxOld - _MinOld))*(x1-_MinOld)+_MinNew;
        x2 = ((_MaxNew - _MinNew) / (_MaxOld - _MinOld))*(x2-_MinOld)+_MinNew;

        stepSize *= ((_MaxNew - _MinNew) / (_MaxOld - _MinOld));

        QwtScaleDiv Div = QwtLinearScaleEngine::divideScale(x1, x2, numMajorSteps, numMinorSteps, stepSize);

        QList<double> Ticks[QwtScaleDiv::NTickTypes];

        Ticks[QwtScaleDiv::MajorTick] = Div.ticks(QwtScaleDiv::MajorTick);
        for (unsigned int i = 0; i < Ticks[QwtScaleDiv::MajorTick].count(); i++) {
            Ticks[QwtScaleDiv::MajorTick][i] = ((_MaxOld - _MinOld) / (_MaxNew - _MinNew))*(Ticks[QwtScaleDiv::MajorTick][i]-_MinNew)+_MinOld;
        }
        Ticks[QwtScaleDiv::MediumTick] = Div.ticks(QwtScaleDiv::MediumTick);
        for (unsigned int i = 0; i < Ticks[QwtScaleDiv::MediumTick].count(); i++) {
            Ticks[QwtScaleDiv::MediumTick][i] = ((_MaxOld - _MinOld) / (_MaxNew - _MinNew))*(Ticks[QwtScaleDiv::MediumTick][i]-_MinNew)+_MinOld;
        }
        Ticks[QwtScaleDiv::MinorTick] = Div.ticks(QwtScaleDiv::MinorTick);
        for (unsigned int i = 0; i < Ticks[QwtScaleDiv::MinorTick].count(); i++) {
            Ticks[QwtScaleDiv::MinorTick][i] = ((_MaxOld - _MinOld) / (_MaxNew - _MinNew))*(Ticks[QwtScaleDiv::MinorTick][i]-_MinNew)+_MinOld;
        }
        return QwtScaleDiv(QwtInterval(((_MaxOld - _MinOld) / (_MaxNew - _MinNew))*(x1-_MinNew)+_MinOld, ((_MaxOld - _MinOld) / (_MaxNew - _MinNew))*(x2-_MinNew)+_MinOld), Ticks);
    }
protected:
    double _MinOld;
    double _MaxOld;
    double _MinNew;
    double _MaxNew;
};

/* Class for creating a logarithmic ScaleEngine that has a modified divideScale, such that the Labels to be drawn by the ScaleDraw
 * have the correct spacing.
 * */
class PlotLog10ScaleEngine: public QwtLogScaleEngine {
public:
    PlotLog10ScaleEngine(double MinOld = 10, double MaxOld = 100, double MinNew = 10, double MaxNew = 100) : QwtLogScaleEngine(), _MinOld(MinOld), _MaxOld(MaxOld), _MinNew(MinNew), _MaxNew(MaxNew)
    {
    }

    QwtScaleDiv divideScale(double x1, double x2, int numMajorSteps, int numMinorSteps, double stepSize = 0.0) const{
        x1 = _MinNew*(pow((_MaxNew/_MinNew),(std::log10(x1/_MinOld)/std::log10(_MaxOld/_MinOld))));
        x2 = _MinNew*(pow((_MaxNew/_MinNew),(std::log10(x2/_MinOld)/std::log10(_MaxOld/_MinOld))));
        stepSize *= std::log10(_MaxNew/_MinNew) / std::log10(_MaxOld/_MinOld);

        QwtScaleDiv Div = QwtLogScaleEngine::divideScale(x1, x2, numMajorSteps, numMinorSteps, stepSize);

        QList<double> Ticks[QwtScaleDiv::NTickTypes];

        Ticks[QwtScaleDiv::MajorTick] = Div.ticks(QwtScaleDiv::MajorTick);
        for (unsigned int i = 0; i < Ticks[QwtScaleDiv::MajorTick].count(); i++) {
            Ticks[QwtScaleDiv::MajorTick][i] = _MinOld*(pow((_MaxOld/_MinOld),(std::log10(Ticks[QwtScaleDiv::MajorTick][i]/_MinNew)/std::log10(_MaxNew/_MinNew))));
        }
        Ticks[QwtScaleDiv::MediumTick] = Div.ticks(QwtScaleDiv::MediumTick);
        for (unsigned int i = 0; i < Ticks[QwtScaleDiv::MediumTick].count(); i++) {
            Ticks[QwtScaleDiv::MediumTick][i] = _MinOld*(pow((_MaxOld/_MinOld),(std::log10(Ticks[QwtScaleDiv::MediumTick][i]/_MinNew)/std::log10(_MaxNew/_MinNew))));
        }
        Ticks[QwtScaleDiv::MinorTick] = Div.ticks(QwtScaleDiv::MinorTick);
        for (unsigned int i = 0; i < Ticks[QwtScaleDiv::MinorTick].count(); i++) {
            Ticks[QwtScaleDiv::MinorTick][i] = _MinOld*(pow((_MaxOld/_MinOld),(std::log10(Ticks[QwtScaleDiv::MinorTick][i]/_MinNew)/std::log10(_MaxNew/_MinNew))));
        }
        return QwtScaleDiv(QwtInterval(_MinOld*(pow((_MaxOld/_MinOld),(std::log10(x1/_MinNew)/std::log10(_MaxNew/_MinNew)))), _MinOld*(pow((_MaxOld/_MinOld),(std::log10(x2/_MinNew)/std::log10(_MaxNew/_MinNew))))), Ticks);
    }
protected:
    double _MinOld;
    double _MaxOld;
    double _MinNew;
    double _MaxNew;
};

/* Class for creating a  ScaleDraw that modifies the value to show different Scales
 * than the one currently drawn. Doesn't modify the actuals curves.
 * Depending on the axistype (linear, log10) the values will be modified accordingly.
 * */
class PlotScaleDraw: public QwtScaleDraw
{
public:
    // Default Limits are the same for new and old ones to not have any convertion of values.
    // Default minimum is 1e-20 to ensure proper precision.
    PlotScaleDraw(double MinOld = 1e-20, double MaxOld = 100.0, double MinNew = 1e-20, double MaxNew = 100.0, bool IsLinear = true): QwtScaleDraw(), _MinOld(MinOld), _MaxOld(MaxOld), _MinNew(MinNew), _MaxNew(MaxNew), _IsLinear(IsLinear)
    {
    }

    void setConversion(double MinOld, double MaxOld, double MinNew, double MaxNew, bool IsLinear)
    {
        _MinOld = MinOld;
        _MaxOld = MaxOld;
        _MinNew = MinNew;
        _MaxNew = MaxNew;
        _IsLinear = IsLinear;
    }

    QwtText label(double v) const
    {
        double newLabel;
        if (_IsLinear) {
            newLabel = ((_MaxNew - _MinNew) / (_MaxOld - _MinOld))*(v-_MinOld)+_MinNew;
        } else {
            newLabel = _MinNew*(pow((_MaxNew/_MinNew),(std::log10(v/_MinOld)/std::log10(_MaxOld/_MinOld))));
        }
        return QwtScaleDraw::label(newLabel);
    }

private:
    double _MinOld;
    double _MaxOld;
    double _MinNew;
    double _MaxNew;
    bool _IsLinear;
};

class DynamicPlotPicker : public QwtPlotPicker
{
public:
    DynamicPlotPicker(QwtPlot::Axis xAxisId, QwtPlot::Axis yAxisId, QwtPicker::RubberBand  rubberBand, QwtPicker::DisplayMode trackerMode, QWidget * widget) : QwtPlotPicker( xAxisId,  yAxisId,  rubberBand,  trackerMode, widget)
    {
        _MinOld = 1e-20;
        _MaxOld = 100;
        _MinNew = 1e-20;
        _MaxNew = 100;
        _IsLinear = true;
        _IsXAxisTimeSinceEpoch = false;
        _IsXAxisAlreadyCorrect = false;
    }

    void setConversion(double MinOld, double MaxOld, double MinNew, double MaxNew, bool IsLinear)
    {
        _MinOld = MinOld;
        _MaxOld = MaxOld;
        _MinNew = MinNew;
        _MaxNew = MaxNew;
        _IsLinear = IsLinear;
    }

    void setStartTime(long long startTime, double period)
    {
        _StartTime = QDateTime::fromMSecsSinceEpoch(startTime*1000);
        _Period = period;
    }


    bool IsXAxisTimeSinceEpoch() const
    {
        return _IsXAxisTimeSinceEpoch;
    }

    void setIsXAxisTimeSinceEpoch(bool newIsXAxisTimeSinceEpoch)
    {
        _IsXAxisTimeSinceEpoch = newIsXAxisTimeSinceEpoch;
    }

    bool IsXAxisAlreadyCorrect() const
    {
        return _IsXAxisAlreadyCorrect;
    }

    void setIsXAxisAlreadyCorrect(bool newIsXAxisAlreadyCorrect)
    {
        _IsXAxisAlreadyCorrect = newIsXAxisAlreadyCorrect;
    }

protected:
    QwtText trackerText( const QPoint& pos ) const
    {
        // default exemption handling by QwtPlotPicker
        if ( plot() == NULL ) {
            return QwtText();
        }

        // get values from pixels
        QPointF coordinates = invTransform(pos);

        // get time in the plot where mouse is
        QDateTime timeOnHover;
        if (_IsXAxisTimeSinceEpoch) {
            timeOnHover = QDateTime::fromMSecsSinceEpoch(coordinates.x());
        } else {
            timeOnHover = _StartTime.addMSecs((coordinates.x() - _Period)*1000);
        }

        // convert value to match the current limits
        if (_IsLinear) {
            coordinates.setY(((_MaxNew - _MinNew) / (_MaxOld - _MinOld))*(coordinates.y()-_MinOld)+_MinNew);
        } else {
            coordinates.setY(_MinNew*(pow((_MaxNew/_MinNew),(std::log10(coordinates.y()/_MinOld)/std::log10(_MaxOld/_MinOld)))));
        }

        // create new QwtText --> Did not use QString.setNum() because then fixed precision would destroy logarithmic values
        QwtText newText;
        if (_IsXAxisAlreadyCorrect) {
            newText = (QString("%1 | %2").arg(coordinates.x()).arg(coordinates.y()));
        } else {
            newText = (timeOnHover.toString("hh:mm:ss") + QString(" | %1").arg(coordinates.y()));
        }
        newText.setBackgroundBrush(Qt::white);
        newText.setBorderRadius(1);

        return newText;
    }
private:
    double _MinOld;
    double _MaxOld;
    double _MinNew;
    double _MaxNew;
    bool _IsLinear;
    bool _IsXAxisTimeSinceEpoch;
    bool _IsXAxisAlreadyCorrect;
    QDateTime _StartTime;
    double _Period;
};

class PlotZoomer: public QwtPlotZoomer
{
public:
    PlotZoomer(QwtPlotCanvas *canvas):
        QwtPlotZoomer(canvas)
    {
        setTrackerMode(AlwaysOn);
    }

    virtual QwtText trackerTextF(const QPointF &pos) const
    {
        QColor bg(Qt::white);
        bg.setAlpha(200);

        QwtText text("(" + QString::number(pos.x()) + "," + QString::number(pos.y()) + ") ");
        text.setBackgroundBrush( QBrush( bg ));
        return text;
    }
};

#endif // PLOTHELPERCLASSES_H
