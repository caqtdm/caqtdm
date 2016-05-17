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
#if defined(_MSC_VER)
#include <float.h>
#define isnan _isnan
#define QWT_DLL
#endif

#include "qwtplotcurvenan.h"

QwtPlotCurveNaN::QwtPlotCurveNaN(const QString& title)
{
    setTitle(title);
}

void QwtPlotCurveNaN::setSamplesList(const QVector<QPointF>& Samples)
{
    samples = Samples;
}

void QwtPlotCurveNaN::drawSeries(QPainter *painter, const QwtScaleMap &xMap,const QwtScaleMap &yMap, const QRectF &canvRect, int from, int to) const
{
    Q_UNUSED(from);
    Q_UNUSED(to);

    //int nbCount = 0;
    if(samples.size() < 1) return;
    QPointF Pstart = samples.at(0);
    QPointF P;

    for (int counter = 1; counter < samples.size(); counter++)
    {
        P = samples.at(counter);

        if(isnan(P.y())) continue;  // continue = skip next instruction in loop
        if((CurvType == ValueCurv) && (P.x() < -Interval)) break;
        else if((CurvType == TimeCurv) && ((P.x() - Pstart.x()) < -Interval)) break;

        QwtPlotCurve::drawSeries(painter, xMap, yMap, canvRect, counter-1, counter);

        //nbCount++;
    }
    //printf("plotted 1 = %d\n", nbCount);
}

void QwtPlotCurveNaN::setInterval(curvType type, double interval)
{
   CurvType = type;
   Interval = interval;
}

QwtPlotIntervalCurveNaN::QwtPlotIntervalCurveNaN(const QString& title)
{
    setTitle(title);
}

void QwtPlotIntervalCurveNaN::setSamplesList(const QVector<QwtIntervalSample> &Samples)
{
    samples = Samples;
}


void QwtPlotIntervalCurveNaN::drawSeries(QPainter *painter, const QwtScaleMap &xMap,const QwtScaleMap &yMap, const QRectF &canvRect, int from, int to) const
{
    Q_UNUSED(from);
    Q_UNUSED(to);

    //int nbCount = 0;

    if(samples.size() < 1) return;
    QwtIntervalSample Pstart = samples.at(0);
    QwtIntervalSample P;

    for (int counter = 1; counter < samples.size(); counter++)
    {
        P = samples.at(counter);

        if(isnan(P.interval.minValue()) || isnan(P.interval.maxValue())) continue; // continue = skip next instruction in loop
        if((CurvType == ValueCurv) && (P.value < -Interval)) break;
        else if((CurvType == TimeCurv)  && ((P.value - Pstart.value) < -Interval)) break;

        QwtPlotIntervalCurve::drawSeries(painter, xMap, yMap, canvRect, counter-1, counter);

        //nbCount++;
    }

    //printf("plotted 2 = %d @ %03d\n", nbCount, timeR.millitm);
}

void QwtPlotIntervalCurveNaN::setInterval(curvType type, double interval)
{
    CurvType = type;
    Interval = interval;
}


