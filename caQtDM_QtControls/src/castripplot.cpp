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
 *  Authors:
 *    Anton Mezger, Erik Schwarz
 *  Contact details:
 *    erik.schwarz@psi.ch
 */

#if defined(_MSC_VER)
#define NOMINMAX
#include <windows.h>
#include <float.h>
#define isnan _isnan
#define QWT_DLL
#endif
#include <qapplication.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpainter.h>
#include <QMetaProperty>
#include "castripplot.h"
#include <cmath>
#include <qwt_picker_machine.h>
#include <qelapsedtimer.h>
#include <plotHelperClasses.h>

#if QT_VERSION > QT_VERSION_CHECK(5, 12, 0)
#if !defined(NAN)
  #define NAN (double)qQNaN()
#endif
#if !defined(INFINITY)
  #define INFINITY qInf()
#endif
#endif

#if defined(_MSC_VER)
    #ifndef snprintf
     #define snprintf _snprintf
    #endif
#endif

// increase the array size given by the canvas width to be sure that the whole range is covered
#define MAXIMUMSIZE 5000
#define SOMEMORE 500

caStripPlot::~caStripPlot() {

    emit timerThreadStop();
    timerThread->wait(200);
    timerThread->deleteLater();
}

caStripPlot::caStripPlot(QWidget *parent): QwtPlot(parent)
{
    // initialisations
    initCurves = true;
    timerID = false;
    thisXaxisType = TimeScale;
    thisYaxisScaling = fixedScale;
    thisXaxisSyncGroup = 0;
    thisXticks = 6;
    thisUnits = Second;
    HISTORY = thisPeriod = 60;
    NumberOfCurves = MAXCURVES;
    onInit = true;
    timeInterval = 1.0;
    setAutoReplot(false);
    setAutoFillBackground(true);
    RestartPlot1 = true;
    RestartPlot2 = false;
    ResizeFactorX = ResizeFactorY = 1.0;
    oldResizeFactorX = oldResizeFactorY = 1.0;
    thisYaxisType = linear;
    YAxisIndex = 0;
    plotIsPaused = false;
    setProperty("xAxisToleranceFactor", 0.01);
    thisIterableCurves = thisSelectableCurves = false;
    thisPlotPicker = off;
    autoscaleMinYOverride = false;

#ifdef QWT_USE_OPENGL
    printf("caStripplot uses opengl ?\n");
    GLCanvas *canvas = new GLCanvas();
    canvas->setPalette( QColor( "khaki" ) );
    setCanvas(canvas);
#endif

    setUsageCPU(Medium);

    // plotpicker
    plotPicker = new DynamicPlotPicker(this->xBottom , this->yLeft, QwtPicker::CrossRubberBand, QwtPicker::AlwaysOff, this->canvas());
    QwtPickerMachine* pickerMachine = new QwtPickerClickPointMachine();
    plotPicker->setStateMachine(pickerMachine);
    connect(plotPicker, SIGNAL(selected(const QPointF&)), this, SLOT(onSelected(const QPointF&)));

    // define a grid
    plotGrid = new QwtPlotGrid();
    plotGrid->attach(this);

    plotLayout()->setAlignCanvasToScales(true);

    setAxisLabelAlignment(QwtPlot::xBottom, Qt::AlignLeft | Qt::AlignBottom);
    setAxisScale(QwtPlot::yLeft, 0, 1000);

    // we have to add some space to the xaxis
    if(thisXaxisType != ValueScale) {
        // due to time scale we need some distance
        scaleWidget = axisWidget(QwtPlot::xBottom);
        const int fmh = QFontMetrics(scaleWidget->font()).height();
        scaleWidget->setMinBorderDist(fmh * 2, fmh * 2);
    }

    // define xaxis
    setXaxis(thisUnits,thisPeriod);

    // define our curves
    for(int i=0; i< MAXCURVES; i++) {
        curve[i] = new QwtPlotCurve();
        errorcurve[i] = new QwtPlotIntervalCurveNaN();
        fillcurve[i] = new QwtPlotCurveNaN();
        curve[i]->setZ(i);
        fillcurve[i]->setZ(i);
        errorcurve[i]->setZ(i+10);

#if QWT_VERSION >= 0x060100
        curve[i]->setRenderThreadCount( 0 ); // 0: use QThread::idealThreadCount()
        fillcurve[i]->setRenderThreadCount( 0 );
        errorcurve[i]->setRenderThreadCount( 0 );
#endif
        curve[i]->setPaintAttribute(QwtPlotCurve::ClipPolygons, true);
        fillcurve[i]->setPaintAttribute(QwtPlotCurve::ClipPolygons, true);
        errorcurve[i]->setPaintAttribute(QwtPlotIntervalCurve::ClipPolygons, true);

        curve[i]->attach(this);
        errorcurve[i]->attach(this);
        fillcurve[i]->attach(this);
        showCurve(i, false);

        thisYaxisLimitsMax[i] = 100;
        thisYaxisLimitsMin[i] = 0;

        sAutoScaleCurves[i] = true;
    }

    // default colors and styles
    setTitlePlot("");
    setTitleX("");
    setTitleY("");
    setBackground(Qt::black);
    setForeground(QColor(133, 190, 232));
    setScaleColor(Qt::black);
    setGrid(true);
    setGridColor(Qt::gray);
    for(int i=0; i< MAXCURVES; i++) setStyle(Lines, i);
    setColor(Qt::white, 0);
    setColor(Qt::red, 1);
    setColor(Qt::yellow, 2);
    setColor(Qt::cyan, 3);
    setColor(Qt::magenta, 4);
    setColor(Qt::green, 5);
    setColor(Qt::blue, 6);

    setXaxisEnabled(true);
    setYaxisEnabled(true);
    setLegendEnabled(true);

    setAxisFont(QwtPlot::xBottom, QFont("Arial", 9));
    setAxisFont(QwtPlot::yLeft, QFont("Arial", 9));

    installEventFilter(this);

    // display timer
    Timer = new QTimer(this);
    Timer->setInterval(1000);
    connect(Timer, SIGNAL(timeout()), this, SLOT(TimeOut()));

    // data collection thread
    timerThread = new stripplotthread();
    timerThread->start();
    timerThread->threadSetTimer(100);
    timerThread->setPriority(QThread::HighPriority);
    connect(this, SIGNAL(timerThreadStop()), timerThread, SLOT(runStop()));
    connect(timerThread, SIGNAL(update()), this, SLOT(TimeOutThread()),  Qt::DirectConnection);
}


/* Slot to stop the plot in its current state, e.g by a Button or Checkbox.
 * Can be used to analyze the curves.
 * */
void caStripPlot::stopPlot(){
    plotIsPaused = true;
}

/* Slot to resume the plot after stopping, e.g by a Button or Checkbox.
 * */
void caStripPlot::resumePlot(){
    plotIsPaused = false;
}

/* Slot to restart  the plot, e.g by a Button or Checkbox.
 * Can be used to restart the plot e.g after having it stopped.
 * */
