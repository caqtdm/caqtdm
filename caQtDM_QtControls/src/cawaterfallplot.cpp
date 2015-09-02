
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
    d_spectrogram->setColorMap(new ColorMap_Wavelength());

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

    setCustomMap("");
    setDiscreteCustomMap(false);

    updatePlot();

    firstMonitorPlot = firstDemoPlot = firstTimerPlot = true;

    // set a timer for nice demo

    thisUnits = Millisecond;
    thisPeriod = 200;

    Timer = new QTimer(this);
    Timer->setInterval((int) thisPeriod);
    Timer->start();
    position = 0.0;
    drift = 1.0;
    disableDemo = false;
    connect(Timer, SIGNAL(timeout()), this, SLOT(TimeOut()));
}

bool caWaterfallPlot::isPropertyVisible(Properties property)
{
    return designerVisible[property];
}

void caWaterfallPlot::setPropertyVisible(Properties property, bool visible)
{
    designerVisible[property] = visible;
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
    rightAxis->setColorMap(QwtInterval(thisIntensityMin, thisIntensityMax), new ColorMap_Wavelength());
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
        //printf("\nunknown unit\n");
        INTERVAL = (int) period;
    }
    if(INTERVAL < 100) INTERVAL = 100;
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

void caWaterfallPlot::setColormap(colormap const &map)
{
    thisColormap = map;
    setPropertyVisible(customcolormap, false);
    setPropertyVisible(discretecolormap, false);

    switch (map) {

    case grey:
        d_spectrogram->setColorMap(new ColorMap_Grey());
        break;
    case spectrum_wavelength:
        d_spectrogram->setColorMap(new ColorMap_Wavelength());
        break;
    case spectrum_hot:
        d_spectrogram->setColorMap(new ColorMap_Hot());
        break;
    case spectrum_heat:
        d_spectrogram->setColorMap(new ColorMap_Heat());
        break;
    case spectrum_jet:
        d_spectrogram->setColorMap(new ColorMap_Jet());
        break;
    case spectrum_custom: {
        int *colorIndexes=NULL;
        setPropertyVisible(customcolormap, true);
        setPropertyVisible(discretecolormap, true);
        // user has the possibility to input its own colormap with discrete QtColors from 2 t0 18
        // when nothing given, fallback to default colormap
        if(thisCustomMap.count() > 2) {
            colorIndexes=(int *) malloc(thisCustomMap.count()*sizeof(int));

            // get the discrete colors
            for(int i=0; i< thisCustomMap.count(); i++) {
                bool ok;
                int index = thisCustomMap.at(i).toInt(&ok);
                if(ok) colorIndexes[i] = index; else colorIndexes[i] = 2; // black
                if(colorIndexes[i] < 2) colorIndexes[i] = 2;
                if(colorIndexes[i] > 18) colorIndexes[i] = 18;
            }

            // create colormap
            ColorMap_Custom * colormap =  new ColorMap_Custom();
            colormap->initColormap(colorIndexes, thisCustomMap.count(), thisDiscreteMap);
            d_spectrogram->setColorMap(colormap);
            free(colorIndexes);

        } else {
            d_spectrogram->setColorMap(new ColorMap_Wavelength());
        }
    }
        break;
    default:
        d_spectrogram->setColorMap(new ColorMap_Wavelength());
        break;
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
    plotGrid->setVisible(m ? true : false);
    plot->replot();
}


