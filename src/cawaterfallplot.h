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

#include <qtcontrols_global.h>

class SpectrogramData: public QwtMatrixRasterData
{
private:
    QVector<double> values;
    int NumberOfColumns;
    int NumberOfRows;

public:

    SpectrogramData() {
    }

    void initData(int numCols, int numRows)
    {
        NumberOfColumns = numCols;
        NumberOfRows = numRows;
        values.clear();
        values.reserve(NumberOfColumns * NumberOfRows);
        for ( int i = 0; i < NumberOfColumns * NumberOfRows; i++ ) values += 0.0;
        setValueMatrix(values, NumberOfColumns);
    }

    void setData(const QVector<double>&  Array, int &count)
    {
        // in case of a plot down to the bottom, start from the top and go to botton
        if(count <  NumberOfRows) {
            int start = NumberOfColumns * count;
            int stop = start + NumberOfColumns;
            for ( int i = start; i < stop; i++ ) {
                values[i] = Array[i-start];
            }
            count++;
            // otherwise shift plot and add to the end
        } else {
            // delete first row
            values.remove(0, NumberOfColumns);
            // resize to full size
            values.resize(NumberOfColumns * NumberOfRows);

            // copy array to the end of the array data
            int start = NumberOfColumns * NumberOfRows - NumberOfColumns;
            int stop = NumberOfColumns * NumberOfRows;
            for ( int i = start; i < stop; i++ ) {
                values[i] = Array[i-start];
            }
        }

        // update the matrix

        setValueMatrix(values, NumberOfColumns);
    }

    void setLimits(double xmin, double xmax, double ymin, double ymax, double zmin, double zmax) {

        setInterval( Qt::XAxis, QwtInterval( xmin, xmax ) );
        setInterval( Qt::YAxis, QwtInterval( ymin, ymax) );
        setInterval( Qt::ZAxis, QwtInterval( zmin, zmax) );
    }
};

class QTCON_EXPORT caWaterfallPlot: public QWidget
{
    Q_OBJECT

    Q_ENUMS(units)
    Q_ENUMS(intensityScaling)

    Q_PROPERTY(QString Title READ getTitlePlot WRITE setTitlePlot)
    Q_PROPERTY(QString TitleX READ getTitleX WRITE setTitleX)
    Q_PROPERTY(QString TitleY READ getTitleY WRITE setTitleY)
    Q_PROPERTY(QString channel READ getPV WRITE setPV)
    Q_PROPERTY(units units READ getUnits WRITE setUnits)
    Q_PROPERTY(double period READ getPeriod WRITE setPeriod)

    Q_PROPERTY(int NumberOfTraces READ getRows WRITE setRows)

    Q_PROPERTY(double IntensityMax  READ getIntensityMax WRITE setIntensityMax)
    Q_PROPERTY(double IntensityMin  READ getIntensityMin WRITE setIntensityMin)
    Q_PROPERTY(intensityScaling IntensityScalingMax READ getIntensityScalingMax WRITE setIntensityScalingMax)
    Q_PROPERTY(intensityScaling IntensityScalingMin READ getIntensityScalingMin WRITE setIntensityScalingMin)

    Q_PROPERTY(bool grid READ getGrid WRITE setGrid)
    Q_PROPERTY(bool XaxisEnabled READ getXaxisEnabled WRITE setXaxisEnabled)
    Q_PROPERTY(bool YaxisEnabled READ getYaxisEnabled WRITE setYaxisEnabled)


public:

    enum units {Monitor=0,  Millisecond, Second, Minute};
    enum intensityScaling {Channel, User};

    caWaterfallPlot(QWidget * = NULL);
    void GausCurv(double middle);

    bool getXaxisEnabled() const { return thisXshow; }
    void setXaxisEnabled(bool thisXshow);
    bool getYaxisEnabled() const { return thisYshow; }
    void setYaxisEnabled(bool thisYshow);

    double getIntensityMax() const {return thisIntensityMax;}
    void setIntensityMax(double const &newY) {thisIntensityMax = newY;}

    double getIntensityMin() const {return thisIntensityMin;}
    void setIntensityMin(double const &newY) {thisIntensityMin = newY;}

    void setIntensityScalingMax(intensityScaling s) {thisIntensityScalingMax = s;}
    intensityScaling getIntensityScalingMax() const {return thisIntensityScalingMax; }

    void setIntensityScalingMin(intensityScaling s) {thisIntensityScalingMin = s;}
    intensityScaling getIntensityScalingMin() const {return thisIntensityScalingMin;}

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

    void setIntensityLimits(double &zmin, double &zmax);
    void InitData(int nbCols);
    void SetData(const QVector<double>& vector);
    void updatePlot();
    void defineTimerUpdate(units unit, double period);

private:

    enum {nbRows = 200};
    enum {nbCols = 1000};

    QwtPlot *plot;
    QwtPlotSpectrogram *d_spectrogram;
    QwtLinearColorMap m_colorMap;
    QwtPlotGrid * plotGrid;
    QTimer *Timer;
    double position, drift;
    QVector<double> vector;
    SpectrogramData *m_data;

    QString thisTitle, thisTitleX, thisTitleY;
    units thisUnits;
    QString	 thisPV;
    double thisIntensityMax, thisIntensityMin;

    intensityScaling thisIntensityScalingMax, thisIntensityScalingMin;
    double thisPeriod;
    bool thisXshow, thisYshow, thisGrid;
    int thisRows, thisCols, countRows;
    bool disableDemo;

public  Q_SLOTS:
    void TimeOut();
};