void caStripPlot::restartPlot()
{
    plotIsPaused = false;
    for(int i=0; i < MAXCURVES; i++) {
        // reset interval data for error and fill curves
        rangeData[i].clear();
        rangeData[i].reserve(MAXIMUMSIZE+5);
        fillData[i].clear();
        fillData[i].reserve(MAXIMUMSIZE+5);

        //reset interval data for raw data curves
        rangeDataRaw[i].clear();
        rangeDataRaw[i].reserve(MAXIMUMSIZE+5);
        fillDataRaw[i].clear();
        fillDataRaw[i].reserve(MAXIMUMSIZE+5);

        if(i==0)  {
            base.clear();
            base.reserve(MAXIMUMSIZE+5);
        }
        for ( int j = 0; j <  MAXIMUMSIZE; j++ ) {
            rangeData[i].append(QwtIntervalSample(0, QwtInterval(NAN, NAN)));
            fillData[i].append(QPointF(NAN,NAN));
            if(i==0) base.append(QwtIntervalSample(0, QwtInterval(NAN, NAN)));

            rangeDataRaw[i].append(QwtIntervalSample(0, QwtInterval(NAN, NAN)));
            fillDataRaw[i].append(QPointF(NAN,NAN));
        }
    }
    // set the data to the curves
    for(int i=0; i < NumberOfCurves; i++) {
        if(thisStyle[i] == FillUnder) {
            fillcurve[i]->setSamplesList(fillData[i]);
            fillcurve[i]->setSamples(fillData[i]);
        }
        errorcurve[i]->setSamplesList(rangeData[i]);
        errorcurve[i]->setSamples(rangeData[i]);
    }
    replot();
}

/* Slot to pause or resume the plot, e.g by a Button or Checkbox.
 * Can be used to pause the plot in its current state or resume it with the data of the past Interval
 * The difference to stopping and restarting the plot is that the plot is not cleared after resuming, but drawn past record-time.
 * */
void caStripPlot::pausePlot(bool pausePlot)
{
    pausePlot ? plotIsPaused = true : plotIsPaused = false;
}

/* Slot to select a curve whose Y Axis is to be displayed.
 * If used while autoscale is on, it will switch to fixed scaling.
 * */
void caStripPlot::selectFixedYAxis(int newYAxisIndex){
    if (newYAxisIndex >= NumberOfCurves) return;
    setYaxisScaling(fixedScale);
    selectYAxis(newYAxisIndex);
}

/* Slot to set the plotPicker Mode.
 * Can enable or disable the plotPicker Tooltip. 0 = off | 1 = on.
 * */
void caStripPlot::setPlotPickerMode(int mode)
{
    if (mode == 0) {
        plotPicker->setTrackerMode(QwtPicker::AlwaysOff);
    } else {
        plotPicker->setTrackerMode(QwtPicker::AlwaysOn);
    }
}

/* Slot to handle clicks within the curve-canvas.
 * Is used to decide wether a specific curve was clicked and changes the displayed Y-axis accordingly to show the corresponding axis.
 * */
void caStripPlot::onSelected(const QPointF& point)
{
    if (thisYaxisScaling != fixedScale || NumberOfCurves == 1 || !thisSelectableCurves) return;
    const double yAxisTolerance = (thisYaxisLimitsMax[0] - thisYaxisLimitsMin[0]) * 0.01;
    const double xAxisTolerance = thisPeriod * xAxisToleranceFactor;
    double lDist = 100000000000;
    qint8 lIndex = -1;

    // get nearest curve point by iterating over plot items, filtering for curves and calculating minimum distance
    for(int curvIndex=0; curvIndex < NumberOfCurves; curvIndex++) {
        if(curve[curvIndex]->isVisible() == false){
            continue;
        }
        double lCurveDist = 100000000000;
        double lTmpDist;
        // loop over all samples that are somewhere around the selected point
        for (quint32 j = 0; rangeData[curvIndex][j].value > (rangeData[curvIndex][0].value - thisPeriod) && rangeData[curvIndex][j].interval.isValid() && rangeData[curvIndex][j].value != 0; j++) {
            if (rangeData[curvIndex][j].value > (point.x() + xAxisTolerance) || rangeData[curvIndex][j].value < (point.x() - xAxisTolerance)) {
                continue;
            }
            double yAverage = (rangeData[curvIndex][j].interval.maxValue() + rangeData[curvIndex][j].interval.minValue()) / 2;

            lTmpDist = std::abs(yAverage - point.y()); // simpler and as good as hypothenuse
            lCurveDist = std::min(lTmpDist, lCurveDist);
        }
        if (lCurveDist < lDist) {
            lDist = lCurveDist;
            lIndex = curvIndex;
        }
    }

    // check if mouse position is within tolerance
    if ( lDist > yAxisTolerance || lIndex == -1) return;
    selectYAxis(lIndex);
    return;
}

/*  Select the Y-axis which is to be used for the whole Plot. Argument is given by the index of the curve whose axis is to be used.
 *  This Function converts all existing values for the new scale and sets YAxisIndex so new Values will be recalculated accordingly.
 * */
void caStripPlot::selectYAxis(quint8 newYAxisIndex){

    YAxisIndex = newYAxisIndex;
    if (YAxisIndex > (NumberOfCurves-1)) YAxisIndex = 0;

    savedTitles = originalTitles;
    savedTitles.replace(YAxisIndex, ("<u>" + savedTitles.at(YAxisIndex) + "<u>"));
    for(quint8 i=0; i < NumberOfCurves; i++) {
        curve[i]->setTitle(legendText(i));
    }

    QwtScaleWidget *scaleY =axisWidget(QwtPlot::yLeft);
    QPalette palette = scaleY->palette();
    palette.setColor( QPalette::WindowText, thisLineColor[YAxisIndex].rgba());
    scaleY->setPalette (palette);

    double oldYAxisMin = QwtPlot::axisScaleDiv(QwtPlot::yLeft).interval().minValue();
    double oldYAxisMax = QwtPlot::axisScaleDiv(QwtPlot::yLeft).interval().maxValue();
    double newYAxisMin = thisYaxisLimitsMin[YAxisIndex];
    double newYAxisMax = thisYaxisLimitsMax[YAxisIndex];
    if (thisYaxisType == log10){
        oldYAxisMin = qMax(oldYAxisMin, 1e-20);
        oldYAxisMax = qMax(oldYAxisMax, 1e-19);
        newYAxisMin = qMax(newYAxisMin, 1e-20);
        newYAxisMax = qMax(newYAxisMax, 1e-19);
    }

    qDebug() << "going from" << oldYAxisMin << "-" <<oldYAxisMax << "to" << newYAxisMin << "-" << newYAxisMax;

    bool isLinear = (thisYaxisType == linear);
    static_cast<PlotScaleDraw*>(axisScaleDraw(yLeft))->setConversion(oldYAxisMin, oldYAxisMax, newYAxisMin, newYAxisMax, isLinear);
    static_cast<DynamicPlotPicker*>(plotPicker)->setConversion(oldYAxisMin, oldYAxisMax, newYAxisMin, newYAxisMax, isLinear);
    if (isLinear) {
        setAxisScaleEngine(yLeft, new PlotLinearScaleEngine(oldYAxisMin, oldYAxisMax, newYAxisMin, newYAxisMax));
    } else {
        setAxisScaleEngine(yLeft, new PlotLog10ScaleEngine(oldYAxisMin, oldYAxisMax, newYAxisMin, newYAxisMax));
    }


    replot();
}

void caStripPlot::defineXaxis(units unit, double period)
{
    double interval;

    if(unit == Millisecond) {
        interval = period / 1000.0;
    } else if(unit == Second) {
        interval = period;
    } else if(unit == Minute) {
        interval = period * 60;
    } else {
        interval = 60;
        printf("\nunknown unit\n");
    }

    // set xaxis
    setXaxis(interval, period);
    replot();
}

