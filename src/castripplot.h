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

#ifndef caStripPlot_H
#define caStripPlot_H

#include <sys/timeb.h>
#include <QTime>
#include <QTimer>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_intervalcurve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_canvas.h>
#include <qwt_interval_symbol.h>
#include <qwt_symbol.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_curve.h>
#include <qwt_scale_draw.h>
#include <qwt_legend.h>
#if QWT_VERSION >= 0x060100
  #include <qwt_legend_label.h>
#else
  #include <qwt_legend_item.h>
#endif
#include <qwt_plot_canvas.h>
#include <qwt_plot_marker.h>
#include <QMouseEvent>
#include <qtcontrols_global.h>

class QwtPlotCurve;

class QTCON_EXPORT caStripPlot : public QwtPlot
{

    Q_OBJECT

#if QWT_VERSION >= 0x060100
    // suppress theese properties for the designer
    Q_PROPERTY( QBrush canvasBackground READ canvasBackground WRITE setCanvasBackground DESIGNABLE false)
    Q_PROPERTY( bool autoReplot READ autoReplot WRITE setAutoReplot DESIGNABLE false)
#endif

    Q_ENUMS(curvStyle)
    Q_ENUMS(axisScaling)
    Q_ENUMS(units)
    Q_ENUMS(xAxisType)

    Q_PROPERTY(QString Title READ getTitlePlot WRITE setTitlePlot)
    Q_PROPERTY(QString TitleX READ getTitleX WRITE setTitleX)
    Q_PROPERTY(QString TitleY READ getTitleY WRITE setTitleY)
    Q_PROPERTY(QString channels READ getPVS WRITE setPVS)
    Q_PROPERTY(units units READ getUnits WRITE setUnits)
    Q_PROPERTY(double period READ getPeriod WRITE setPeriod)
    Q_PROPERTY(xAxisType XaxisType READ getXaxisType WRITE setXaxisType)

    // would have been nice to define all this with a define statement, however moc does not support that

    //============================== Curve 1 ===========================================
    Q_PROPERTY(double YaxisLimitsMax_1  READ getYaxisLimitsMax_1 WRITE setYaxisLimitsMax_1)
    Q_PROPERTY(double YaxisLimitsMin_1  READ getYaxisLimitsMin_1 WRITE setYaxisLimitsMin_1)
    Q_PROPERTY(axisScaling YaxisScalingMax_1 READ getYscalingMax_1 WRITE setYscalingMax_1)
    Q_PROPERTY(axisScaling YaxisScalingMin_1 READ getYscalingMin_1 WRITE setYscalingMin_1)
    Q_PROPERTY(curvStyle Style_1 READ getStyle_1 WRITE setStyle_1)
    Q_PROPERTY(QColor color_1 READ getColor_1 WRITE setColor_1)

    //============================== Curve 2 ===========================================
    Q_PROPERTY(double YaxisLimitsMax_2  READ getYaxisLimitsMax_2 WRITE setYaxisLimitsMax_2)
    Q_PROPERTY(double YaxisLimitsMin_2  READ getYaxisLimitsMin_2 WRITE setYaxisLimitsMin_2)
    Q_PROPERTY(axisScaling YaxisScalingMax_2 READ getYscalingMax_2 WRITE setYscalingMax_2)
    Q_PROPERTY(axisScaling YaxisScalingMin_2 READ getYscalingMin_2 WRITE setYscalingMin_2)
    Q_PROPERTY(curvStyle Style_2 READ getStyle_2 WRITE setStyle_2)
    Q_PROPERTY(QColor color_2 READ getColor_2 WRITE setColor_2)

    //============================== Curve 3 ===========================================
    Q_PROPERTY(double YaxisLimitsMax_3  READ getYaxisLimitsMax_3 WRITE setYaxisLimitsMax_3)
    Q_PROPERTY(double YaxisLimitsMin_3  READ getYaxisLimitsMin_3 WRITE setYaxisLimitsMin_3)
    Q_PROPERTY(axisScaling YaxisScalingMax_3 READ getYscalingMax_3 WRITE setYscalingMax_3)
    Q_PROPERTY(axisScaling YaxisScalingMin_3 READ getYscalingMin_3 WRITE setYscalingMin_3)
    Q_PROPERTY(curvStyle Style_3 READ getStyle_3 WRITE setStyle_3)
    Q_PROPERTY(QColor color_3 READ getColor_3 WRITE setColor_3)

