
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

#include "cawaterfallplot.h"



__inline double gauss(double x)
{
    return exp(-0.5*x*x);
}

class ColorMap_Color: public QwtLinearColorMap
{
public:
    ColorMap_Color():QwtLinearColorMap(Qt::darkCyan, Qt::red)
    {
        addColorStop(0.1, Qt::cyan);
        addColorStop(0.6, Qt::green);
        addColorStop(0.9, Qt::yellow);
    }

};

class ColorMap_Grey: public QwtLinearColorMap
{
public:
    ColorMap_Grey():QwtLinearColorMap(Qt::black, Qt::white)
    {
    }
};

class ColorMap_Heat: public QwtLinearColorMap
{
public:
    ColorMap_Heat(): QwtLinearColorMap(QColor(0,0,189), QColor(132,0,0))
    {
        double pos;
        pos = 1.0/13.0*1.0; addColorStop(pos, QColor(0,0,255));
        pos = 1.0/13.0*2.0; addColorStop(pos, QColor(0,66,255));
        pos = 1.0/13.0*3.0; addColorStop(pos, QColor(0,132,255));
        pos = 1.0/13.0*4.0; addColorStop(pos, QColor(0,189,255));
        pos = 1.0/13.0*5.0; addColorStop(pos, QColor(0,255,255));
        pos = 1.0/13.0*6.0; addColorStop(pos, QColor(66,255,189));
        pos = 1.0/13.0*7.0; addColorStop(pos, QColor(132,255,132));
        pos = 1.0/13.0*8.0; addColorStop(pos, QColor(189,255,66));
        pos = 1.0/13.0*9.0; addColorStop(pos, QColor(255,255,0));
        pos = 1.0/13.0*10.0; addColorStop(pos, QColor(255,189,0));
        pos = 1.0/13.0*12.0; addColorStop(pos, QColor(255,66,0));
        pos = 1.0/13.0*13.0; addColorStop(pos, QColor(189,0,0));
    }
};

class ColorMap_Custom: public QwtLinearColorMap
{
public:
    ColorMap_Custom():QwtLinearColorMap(QColor(80,80,0), QColor(100,0,0))
    {
        double r,g,b;
        for (int i = 0; i < 255; ++i) {
            rgbFromWaveLength(380.0 + (i * 400.0 / 255), r, g, b);
            addColorStop((double)(i)/255.0, QColor((int) r, (int) g, (int) b));
        }
    }
private:
    void rgbFromWaveLength(double wave, double &r, double &g, double &b)
    {
        r = 0.0;
        g = 0.0;
        b = 0.0;

        if (wave >= 380.0 && wave <= 440.0) {
            r = -1.0 * (wave - 440.0) / (440.0 - 380.0);
            b = 1.0;
        } else if (wave >= 440.0 && wave <= 490.0) {
            g = (wave - 440.0) / (490.0 - 440.0);
            b = 1.0;
        } else if (wave >= 490.0 && wave <= 510.0) {
            g = 1.0;
            b = -1.0 * (wave - 510.0) / (510.0 - 490.0);
        } else if (wave >= 510.0 && wave <= 580.0) {
            r = (wave - 510.0) / (580.0 - 510.0);
            g = 1.0;
        } else if (wave >= 580.0 && wave <= 645.0) {
            r = 1.0;
            g = -1.0 * (wave - 645.0) / (645.0 - 580.0);
        } else if (wave >= 645.0 && wave <= 780.0) {
            r = 1.0;
        }

        double s = 1.0;
        if (wave > 700.0)
            s = 0.3 + 0.7 * (780.0 - wave) / (780.0 - 700.0);
        else if (wave <  420.0)
            s = 0.3 + 0.7 * (wave - 380.0) / (420.0 - 380.0);

        r = pow(r * s, 0.8) * 255;
        g = pow(g * s, 0.8) * 255;
        b = pow(b * s, 0.8) * 255;
    }
};