void caStripPlot::setXaxis(double interval, double period)
{
    // set axis and in case of a time scale define the time axis
    int nbTicks;
    if(thisXticks < 1) nbTicks = 1; else nbTicks =  thisXticks;

    if(thisXaxisType != ValueScale) {
        plotPicker->setIsXAxisAlreadyCorrect(false);
        QDateTime timeNow= QDateTime::currentDateTime();
        timeNow = timeNow.addSecs((int) -interval);
        setAxisScale(QwtPlot::xBottom, 0, interval, interval/nbTicks);
        setAxisScaleDraw(QwtPlot::xBottom, new PlotDateScaleDraw(timeNow) );

        if(thisXaxisType == TimeScaleFix) {
              PlotScaleEngine *scaleEngine = new PlotScaleEngine(nbTicks);
              setAxisScaleEngine(QwtPlot::xBottom, scaleEngine);
        } else {
           QwtLinearScaleEngine *scaleEngine= new QwtDateScaleEngine();
           setAxisScaleEngine(QwtPlot::xBottom, scaleEngine);
        }

    } else {
        plotPicker->setIsXAxisAlreadyCorrect(true);
        setAxisScale(QwtPlot::xBottom, -period, 0, period/nbTicks);
        setAxisScaleDraw(QwtPlot::xBottom, new QwtScaleDraw());
    }
}

void caStripPlot::setTicksResizeFactor(float factX, float factY)
{
    ResizeFactorX = factX;
    ResizeFactorY = factY;
}

void caStripPlot::setYaxisType(yAxisType s)
{
    thisYaxisType = s;
    if(s == log10) {
#if QWT_VERSION < 0x060100
        setAxisScaleEngine(QwtPlot::yLeft, new QwtLog10ScaleEngine);
        setAxisScaleDraw(QwtPlot::yLeft, new PlotScaleDraw());
#else
        // Leaves default parameter IsLinear = true, doesn't matter though because we don't want any conversion so far.
        setAxisScaleDraw(QwtPlot::yLeft, new PlotScaleDraw());
        setAxisScaleEngine(QwtPlot::yLeft, new QwtLogScaleEngine());
#endif
    } else {
        setAxisScaleDraw(QwtPlot::yLeft, new PlotScaleDraw());
        setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine());
    }

    replot();
}

/* Function to remap a curve to a new Scale.
 * Is used to correct data already on the plot if axis scaling or type is changed.
 * Always calculates values from raw data.
 * */
void caStripPlot::remapCurve(double newMin, double newMax,  quint8 curvIndex, bool isNewLog = false)
{
    //qDebug() << "remapping Curve: " << curvIndex << "min/max:" << newMin << newMax << "isNewLog:"<< isNewLog;
    // Start timer to measure performance:
    //QElapsedTimer timer;
    //timer.start();

    // Variables
    // Because the values are calculated from the raw data, old min and old max are always given.
    double oldMin = thisYaxisLimitsMin[curvIndex];
    double oldMax = thisYaxisLimitsMax[curvIndex];

    // Make sure no bad values are used for logarithmic conversions.
    if (isNewLog) {
        newMin = qMax(newMin, 1e-20);
        newMax = qMax(newMax, 1e-19);
        oldMin = qMax(oldMin, 1e-20);
        oldMax = qMax(oldMax, 1e-19);
    }

    double percentMin;
    double percentMax;
    double percentFillY;

    double oldMaxMinDiff = oldMax - oldMin;
    double oldLogMaxMinDiv = std::log10(oldMax/oldMin);
    double newLogMaxMinDiv = std::log10(newMax/newMin);
    double newMaxMinDiff = newMax - newMin;
    double log10NewMin = std::log10(newMin);
    double log10OldMin = std::log10(oldMin);

    // efficiently do default conversion to original values
    if (oldMin == newMin && oldMax == newMax) {
        if (isNewLog){ // treat incorrect values
           mutex.lock();
           for (quint32 j = 0; rangeDataRaw[curvIndex][j].value > (rangeDataRaw[curvIndex][0].value - thisPeriod) && rangeDataRaw[curvIndex][j].interval.isValid() && rangeDataRaw[curvIndex][j].value != 0; j++){
                rangeData[curvIndex][j].interval.setMinValue(qMax(rangeDataRaw[curvIndex][j].interval.minValue(), 1e-20));
                rangeData[curvIndex][j].interval.setMaxValue(qMax(rangeDataRaw[curvIndex][j].interval.maxValue(), 1e-20));
                fillData[curvIndex][j].setY(qMax(fillDataRaw[curvIndex][j].y(), 1e-20));
           }
           mutex.unlock();
        } else {
           mutex.lock();
           rangeData[curvIndex] = rangeDataRaw[curvIndex];
           fillData[curvIndex] = fillDataRaw[curvIndex];
           mutex.unlock();
        }
    } else { // remap the curves for any arbitrary conversion
        // Different equations are used because the logarithmic conversion is most performant the way it is (least amount of log10() calls)
        // but the linear conversion needs to be different due to division by zero errors which can't happen on a log scale (are prevented by setting minimum values to 1e-20).
        if (isNewLog) {
           mutex.lock();
           for (quint32 j = 0; rangeDataRaw[curvIndex][j].value > (rangeDataRaw[curvIndex][0].value - thisPeriod) && rangeDataRaw[curvIndex][j].interval.isValid() && rangeDataRaw[curvIndex][j].value != 0; j++){
                double minPositive = qMax(rangeDataRaw[curvIndex][j].interval.minValue(), 1e-20);
                double maxPositive = qMax(rangeDataRaw[curvIndex][j].interval.maxValue(), 1e-20);
                double fillYPositive = qMax(fillDataRaw[curvIndex][j].y(), 1e-20);

                percentMin = (std::log10(minPositive) - log10OldMin)/oldLogMaxMinDiv;
                percentMax = (std::log10(maxPositive) - log10OldMin)/oldLogMaxMinDiv;
                percentFillY = (std::log10(fillYPositive) - log10OldMin)/oldLogMaxMinDiv;
                rangeData[curvIndex][j].interval.setMinValue(pow(10, ((percentMin * newLogMaxMinDiv) + log10NewMin)));
                rangeData[curvIndex][j].interval.setMaxValue(pow(10, ((percentMax * newLogMaxMinDiv) + log10NewMin)));
                fillData[curvIndex][j].setY(pow(10, ((percentFillY * newLogMaxMinDiv) + log10NewMin)));
           }
           mutex.unlock();
        } else {
           mutex.lock();
           for (quint32 j = 0; rangeDataRaw[curvIndex][j].value > (rangeDataRaw[curvIndex][0].value - thisPeriod) && rangeDataRaw[curvIndex][j].interval.isValid() && rangeDataRaw[curvIndex][j].value != 0; j++){
                percentMin = (rangeDataRaw[curvIndex][j].interval.minValue() - oldMin)/(oldMaxMinDiff);
                percentMax = (rangeDataRaw[curvIndex][j].interval.maxValue() - oldMin)/(oldMaxMinDiff);
                percentFillY = (fillDataRaw[curvIndex][j].y() - oldMin)/(oldMaxMinDiff);
                rangeData[curvIndex][j].interval.setMinValue((percentMin * newMaxMinDiff) + newMin);
                rangeData[curvIndex][j].interval.setMaxValue((percentMax * newMaxMinDiff) + newMin);
                fillData[curvIndex][j].setY((percentFillY * newMaxMinDiff) + newMin);
           }
           mutex.unlock();
        }
    }

    // Set the data to the curves
    if(thisStyle[curvIndex] == FillUnder) {
        fillcurve[curvIndex]->setSamplesList(fillData[curvIndex]);
        fillcurve[curvIndex]->setSamples(fillData[curvIndex]);
    }
    errorcurve[curvIndex]->setSamplesList(rangeData[curvIndex]);
    errorcurve[curvIndex]->setSamples(rangeData[curvIndex]);

    replot();

    // Print time it took to do conversions
    //qDebug() << "Conversion took ms: " << timer.nsecsElapsed()/1000000.0;
}

