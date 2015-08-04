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

#include <QTimer>
#include <QtGui>
#include <QApplication>
#include <QHBoxLayout>
#include <QMutex>
#include <qwt_plot.h>
#include <qwt_plot_canvas.h>
#include <qwt_color_map.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_draw.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_renderer.h>
#include <qwt_plot_grid.h>
#include <qwt_matrix_raster_data.h>
#include <qwt_point_3d.h>
#include <qwt_plot_spectrocurve.h>
#include <sys/timeb.h>
#include <stdint.h>
//#include <sys/timeb.h>

#include <qtcontrols_global.h>

#include "colormaps.h"

#define MAXCOLUMNS 500

class SpectrogramData: public QwtMatrixRasterData
{
private:
    QVector<double> values;
    QVector<double> valuesAveraged;

    int NumberOfColumns;
    int NumberOfRows;
    int ActualNumberOfColumns;
    int ratio;

public:

    SpectrogramData() {
    }

    template <typename pureData>
    void AverageVector(pureData *vec, int size, QVector<double> &avg, int arraySize)
    {
        avg.clear();
        for (int i=0; i< size-ratio; i+=ratio) {
            double mean = 0;
            for(int j=0; j< ratio; j++) {
                if((i+j) >= arraySize) {
                    break;
                } else {
                   mean += vec[i+j];
                }
            }
            avg += mean / (double) ratio;
        }
    }

    int getRatio(int NumberOfColumns, int &MaximumColumns) {
        ratio = 1;
        if(NumberOfColumns > MAXCOLUMNS) {
           ratio = (int) ((double) NumberOfColumns / (double) MAXCOLUMNS);
           if(ratio < 1) ratio = 1;
           MaximumColumns = NumberOfColumns/ratio;
        }
        return ratio;
    }

    int initData(int numCols, int numRows)
    {
        ActualNumberOfColumns = NumberOfColumns = numCols;
        NumberOfRows = numRows;

        ratio = getRatio(NumberOfColumns, ActualNumberOfColumns);

        // too many columns, calculate size of reduced data vector
        if(ratio != 1) {
            valuesAveraged.resize(ActualNumberOfColumns);
        }
        values.clear();
        valuesAveraged.clear();

        return ActualNumberOfColumns;
    }

    template <typename pureData> int setData(pureData* Array, int &count, int numCols, int numRows, int arraySize)
    {
        //struct timeb now, last;
        //ftime(&last);
        ActualNumberOfColumns = NumberOfColumns = numCols;
        NumberOfRows = numRows;

        ratio = getRatio(NumberOfColumns, ActualNumberOfColumns);

        // too many columns, calculate size of reduced data vector
        if(ratio != 1) {
            valuesAveraged.resize(ActualNumberOfColumns);
        }
        values.resize(ActualNumberOfColumns * NumberOfRows);

        // calculate reduced data vector
        if(ratio != 1) {
            AverageVector(Array, NumberOfColumns, valuesAveraged, arraySize);
        }

        // in case of a plot down to the bottom, start from the top and go to bottom
        if(count <  NumberOfRows) {
            int start = ActualNumberOfColumns * count;
            int stop = start + ActualNumberOfColumns;
            if(ratio != 1) {
                for ( int i = start; i < stop; i++ ) {
                    values[i] = valuesAveraged[i-start];
                }
            } else {
                for ( int i = start; i < stop; i++ ) {
                    if((i-start) >= arraySize) {
                        break;
                    } else {
                       values[i] = Array[i-start];
                    }
                }
            }
            count++;
            // otherwise shift plot and add to the end
        } else {
            // delete first row
            values.remove(0, ActualNumberOfColumns);
            // resize to full size
            values.resize(ActualNumberOfColumns * NumberOfRows);

            // copy array to the end of the array data
            int start = ActualNumberOfColumns * NumberOfRows - ActualNumberOfColumns;
            int stop = ActualNumberOfColumns * NumberOfRows;
            if(ratio != 1) {
                for ( int i = start; i < stop; i++ ) {
                    values[i] = valuesAveraged[i-start];
                }
            } else {
                for ( int i = start; i < stop; i++ ) {
                    if((i-start) >= arraySize) {
                        break;
                    } else {
                       values[i] = Array[i-start];
                    }
                }
            }
        }

        //ftime(&now);
        //double diff = ((double) now.time + (double) now.millitm / (double)1000) -
        //        ((double) last.time + (double) last.millitm / (double)1000);
        //printf("%f\n", diff);

        // update the matrix
        setValueMatrix(values, ActualNumberOfColumns);

        return ActualNumberOfColumns;
    }