caWaterfallPlot::caWaterfallPlot(QWidget *parent): QWidget(parent)
{

    QHBoxLayout *hboxLayout = new QHBoxLayout(this);

    thisCountNumber = 0;

    datamutex = new QMutex;

    // define a new plot
    plot = new QwtPlot(this);
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    plot->setSizePolicy(sizePolicy);

    // add it to layout
    hboxLayout->addWidget(plot);

    // define spectrogram
    d_spectrogram = new QwtPlotSpectrogram();
    d_spectrogram->setRenderThreadCount(0); // use system specific thread count

    // set a colormap for it
    d_spectrogram->setColorMap(new ColorMap_Heat());

    // define data
    m_data = new SpectrogramData();

    // set 200 rows ( NumberofColumns columns for demo
    setRows(nbRows);
    setCols(nbCols);
    ActualNumberOfColumns = NumberOfColumns = nbCols;
    reducedArray = (double*) malloc(ActualNumberOfColumns * sizeof(double));
    memset(reducedArray, 0, ActualNumberOfColumns *sizeof(double));

    // initialize data

    m_data->initData(NumberOfColumns, getRows());
    thisIntensityMin = 0;
    thisIntensityMax = 1000;
    setIntensityScalingMax(Channel);
    setIntensityScalingMin(Channel);

    // set data to spectrogram
    countRows = 0;
    d_spectrogram->setData(m_data);
    d_spectrogram->attach(plot);

    // define a grid
    plotGrid = new QwtPlotGrid();
    plotGrid->attach(plot);

    // no titles yet
    setTitlePlot("");
    setTitleX("");
    setTitleY("");

    // enable axis
    setXaxisEnabled(true);
    setYaxisEnabled(true);

    updatePlot();

    firstMonitorPlot = firstDemoPlot = firstTimerPlot = true;

    // set a timer for nice demo

    thisUnits = Millisecond;
    thisPeriod = 100;

    Timer = new QTimer(this);
    Timer->setInterval((int) thisPeriod);
    Timer->start();
    position = 0.0;
    drift = 1.0;
    disableDemo = false;
    connect(Timer, SIGNAL(timeout()), this, SLOT(TimeOut()));
}

void caWaterfallPlot::setRows(int const &rows)
{
    thisRows = rows;
}

void caWaterfallPlot::setCols(int const &cols)
{
    thisCols = cols;
}

void caWaterfallPlot::updatePlot()
{
    // A color bar on the right axis
    QwtScaleWidget *rightAxis = plot->axisWidget(QwtPlot::yRight);
    rightAxis->setTitle("Intensity");
    rightAxis->setColorBarEnabled(true);
    rightAxis->setColorMap(QwtInterval(thisIntensityMin, thisIntensityMax), new ColorMap_Heat());
    plot->setAxisScale(QwtPlot::yRight, thisIntensityMin, thisIntensityMax);
    plot->enableAxis(QwtPlot::yRight);

    // disble labels of left axis
    plot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Labels, false);
    plot->setAxisFont(QwtPlot::xBottom, QFont("Arial", 10));
    plot->setAxisFont(QwtPlot::yLeft, QFont("Arial", 10));

    plot->plotLayout()->setAlignCanvasToScales(true);
    plot->setAxisScale(QwtPlot::xTop, 0, getCols());
    plot->setAxisScale(QwtPlot::xBottom, 0, getCols());
    plot->setAxisMaxMinor(QwtPlot::xTop, 0);
    plot->setAxisScale(QwtPlot::yLeft, getRows(), 0.0);
    plot->setAxisMaxMinor(QwtPlot::yLeft, 0);

    plot->replot();
}

void caWaterfallPlot::setCountPV(QString const &newPV)  {
    bool isNumber = false;
    thisCountPV = newPV;
    thisCountNumber=0;
    if(thisCountPV.trimmed().length() > 0) {
        thisCountNumber = thisCountPV.toInt(&isNumber);
        if(!isNumber) {
            //printf("however not a number\n");
            thisCountNumber=0;
        }
    }
}

bool caWaterfallPlot::hasCountNumber(int *Number) {
    bool isNumber = false;
    if(thisCountPV.trimmed().length() > 0) {
        *Number = thisCountPV.toInt(&isNumber);
    }
    return isNumber;
}

void caWaterfallPlot::setCountNumber(int number) {
    thisCountNumber = number;
}