void caStripPlot::RescaleCurves(int width, units unit, double period)
{

    //printf("canvas width=%d\n",width);

    HISTORY = width; // equals canvas width

    // set the right interval
    if(unit == Millisecond) {
        timeInterval = period / (double) HISTORY;
        INTERVAL = period / 1000.0;

        // limit update rate
        if(timeInterval < 2) {
            timeInterval = 2;
            double newPeriod = timeInterval * HISTORY;
            INTERVAL = newPeriod / 1000.0;
            setXaxis(INTERVAL, newPeriod);
        }

    } else if(unit == Minute) {
        timeInterval =  1000.0 * period * 60.0 / (double) HISTORY;
        INTERVAL = period * 60;

    } else { // seconds or undefined
        timeInterval = 1000.0 * period / (double) HISTORY;
        INTERVAL = period;

        // limit update rate
        if(timeInterval < 2) {
            timeInterval = 2;
            double newPeriod = timeInterval * HISTORY / 1000.0;
            INTERVAL = newPeriod;
            setXaxis(INTERVAL, newPeriod);
        }
    }

    if(!initCurves) return;
    initCurves = false;

    mutex.lock();

    // initialize the arrays, with nan data
    for(int i=0; i < MAXCURVES; i++) {
        // define interval data for error and fill curves
        rangeData[i].clear();
        rangeData[i].reserve(MAXIMUMSIZE+5);
        fillData[i].clear();
        fillData[i].reserve(MAXIMUMSIZE+5);

        //reset interval data for raw data curves
        rangeDataRaw[i].clear();
        rangeDataRaw[i].reserve(MAXIMUMSIZE+5);
        fillDataRaw[i].clear();
        fillDataRaw[i].reserve(MAXIMUMSIZE+5);
        if(i==0)  {
            base.clear();
            base.reserve(MAXIMUMSIZE+5);
        }
        for ( int j = 0; j <  MAXIMUMSIZE; j++ ) {
            rangeData[i].append(QwtIntervalSample(0, QwtInterval(NAN, NAN)));
            fillData[i].append(QPointF(NAN,NAN));
            rangeDataRaw[i].append(QwtIntervalSample(0, QwtInterval(NAN, NAN)));
            fillDataRaw[i].append(QPointF(NAN,NAN));
            if(i==0) base.append(QwtIntervalSample(0, QwtInterval(NAN, NAN)));
        }
    }

    // set the data to the curves
    for(int i=0; i < NumberOfCurves; i++) {
        if(thisStyle[i] == FillUnder) {
            fillcurve[i]->setSamplesList(fillData[i]);
            fillcurve[i]->setSamples(fillData[i]);
        }
        errorcurve[i]->setSamplesList(rangeData[i]);
        errorcurve[i]->setSamples(rangeData[i]);
    }

    mutex.unlock();

    // define update rate
    dataCount = 0;

    if(timerID) {
        TimersStart();
    }

    RestartPlot1 = true;
    RestartPlot2 = false;

    replot();
}

void caStripPlot::TimersStart()
{
    // change display timer interval, limit to 10 Hz
    int interval;
    if(thisUsageCPU == Medium) interval = qMax((int)timeInterval, 200);
    else if(thisUsageCPU == High) interval = qMax((int)timeInterval, 100);
    else if(thisUsageCPU == Low) interval = qMax((int)timeInterval, 400);
    else  interval = qMax((int)timeInterval, 200);

    Timer->stop();
    Timer->setInterval(interval);
    Timer->start();

    // stop run method of thread and redefine repetition time then start run
    emit timerThreadStop();
    timerThread->wait(200);
    timerThread->threadSetTimer((int) timeInterval);
    timerThread->start();

    // run timeout slot immedialety
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(TimeOut()));
    timer->setSingleShot(true);
    timer->start(0);
    timer->start(0);
    timer->start(0);
}

// reinitalize plot when resizing
void caStripPlot::resizeEvent ( QResizeEvent * event )
{
    QwtPlot::resizeEvent(event);
    RescaleCurves(canvas()->size().width(), thisUnits, thisPeriod);
    replot();
    if(timerID) RescaleAxis();
}

void caStripPlot::UpdateScaling()
{
    RescaleCurves(canvas()->size().width(), thisUnits, thisPeriod);
    if (thisYaxisScaling == fixedScale){
        QwtScaleWidget *scaleY =axisWidget(QwtPlot::yLeft);
        QPalette palette = scaleY->palette();
        palette.setColor( QPalette::WindowText, thisLineColor[YAxisIndex].rgba());
        scaleY->setPalette (palette);

        savedTitles.replace(YAxisIndex, ("<u>" + savedTitles.at(YAxisIndex) + "<u>"));
        curve[YAxisIndex]->setTitle(legendText(YAxisIndex));
    } else {
        QwtScaleWidget *scaleY =axisWidget(QwtPlot::yLeft);
        QPalette palette = scaleY->palette();
        palette.setColor( QPalette::WindowText, getScaleColor());
        scaleY->setPalette (palette);

        savedTitles = originalTitles;
        for(quint8 i=0; i < NumberOfCurves; i++) {
            curve[i]->setTitle(legendText(i));
        }
    }
    if (thisYaxisScaling == fixedScale){
        for (quint8 curvIndex = 0 ; curvIndex < NumberOfCurves ; curvIndex++) {
            remapCurve(thisYaxisLimitsMin[0], thisYaxisLimitsMax[0], curvIndex, thisYaxisType == log10);
        }
    } else {
        for (quint8 curvIndex = 0 ; curvIndex < NumberOfCurves ; curvIndex++) {
            remapCurve(thisYaxisLimitsMin[curvIndex], thisYaxisLimitsMax[curvIndex], curvIndex, thisYaxisType == log10);
        }
    }
    selectYAxis(YAxisIndex);

    replot();
    if(timerID) RescaleAxis();
}

QString caStripPlot::legendText(int i)
{
#define MAXLEN 21
    char min[MAXLEN], max[MAXLEN];
    QString MinMax;
    QString title(savedTitles.at(i));

    // in case of fixed scales, concatenate the limits that are used
    if(thisYaxisScaling == fixedScale) {
        snprintf(min, MAXLEN - 1, "%.1f", thisYaxisLimitsMin[i]);
        snprintf(max, MAXLEN - 1, "%.1f", thisYaxisLimitsMax[i]);
        ReplaceTrailingZerosByBlancs(min);
        ReplaceTrailingZerosByBlancs(max);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        MinMax.sprintf("[%s,%s]", min, max);
#else
        MinMax=QString("[%1,%2]").arg(min).arg(max);
#endif

        MinMax = MinMax.simplified(); MinMax.replace( " ", "" );
        title.append(" ");
        title.append(MinMax);
    }
    return title;
}

