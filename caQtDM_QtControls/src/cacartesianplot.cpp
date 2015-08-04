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
#define NOMINMAX
#include <windows.h>
#define QWT_DLL
#endif

#include "cacartesianplot.h"
#include <QtCore>

class MyZoomer: public QwtPlotZoomer
{
public:
    MyZoomer(QwtPlotCanvas *canvas):
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


caCartesianPlot::caCartesianPlot(QWidget *parent) : QwtPlot(parent)
{
    const char *text =
           "You can zoom in using the left mouse button.\n"
           "You can pan by dragging with the middle mouse button.\n"
           "Choose reset zoom in the context menu for original scale.\n ";

    thisToBeTriggered = false;
    thisTriggerNow = true;
    thisCountNumber = 0;

    plotGrid = new QwtPlotGrid();
    plotGrid->attach(this);

    setWhatsThis(text);

    setTitlePlot("");
    setTitleX("");
    setTitleY("");

    setBackground(Qt::black);
    setForeground(Qt::white);
    setScaleColor(Qt::black);
    setGrid(true);
    setGridColor(Qt::gray);

    setAutoFillBackground(true);

    enableAxis(xBottom, true);
    enableAxis(yLeft, true);

    // canvas

#if QWT_VERSION < 0x060100
    zoomer = new MyZoomer(canvas());
    QwtPlotPanner *panner = new QwtPlotPanner(canvas());
#else
    QwtPlotCanvas *canvas =  (QwtPlotCanvas *) this->canvas();
    zoomer = new MyZoomer(canvas);
    QwtPlotPanner *panner = new QwtPlotPanner(canvas);
#endif

    panner->setAxisEnabled(QwtPlot::yRight, false);
    panner->setAxisEnabled(QwtPlot::yLeft, true);
    panner->setAxisEnabled(QwtPlot::xBottom, true);
    panner->setMouseButton(Qt::MidButton);

    const QColor c(Qt::red);
   zoomer->setRubberBandPen(c);
   zoomer->setTrackerPen(c);
   zoomer->setMousePattern(QwtEventPattern::MouseSelect2,Qt:: NoButton);
   zoomer->setMousePattern(QwtEventPattern::MouseSelect3,Qt:: NoButton);
   zoomer->setMousePattern(QwtEventPattern::MouseSelect4,Qt:: NoButton);
   zoomer->setMousePattern(QwtEventPattern::MouseSelect5,Qt:: NoButton);
   zoomer->setMousePattern(QwtEventPattern::MouseSelect6,Qt:: NoButton);

    // curves
    for(int i=0; i < curveCount; i++) {
        curve[i].setLegendAttribute(QwtPlotCurve::LegendShowLine, true);
        curve[i].setStyle(QwtPlotCurve::Lines);
        curve[i].attach(this);
        curve[i].setOrientation(Qt::Vertical);
    }

    setStyle_1(Lines);
    setStyle_2(Lines);
    setStyle_3(Lines);
    setStyle_4(Lines);
    setStyle_5(Lines);
    setStyle_6(Lines);
    setSymbol_1(NoSymbol);
    setSymbol_2(NoSymbol);
    setSymbol_3(NoSymbol);
    setSymbol_4(NoSymbol);
    setSymbol_5(NoSymbol);
    setSymbol_6(NoSymbol);
    setColor_1(Qt::white);
    setColor_2(Qt::red);
    setColor_3(Qt::yellow);
    setColor_4(Qt::blue);
    setColor_5(Qt::green);
    setColor_6(Qt::magenta);

    setXaxisEnabled(true);
    setYaxisEnabled(true);
    setXscaling(Auto);
    setYscaling(Auto);
    setXaxisLimits("0;1");
    setYaxisLimits("0;1");
    setAxisFont(QwtPlot::xBottom, QFont("Arial", 9));
    setAxisFont(QwtPlot::yLeft, QFont("Arial", 9));

    // this allows to have a transparent widget
#if QWT_VERSION < 0x060100
    canvas()->setPaintAttribute(QwtPlotCanvas::BackingStore, false);
    canvas()->setPaintAttribute(QwtPlotCanvas::Opaque, false);
    canvas()->setAttribute( Qt::WA_OpaquePaintEvent, false );
    canvas()->setAutoFillBackground( false );   // use in ui file this parameter for transparency
#else
    canvas->setPaintAttribute(QwtPlotCanvas::BackingStore, false);
    canvas->setPaintAttribute(QwtPlotCanvas::Opaque, false);
    canvas->setAttribute( Qt::WA_OpaquePaintEvent, false );
    canvas->setAutoFillBackground( false );   // use in ui file this parameter for transparency
#endif

    installEventFilter(this);
}

void caCartesianPlot::resetZoom() {
    double minX, maxX, minY, maxY;

    if(getXLimits(minX, maxX)) setScaleX(minX, maxX);
    if(getYLimits(minY, maxY)) setScaleY(minY, maxY);
    if(thisYscaling == Auto) setAxisAutoScale(yLeft, true);
    if(thisXscaling == Auto) setAxisAutoScale(xBottom, true);
    replot();
}

void caCartesianPlot::setTriggerPV(QString const &newPV)  {
    thisTriggerPV = newPV;
    if(thisTriggerPV.trimmed().length() > 0) thisToBeTriggered = true;
    else thisToBeTriggered = false;
}

void caCartesianPlot::setCountPV(QString const &newPV)  {
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

bool caCartesianPlot::hasCountNumber(int *Number) {
    bool isNumber = false;
    if(thisCountPV.trimmed().length() > 0) {
        *Number = thisCountPV.toInt(&isNumber);
    }
    return isNumber;
}

void caCartesianPlot::setCountNumber(int number) {
    thisCountNumber = number;
}

void caCartesianPlot::setXaxisEnabled(bool show)
{
    thisXshow = show;
    enableAxis(xBottom, show);
}

void caCartesianPlot::setYaxisEnabled(bool show)
{
    thisYshow = show;
    enableAxis(yLeft, show);
}

void caCartesianPlot::erasePlots()
{
     for(int i=0; i < curveCount; i++) {
         X[i].clear();
         Y[i].clear();
         accumulX[i].clear();
         accumulY[i].clear();
         setSamplesData(i, X[i].data(), Y[i].data(), Y[i].size(), true);
     }
     replot();
}

void caCartesianPlot::AverageData(double *array, double *avg, int size, int ratio)
{
    int counter = 0;
    for (int i=0; i< size; i+=ratio) {
        double mean = 0;
        for(int j=0; j< ratio; j++) {
           mean += array[i+j];
        }
        avg[counter++] = mean / (double) ratio;
    }
}

void caCartesianPlot::setData(double *array, int size, int curvIndex, int curvType, int curvXY)
{
     fillData(array, size, curvIndex, curvType, curvXY);
}

void caCartesianPlot::setData(float *array, int size, int curvIndex, int curvType, int curvXY)
{
    fillData(array, size, curvIndex, curvType, curvXY);
}

void caCartesianPlot::setData(int16_t *array, int size, int curvIndex, int curvType, int curvXY)
{
    fillData(array, size, curvIndex, curvType, curvXY);
}

void caCartesianPlot::setData(int32_t *array, int size, int curvIndex, int curvType, int curvXY)
{
    fillData(array, size, curvIndex, curvType, curvXY);
}

template <typename pureData>
void caCartesianPlot::fillData(pureData *array, int size, int curvIndex, int curvType, int curvXY)
{
    if(curvXY == CH_X || curvXY == CH_Y) {         // x or y
        // keep data points
        if(curvXY == CH_X) {                       // X
            X[curvIndex].resize(size);
            X[curvIndex].clear();
            for(int i=0; i< size; i++) X[curvIndex].append(array[i]);
        } else {                                   // Y
            Y[curvIndex].resize(size);
            Y[curvIndex].clear();
            for(int i=0; i< size; i++) Y[curvIndex].append(array[i]);
        }

        // only x channel was specified, use index as y
        if(curvType == X_only) {
            Y[curvIndex].clear();
            for(int i=0; i< size; i++) Y[curvIndex].append(i);
            // only y channel was specified, use index as x
        } else if(curvType == Y_only) {
            X[curvIndex].clear();
            for(int i=0; i< size; i++) X[curvIndex].append(i);
        }

        // when triggering is specified, we will return here
        if(thisToBeTriggered) {
            thisTriggerNow = false;
            //printf("we will return and show plot when trigger comes\n");
         }
    } else if(curvXY == CH_Trigger) {
/*
        int i = X[curvIndex].size();
        int j = Y[curvIndex].size();    
        if(i != 0 && j != 0)
         printf("trigger came for curvIndex=%d %d %d %f %f\n", curvIndex,
                X[curvIndex].size(), Y[curvIndex].size(),
                X[curvIndex][i-1], Y[curvIndex][j-1]);
*/
         thisTriggerNow = true;
    }
}

void caCartesianPlot::displayData(int curvIndex, int curvType)
{
    if(thisToBeTriggered) {
        if(!thisTriggerNow) {
            //printf("display data return\n");
            return;
        } else {
            thisTriggerNow = false;
        }
    }
    // draw curve
    if(X[curvIndex].size() > 0 && Y[curvIndex].size() > 0) {

        // x vector, y scalar
        if(X[curvIndex].size() > 1 && Y[curvIndex].size() == 1) {
            //printf("x vector, y scalar\n");
            int nbPoints = X[curvIndex].size();
#if QT_VERSION < 0x040700
            double aux = Y[curvIndex][0];
#else
            double aux =  Y[curvIndex].at(0);
#endif
            Y[curvIndex].resize(nbPoints);      // increase to correct size
            double *data = Y[curvIndex].data();
            for(int i=0; i < X[curvIndex].size(); i++) data[i] = aux;
            if(thisCountNumber > 0) nbPoints = qMin(thisCountNumber, X[curvIndex].size());
            setSamplesData(curvIndex, X[curvIndex].data(), Y[curvIndex].data(), nbPoints, true);

        // x scalar, y vector
        } else if(X[curvIndex].size() == 1 && Y[curvIndex].size() > 1) {
            //printf("x scalar, y vector\n" );
            int nbPoints = Y[curvIndex].size(); 
#if QT_VERSION < 0x040700
            double aux = X[curvIndex][0];
#else
            double aux =  X[curvIndex].at(0);
#endif
            X[curvIndex].resize(nbPoints);      // increase to correct size
            double *data = X[curvIndex].data();
            for(int i=0; i < Y[curvIndex].size(); i++) data[i] = aux;  // and set values to first datapoint
            if(thisCountNumber > 0) nbPoints = qMin(thisCountNumber, Y[curvIndex].size());
            setSamplesData(curvIndex, X[curvIndex].data(), Y[curvIndex].data(), nbPoints, true);

        // x scalar, y scalar
        } else if(X[curvIndex].size() == 1 && Y[curvIndex].size() == 1) {
            //printf("x scalar, y scalar\n");
            // when no count is specified or count == 1 then yust plot the point
            if(thisCountNumber <= 1) {
               setSamplesData(curvIndex, X[curvIndex].data(), Y[curvIndex].data(), qMin(X[curvIndex].size(), Y[curvIndex].size()), true);

            // scalar scalar more than one point specified
            } else {
                double *dataX, *dataY;
                if(thisPlotMode == PlotNPointsAndStop) {
                    //printf("when count reached then we stop plotting\n");

                    if(accumulX[curvIndex].size() >= thisCountNumber) return;
                } else {
                    //printf("array size=%d wanted count=%d\n", accumulX[curvIndex].size(), thisCountNumber);

                    if(accumulX[curvIndex].size() > thisCountNumber) {
                        //printf("it seems that the count number changed and is less than before?\n");
                        accumulX[curvIndex].clear();
                        accumulY[curvIndex].clear();
                    }
                    //printf("accumulate until count, then shift down and add point\n");

                    if(accumulX[curvIndex].size() >= thisCountNumber) {
                        dataX = accumulX[curvIndex].data();
                        dataY = accumulY[curvIndex].data();
                        //shift and remove last point

                        // shift the X array and remove last point from X array
                        if(curvType == X_only) {
                            for(int i=0; i < accumulX[curvIndex].size() -1; i++)  {
                                dataX[i] = dataX[i+1];
                            }
                            accumulX[curvIndex].removeLast();

                        // shift the Y array and remove last point from Y array
                        } else if (curvType == Y_only) {
                            for(int i=0; i < accumulX[curvIndex].size() -1; i++)  {
                                dataY[i] = dataY[i+1];
                            }
                            accumulY[curvIndex].removeLast();

                        // shift the two arrays and remove last point from both
                        } else {
                            for(int i=0; i < accumulX[curvIndex].size() -1; i++)  {
                                dataX[i] = dataX[i+1];
                                dataY[i] = dataY[i+1];
                            }
                            accumulX[curvIndex].removeLast();
                            accumulY[curvIndex].removeLast();
                        }
                    }
                }
                dataX = X[curvIndex].data();
                dataY = Y[curvIndex].data();

                // add new point
                if(curvType == X_only) {
                    if(accumulX[curvIndex].size() < thisCountNumber)
                        accumulY[curvIndex].append(accumulY[curvIndex].size());
                    accumulX[curvIndex].append(dataX[0]);
                } else if (curvType == Y_only) {
                    if(accumulX[curvIndex].size() < thisCountNumber)
                        accumulX[curvIndex].append(accumulX[curvIndex].size());
                    accumulY[curvIndex].append(dataY[0]);
                } else {
                    accumulX[curvIndex].append(dataX[0]);
                    accumulY[curvIndex].append(dataY[0]);
                }
/*
                dataX = accumulX[curvIndex].data();
                dataY = accumulY[curvIndex].data();

                printf("array size=%d wanted count=%d\n", accumulX[curvIndex].size(), thisCountNumber);
                for(int i=0; i< accumulX[curvIndex].size(); i++) {
                    printf("%d %f %f\n", i, dataX[i], dataY[i]);
                }
*/
                setSamplesData(curvIndex, accumulX[curvIndex].data(), accumulY[curvIndex].data(), accumulY[curvIndex].size(), true);
            }

        // x vector, y vector
        } else {
            //printf("x vector, y vector curv=%d\n", curvIndex);
            int nbPoints = qMin(X[curvIndex].size(), Y[curvIndex].size());
            if(thisCountNumber > 0) nbPoints = qMin(thisCountNumber, nbPoints);
            setSamplesData(curvIndex, X[curvIndex].data(), Y[curvIndex].data(), nbPoints, true);
        }

        zoomer->setZoomBase();

        replot();
    }
}

// this routine will prevent that we have problems with negative values when logarithmic scale
// and will keep the values in order to switch between log and linear scale
void caCartesianPlot::setSamplesData(int index, double *x, double *y, int size, bool saveFlag)
{
    // saving the data allows to switch between log and lin when no new monitor is coming

    if(saveFlag) {
        XSAVE[index].resize(size);
        YSAVE[index].resize(size);
        memcpy(XSAVE[index].data(), x, size*sizeof(double));
        memcpy(YSAVE[index].data(), y, size*sizeof(double));
    }

    // use auxiliary arrays, in order not to overwrite the original data
    if((thisXtype == log10) || (thisYtype == log10)) {
        XAUX.resize(size);
        YAUX.resize(size);
        memcpy(XAUX.data(), x, size*sizeof(double));
        memcpy(YAUX.data(), y, size*sizeof(double));

        if(thisXtype == log10) {
            for(int i=0; i< size; i++) {
                if(x[i] < 1.e-20) XAUX[i] = 1.e-20;
            }
        }
        if(thisYtype == log10) {
            for(int i=0; i< size; i++) {
                if(y[i] < 1.e-20) YAUX[i] = 1.e-20;
            }
        }
        curve[index].setSamples(XAUX.data(), YAUX.data(), size);
    }
    else {
        curve[index].setSamples(x, y, size);
    }
}

void caCartesianPlot::setTitlePlot(QString const &titel)
{
    thisTitle=titel;
    if(titel.size() != 0) {
        QwtText title(titel);
        title.setFont(QFont("Arial", 10));
        setTitle(title);
        replot();
    }
}

void caCartesianPlot::setTitleX(QString const &titel)
{
    thisTitleX=titel;
    if(titel.size() != 0) {
        QwtText xAxis(titel);
        xAxis.setFont(QFont("Arial", 10));
        setAxisTitle(xBottom, xAxis);
    }
    replot();
}

void caCartesianPlot::setTitleY(QString const &titel)
{
    thisTitleY=titel;
    if(titel.size() != 0) {
        QwtText xAxis(titel);
        xAxis.setFont(QFont("Arial", 10));
        setAxisTitle(yLeft, xAxis);
    }
    replot();
}

void caCartesianPlot::setGrid(bool m){
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

QwtSymbol::Style caCartesianPlot::myMarker(curvSymbol m)
{
    QwtSymbol::Style ms;
    switch ( m ) {
    case Ellipse:
        ms = QwtSymbol::Ellipse;  break;
    case Rect:
        ms = QwtSymbol::Ellipse;  break;
    case Diamond:
        ms = QwtSymbol::Diamond;  break;
    case Triangle:
        ms = QwtSymbol::Triangle;  break;
    case DTriangle:
        ms = QwtSymbol::DTriangle;  break;
    case UTriangle:
        ms = QwtSymbol::UTriangle;  break;
    case LTriangle:
        ms = QwtSymbol::LTriangle;  break;
    case RTriangle:
        ms = QwtSymbol::RTriangle;  break;
    case Cross:
        ms = QwtSymbol::Cross;  break;
    case XCross:
        ms = QwtSymbol::XCross;  break;
    case HLine:
        ms = QwtSymbol::HLine;  break;
    case VLine:
        ms = QwtSymbol::VLine;  break;
    case Star1:
        ms = QwtSymbol::Star1;  break;
    case Star2:
        ms = QwtSymbol::Star2;  break;
    case Hexagon:
        ms = QwtSymbol::Hexagon;  break;
    default:
        ms = QwtSymbol::NoSymbol; break;
    }
    return ms;
}

QwtPlotCurve::CurveStyle caCartesianPlot::myStyle(curvStyle s)
{
    QwtPlotCurve::CurveStyle ms;
    switch ( s ) {
    case NoCurve:
        ms = QwtPlotCurve::NoCurve;  break;
    case Lines:
        ms = QwtPlotCurve::Lines;  break;
    case Sticks:
        ms = QwtPlotCurve::Sticks;  break;
    case Steps:
        ms = QwtPlotCurve::Steps;  break;
    case Dots:
        ms = QwtPlotCurve::Dots;  break;
    case FillUnder:
        ms = QwtPlotCurve::Lines;  break;
    default:
        ms = QwtPlotCurve::Lines; break;
    }
    return ms;
}

void caCartesianPlot::setGridsColor(QColor c)
{
    penGrid = QPen(c);
    penGrid.setStyle(Qt::DashLine);

    plotGrid->setPen(penGrid);
    plotGrid->setVisible(penGrid.style() != Qt::NoPen);
    replot();
}

void caCartesianPlot::setBackgroundColor(QColor c)
{
    QPalette canvasPalette(c);
    canvasPalette.setColor(QPalette::Foreground, QColor(133, 190, 232));
    canvas()->setPalette(canvasPalette);
    replot();
}

void caCartesianPlot::setForegroundColor(QColor c)
{
    setPalette(QPalette(c));
    replot();
}

void caCartesianPlot::setBackground(QColor c) { thisBackColor  = c; setBackgroundColor(c);}
void caCartesianPlot::setForeground(QColor c) { thisForeColor  = c; setForegroundColor(c);}
void caCartesianPlot::setScaleColor(QColor c) { thisScaleColor = c; setScalesColor(c);}
void caCartesianPlot::setGridColor(QColor c)  { thisGridColor  = c; setGridsColor(c);}

void caCartesianPlot::setScalesColor(QColor c)
{
    QwtScaleWidget *scaleX =axisWidget(QwtPlot::xBottom);
    QwtScaleWidget *scaleY =axisWidget(QwtPlot::yLeft);
    QPalette palette = scaleX->palette();
    palette.setColor( QPalette::WindowText, c); // for ticks
    palette.setColor( QPalette::Text, c);       // for ticks' labels
    scaleX->setPalette( palette);
    scaleY->setPalette (palette);
    titleLabel()->setPalette(palette);
    replot();
}

void caCartesianPlot::setColor(QColor c, int indx)
{
    QBrush brush;
    if(thisStyle[indx] == FillUnder) {
        brush.setColor(c);
        brush.setStyle(Qt::SolidPattern);
    } else {
        brush.setStyle(Qt::NoBrush);
    }
    curve[indx].setBrush(brush);

    if(thisStyle[indx] != FillUnder) {
        if(thisStyle[indx] == ThinLines) {
            curve[indx].setPen(QPen(c, 1));
        } else {
            curve[indx].setPen(QPen(c, 2));
        }
    } else {
        curve[indx].setPen(QPen(thisScaleColor, 2));  // normally black
    }
    replot();
}

void caCartesianPlot::setColor_1(QColor c) { thisLineColor[0] = c; setColor(c, 0);}
void caCartesianPlot::setColor_2(QColor c) { thisLineColor[1] = c; setColor(c, 1);}
void caCartesianPlot::setColor_3(QColor c) { thisLineColor[2] = c; setColor(c, 2);}
void caCartesianPlot::setColor_4(QColor c) { thisLineColor[3] = c; setColor(c, 3);}
void caCartesianPlot::setColor_5(QColor c) { thisLineColor[4] = c; setColor(c, 4);}
void caCartesianPlot::setColor_6(QColor c) { thisLineColor[5] = c; setColor(c, 5);}

void caCartesianPlot::setStyle(curvStyle s, int indx)
{
    QwtPlotCurve::CurveStyle ms;
    if(s == ThinLines) {
        ms = myStyle(Lines);
    } else {
        ms = myStyle(s);
    }
    if(s == HorSticks) {
        ms= myStyle(Sticks);
        curve[indx].setOrientation(Qt::Horizontal);
    } else {
        curve[indx].setOrientation(Qt::Vertical);
    }

    curve[indx].setStyle(ms);
    replot();
}

void caCartesianPlot::setStyle_1(curvStyle s) { thisStyle[0] = s; setStyle(s, 0);}
void caCartesianPlot::setStyle_2(curvStyle s) { thisStyle[1] = s; setStyle(s, 1);}
void caCartesianPlot::setStyle_3(curvStyle s) { thisStyle[2] = s; setStyle(s, 2);}
void caCartesianPlot::setStyle_4(curvStyle s) { thisStyle[3] = s; setStyle(s, 3);}
void caCartesianPlot::setStyle_5(curvStyle s) { thisStyle[4] = s; setStyle(s, 4);}
void caCartesianPlot::setStyle_6(curvStyle s) { thisStyle[5] = s; setStyle(s, 5);}

// resize symbols when resizing
void caCartesianPlot::resizeEvent ( QResizeEvent * event )
{
    QwtPlot::resizeEvent(event);
    for(int i=0; i<6; i++) {
        setSymbol(thisSymbol[i], i);
    }
}

void caCartesianPlot::setSymbol(curvSymbol s, int indx)
{
    int size;
    QBrush brush;
    QwtSymbol::Style ms = myMarker(s);
    brush.setColor(thisLineColor[indx]);
    brush.setStyle(Qt::SolidPattern);
    size=qMax(5, (int) (this->geometry().height()/70.0));
    curve[indx].setSymbol(new QwtSymbol(ms, brush, QPen(thisLineColor[indx]), QSize(size, size)));
    replot();
}

void caCartesianPlot::setSymbol_1(curvSymbol s) { thisSymbol[0] = s; setSymbol(s, 0);}
void caCartesianPlot::setSymbol_2(curvSymbol s) { thisSymbol[1] = s; setSymbol(s, 1);}
void caCartesianPlot::setSymbol_3(curvSymbol s) { thisSymbol[2] = s; setSymbol(s, 2);}
void caCartesianPlot::setSymbol_4(curvSymbol s) { thisSymbol[3] = s; setSymbol(s, 3);}
void caCartesianPlot::setSymbol_5(curvSymbol s) { thisSymbol[4] = s; setSymbol(s, 4);}
void caCartesianPlot::setSymbol_6(curvSymbol s) { thisSymbol[5] = s; setSymbol(s, 5);}

void caCartesianPlot::setXscaling(axisScaling s)
{
    thisXscaling = s;
    if(s == Auto) setAxisAutoScale(xBottom, true);
    replot();
}

void caCartesianPlot::setYscaling(axisScaling s)
{
    thisYscaling = s;
    if(s == Auto) setAxisAutoScale(yLeft, true);
    replot();
}

void caCartesianPlot::setXaxisLimits(QString const &newX)
{
    bool ok1,ok2;
    QStringList list = newX.split(";", QString::SkipEmptyParts);

    if(list.count() == 2) {
        double minX = list.at(0).toDouble(&ok1);
        double maxX = list.at(1).toDouble(&ok2);
        if(ok1 && ok2) {
            if(thisXscaling != Auto) setScaleX(minX, maxX);
            if(thisXscaling == Auto) setAxisAutoScale(0, true);
        }
    }
    thisXaxisLimits = newX.split(";");
    replot();
    return;
}

void caCartesianPlot::setYaxisLimits(QString const &newY)
{
    bool ok1,ok2;
    QStringList list = newY.split(";", QString::SkipEmptyParts);

    if(list.count() == 2) {
        double minY = list.at(0).toDouble(&ok1);
        double maxY = list.at(1).toDouble(&ok2);
        if(ok1 && ok2) {
            if(thisYscaling != Auto) setScaleY(minY, maxY);
            if(thisYscaling == Auto) setAxisAutoScale(1, true);
        }
    }
    thisYaxisLimits = newY.split(";");
    replot();
    return;
}

int caCartesianPlot::getXLimits(double &minX, double &maxX)
{
    bool ok1,ok2;
    minX = maxX = 0.0;

    if(thisXaxisLimits.count() == 2) {
        minX = thisXaxisLimits.at(0).toDouble(&ok1);
        maxX = thisXaxisLimits.at(1).toDouble(&ok2);
        if(ok1 && ok2) {
            return true;
        }
    }
    return false;
}

int caCartesianPlot::getYLimits(double &minY, double &maxY)
{
    bool ok1,ok2;
    minY = maxY = 0.0;

    if(thisYaxisLimits.count() == 2) {
        minY = thisYaxisLimits.at(0).toDouble(&ok1);
        maxY = thisYaxisLimits.at(1).toDouble(&ok2);
        if(ok1 && ok2) {
            return true;
        }
    }
    return false;
}

void caCartesianPlot::setScaleXlimits(double value, int maxormin)
{
 #if QWT_VERSION < 0x060100
   double minX = axisScaleDiv(xBottom)->lowerBound();
   double maxX = axisScaleDiv(xBottom)->upperBound();
#else
   double minX = axisScaleDiv(xBottom).lowerBound();
   double maxX = axisScaleDiv(xBottom).upperBound();
#endif
   if(maxormin == 0) {
     setAxisScale(xBottom, value, maxX);
   } else {
     setAxisScale(xBottom, minX, value);
   }
   replot();
}

void caCartesianPlot::setScaleYlimits(double value, int maxormin)
{
 #if QWT_VERSION < 0x060100
   double minX = axisScaleDiv(yLeft)->lowerBound();
   double maxX = axisScaleDiv(yLeft)->upperBound();
#else
    double minX = axisScaleDiv(yLeft).lowerBound();
    double maxX = axisScaleDiv(yLeft).upperBound();
#endif
   if(maxormin == 0) {
     setAxisScale(yLeft, value, maxX);
   } else {
     setAxisScale(yLeft, minX, value);
   }
   replot();
}

void caCartesianPlot::setScaleX(double minX, double maxX)
{
    if(minX == maxX) {
        setAxisScale(xBottom, 0.0, 10.0);
    } else {
        setAxisScale(xBottom, minX, maxX);
    }
    replot();
}

void caCartesianPlot::setScaleY(double minY, double maxY)
{
    if(minY == maxY) {
        setAxisScale(yLeft, 0.0, 10.0);
    } else {
        setAxisScale(yLeft, minY, maxY);
    }
    replot();
}


void caCartesianPlot::setXaxisType(axisType s)
{
    thisXtype = s;
    if(s == log10) {
#if QWT_VERSION < 0x060100
        setAxisScaleEngine(QwtPlot::xBottom, new QwtLog10ScaleEngine);
#else
        setAxisScaleEngine(QwtPlot::xBottom, new QwtLogScaleEngine);
#endif
    } else {
        setAxisScaleEngine(QwtPlot::xBottom, new QwtLinearScaleEngine);
    }

    setXaxisLimits(getXaxisLimits());

    for(int i=0; i < curveCount; i++) {
        if(XSAVE[i].size() > 0) setSamplesData(i, XSAVE[i].data(), YSAVE[i].data(), XSAVE[i].size(), false);
    }
    replot();
}

void caCartesianPlot::setYaxisType(axisType s)
{
    thisYtype = s;
    if(s == log10) {
#if QWT_VERSION < 0x060100
        setAxisScaleEngine(QwtPlot::yLeft, new QwtLog10ScaleEngine);
#else
        setAxisScaleEngine(QwtPlot::yLeft, new QwtLogScaleEngine);
#endif
    } else {
        setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine);
    }

    setYaxisLimits(getYaxisLimits());

    for(int i=0; i < curveCount; i++) {
        if(XSAVE[i].size() > 0) setSamplesData(i, XSAVE[i].data(), YSAVE[i].data(), XSAVE[i].size(), false);
    }

    replot();
}


void caCartesianPlot::setWhiteColors()
{
    QColor c = QColor(Qt::white);
    for(int i=0; i<curveCount; i++) {
        setColor(c, i);
    }
    setBackgroundColor(c);
    setForegroundColor(c);
    setScalesColor(c);
    setGridsColor(c);
}

void caCartesianPlot::setAllProperties()
{
    for(int i=0; i<curveCount; i++) {
        setColor(thisLineColor[i], i);
    }
    setBackgroundColor(thisBackColor);
    setForegroundColor(thisForeColor);
    setScalesColor(thisScaleColor);
    setGridsColor(thisGridColor);

    // this will set the correct count again (after a disconnect!)
     setCountPV(thisCountPV);
}

bool caCartesianPlot::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        int nButton = ((QMouseEvent*) event)->button();
        if(nButton==2) {
            QPoint p;
            emit ShowContextMenu(p);
        }
    }
    return QObject::eventFilter(obj, event);
}

#include "moc_cacartesianplot.cpp"