void caWaterfallPlot::InitData(int numCols)
{
    disableDemo = true;
    if(reducedArray != (double *) 0) {
        free(reducedArray);
        reducedArray = (double *) 0;
    }

    countRows = 0;
    setCols(numCols);
    ActualNumberOfColumns = NumberOfColumns = numCols;

    ActualNumberOfColumns = m_data->initData(NumberOfColumns, getRows());
    if(ActualNumberOfColumns == 0) return;
    setCols(ActualNumberOfColumns);
}

void caWaterfallPlot::myReplot()
{
#if QWT_VERSION >= 0x060100
    QwtPlotCanvas *canvas =  (QwtPlotCanvas *) plot->canvas();
    canvas->replot();
#else
    plot->canvas()->replot();
#endif
}

template <typename pureData> void caWaterfallPlot::AverageArray(pureData *vec, int size, int arraySize, double *avg, int ratio)
{
    int AverageCounter = 0;
    for (int i=0; i<size-ratio; i+=ratio) {
        double mean = 0;
        for(int j=0; j<ratio; j++) {
            if((i+j) >= arraySize) {
                break;
            } else {
               mean += vec[i+j];
            }
        }
        avg[AverageCounter++]= mean / (double) ratio;
    }
}

template <typename pureData> void caWaterfallPlot::CompressAndkeepArray(pureData *vec, int size, int arraySize)
{
    datamutex->lock();
    int ratio = m_data->getRatio(NumberOfColumns, ActualNumberOfColumns);
    if(reducedArray != (double *) 0) {
        free(reducedArray);
        reducedArray = (double *) 0;
    }
    reducedArray = (double*) malloc(ActualNumberOfColumns * sizeof(double));
    memset(reducedArray, 0, ActualNumberOfColumns *sizeof(double));
    AverageArray(vec, size, arraySize, reducedArray, ratio);
    datamutex->unlock();
}

void caWaterfallPlot::setData(double *array, int size)
{
    //printf("size=%d count=%d\n", size, thisCountNumber);
    int newSize = size;

    if(thisCountNumber > 0) {
        newSize = thisCountNumber;
    } else {
        newSize = thisCountNumber = size;
    }

    //if(thisCountNumber > 0) newSize = qMin(thisCountNumber, size);

    ActualNumberOfColumns = NumberOfColumns = newSize;
    if(thisUnits != Monitor) {
        CompressAndkeepArray(array, newSize, size);
    } else {
        int actualColumns = m_data->setData(array, countRows, NumberOfColumns, getRows(), size);
        setCols(actualColumns);
    }
}

void caWaterfallPlot::setData(float *array, int size)
{
    //printf("size=%d count=%d\n", size, thisCountNumber);
    int newSize = size;

    if(thisCountNumber > 0) {
        newSize = thisCountNumber;
    } else {
        newSize = thisCountNumber = size;
    }

    //if(thisCountNumber > 0) newSize = qMin(thisCountNumber, size);

    ActualNumberOfColumns = NumberOfColumns = newSize;
    if(thisUnits != Monitor) {
        CompressAndkeepArray(array, newSize, size);
    } else {
        int actualColumns = m_data->setData(array, countRows, NumberOfColumns, getRows(), size);
        setCols(actualColumns);
    }
}

void caWaterfallPlot::setData(int16_t *array, int size)
{
    //printf("size=%d count=%d\n", size, thisCountNumber);
    int newSize = size;

    if(thisCountNumber > 0) {
        newSize = thisCountNumber;
    } else {
        newSize = thisCountNumber = size;
    }

    //if(thisCountNumber > 0) newSize = qMin(thisCountNumber, size);

    ActualNumberOfColumns = NumberOfColumns = newSize;
    if(thisUnits != Monitor) {
        CompressAndkeepArray(array, newSize, size);
    } else {
        int actualColumns = m_data->setData(array, countRows, NumberOfColumns, getRows(), size);
        setCols(actualColumns);
    }
}

void caWaterfallPlot::setData(int32_t *array, int size)
{
     //printf("size=%d count=%d\n", size, thisCountNumber);
     int newSize = size;

     if(thisCountNumber > 0) {
         newSize = thisCountNumber;
     } else {
         newSize = thisCountNumber = size;
     }

     //if(thisCountNumber > 0) newSize = qMin(thisCountNumber, size);

    ActualNumberOfColumns = NumberOfColumns = newSize;
    if(thisUnits != Monitor) {
        CompressAndkeepArray(array, newSize, size);
    } else {
        int actualColumns = m_data->setData(array, countRows, NumberOfColumns, getRows(), size);
        setCols(actualColumns);
    }
}