void caStripPlot::defineCurves(QStringList titles, units unit, double period, int width, int nbCurves)
{
    int min, max;
    NumberOfCurves = nbCurves;
    scaleWidget->getBorderDistHint(min, max);
    originalTitles = savedTitles = titles;

    savedTitles.replace(YAxisIndex, ("<u>" + savedTitles.at(YAxisIndex) + "<u>"));

    defineXaxis(unit, period);

    // define curves
    for(int i=0; i < MAXCURVES; i++) {
        curvStyle s = Lines;
        QColor c;

        if(curve[i] != (QwtPlotCurve*) Q_NULLPTR) {
            s = getStyle(i);
            c = getColor(i);
            delete curve[i];
        }

        if(errorcurve[i] != (QwtPlotIntervalCurveNaN*) Q_NULLPTR) {
            delete errorcurve[i];
        }
        if(fillcurve[i] != (QwtPlotCurve*) Q_NULLPTR) {
            delete fillcurve[i];
        }

        if(i < NumberOfCurves) {
            // set title and limits to legend
            QString title = legendText(i);

            curve[i] = new QwtPlotCurve(title);
            errorcurve[i] = new QwtPlotIntervalCurveNaN(title+"?error?");
            fillcurve[i] = new QwtPlotCurveNaN(title+"?fill?");
            setStyle(s, i);

            curve[i]->setZ(i);
            curve[i]->attach(this);
            fillcurve[i]->setZ(i);
            fillcurve[i]->attach(this);
            fillcurve[i]->setItemAttribute(QwtPlotItem::Legend, false);
            errorcurve[i]->setZ(i+10);
            errorcurve[i]->attach(this);
            errorcurve[i]->setItemAttribute(QwtPlotItem::Legend, false);

            showCurve(i, false);

            realMax[i] = maxVal[i] = -1000000;
            realMin[i] = minVal[i] =  1000000;
            actVal[i] = realVal[i] = NAN;
        }
    }

    showCurve(0, true);

    // draw legend
    if(thisLegendshow) {
        QwtLegend *lgd = new QwtLegend;
        insertLegend(lgd, QwtPlot::BottomLegend);
        // set color on legend texts
        setLegendAttribute(thisScaleColor, QFont("arial",9), COLOR);
    }
    RescaleCurves(width, unit, period);

    // Set xAxisToleranceFactor, because object is now fully constructed.
    bool propertyConversionOk;
    xAxisToleranceFactor = this->property("xAxisToleranceFactor").toFloat(&propertyConversionOk);
    if (!propertyConversionOk || xAxisToleranceFactor <= 0 || xAxisToleranceFactor >= 1){
        qDebug() << "The Dynamic Property xAxisToleranceFactor is either not set or set incorrectly (not between 0 and 1) and will be replaced by default value 0.01 for Object:" << this->objectName();
        xAxisToleranceFactor = 0.01f;
    }
    // set plotpicker, same reason as xAxisToleranceFactor;
    if (thisPlotPicker == on) plotPicker->setTrackerMode(QwtPicker::AlwaysOn);
}

void caStripPlot::startPlot()
{
    //change display timer interval
    timerID = true;
    TimersStart();
}

void caStripPlot::addText(double x, double y, char* text, QColor c, int fontsize)
{
    QwtPlotMarker *mY = new QwtPlotMarker();
    QwtText txt;
    txt.setText(text);
    txt.setFont(QFont("Helvetica",fontsize, QFont::Bold));
    txt.setColor(c);
    mY->setLabel(txt);
    mY->setLabelAlignment(Qt::AlignRight | Qt::AlignTop);
    mY->setXValue(x);
    mY->setYValue(y);
    mY->attach(this);
    replot();
}

// data collection done by timerthread
void caStripPlot::TimeOutThread()
{
    int c, i;
    double elapsedTime = 0.0;
    QwtIntervalSample tmp;
    QPointF tmpP;
    double value = 0.0;
    double interval=0.0;

    if(!timerID) return;

    mutex.lock();

    int dataCountLimit = HISTORY - 1 + SOMEMORE;

    // we need an exact time scale
    if(RestartPlot1) {
        ftime(&timeStart);
        static_cast<DynamicPlotPicker*>(plotPicker)->setStartTime(timeStart.time, thisPeriod);
        RestartPlot1 = false;
        RestartPlot2 = true;
    }
    ftime(&timeNow);

    elapsedTime = ((double) timeNow.time + (double) timeNow.millitm / (double)1000) -
                  ((double) timeStart.time + (double) timeStart.millitm / (double)1000);

    timeData = INTERVAL + elapsedTime;  // in seconds
    interval = INTERVAL;
    /*
    printf("dataCountLimit = %d datacount=%d history=%d interval=%f elapsed=%f siz=%d\n",
           dataCountLimit, dataCount, HISTORY, interval, elapsedTime,  rangeData[0].size());
*/
    // correct value to fit again inside the interval (only for the fixed scale)
    if(thisXaxisType == ValueScale) {
        if(thisUnits == Millisecond) {
            timeData = timeData * 1000.0;
            interval = INTERVAL * 1000.0;
        } else if(thisUnits == Minute) {
            timeData = timeData / 60.0;
            interval = INTERVAL / 60.0;
        } else {
            interval = INTERVAL;
        }
    }

    // tell interval to base class nan
    for (c = 0; c < NumberOfCurves; c++ ) {
        if(thisXaxisType == ValueScale) {
            fillcurve[c]->setInterval(ValueCurv, interval);
            errorcurve[c]->setInterval(ValueCurv, interval);
        } else {
            fillcurve[c]->setInterval(TimeCurv, interval);
            errorcurve[c]->setInterval(TimeCurv, interval);
        }
    }

    // shift data back
    if(dataCount > 1) {
        // shift data and our timebase for the fixed scale
        if(thisXaxisType == ValueScale) {
            for (c = 0; c < NumberOfCurves; c++ ) {
                // shift left and cur last
                rangeData[c].prepend(tmp);
                rangeData[c].erase(rangeData[c].end() - 1);
                rangeDataRaw[c].prepend(tmp);
                rangeDataRaw[c].erase(rangeDataRaw[c].end() - 1);
                if(thisStyle[c] == FillUnder) {
                    fillData[c].prepend(tmpP);
                    fillData[c].erase(fillData[c].end() - 1);
                    fillDataRaw[c].prepend(tmpP);
                    fillDataRaw[c].erase(fillDataRaw[c].end() - 1);
                }
            }

            base.prepend(tmp);
            base.erase(base.end() - 1);

            for (i = dataCount; i > 0; i-- ) {
                value = base[i].value - timeData;
                for (c = 0; c < NumberOfCurves; c++ ) {
                    rangeData[c][i].value = value;
                    fillData[c][i].setX(value);
                    rangeDataRaw[c][i].value = value;
                    fillDataRaw[c][i].setX(value);
                }
            }

            // shift data
        } else {
            for (c = 0; c < NumberOfCurves; c++ ) {
                rangeData[c].prepend(tmp);
                rangeDataRaw[c].prepend(tmp);
                if(thisStyle[c] == FillUnder) {
                    fillData[c].prepend(tmpP);
                    fillDataRaw[c].prepend(tmpP);
                }
                rangeData[c].erase(rangeData[c].end() - 1);
                rangeDataRaw[c].erase(rangeDataRaw[c].end() - 1);
                if(thisStyle[c] == FillUnder) {
                    fillData[c].erase(fillData[c].end() - 1);
                    fillDataRaw[c].erase(fillDataRaw[c].end() - 1);
                }
            }
        }

    }

    // update last point
    for (c = 0; c < NumberOfCurves; c++ ) {
        double valueMin = minVal[c];
        double valueMax = maxVal[c];
        double valueMinRaw = realMin[c];
        double valueMaxRaw = realMax[c];

        if(thisYaxisType == log10) {
            if(valueMin < 1e-20) valueMin=1e-20;
            if(valueMax < 1e-20) valueMax=1e-20;
        }

        QwtInterval newInterval;
        newInterval.setMaxValue( valueMax);
        newInterval.setMinValue( valueMin);

        QwtInterval newIntervalRaw;
        newIntervalRaw.setMaxValue(valueMaxRaw);
        newIntervalRaw.setMinValue(valueMinRaw);

        rangeData[c][0] = QwtIntervalSample( timeData, newInterval);
        rangeDataRaw[c][0] = QwtIntervalSample( timeData, newIntervalRaw);
        if(thisXaxisType == ValueScale) {
            base[0] = QwtIntervalSample(timeData, newInterval);
        }
        if(thisStyle[c] == FillUnder) {
            fillData[c][0] = QPointF(timeData, (valueMax+valueMin)/2);
            fillDataRaw[c][0] = QPointF(timeData, (valueMaxRaw+valueMinRaw)/2);
        }
    }

    // advance data points
    if (dataCount < 2 && dataCount < dataCountLimit) dataCount++;
    else if(dataCount < dataCountLimit) {
        if(thisXaxisType == ValueScale) {
            if(fillData[0][dataCount-1].x() > -interval) dataCount++;
        } else {
            if(elapsedTime < interval) dataCount++;
        }
    }

    // keep max and min of every curve
    for (c = 0; c < NumberOfCurves; c++ ) {
        maxVal[c] = minVal[c] = actVal[c];
        realMax[c] = realMin[c] = realVal[c];
    }

    // in case of automatic y scale we need the minimum and maximum of our curves
    if(thisYaxisScaling == autoScale) {
        AutoscaleMaxY = -INFINITY;
        AutoscaleMinY = INFINITY;

        for (c = 0; c < NumberOfCurves; c++ ) {
            for (int counter = 0; counter < dataCount; counter++) {
                QwtIntervalSample P = rangeData[c].at(counter);
                if(!qIsNaN(P.interval.maxValue()) && P.interval.maxValue() > AutoscaleMaxY) AutoscaleMaxY = P.interval.maxValue();
                if(!qIsNaN(P.interval.minValue()) && P.interval.minValue() < AutoscaleMinY) AutoscaleMinY = P.interval.minValue();
            }
        }

        if(AutoscaleMaxY == AutoscaleMinY) {
            AutoscaleMaxY += 0.5;
            AutoscaleMinY -= 0.5;
        } else {
            if(qAbs(AutoscaleMaxY) > 1.e-9) AutoscaleMaxY += (AutoscaleMaxY - AutoscaleMinY)/20.0;
            if(qAbs(AutoscaleMinY) > 1.e-9 && thisYaxisType != log10) AutoscaleMinY -= (AutoscaleMaxY - AutoscaleMinY)/20.0;
        }

        /*if(thisYaxisType == log10) {
            for (c = 0; c < NumberOfCurves; c++ ) {
                if(AutoscaleMinY < thisYaxisLimitsMin[c]) AutoscaleMinY = thisYaxisLimitsMin[c];
            }
        }*/
    }

    if(thisYaxisScaling == selectiveAutoScale) {
        AutoscaleMaxY = -INFINITY;
        AutoscaleMinY = INFINITY;

        for (c = 0; c < NumberOfCurves; c++ ) {
            if (!sAutoScaleCurves[c]) continue;
            for (int counter = 0; counter < dataCount; counter++) {
                QwtIntervalSample P = rangeData[c].at(counter);
                if(!qIsNaN(P.interval.maxValue()) && P.interval.maxValue() > AutoscaleMaxY) AutoscaleMaxY = P.interval.maxValue();
                if(!qIsNaN(P.interval.minValue()) && P.interval.minValue() < AutoscaleMinY) AutoscaleMinY = P.interval.minValue();
            }
        }

        if(AutoscaleMaxY == AutoscaleMinY) {
            AutoscaleMaxY += 0.5;
            AutoscaleMinY -= 0.5;
        } else {
            if(qAbs(AutoscaleMaxY) > 1.e-9) AutoscaleMaxY += (AutoscaleMaxY - AutoscaleMinY)/20.0;
            if(qAbs(AutoscaleMinY) > 1.e-9 && thisYaxisType != log10) AutoscaleMinY -= (AutoscaleMaxY - AutoscaleMinY)/20.0;
        }

        /*if(thisYaxisType == log10) {
            for (c = 0; c < NumberOfCurves; c++ ) {
                if (!sAutoScaleCurves[c]) continue;
                if(AutoscaleMinY < thisYaxisLimitsMin[c]) AutoscaleMinY = thisYaxisLimitsMin[c];
            }
        }*/
    }

    if(thisYaxisScaling != fixedScale && thisYaxisType == log10){
        if (autoscaleMinYOverride) {
            AutoscaleMinY = manualAutoscaleMinY;
        } else manualAutoscaleMinY = AutoscaleMinY;
    }

    mutex.unlock();
}