    void setLimits(double xmin, double xmax, double ymin, double ymax, double zmin, double zmax)
    {
        setInterval( Qt::XAxis, QwtInterval( xmin, xmax ) );
        setInterval( Qt::YAxis, QwtInterval( ymin, ymax) );
        setInterval( Qt::ZAxis, QwtInterval( zmin, zmax+(zmax-zmin)*5.0/1000.0) );
    }

};


class QTCON_EXPORT caWaterfallPlot: public QWidget
{
    Q_OBJECT

    Q_ENUMS(units)
    Q_ENUMS(intensityScaling)
    Q_ENUMS(colormap)

    Q_PROPERTY(QString Title READ getTitlePlot WRITE setTitlePlot)
    Q_PROPERTY(QString TitleX READ getTitleX WRITE setTitleX)
    Q_PROPERTY(QString TitleY READ getTitleY WRITE setTitleY)

    Q_PROPERTY(QString channel READ getPV WRITE setPV)
    Q_PROPERTY(QString countNumOrChannel READ getCountPV WRITE setCountPV)

    Q_PROPERTY(units units READ getUnits WRITE setUnits)
    Q_PROPERTY(double period READ getPeriod WRITE setPeriod)

    Q_PROPERTY(int NumberOfTraces READ getRows WRITE setRows)

    Q_PROPERTY(double IntensityMax  READ getIntensityMax WRITE setIntensityMax)
    Q_PROPERTY(double IntensityMin  READ getIntensityMin WRITE setIntensityMin)
    Q_PROPERTY(intensityScaling IntensityScalingMax READ getIntensityScalingMax WRITE setIntensityScalingMax)
    Q_PROPERTY(intensityScaling IntensityScalingMin READ getIntensityScalingMin WRITE setIntensityScalingMin)

    Q_PROPERTY(colormap ColorMap READ getColormap WRITE setColormap)
    Q_PROPERTY(QString customColorMap READ getCustomMap WRITE setCustomMap  DESIGNABLE isPropertyVisible(customcolormap))
    Q_PROPERTY(bool discreteCustomColorMap READ getDiscreteCustomMap WRITE setDiscreteCustomMap DESIGNABLE isPropertyVisible(discretecolormap))

    Q_PROPERTY(bool grid READ getGrid WRITE setGrid)
    Q_PROPERTY(bool XaxisEnabled READ getXaxisEnabled WRITE setXaxisEnabled)
    Q_PROPERTY(bool YaxisEnabled READ getYaxisEnabled WRITE setYaxisEnabled)


public:

    enum units {Monitor=0,  Millisecond, Second, Minute};
    enum intensityScaling {Channel, User};

    enum colormap {grey=0, spectrum_wavelength, spectrum_hot, spectrum_heat, spectrum_jet, spectrum_custom};
    enum Properties { customcolormap = 0, discretecolormap};

    caWaterfallPlot(QWidget * = NULL);
    void GausCurv(double middle);

    bool getXaxisEnabled() const { return thisXshow; }
    void setXaxisEnabled(bool thisXshow);
    bool getYaxisEnabled() const { return thisYshow; }
    void setYaxisEnabled(bool thisYshow);

    double getIntensityMax() const {return thisIntensityMax;}
    void setIntensityMax(double const &newY) {thisIntensityMax = newY; firstDemoPlot=true; updatePlot();}

