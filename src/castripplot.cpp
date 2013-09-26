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

#if defined(_MSC_VER)
#include <windows.h>
#define QWT_DLL
#endif

#include <qapplication.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpainter.h>
#include "castripplot.h"
#include <QList>

#define MULTFOROVERLAPPINGTIMES 3.0

class TimeScaleDraw: public QwtScaleDraw
{
public:

    TimeScaleDraw(const QTime &base): baseTime(base)
    {
    }
    virtual QwtText label(double v) const
    {
        // while addsecs has a problem after some 3/4 of a month in seconds, we normalize the seconds to 24 * 3600 seconds
        double day = 24 * 3600;
        double normalized = v - day * floor(v/day);

        QTime upTime = baseTime.addSecs((int) normalized);
        //printf("label = addseconds=%d start plottime=%s labeltime=%s\n", seconds, baseTime.toString().toAscii().constData(),  upTime.toString().toAscii().constData());
        return upTime.toString();
    }

private:
    QTime baseTime;

};

caStripPlot::~caStripPlot() {

    emit timerThreadStop();
    timerThread->wait(200);
    timerThread->deleteLater();
}

caStripPlot::caStripPlot(QWidget *parent): QwtPlot(parent)
{
    timerID = false;
    thisXaxisType = TimeScale;
    HISTORY = 60;
    thisUnits = Second;
    thisPeriod = 60;
    NumberOfCurves = MAXCURVES;
    onInit = true;
    timeInterval = 1.0;
    setAutoReplot(false);
    setAutoFillBackground(true);
    Start = true;

    // define a grid
    plotGrid = new QwtPlotGrid();
    plotGrid->attach(this);

    plotLayout()->setAlignCanvasToScales(true);

    // define our axis
    if(thisXaxisType == TimeScale) {
        setAxisScale(QwtPlot::xBottom, 0, HISTORY);
    } else {
        setAxisScale(QwtPlot::xBottom, -HISTORY, 0);
    }

    setAxisLabelAlignment(QwtPlot::xBottom, Qt::AlignLeft | Qt::AlignBottom);
    setAxisScale(QwtPlot::yLeft, 0, 1000);

    if(thisXaxisType == TimeScale) {
        // due to time scale we need some distance
        scaleWidget = axisWidget(QwtPlot::xBottom);
        const int fmh = QFontMetrics(scaleWidget->font()).height();
        scaleWidget->setMinBorderDist(fmh * 2, fmh * 2);
        // define time axis
        QTime timeNow= QTime::currentTime();
        setAxisScaleDraw ( QwtPlot::xBottom, new TimeScaleDraw ( timeNow ) );
    }

    // define our curves
    for(int i=0; i< MAXCURVES; i++) {
        curve[i] = new QwtPlotCurve();
        errorcurve[i] = new QwtPlotIntervalCurve();
        fillcurve[i] = new QwtPlotIntervalCurve();
        if(i>0) {
            curve[i]->setZ(curve[i]->z() - i);
            fillcurve[i]->setZ(fillcurve[i]->z() - i);
            errorcurve[i]->setZ(errorcurve[i]->z() - i);
        }
        curve[i]->attach(this);
        errorcurve[i]->attach(this);
        fillcurve[i]->attach(this);
        showCurve(i, false);

        thisYaxisLimitsMax[i] = 100;
        thisYaxisLimitsMin[i] = 0;
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

    setAxisFont(QwtPlot::xBottom, QFont("Arial", 10));
    setAxisFont(QwtPlot::yLeft, QFont("Arial", 10));

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

void caStripPlot::defineAxis(units unit, double period)
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

    // set axis and in case of a time scale define the time axis
    setAxis(interval, period);
    replot();
}

void caStripPlot::setAxis(double interval, double period)
{
    // set axis and in case of a time scale define the time axis
    if(thisXaxisType == TimeScale) {
        QTime timeNow= QTime::currentTime();
        timeNow = timeNow.addSecs((int) -interval);
        setAxisScale(QwtPlot::xBottom, 0, interval);
        setAxisScaleDraw ( QwtPlot::xBottom, new TimeScaleDraw (timeNow) );
    } else {
        setAxisScale(QwtPlot::xBottom, -period, 0);
        setAxisScaleDraw(QwtPlot::xBottom, new QwtScaleDraw());
    }
}

void caStripPlot::RescaleCurves(int width, units unit, double period)
{
    HISTORY = width; // equals canvas width

    if(unit == Millisecond) {
        timeInterval = period / (double) HISTORY;
        INTERVAL = period / 1000.0;
    } else if(unit == Second) {
        timeInterval = 1000.0 * period / (double) HISTORY;
        INTERVAL = period;
    } else if(unit == Minute) {
        timeInterval =  1000.0 * period * 60.0 / (double) HISTORY;
        INTERVAL = period * 60;
    } else {
        // printf("\nunknown unit\n");
        timeInterval =  1000.0 * period * 60.0 / (double) HISTORY;
        INTERVAL = period;
    }

    mutex.lock();

    for(int i=0; i < MAXCURVES; i++) {
        // define interval data for error and fill curves
        rangeData[i].clear();
        fillData[i].clear();
        if(i==0)  base.clear();
        for ( int j = 0; j < MULTFOROVERLAPPINGTIMES * HISTORY; j++ ) {
            rangeData[i].append(QwtIntervalSample(0, QwtInterval(0.0, 0.0)));
            fillData[i].append(QwtIntervalSample(0, QwtInterval(0.0, 0.0)));
            if(i==0) base.append(QwtIntervalSample(0, QwtInterval(0.0, 0.0)));
        }
    }

    for(int i=0; i < NumberOfCurves; i++) {
        if(thisStyle[i] == FillUnder) fillcurve[i]->setSamples(fillData[i].toVector());
        errorcurve[i]->setSamples(rangeData[i].toVector());
    }

    mutex.unlock();

    // define update rate
    dataCount = 0;
    updateRate = (int) (100.0 / timeInterval / 2.0);
    if(updateRate < 1) updateRate=1;
    //printf("\n width=%d period=%f timerInterval=%d updaterate=%d\n", HISTORY, period, (int) timeInterval, updateRate);

    timerCount = 0;

    if(timerID) {
        // change display timer interval
        int interval = qMax((int)timeInterval, 20);
        Timer->setInterval(interval);
        Timer->start();

        // stop run method of thread and redefine repetition time then start run
        emit timerThreadStop();
        timerThread->wait(200);
        timerThread->threadSetTimer((int) timeInterval);
        timerThread->start();
    }

    replot();
}

// resize plot when resizing
void caStripPlot::resizeEvent ( QResizeEvent * event )
{
    QwtPlot::resizeEvent(event);
    RescaleCurves(canvas()->size().width(), Unit, Period);
    replot();
    if(timerID) RescaleAxis();
}

QString caStripPlot::legendText(int i)
{
    char min[20], max[20];
    QString MinMax;
    QString titre(savedTitres.at(i));
    sprintf(min, "%.1f", thisYaxisLimitsMin[i]);
    sprintf(max, "%.1f", thisYaxisLimitsMax[i]);
    ReplaceTrailingZerosByBlancs(min);
    ReplaceTrailingZerosByBlancs(max);
    MinMax.sprintf("[%s,%s]", min, max);
    MinMax = MinMax.simplified(); MinMax.replace( " ", "" );
    titre.append(" ");
    titre.append(MinMax);
    return titre;
}

void caStripPlot::defineCurves(QStringList titres, units unit, double period, int width, int nbCurves)
{
    int min, max;
    NumberOfCurves = nbCurves;
    Unit = unit;
    Period = period;
    scaleWidget->getBorderDistHint(min, max);
    savedTitres = titres;

    defineAxis(unit, period);

    // define curves
    for(int i=0; i < MAXCURVES; i++) {
        curvStyle s = Lines;
        QColor c;

        if(curve[i] != (QwtPlotCurve*) 0) {
            s = getStyle(i);
            c = getColor(i);
            delete curve[i];
        }

        if(errorcurve[i] != (QwtPlotIntervalCurve*) 0) {
            delete errorcurve[i];
        }
        if(fillcurve[i] != (QwtPlotIntervalCurve*) 0) {
            delete fillcurve[i];
        }

        if(i < NumberOfCurves) {
            // set title and limits to legend
            QString titre = legendText(i);

            curve[i] = new QwtPlotCurve(titre);
            errorcurve[i] = new QwtPlotIntervalCurve(titre);
            fillcurve[i] = new QwtPlotIntervalCurve(titre);
            setStyle(s, i);

            curve[i]->setZ(i);
            curve[i]->attach(this);
            fillcurve[i]->setZ(i);
            fillcurve[i]->attach(this);
            fillcurve[i]->setItemAttribute(QwtPlotItem::Legend, false);
            errorcurve[i]->setZ(i);
            errorcurve[i]->attach(this);
            errorcurve[i]->setItemAttribute(QwtPlotItem::Legend, false);

            showCurve(i, false);

            realMax[i] = maxVal[i] = -1000000;
            realMin[i] = minVal[i] =  1000000;
            actVal[i] = realVal[i] = 0;
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
}

void caStripPlot::startPlot()
{
    //change display timer interval
    int interval = qMax((int)timeInterval, 20);
    Timer->setInterval(interval);
    Timer->start();

    // stop run method of thread and redefine repetition time then start run
    emit timerThreadStop();
    timerThread->wait(200);
    timerThread->threadSetTimer((int)timeInterval);
    timerThread->start();

    timerID = true;

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
    double value = 0.0;
    double elapsed=0.0;

    if(!timerID) return;

     mutex.lock();

    int dataCountLimit = (int) (MULTFOROVERLAPPINGTIMES * HISTORY -1);

    // we need an exact time scale
    if(Start) {
        Start = false;
        ftime(&timeStart);
    }
    ftime(&timeNow);

    elapsedTime = ((double) timeNow.time + (double) timeNow.millitm / (double)1000) -
            ((double) timeStart.time + (double) timeStart.millitm / (double)1000);

    timeData = INTERVAL + elapsedTime;

    if(dataCount > 1) {

        // shift data and our timebase for the fixed scale
        if(thisXaxisType != TimeScale) {
            for (c = 0; c < NumberOfCurves; c++ ) {
                // shift left and cur last
                rangeData[c].prepend(tmp);
                rangeData[c].removeLast();
                if(thisStyle[c] == FillUnder) {
                    fillData[c].prepend(tmp);
                    fillData[c].removeLast();
                }
            }

            base.prepend(tmp);
            base.removeLast();
            elapsed = elapsedTime + INTERVAL;
            for (i = dataCount; i > 0; i-- ) {
                value = base[i].value - elapsed;
                // correct value to fit again inside the interval
                if(Unit == Millisecond) {
                    value = value * 1000.0;
                } else if(Unit == Minute) {
                    value = value / 60.0;
                }
                for (c = 0; c < NumberOfCurves; c++ ) {
                    rangeData[c][i].value = fillData[c][i].value = value;
                }
            }

            // shift data
        } else {
            for (c = 0; c < NumberOfCurves; c++ ) {
                rangeData[c].prepend(tmp);
                if(thisStyle[c] == FillUnder) fillData[c].prepend(tmp);
                rangeData[c].removeLast();
                if(thisStyle[c] == FillUnder) fillData[c].removeLast();
            }
        }
    }

    // advance data points
    if (dataCount < dataCountLimit) dataCount++;

    // update last point
    for (int c = 0; c < NumberOfCurves; c++ ) {
        double y0, y1, y2, y3;

        // smallest vertical width of error bar must not be zero
        y0 = transform(QwtPlot::yLeft, minVal[c]);
        y1 = transform(QwtPlot::yLeft, maxVal[c]);
        if(qAbs(y1-y0) < 2.0) y0 = y0 + 1.0; y1= y1 - 1.0;
        y2 = invTransform(QwtPlot::yLeft, (int) (y0+0.5));
        y3 = invTransform(QwtPlot::yLeft, (int) (y1+0.5));

        // set the range into the array
        rangeData[c][0] = QwtIntervalSample( timeData, QwtInterval(y2, y3));
        // and set also the base time
        if(thisXaxisType != TimeScale) {
            base[0] = QwtIntervalSample(  timeData, QwtInterval(y2, y3));
        }

        //  and to fillunder we add a range to the fill curve

        if(thisStyle[c] == FillUnder) {
            double value, height, y4, y5;
            if(minVal[c] < 0.0)  value = y0; else value = y1;
            height = canvas()->contentsRect().height();
            y1 = transform(QwtPlot::yLeft, 0.0);
            if(y1 > height) y1 = height;
            if(y1 < 0.0) y1 =0.0;
            y4 = invTransform(QwtPlot::yLeft, (int) (value+0.5));
            y5 = invTransform(QwtPlot::yLeft, (int) (y1+0.5));

            // set the range into the array
            fillData[c][0] = QwtIntervalSample( timeData, QwtInterval(y4, y5));
        }

    }

    // keep max and min
    for (int c = 0; c < NumberOfCurves; c++ ) {
        maxVal[c] = minVal[c] = actVal[c];
        realMax[c] = realMin[c] = realVal[c];
    }

    mutex.unlock();
}

// display
void caStripPlot::TimeOut()
{
    int c, j;
    double elapsedTime = 0.0;
    int delta = 0;
    double x0, x1, increment;
    int totalMissed = 0;

    int dataCountLimit = (int) (MULTFOROVERLAPPINGTIMES * HISTORY -1);

    if(!timerID) return;

    mutex.lock();

    // we need an exact time scale
    if(Start) {
        Start = false;
        ftime(&timeStart);
    }
    ftime(&timeNow);

    elapsedTime = ((double) timeNow.time + (double) timeNow.millitm / (double)1000) -
            ((double) timeStart.time + (double) timeStart.millitm / (double)1000);

    // change scale base in case of running time scale
    if(thisXaxisType == TimeScale) {
        timeData = INTERVAL + elapsedTime;
        setAxisScale(QwtPlot::xBottom, timeData - INTERVAL, timeData);
    }
    
    // replot in order to get the exact transformation of real coordinates to pixels
    if(thisXaxisType == TimeScale) {
        replot();
    } else {
#if QWT_VERSION >= 0x060100
        QwtPlotCanvas *canvas =  (QwtPlotCanvas *) this->canvas();
        canvas->replot();
#else
        canvas()->replot();
#endif
    }

    // we want to be sure that no pixels are missed, this is particularly important for ms windows
    // this is really not nice and time consuming, up to now no better solution
    for (int i = 2; i < dataCount; i++ ) {
        x0 = transform(QwtPlot::xBottom, rangeData[0][i-1].value);
        x1 = transform(QwtPlot::xBottom, rangeData[0][i].value);
        delta = (int) (x0+0.5) - (int) (x1+0.5) - 1;

        if(delta > 0 && x0 > 0 && x1 > 0 && delta < 20) {
            increment = (base[i-1].value - base[i].value)/ (double) (delta+1);
            //printf("===============> missed ticks=%d at=%d x0=%.1f x1=%.1f %d %d\n", delta, i, x0, x1, (int) (x0+0.5) , (int) (x1+0.5));
            totalMissed++;

            // insert missing time base data and adjust time holes
            if(thisXaxisType != TimeScale) {
                for(j = 0; j < delta; j++) {
                    base.insert(i, base[i]);
                    base.removeLast();
                }
                for(j = 1; j < delta+1; j++) {
                    base[j+i-1].value = base[i-1].value - increment * (double) j;
                }
            }

            // insert missing data and timebase
            for (c = 0; c < NumberOfCurves; c++ ) {
                for(j = 0; j < delta; j++) {
                    if(thisStyle[c] == FillUnder) {
                        fillData[c].insert(i, fillData[c][i]);
                        fillData[c].removeLast();
                    }
                    rangeData[c].insert(i, rangeData[c][i]);
                    rangeData[c].removeLast();
                }
                for(j = 1; j < delta+1; j++) {
                    rangeData[c][j+i-1].value = fillData[c][j+i-1].value = invTransform(QwtPlot::xBottom, (int) (x0+0.5)-j);
                }
            }

            if ((dataCount + delta) < dataCountLimit) dataCount = dataCount + delta;

        }
    }

    for (int c = 0; c < NumberOfCurves; c++ ) {

        if(thisStyle[c] == FillUnder) {
            fillcurve[c]->setSamples(fillData[c].toVector());
        }
        errorcurve[c]->setSamples(rangeData[c].toVector());
    }

    if(thisXaxisType == TimeScale) {
        replot();
    } else {
#if QWT_VERSION >= 0x060100
        QwtPlotCanvas *canvas =  (QwtPlotCanvas *) this->canvas();
        canvas->replot();
#else
        canvas()->replot();
#endif
    }
    timerCount=0;
    
    mutex.unlock();

}

void caStripPlot::setYscale(double ymin, double ymax) {
    setAxisScale(QwtPlot::yLeft, ymin, ymax);
    replot();
}

void caStripPlot::RescaleAxis()
{
    int i;
    // recale axis
    for(i=0; i < NumberOfCurves; i++) {
        setData(realTim[i], realVal[i], i);
    }

    // redraw legend if any
    setLegendAttribute(thisScaleColor, QFont("arial", 9), TEXT);
}

void caStripPlot::setLegendAttribute(QColor c, QFont f, LegendAtttribute SW)
{
    int i;

#if QWT_VERSION < 0x060100
    for(i=0; i < NumberOfCurves; i++) {

        switch (SW) {
        case TEXT:
            if(thisLegendshow) {
                QwtText text;
                text.setText(legendText(i));
                qobject_cast<QwtLegendItem*>(this->legend()->find(curve[i]))->setText(text);
            }
            break;

        case FONT:

            if(getLegendEnabled()) {
                if(legend() != (QwtLegend*) 0) {
                    QList<QWidget *> list =  legend()->legendItems();
                    for (QList<QWidget*>::iterator it = list.begin(); it != list.end(); ++it ) {
                        QWidget *w = *it;
                        w->setFont(f);
                    }
                }
            }
            break;

        case COLOR:
            if(legend() != (QwtLegend*) 0) {
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
    i=0;
    foreach (QwtPlotItem *plt_item, itemList()) {
        if ( plt_item->rtti() == QwtPlotItem::Rtti_PlotCurve ) {
            QwtLegend *lgd = qobject_cast<QwtLegend *>(legend());
            QList<QWidget *> legendWidgets = lgd->legendWidgets(itemToInfo(plt_item));
            if ( legendWidgets.size() == 1 ) {
                QwtLegendLabel *b = qobject_cast<QwtLegendLabel *>(legendWidgets[0]);
                switch (SW) {

                case TEXT:

                    if(thisLegendshow) {

                        QwtText text;
                        text.setText(legendText(i++));
                        //printf("%s %s\n", b->plainText().toAscii().constData(), legendText(i-1).toAscii().constData());
                        b->setText(text);
                        b->update();


                    }
                    break;

                case FONT:
                    //printf("%s %s\n", b->plainText().toAscii().constData(), legendText(i-1).toAscii().constData());

                    b->setFont(f);
                    b->update();

                    break;

                case COLOR:

                    //printf("%s %s\n", b->plainText().toAscii().constData(), legendText(i-1).toAscii().constData());
                    QPalette palette = b->palette();
                    palette.setColor( QPalette::WindowText, c); // for ticks
                    palette.setColor( QPalette::Text, c);       // for ticks' labels
                    b->setPalette(palette);
                    b->update();

                    break;

                }


            }
        }
    }
#endif

}

void caStripPlot::setData(struct timeb now, double Y, int curvIndex)
{
    if(curvIndex < 0 || curvIndex > (MAXCURVES-1)) return;

    mutex.lock();

    realVal[curvIndex] = Y;
    realTim[curvIndex] = now;
    if(Y> realMax[curvIndex]) realMax[curvIndex]  = Y;
    if(Y< realMin[curvIndex]) realMin[curvIndex]  = Y;

    double y0min = thisYaxisLimitsMin[0];
    double y0max = thisYaxisLimitsMax[0];
    double ymin =  thisYaxisLimitsMin[curvIndex];
    double ymax =  thisYaxisLimitsMax[curvIndex];

    actVal[curvIndex] = (y0max - y0min) / (ymax -ymin) * (realVal[curvIndex] - ymin) + y0min;
    minVal[curvIndex] = (y0max - y0min) / (ymax -ymin) * (realMin[curvIndex] - ymin) + y0min;
    maxVal[curvIndex] = (y0max - y0min) / (ymax -ymin) * (realMax[curvIndex] - ymin) + y0min;

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
        title.setFont(QFont("Arial", 11));
        setTitle(title);
        replot();
    }
}

void caStripPlot::setTitleX(QString const &titel)
{
    thisTitleX=titel;
    if(titel.size() != 0) {
        QwtText xAxis(titel);
        xAxis.setFont(QFont("Arial", 11));
        setAxisTitle(xBottom, xAxis);
    }
    replot();
}

void caStripPlot::setTitleY(QString const &titel)
{
    thisTitleY=titel;
    if(titel.size() != 0) {
        QwtText xAxis(titel);
        xAxis.setFont(QFont("Arial", 11));
        setAxisTitle(yLeft, xAxis);
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
    QwtPlotCurve::CurveStyle ms = QwtPlotCurve::Lines;
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
    canvasPalette.setColor(QPalette::Foreground, QColor(133, 190, 232));
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

    if(curve[number] != (QwtPlotCurve *) 0) {
        curve[number]->setPen(QPen(c, 0));
    }

    if(fillcurve[number] != (QwtPlotIntervalCurve *) 0) {
        fillcurve[number]->setStyle( QwtPlotIntervalCurve::NoCurve);
        QwtIntervalSymbol *errorBar1 = new QwtIntervalSymbol( QwtIntervalSymbol::Bar );
        errorBar1->setWidth(1);
        errorBar1->setPen(QPen(c, 1));
        fillcurve[number]->setSymbol( errorBar1 );
        fillcurve[number]->setRenderHint( QwtPlotItem::RenderAntialiased, false );
    }

    if(errorcurve[number] != (QwtPlotIntervalCurve *) 0) {
        errorcurve[number]->setStyle( QwtPlotIntervalCurve::NoCurve);
        QwtIntervalSymbol *errorBar2 = new QwtIntervalSymbol( QwtIntervalSymbol::Bar );
        errorBar2->setWidth(1);
        if(thisStyle[number] == FillUnder) {
            errorBar2->setPen(QPen(thisScaleColor,1));
        } else {
            errorBar2->setPen(QPen(c, 1));
        }
        errorcurve[number]->setSymbol( errorBar2 );
        errorcurve[number]->setRenderHint( QwtPlotItem::RenderAntialiased, false );
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
        for (i = strlen(asc) - 1; i >= 0; i--) {
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
        int nButton = ((QMouseEvent*) event)->button();
        if(nButton==2) {
            //printf("emit from %s\n", this->objectName().toAscii().constData());
            QPoint p;
            emit ShowContextMenu(p);
        }
    }
    return QObject::eventFilter(obj, event);
}

#include "moc_castripplot.cpp"