// display the curves
void caStripPlot::TimeOut()
{
    if (plotIsPaused) return;

    int nbTicks;
    double elapsedTime = 0.0;

    if(!timerID) return;

    if(thisXticks < 1) nbTicks = 1; else nbTicks = thisXticks;

    //printf("timeout for numberofcurves=%d\n", NumberOfCurves);

    mutex.lock();

    // in case of restart plot, get start time and for the running time scale the new scale
    if(RestartPlot2) {
        RestartPlot2 = false;
        ftime(&plotStart);
        if(thisXaxisType != ValueScale) {
            QTime timeNow= QTime::currentTime();
            timeNow = timeNow.addSecs((int) -INTERVAL);
            setAxisScale(QwtPlot::xBottom, 0, INTERVAL, INTERVAL/nbTicks);

            if(thisXaxisType == TimeScaleFix) {
                PlotScaleEngine *scaleEngine = new PlotScaleEngine(nbTicks);
                setAxisScaleEngine(QwtPlot::xBottom, scaleEngine);
            } else {
                QwtLinearScaleEngine *scaleEngine= new QwtDateScaleEngine();
                setAxisScaleEngine(QwtPlot::xBottom, scaleEngine);
            }
            QDateTime dateTimeNow= QDateTime::currentDateTime();
            dateTimeNow = dateTimeNow.addSecs((int) -INTERVAL);
            setAxisScaleDraw (QwtPlot::xBottom, new PlotDateScaleDraw(dateTimeNow));
        }
        if(thisYaxisType == linear){
            setAxisScaleEngine(yLeft, new PlotLinearScaleEngine());
            setAxisScaleDraw(yLeft, new PlotScaleDraw);
        }
    }

    // get actual time
    ftime(&timeNow);

    // get time since restart
    elapsedTime = ((double) timeNow.time + (double) timeNow.millitm / (double)1000) -
                  ((double) plotStart.time + (double) plotStart.millitm / (double)1000);

    // change scale base in case of running time scale
    if(thisXaxisType != ValueScale) {
        timeData = INTERVAL + elapsedTime;
        setAxisScale(QwtPlot::xBottom, timeData - INTERVAL, timeData, INTERVAL/nbTicks);
    }

    // set data into the curves
    for (int c = 0; c < NumberOfCurves; c++ ) {
        if(thisStyle[c] == FillUnder) {
            fillcurve[c]->setSamplesList(fillData[c]);
            fillcurve[c]->setSamples(fillData[c]);
        }
        errorcurve[c]->setSamplesList(rangeData[c]);
        errorcurve[c]->setSamples(rangeData[c]);
        /*
        if(c==0) {
            printf("-----------------------\n");
            for(int j=0; j!=rangeData[c].size();j++) {
                printf("%d %f %f\n", j, rangeData[c][j].value, rangeData[c][j].interval.maxValue());
                if(qIsNaN(rangeData[c][j].interval.maxValue())) break;
            }
        }
*/
    }

    // in case of autoscale adjust the vertical scale
    if(thisYaxisScaling == autoScale || thisYaxisScaling == selectiveAutoScale) {
        if(!qIsInf(AutoscaleMinY) && !qIsInf(AutoscaleMaxY)) setAxisScale(QwtPlot::yLeft, AutoscaleMinY, AutoscaleMaxY);
    }

    if((ResizeFactorX != oldResizeFactorX) || ResizeFactorY != oldResizeFactorY) {
        axisScaleDraw(QwtPlot::xBottom)->setTickLength(QwtScaleDiv::MajorTick, ResizeFactorX * 8.0);
        axisScaleDraw(QwtPlot::xBottom)->setTickLength(QwtScaleDiv::MediumTick, ResizeFactorX * 6.0);
        axisScaleDraw(QwtPlot::xBottom)->setTickLength(QwtScaleDiv::MinorTick, ResizeFactorX * 4.0);
        axisScaleDraw(QwtPlot::xBottom)->setSpacing(1.0);
        axisScaleDraw(QwtPlot::yLeft)->setTickLength(QwtScaleDiv::MajorTick, ResizeFactorY * 8.0);
        axisScaleDraw(QwtPlot::yLeft)->setTickLength(QwtScaleDiv::MediumTick, ResizeFactorY * 6.0);
        axisScaleDraw(QwtPlot::yLeft)->setTickLength(QwtScaleDiv::MinorTick, ResizeFactorY * 4.0);
        axisScaleDraw(QwtPlot::yLeft)->setSpacing(1.0);
        oldResizeFactorX = ResizeFactorX;
        oldResizeFactorY = ResizeFactorY;
    }

    // replot
    replot();

    mutex.unlock();
}