    //============================== Curve 4 ===========================================
    Q_PROPERTY(double YaxisLimitsMax_4  READ getYaxisLimitsMax_4 WRITE setYaxisLimitsMax_4)
    Q_PROPERTY(double YaxisLimitsMin_4  READ getYaxisLimitsMin_4 WRITE setYaxisLimitsMin_4)
    Q_PROPERTY(axisScaling YaxisScalingMax_4 READ getYscalingMax_4 WRITE setYscalingMax_4)
    Q_PROPERTY(axisScaling YaxisScalingMin_4 READ getYscalingMin_4 WRITE setYscalingMin_4)
    Q_PROPERTY(curvStyle Style_4 READ getStyle_4 WRITE setStyle_4)
    Q_PROPERTY(QColor color_4 READ getColor_4 WRITE setColor_4)

    //============================== Curve 5 ===========================================
    Q_PROPERTY(double YaxisLimitsMax_5  READ getYaxisLimitsMax_5 WRITE setYaxisLimitsMax_5)
    Q_PROPERTY(double YaxisLimitsMin_5  READ getYaxisLimitsMin_5 WRITE setYaxisLimitsMin_5)
    Q_PROPERTY(axisScaling YaxisScalingMax_5 READ getYscalingMax_5 WRITE setYscalingMax_5)
    Q_PROPERTY(axisScaling YaxisScalingMin_5 READ getYscalingMin_5 WRITE setYscalingMin_5)
    Q_PROPERTY(curvStyle Style_5 READ getStyle_5 WRITE setStyle_5)
    Q_PROPERTY(QColor color_5 READ getColor_5 WRITE setColor_5)

    //============================== Curve 6 ===========================================
    Q_PROPERTY(double YaxisLimitsMax_6  READ getYaxisLimitsMax_6 WRITE setYaxisLimitsMax_6)
    Q_PROPERTY(double YaxisLimitsMin_6  READ getYaxisLimitsMin_6 WRITE setYaxisLimitsMin_6)
    Q_PROPERTY(axisScaling YaxisScalingMax_6 READ getYscalingMax_6 WRITE setYscalingMax_6)
    Q_PROPERTY(axisScaling YaxisScalingMin_6 READ getYscalingMin_6 WRITE setYscalingMin_6)
    Q_PROPERTY(curvStyle Style_6 READ getStyle_6 WRITE setStyle_6)
    Q_PROPERTY(QColor color_6 READ getColor_6 WRITE setColor_6)

    //============================== Curve 7 ===========================================
    Q_PROPERTY(double YaxisLimitsMax_7  READ getYaxisLimitsMax_7 WRITE setYaxisLimitsMax_7)
    Q_PROPERTY(double YaxisLimitsMin_7  READ getYaxisLimitsMin_7 WRITE setYaxisLimitsMin_7)
    Q_PROPERTY(axisScaling YaxisScalingMax_7 READ getYscalingMax_7 WRITE setYscalingMax_7)
    Q_PROPERTY(axisScaling YaxisScalingMin_7 READ getYscalingMin_7 WRITE setYscalingMin_7)
    Q_PROPERTY(curvStyle Style_7 READ getStyle_7 WRITE setStyle_7)
    Q_PROPERTY(QColor color_7 READ getColor_7 WRITE setColor_7)


    //============================== other properties ===========================================
    Q_PROPERTY(QColor foreground READ getForeground WRITE setForeground)
    Q_PROPERTY(QColor background READ getBackground WRITE setBackground)
    Q_PROPERTY(QColor scaleColor READ getScaleColor WRITE setScaleColor)

    Q_PROPERTY(bool grid READ getGrid WRITE setGrid)
    Q_PROPERTY(QColor gridColor READ getGridColor WRITE setGridColor)

    Q_PROPERTY(bool XaxisEnabled READ getXaxisEnabled WRITE setXaxisEnabled)
    Q_PROPERTY(bool YaxisEnabled READ getYaxisEnabled WRITE setYaxisEnabled)
    Q_PROPERTY(bool LegendEnabled READ getLegendEnabled WRITE setLegendEnabled)

public:

    enum {MAXCURVES = 7};

    enum axisScaling {Channel, User};
    enum  curvStyle {Lines = 1, FillUnder = 5};
    enum units { Millisecond = 0, Second, Minute};
    enum xAxisType {TimeScale, ValueScale};

    enum LegendAtttribute { COLOR, FONT, TEXT};

    caStripPlot(QWidget * = 0);
    ~caStripPlot();