void caWaterfallPlot::displayData()
{
    // displayData will always be called, but in case of monitor will have to be executed
    // otherwise the timer will display the data
    if(thisUnits == Monitor) {
        if(firstMonitorPlot) {
            updatePlot();
            m_data->setLimits(0., getCols(), 0., getRows(), thisIntensityMin, thisIntensityMax);
            firstMonitorPlot = false;
        } else {
            myReplot();
        }

    }
}

void caWaterfallPlot::defineTimerUpdate(units unit, double period)
{
    int INTERVAL;
    thisUnits = unit;
    thisPeriod = period;
    if(unit == Millisecond) {
        INTERVAL = (int) period;
    } else if(unit == Second) {
        INTERVAL = (int) (period * 1000.0);
    } else if(unit == Minute) {
        INTERVAL = (int) (1000.0 * period * 60);
    } else {
        // printf("\nunknown unit\n");
        INTERVAL = (int) period;
    }
    Timer->setInterval(INTERVAL);
}

void caWaterfallPlot::TimeOut()
{
    // demo curve
    if(thisUnits != Monitor) {
        if(!disableDemo) {
            datamutex->lock();
            GausCurv(position);
            m_data->setData(reducedArray, countRows, ActualNumberOfColumns, getRows(), ActualNumberOfColumns);
            setCols(ActualNumberOfColumns);

            if(firstDemoPlot) {
                updatePlot();
                m_data->setLimits(0., getCols(), 0., getRows(), thisIntensityMin, thisIntensityMax);
                firstDemoPlot = false;
            }

            datamutex->unlock();
            if(drift > 0 && position >= NumberOfColumns) drift = -1;
            if(drift < 0 && position <= 0)  drift = 1;
            position += drift;
        } else {
            if(reducedArray != (double*) 0) {
                datamutex->lock();
                m_data->setData(reducedArray, countRows, ActualNumberOfColumns, getRows(),  ActualNumberOfColumns);
                if(firstTimerPlot) {
                    updatePlot();
                    m_data->setLimits(0., getCols(), 0., getRows(), thisIntensityMin, thisIntensityMax);
                    firstTimerPlot = false;
                }
                datamutex->unlock();
            }
        }
        myReplot();
    }
}

// gaus curve for demo drawing
void caWaterfallPlot::GausCurv(double middle) {

    double min = 0.0;
    double max = 1000.0;
    double range = max - min;
    double sigma = 100.0;
    for (int i=0; i<nbCols; i++) {
        reducedArray[i] =  min + range * gauss((i-middle)/sigma);
    }
}

void caWaterfallPlot::setTitleX(QString const &titel)
{
    thisTitleX=titel;
    if(titel.size() != 0) {
        QwtText xAxis(titel);
        xAxis.setFont(QFont("Arial", 11));
        plot->setAxisTitle(QwtPlot::xBottom, xAxis);
    }
    plot->replot();
}

void caWaterfallPlot::setTitleY(QString const &titel)
{
    thisTitleY=titel;
    if(titel.size() != 0) {
        QwtText yAxis(titel);
        yAxis.setFont(QFont("Arial", 11));
        plot->setAxisTitle(QwtPlot::yLeft, yAxis);
    }
    plot->replot();
}

void caWaterfallPlot::setXaxisEnabled(bool show)
{
    thisXshow = show;
    plot->enableAxis(QwtPlot::xBottom, show);
    plot->replot();
}

void caWaterfallPlot::setYaxisEnabled(bool show)
{
    thisYshow = show;
    plot->enableAxis(QwtPlot::yLeft, show);
    plot->replot();
}

void caWaterfallPlot::setTitlePlot(QString const &titel)
{
    thisTitle=titel;
    if(titel.size() != 0) {
        QwtText title(titel);
        title.setFont(QFont("Arial", 11));
        plot->setTitle(title);
        plot->replot();
    }
}

void caWaterfallPlot::setGrid(bool m)
{
    thisGrid = m;
    if(m) {
        plotGrid->setVisible(true);
    } else {
        plotGrid->setVisible(false);
    }
    plot->replot();
}