void caStripPlot::setYscale(double ymin, double ymax) {
    setAxisScale(QwtPlot::yLeft, ymin, ymax);
    replot();
}

void caStripPlot::RescaleAxis()
{
    int i;
    // rescale axis
    for(i=0; i < NumberOfCurves; i++) {
        setData(realTim[i], realVal[i], i);
        // redraw legend if any
        curve[i]->setTitle(legendText(i));
    }
}

void caStripPlot::setLegendAttribute(QColor c, QFont f, LegendAtttribute SW)
{

    //printf("fontsize=%.1f %s\n", f.pointSizeF(), qasc(this->objectName()));
    //when legend text gets to small, hide it (will give then space for plot)
    setProperty("legendfontsize", f.pointSizeF());

#if QWT_VERSION < 0x060100
    for(int i=0; i < NumberOfCurves; i++) {

        if(f.pointSizeF() <= 4.0) {
            curve[i]->setItemAttribute(QwtPlotItem::Legend, false);
            continue;
        } else {
            if(!curve[i]->title().text().contains("?fill?")) curve[i]->setItemAttribute(QwtPlotItem::Legend, true);
        }

        switch (SW) {
        case TEXT:
            // done now through curve title
            break;

        case FONT:
            if(getLegendEnabled()) {
                if(legend() != (QwtLegend*) Q_NULLPTR) {
                    QList<QWidget *> list =  legend()->legendItems();
                    for (QList<QWidget*>::iterator it = list.begin(); it != list.end(); ++it ) {
                        QWidget *w = *it;
                        w->setFont(f);
                    }
                }
            }
            break;

        case COLOR:
            if(legend() != (QwtLegend*) Q_NULLPTR) {
                QList<QWidget *> list =  legend()->legendItems();
                for (QList<QWidget*>::iterator it = list.begin(); it != list.end(); ++it ) {
                    QWidget *w = *it;
                    QPalette palette = w->palette();
                    palette.setColor( QPalette::WindowText, c); // for ticks
                    palette.setColor( QPalette::Text, c);       // for ticks' labels
                    w->setPalette (palette);
                    w->setFont(f);
                }
            }
            break;
        }

    }
#else

    foreach (QwtPlotItem *plt_item, itemList()) {
        if (plt_item->rtti() == QwtPlotItem::Rtti_PlotCurve) {

            QwtPlotCurve *curve = static_cast<QwtPlotCurve *>(plt_item);

            if(f.pointSizeF() <= 4.0) {
                curve->setItemAttribute(QwtPlotItem::Legend, false);
                continue;
            } else {
                if(!curve->title().text().contains("?fill?")) {
                    curve->setItemAttribute(QwtPlotItem::Legend, false);
                    updateLegend();
                    curve->setItemAttribute(QwtPlotItem::Legend, true);
                }
            }

            QwtLegend *lgd = qobject_cast<QwtLegend *>(legend());
            if (lgd != (QwtLegend *) Q_NULLPTR){
                QList<QWidget *> legendWidgets = lgd->legendWidgets(itemToInfo(plt_item));
                if (legendWidgets.size() == 1) {
                    QwtLegendLabel *b = qobject_cast<QwtLegendLabel *>(legendWidgets[0]);
                    switch (SW) {

                    case TEXT:
                        // done now through curve title
                        break;

                    case FONT:
                        b->setFont(f);
                        b->update();
                        break;

                    case COLOR:
                        QPalette palette = b->palette();
                        palette.setColor(QPalette::WindowText, c); // for ticks
                        palette.setColor(QPalette::Text, c);       // for ticks' labels
                        b->setPalette(palette);
                        b->update();
                        break;

                    }
                }
            }
        }
    }
    updateLegend();
#endif

}

void caStripPlot::setData(struct timeb now, double Y, int curvIndex)
{
    if(curvIndex < 0 || curvIndex > (MAXCURVES-1)) return;

    mutex.lock();

    static std::vector<double> bmvec;
    static std::vector<double> bmvece;

    realVal[curvIndex] = Y;
    realTim[curvIndex] = now;
    if(Y> realMax[curvIndex]) realMax[curvIndex]  = Y;
    if(Y< realMin[curvIndex]) realMin[curvIndex]  = Y;

    // in case of fixed scales, remap the data to the first curve
    if(thisYaxisScaling == fixedScale) {
        // We need a different Algorithm for remapping linear vs logarithmic data
        if (thisYaxisType == linear) {
            double y0min = thisYaxisLimitsMin[0];
            double y0max = thisYaxisLimitsMax[0];
            double ymin =  thisYaxisLimitsMin[curvIndex];
            double ymax =  thisYaxisLimitsMax[curvIndex];
            actVal[curvIndex] = (y0max - y0min) / (ymax -ymin) * (realVal[curvIndex] - ymin) + y0min;
            minVal[curvIndex] = (y0max - y0min) / (ymax -ymin) * (realMin[curvIndex] - ymin) + y0min;
            maxVal[curvIndex] = (y0max - y0min) / (ymax -ymin) * (realMax[curvIndex] - ymin) + y0min;
        } else {
            double ymin =  thisYaxisLimitsMin[curvIndex];
            double ymax =  thisYaxisLimitsMax[curvIndex];
            if (ymin < 1e-20) setYaxisLimitsMin(curvIndex, 1e-20);
            if (ymax < 1e-19) setYaxisLimitsMax(curvIndex, 1e-19);
            double y0min = thisYaxisLimitsMin[0];
            double y0max = thisYaxisLimitsMax[0];
            if (y0min < 1e-20) setYaxisLimitsMin(0, 1e-20);
            if (y0max < 1e-19) setYaxisLimitsMax(0, 1e-19);

            double realValPositive = qMax(realVal[curvIndex], 1e-20);
            double realMinPositive = qMax(realMin[curvIndex], 1e-20);
            double realMaxPositive = qMax(realMax[curvIndex], 1e-20);
            actVal[curvIndex] = y0min*(pow((y0max/y0min),(std::log10(realValPositive/ymin)/std::log10(ymax/ymin))));
            minVal[curvIndex] = y0min*(pow((y0max/y0min),(std::log10(realMinPositive/ymin)/std::log10(ymax/ymin))));
            maxVal[curvIndex] = y0min*(pow((y0max/y0min),(std::log10(realMaxPositive/ymin)/std::log10(ymax/ymin))));
            /*qDebug() << "start";
            QElapsedTimer timer;
            timer.start();
            double timer1result = y0min*(pow((y0max/y0min),(std::log10(realVal[curvIndex]/ymin)/std::log10(ymax/ymin))));
            qDebug() << timer.nsecsElapsed() << timer1result;
            bmvec.insert(bmvec.end(), timer.nsecsElapsed());
            timer.invalidate();
            QElapsedTimer timer2;
            timer2.start();
            double timer2result = y0min*(pow((y0max/y0min),(log(realVal[curvIndex]/ymin)/log(ymax/ymin))));
            qDebug() << timer2.nsecsElapsed() << timer2result;
            bmvece.insert(bmvece.end(), timer2.nsecsElapsed());
            timer2.invalidate();
            qDebug() << "avg log10: " << std::reduce(bmvec.begin(), bmvec.end()) / bmvec.size();
            qDebug() << "avg loge: " << std::reduce(bmvece.begin(), bmvece.end()) / bmvece.size();
            qDebug() << "stop";*/
        }
        setYscale(getYaxisLimitsMin(0),getYaxisLimitsMax(0));
        // otherwise keep the data
    } else {
        actVal[curvIndex] = realVal[curvIndex];
        minVal[curvIndex] = realMin[curvIndex];
        maxVal[curvIndex] = realMax[curvIndex];
    }

    mutex.unlock();
}