    void defineCurves(QStringList titres, units unit, double period, int width, int nb);
    void setData(double Y, int curvIndex);

    bool getXaxisEnabled() const { return thisXshow; }
    void setXaxisEnabled(bool thisXshow);
    bool getYaxisEnabled() const { return thisYshow; }
    void setYaxisEnabled(bool thisYshow);
    bool getLegendEnabled() const { return thisLegendshow; }
    void setLegendEnabled(bool thisLegendshow);

    QString getTitlePlot() const {return thisTitle;}
    void setTitlePlot(QString const &title);

    QString getTitleX() const {return thisTitleX;}
    void setTitleX(QString const &title);

    QString getTitleY() const {return thisTitleY;}
    void setTitleY(QString const &title);

    QString getPVS() const {return thisPVS.join(";");}
    void setPVS(QString const &newPV) {thisPVS = newPV.split(";");}

    units getUnits() const {return thisUnits;}
    void setUnits(units const &newU) {thisUnits = newU; defineAxis(thisUnits, thisPeriod);}

    double getPeriod() const { return thisPeriod; }
    void setPeriod(double const &newP) {thisPeriod = newP; defineAxis(thisUnits, thisPeriod);}

    xAxisType getXaxisType() const {return thisXaxisType;}
    void setXaxisType(xAxisType s) {thisXaxisType=s; defineAxis(thisUnits, thisPeriod);}

    void setColor(QColor c, int number);
    QColor getColor(int number) const {return thisLineColor[number];}

    void setStyle(curvStyle s, int number);
    curvStyle getStyle(int number) const {return thisStyle[number];}

    void setYscalingMax(axisScaling s, int number) {thisYscalingMax[number] = s;}
    axisScaling getYscalingMax(int number) const {
      if(number < MAXCURVES) return thisYscalingMax[number]; else return thisYscalingMax[0];
    }

    void setYscalingMin(axisScaling s, int number) {thisYscalingMin[number] = s;}
    axisScaling getYscalingMin(int number) const {
       if(number < MAXCURVES) return thisYscalingMin[number]; else return thisYscalingMin[0];
    }

    double getYaxisLimitsMin(int number) const {return thisYaxisLimitsMin[number];}
    double getYaxisLimitsMax(int number) const {return thisYaxisLimitsMax[number];}

    void setYscale(double ymin, double ymax);

    QColor getForeground() const {return thisForeColor;}
    void setForeground(QColor c);

    QColor getBackground() const {return thisBackColor;}
    void setBackground(QColor c);

    QColor getScaleColor() const {return thisScaleColor;}
    void setScaleColor(QColor c);

    bool getGrid() const {return thisGrid;}
    void setGrid(bool m);

    QColor getGridColor() const {return thisGridColor;}
    void setGridColor(QColor c);

    QwtPlotCurve::CurveStyle myStyle(curvStyle s);

    //============================== Curve 1 ===========================================
    double getYaxisLimitsMax_1() const {return thisYaxisLimitsMax[0];}
    void setYaxisLimitsMax_1(double const &newY) {thisYaxisLimitsMax[0] = newY;}

    double getYaxisLimitsMin_1() const {return thisYaxisLimitsMin[0];}
    void setYaxisLimitsMin_1(double const &newY) {thisYaxisLimitsMin[0] = newY;}

    axisScaling getYscalingMax_1() const {return thisYscalingMax[0];}
    void setYscalingMax_1(axisScaling s) {setYscalingMax(s, 0); }

    axisScaling getYscalingMin_1() const {return thisYscalingMin[0];}
    void setYscalingMin_1(axisScaling s) {setYscalingMin(s, 0); }

    curvStyle getStyle_1() const {return thisStyle[0];}
    void setStyle_1(curvStyle s) {setStyle(s, 0); }

    QColor getColor_1() const {return thisLineColor[0];}
    void setColor_1(QColor c) {setColor(c, 0); }

   //============================== Curve 2 ===========================================
    double getYaxisLimitsMax_2() const {return thisYaxisLimitsMax[1];}
    void setYaxisLimitsMax_2(double const &newY) {thisYaxisLimitsMax[1] = newY;}

    double getYaxisLimitsMin_2() const {return thisYaxisLimitsMin[1];}
    void setYaxisLimitsMin_2(double const &newY) {thisYaxisLimitsMin[1] = newY;}

    axisScaling getYscalingMax_2() const {return thisYscalingMax[1];}
    void setYscalingMax_2(axisScaling s) {setYscalingMax(s, 1); }