    double getIntensityMin() const {return thisIntensityMin;}
    void setIntensityMin(double const &newY) {thisIntensityMin = newY;  firstDemoPlot=true; updatePlot();}

    void setIntensityScalingMax(intensityScaling s) {thisIntensityScalingMax = s;}
    intensityScaling getIntensityScalingMax() const {return thisIntensityScalingMax; }

    void setIntensityScalingMin(intensityScaling s) {thisIntensityScalingMin = s;}
    intensityScaling getIntensityScalingMin() const {return thisIntensityScalingMin;}

    colormap getColormap() const {return thisColormap;}
    void setColormap(colormap const &map);

    QString getCustomMap() const {return thisCustomMap.join(";");}
    void setCustomMap(QString const &newmap) {thisCustomMap = newmap.split(";"); setColormap(thisColormap);}

    bool getDiscreteCustomMap() const {return thisDiscreteMap;}
    void setDiscreteCustomMap(bool discrete) {thisDiscreteMap = discrete; setColormap(thisColormap);}

    bool isPropertyVisible(Properties property);
    void setPropertyVisible(Properties property, bool visible);

    QString getTitlePlot() const {return thisTitle;}
    void setTitlePlot(QString const &title);

    QString getTitleX() const {return thisTitleX;}
    void setTitleX(QString const &title);

    QString getTitleY() const {return thisTitleY;}
    void setTitleY(QString const &title);

    QString getPV() const {return thisPV;}
    void setPV(QString const &newPV) {thisPV = newPV;}

    units getUnits() const {return thisUnits;}
    void setUnits(units const &newU) {thisUnits = newU; defineTimerUpdate(thisUnits, thisPeriod);}

    double getPeriod() const { return thisPeriod; }
    void setPeriod(double const &newP) {thisPeriod = newP; defineTimerUpdate(thisUnits, thisPeriod);}

    bool getGrid() const {return thisGrid;}
    void setGrid(bool m);

    int getRows() const {return thisRows;}
    void setRows(int const &rows);

    int getCols() const {return thisCols;}
    void setCols(int const &cols);

    QString getCountPV() const {return thisCountPV;}
    void setCountPV(QString const &newPV);
    bool hasCountNumber(int *Number);
    void setCountNumber(int number);

    void setCountReceived(bool received) {thisCountReceived = received;}
    bool getCountReceived() const {return thisCountReceived;}

    void InitData(int nbCols);

    void myReplot();

    void setData(double *array, int size);
    void setData(float *array, int size);
    void setData(int16_t *array, int size);
    void setData(int32_t *array, int size);
    void displayData();

    void updatePlot();
    void defineTimerUpdate(units unit, double period);


private:

    enum {nbRows = 200};
    enum {nbCols = 500};

    enum { ColormapSize = 256 };
    uint ColorMap[ColormapSize];

    int ActualNumberOfColumns;

    template <typename pureData>void AverageArray(pureData *vec, int size, int ArraySize, double *avg, int ratio);
    template <typename pureData> void CompressAndkeepArray(pureData *vec, int size, int arraySize);

    QMutex *datamutex;

    QwtPlot *plot;
    QwtPlotSpectrogram *d_spectrogram;
    QwtPlotGrid * plotGrid;
    QTimer *Timer;
    double position, drift;

    double *reducedArray;

    SpectrogramData *m_data;

    QString thisTitle, thisTitleX, thisTitleY;
    units thisUnits;
    QString	 thisPV, thisCountPV;
    double thisIntensityMax, thisIntensityMin;

    intensityScaling thisIntensityScalingMax, thisIntensityScalingMin;
    double thisPeriod;
    bool thisXshow, thisYshow, thisGrid;
    int thisRows, thisCols, countRows;
    bool disableDemo;
    int NumberOfColumns;

    bool firstMonitorPlot, firstDemoPlot, firstTimerPlot;

    int thisCountNumber;
    bool thisCountReceived;

    bool thisDiscreteMap;
    colormap thisColormap;
    QStringList thisCustomMap;

    bool designerVisible[10];

public  Q_SLOTS:
    void TimeOut();
};