void caStripPlot::showCurve(int number, bool on)
{
    if(number < 0 || number > (MAXCURVES-1)) return;
    curve[number]->setVisible(on);
    fillcurve[number]->setVisible(on);
    errorcurve[number]->setVisible(on);
    replot();
}

void caStripPlot::setXaxisEnabled(bool show)
{
    thisXshow = show;
    enableAxis(xBottom, show);
    replot();
}

void caStripPlot::setYaxisEnabled(bool show)
{
    thisYshow = show;
    enableAxis(yLeft, show);
    replot();
}


void caStripPlot::setLegendEnabled(bool show)
{
    thisLegendshow = show;
}

void caStripPlot::setTitlePlot(QString const &titel)
{
    thisTitle=titel;
    if(titel.size() != 0) {
        QwtText title(titel);
        title.setFont(QFont("Arial", 10));
        setTitle(title);
        replot();
    } else {
        setTitle("");
    }
}

void caStripPlot::setTitleX(QString const &titel)
{
    thisTitleX=titel;
    if(titel.size() != 0) {
        QwtText xAxis(titel);
        xAxis.setFont(QFont("Arial", 10));
        setAxisTitle(xBottom, xAxis);
    } else {
        setAxisTitle(xBottom, "");
    }
    replot();
}

void caStripPlot::setTitleY(QString const &titel)
{
    thisTitleY=titel;
    if(titel.size() != 0) {
        QwtText xAxis(titel);
        xAxis.setFont(QFont("Arial", 10));
        setAxisTitle(yLeft, xAxis);
    } else {
        setAxisTitle(yLeft, "");
    }
    replot();
}

void caStripPlot::setGrid(bool m)
{
    thisGrid = m;
    if(m) {
        penGrid = QPen(Qt::gray);
        penGrid.setStyle(Qt::DashLine);
        plotGrid->setPen(penGrid);
        plotGrid->setVisible(penGrid.style() != Qt::NoPen);
    } else {
        plotGrid->setVisible(false);
    }
    replot();
}

void caStripPlot::setGridColor(QColor c)
{
    thisGridColor = c;
    penGrid = QPen(c);
    penGrid.setStyle(Qt::DashLine);
    plotGrid->setPen(penGrid);
    plotGrid->setVisible(penGrid.style() != Qt::NoPen);
    replot();
}

QwtPlotCurve::CurveStyle caStripPlot::myStyle(curvStyle s)
{
    QwtPlotCurve::CurveStyle ms;
    switch ( s ) {
    case Lines:
        ms = QwtPlotCurve::Lines;  break;
    case FillUnder:
        ms = QwtPlotCurve::Lines;  break;
    default:
        ms = QwtPlotCurve::Lines; break;
    }
    return ms;
}

void caStripPlot::setBackground(QColor c)
{
    thisBackColor = c;
    QPalette canvasPalette(c);
    canvasPalette.setColor(QPalette::WindowText, QColor(133, 190, 232));
    canvas()->setPalette(canvasPalette);
    replot();
}

void caStripPlot::setForeground(QColor c)
{
    thisForeColor = c;
    setPalette(QPalette(c));
    replot();
}

void caStripPlot::setScaleColor(QColor c)
{
    QwtScaleWidget *scaleX =axisWidget(QwtPlot::xBottom);
    QwtScaleWidget *scaleY =axisWidget(QwtPlot::yLeft);
    thisScaleColor = c;
    QPalette palette = scaleX->palette();
    palette.setColor( QPalette::WindowText, c); // for ticks
    palette.setColor( QPalette::Text, c);       // for ticks' labels
    scaleX->setPalette( palette);
    scaleY->setPalette (palette);
    titleLabel()->setPalette(palette);
}

void caStripPlot::setColor(QColor c, int number)
{
    if(number < 0 || number > (MAXCURVES-1)) return;
    thisLineColor[number] = c;

    if(curve[number] != (QwtPlotCurve *) Q_NULLPTR) {
        curve[number]->setPen(QPen(c, 0));
    }

    if(fillcurve[number] != (QwtPlotCurveNaN *) Q_NULLPTR) {
        fillcurve[number]->setPen(c);
        fillcurve[number]->setBrush(QBrush(c, Qt::SolidPattern));
        fillcurve[number]->setRenderHint( QwtPlotItem::RenderAntialiased, true );
    }

    if(errorcurve[number] != (QwtPlotIntervalCurveNaN *) Q_NULLPTR) {
        if(thisStyle[number] == FillUnder) {
            errorcurve[number]->setPen(QPen(thisScaleColor,0));
            errorcurve[number]->setBrush(QBrush(thisScaleColor, Qt::SolidPattern));
        } else {
            errorcurve[number]->setPen(QPen(c, 0));
            errorcurve[number]->setBrush(QBrush(c, Qt::SolidPattern));
        }
        errorcurve[number]->setRenderHint( QwtPlotItem::RenderAntialiased, true);
    }
}

void caStripPlot::setStyle(curvStyle s, int number)
{
    if(number < 0 || number > (MAXCURVES-1)) return;
    thisStyle[number] = s;
    setColor(thisLineColor[number], number);
}

/**
 * eliminate stupid zero in value after the period
 */
void caStripPlot::ReplaceTrailingZerosByBlancs(char *asc)
{
    int i;
    int dot = false;
    for (i = 0; i < (int) strlen(asc); i++) {
        if (asc[i] == '.') {
            dot = true;
            break;
        }
    }
    if (dot) {
        for (i = (int)strlen(asc) - 1; i >= 0; i--) {
            if (asc[i] != '0') {
                if(asc[i] == '.') asc[i]=' ';
                break;
            }
            else asc[i] = ' ';
        }
    }
    if (asc[0] == '+') asc[0] = ' ';
    if (asc[1] == '+') asc[1] = ' ';
}


bool caStripPlot::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        const quint8 nButton = ((QMouseEvent*) event)->button();
        if (nButton == 1){
            // apply testing code like enabling certain features or showing values --> EMPTY IN PROD & COMMITS!
        }
        if (nButton == 1 && thisYaxisScaling == fixedScale && NumberOfCurves > 1 && thisIterableCurves) {
            // Ignore events on the canvas itself and stop them from being processed further, because it would generate another event.
            // Canvas events are ignored, because clicks on there are handled by the qwt plotpicker to select the clicked curve --> They are not for just iterating.
            if (obj->objectName() == "QwtPlotCanvas") return true;
            const quint8 newYAxisIndex = YAxisIndex+1;
            selectYAxis(newYAxisIndex);
            return true;
        }
        if(nButton==2) {
            //printf("emit from %s\n", qasc(this->objectName()));
            QPoint p;
            emit ShowContextMenu(p);
            return true;
        }
    } else if(event->type() == QEvent::Show) {
        //hoi
    }
    return QObject::eventFilter(obj, event);
}

#include "moc_castripplot.cpp"