    axisScaling getYscalingMin_2() const {return thisYscalingMin[1];}
    void setYscalingMin_2(axisScaling s) {setYscalingMin(s, 1); }


    curvStyle getStyle_2() const {return thisStyle[1];}
    void setStyle_2(curvStyle s) {setStyle(s, 1); }

    QColor getColor_2() const {return thisLineColor[1];}
    void setColor_2(QColor c) {setColor(c, 1); }

    //============================== Curve 3 ===========================================
    double getYaxisLimitsMax_3() const {return thisYaxisLimitsMax[2];}
    void setYaxisLimitsMax_3(double const &newY) {thisYaxisLimitsMax[2] = newY;}

    double getYaxisLimitsMin_3() const {return thisYaxisLimitsMin[2];}
    void setYaxisLimitsMin_3(double const &newY) {thisYaxisLimitsMin[2] = newY;}


    axisScaling getYscalingMax_3() const {return thisYscalingMax[2];}
    void setYscalingMax_3(axisScaling s) {setYscalingMax(s, 2); }

    axisScaling getYscalingMin_3() const {return thisYscalingMin[2];}
    void setYscalingMin_3(axisScaling s) {setYscalingMin(s, 2); }

    curvStyle getStyle_3() const {return thisStyle[2];}
    void setStyle_3(curvStyle s) {setStyle(s, 2); }

    QColor getColor_3() const {return thisLineColor[2];}
    void setColor_3(QColor c) {setColor(c, 2); }

    //============================== Curve 4 ===========================================
    double getYaxisLimitsMax_4() const {return thisYaxisLimitsMax[3];}
    void setYaxisLimitsMax_4(double const &newY) {thisYaxisLimitsMax[3] = newY;}

    double getYaxisLimitsMin_4() const {return thisYaxisLimitsMin[3];}
    void setYaxisLimitsMin_4(double const &newY) {thisYaxisLimitsMin[3] = newY;}


    axisScaling getYscalingMax_4() const {return thisYscalingMax[3];}
    void setYscalingMax_4(axisScaling s) {setYscalingMax(s, 3); }

    axisScaling getYscalingMin_4() const {return thisYscalingMin[3];}
    void setYscalingMin_4(axisScaling s) {setYscalingMin(s, 3); }

    curvStyle getStyle_4() const {return thisStyle[3];}
    void setStyle_4(curvStyle s) {setStyle(s, 3); }

    QColor getColor_4() const {return thisLineColor[3];}
    void setColor_4(QColor c) {setColor(c, 3); }

    //============================== Curve 5 ===========================================
    double getYaxisLimitsMax_5() const {return thisYaxisLimitsMax[4];}
    void setYaxisLimitsMax_5(double const &newY) {thisYaxisLimitsMax[4] = newY;}

    double getYaxisLimitsMin_5() const {return thisYaxisLimitsMin[4];}
    void setYaxisLimitsMin_5(double const &newY) {thisYaxisLimitsMin[4] = newY;}

    axisScaling getYscalingMax_5() const {return thisYscalingMax[4];}
    void setYscalingMax_5(axisScaling s) {setYscalingMax(s, 4); }

    axisScaling getYscalingMin_5() const {return thisYscalingMin[4];}
    void setYscalingMin_5(axisScaling s) {setYscalingMin(s, 4); }

    curvStyle getStyle_5() const {return thisStyle[4];}
    void setStyle_5(curvStyle s) {setStyle(s, 4); }

    QColor getColor_5() const {return thisLineColor[4];}
    void setColor_5(QColor c) {setColor(c, 4); }

    //============================== Curve 6 ===========================================
    double getYaxisLimitsMax_6() const {return thisYaxisLimitsMax[5];}
    void setYaxisLimitsMax_6(double const &newY) {thisYaxisLimitsMax[5] = newY;}

    double getYaxisLimitsMin_6() const {return thisYaxisLimitsMin[5];}
    void setYaxisLimitsMin_6(double const &newY) {thisYaxisLimitsMin[5] = newY;}

    axisScaling getYscalingMax_6() const {return thisYscalingMax[5];}
    void setYscalingMax_6(axisScaling s) {setYscalingMax(s, 5); }

    axisScaling getYscalingMin_6() const {return thisYscalingMin[5];}
    void setYscalingMin_6(axisScaling s) {setYscalingMin(s, 5); }

    curvStyle getStyle_6() const {return thisStyle[5];}
    void setStyle_6(curvStyle s) {setStyle(s, 5); }

    QColor getColor_6() const {return thisLineColor[5];}
    void setColor_6(QColor c) {setColor(c, 5); }

    //============================== Curve 6 ===========================================
    double getYaxisLimitsMax_7() const {return thisYaxisLimitsMax[6];}
    void setYaxisLimitsMax_7(double const &newY) {thisYaxisLimitsMax[6] = newY;}

    double getYaxisLimitsMin_7() const {return thisYaxisLimitsMin[6];}
    void setYaxisLimitsMin_7(double const &newY) {thisYaxisLimitsMin[6] = newY;}

    axisScaling getYscalingMax_7() const {return thisYscalingMax[6];}
    void setYscalingMax_7(axisScaling s) {setYscalingMax(s, 6); }

    axisScaling getYscalingMin_7() const {return thisYscalingMin[6];}
    void setYscalingMin_7(axisScaling s) {setYscalingMin(s, 6); }

    curvStyle getStyle_7() const {return thisStyle[6];}
    void setStyle_7(curvStyle s) {setStyle(s, 6); }

    QColor getColor_7() const {return thisLineColor[6];}
    void setColor_7(QColor c) {setColor(c, 6); }
   //===================================================================================

    void setYaxisLimitsMax(int i, double const &newY) {if(i>= MAXCURVES) return; else thisYaxisLimitsMax[i] = newY;}
    void setYaxisLimitsMin(int i, double const &newY) {if(i>= MAXCURVES) return; else thisYaxisLimitsMin[i] = newY;}

    void setYscalingMin(int i, axisScaling s) {if(i>= MAXCURVES) return; else setYscalingMin(s, i); }
    void setYscalingMax(int i, axisScaling s) {if(i>= MAXCURVES) return; else setYscalingMax(s, i); }

    void showCurve(int number, bool on);
    void addText(double x, double y, char* text, QColor c, int fontsize);
    void startPlot();

    void setLegendAttribute(QColor c, QFont f, LegendAtttribute sw);

protected:
    void resizeEvent ( QResizeEvent * event);

signals:
    void ShowContextMenu(const QPoint&);

private slots:
     void TimeOut();

private:

    int HISTORY;
    int timerID;
    double INTERVAL;
    int updateRate;
    int timerCount;
    int NumberOfCurves;
    double Period;
    units Unit;
    QTimer *Timer;
    struct timeb  timeNow;
    struct timeb  timeStart;
    bool Start;

    bool eventFilter(QObject *obj, QEvent *event);
    void setAxis(double interval, double period);
    void defineAxis(units unit, double period);
    void RescaleCurves(int width, units unit, double period);
    void RescaleAxis();

    // curve only used to define nicely the legend
    QwtPlotCurve *curve[MAXCURVES];
    // curve for plotting line/point from minimum to maximum value
    QwtPlotIntervalCurve *errorcurve[MAXCURVES];
    QwtPlotIntervalCurve *fillcurve[MAXCURVES];

    // y data for error curve
    QList<QwtIntervalSample> base;
    QList<QwtIntervalSample> rangeData[MAXCURVES];
    QList<QwtIntervalSample> fillData[MAXCURVES];

    // x time axis data
    //double *timeData;
    double timeData;
    int dataCount;

    bool thisXshow, thisYshow, thisLegendshow, thisGrid;
    xAxisType thisXaxisType;

    QString thisTitle, thisTitleX, thisTitleY;
    units thisUnits;
    QStringList	 thisPVS;
    double thisYaxisLimitsMax[MAXCURVES], thisYaxisLimitsMin[MAXCURVES];
    curvStyle thisStyle[MAXCURVES];
    QColor thisForeColor;
    QColor thisBackColor;
    QColor thisScaleColor;
    QColor thisLineColor[MAXCURVES], thisGridColor;

    axisScaling thisXscaling, thisYscalingMax[MAXCURVES], thisYscalingMin[MAXCURVES];

    double thisPeriod;
    double AxisLowX;
    double AxisUpX;
    double AxisLowY;
    double AxisUp;

    QwtPlotGrid *plotGrid;
    QPen penGrid;

    QwtScaleWidget *scaleWidget;

    int onInit;
    double timeInterval;

    void ReplaceTrailingZerosByBlancs(char *asc);

    double maxVal[MAXCURVES], minVal[MAXCURVES], actVal[MAXCURVES];
    double realVal[MAXCURVES], realMax[MAXCURVES], realMin[MAXCURVES];

    QStringList savedTitres;
    QString legendText(int i);

};
#endif
