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
 *  Authors:
 *    Anton Mezger, Erik Schwarz
 *  Contact details:
 *    erik.schwarz@psi.ch
 */

#ifndef caStripPlot_H
#define caStripPlot_H

#include "plotHelperClasses.h"
#ifndef MOBILE_ANDROID
  #include <sys/timeb.h>
#else
  #include "androidtimeb.h"
#endif

#include <QTime>
#include <QTimer>
#include <QThread>
#include <QMutex>
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
#include <qwt_scale_engine.h>
#if QWT_VERSION >= 0x060100
  #include <qwt_legend_label.h>
#else
  #include <qwt_legend_item.h>
#endif
#include <qwt_plot_canvas.h>
#include <qwt_plot_marker.h>
#include <QMouseEvent>
#include <qtcontrols_global.h>
#include <qnumeric.h>
#include <qwt_date_scale_draw.h>
#include <qwt_date_scale_engine.h>

#include <stripplotthread.h>

class QwtPlotCurve;

#include "qwtplotcurvenan.h"

#ifdef QWT_USE_OPENGL
#include <qevent.h>
#include <qgl.h>
#include <qwt_plot_glcanvas.h>
#endif

#include "caPropHandleDefs.h"

class QTCON_EXPORT caStripPlot : public QwtPlot
{
    Q_OBJECT

#if QWT_VERSION >= 0x060100
    // suppress these properties for the designer
    Q_PROPERTY( QBrush canvasBackground READ canvasBackground WRITE setCanvasBackground DESIGNABLE false)
    Q_PROPERTY( bool autoReplot READ autoReplot WRITE setAutoReplot DESIGNABLE false)
#endif

    Q_ENUMS(curvStyle)
    Q_ENUMS(axisScaling)
    Q_ENUMS(units)
    Q_ENUMS(xAxisType)
    Q_ENUMS(yAxisType)
    Q_ENUMS(yAxisScaling)
    Q_ENUMS(cpuUsage)
    Q_ENUMS(PlotPicker)

    Q_PROPERTY(QString Title READ getTitlePlot WRITE setTitlePlot)
    Q_PROPERTY(QString TitleX READ getTitleX WRITE setTitleX)
    Q_PROPERTY(QString TitleY READ getTitleY WRITE setTitleY)

    // for compatibility either one can be used, both will be updated
    Q_PROPERTY(QStringList channelsList READ getPVSList WRITE setPVSList STORED false)
    Q_PROPERTY(QString channels READ getPVS WRITE setPVS DESIGNABLE false)

    Q_PROPERTY(units units READ getUnits WRITE setUnits)
    Q_PROPERTY(double period READ getPeriod WRITE setPeriod)
    Q_PROPERTY(cpuUsage refreshRate READ getUsageCPU WRITE setUsageCPU)
    Q_PROPERTY(xAxisType XaxisType READ getXaxisType WRITE setXaxisType)
    Q_PROPERTY( int numberOfXticks READ getXticks WRITE setXticks)
    Q_PROPERTY(yAxisType YAxisType READ getYaxisType WRITE setYaxisType)
    Q_PROPERTY(yAxisScaling YAxisScaling READ getYaxisScaling WRITE setYaxisScaling)
    Q_PROPERTY(PlotPicker plotpicker READ getPlotPicker WRITE setPlotPicker)
    Q_PROPERTY(bool CurvesIterableInLegend READ getIterableCurves WRITE setIterableCurves)
    Q_PROPERTY(bool CurvesSelectableInPlot READ getSelectableCurves WRITE setSelectableCurves)

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

    Q_PROPERTY(int XaxisSyncGroup READ getXaxisSyncGroup WRITE setXaxisSyncGroup)

    // this will prevent user interference
    Q_PROPERTY(QString styleSheet READ styleSheet WRITE noStyle DESIGNABLE false)

public:
#include "caPropHandle.h"

    void noStyle(QString style) {Q_UNUSED(style);}

    enum {MAXCURVES = 7};

    enum cpuUsage {Low, Medium, High};

    enum axisScaling {Channel, User};
    enum curvStyle {Lines = 1, FillUnder = 5};
    enum units {Millisecond = 0, Second, Minute};
    enum xAxisType {ValueScale, TimeScale, TimeScaleFix};
    enum yAxisType {linear=0, log10};
    enum yAxisScaling {fixedScale=0, autoScale=1, selectiveAutoScale=2};
    enum PlotPicker {off = 0, on = 1};

    enum LegendAtttribute { COLOR, FONT, TEXT};

    void setXaxisSyncGroup( int group ) {thisXaxisSyncGroup = group;}
    int getXaxisSyncGroup() {return thisXaxisSyncGroup;}

    void setXticks( int nb ) {thisXticks = nb; defineXaxis(thisUnits, thisPeriod);}
    int getXticks() {return thisXticks;}


    void setPlotPicker(PlotPicker p) {thisPlotPicker = p;}
    PlotPicker getPlotPicker() {return thisPlotPicker;}

    bool getIterableCurves() const {return thisIterableCurves;}
    // setter defined as public slot

    bool getSelectableCurves() const {return thisSelectableCurves;}
    // setter defined as public slot

    caStripPlot(QWidget * = 0);
    ~caStripPlot();

    void defineCurves(QStringList titles, units unit, double period, int width, int nb);
    void setData(struct timeb now, double Y, int curvIndex);

    bool getXaxisEnabled() const { return thisXshow; }
    void setXaxisEnabled(bool thisXshow);
    bool getYaxisEnabled() const { return thisYshow; }
    void setYaxisEnabled(bool thisYshow);
    bool getLegendEnabled() const { return thisLegendshow; }
    void setLegendEnabled(bool thisLegendshow);

    bool getAutoscaleMinYOverride() const {return autoscaleMinYOverride;}
    void setAutoscaleMinYOverride(bool const newAutoscaleMinYOverride) {autoscaleMinYOverride = newAutoscaleMinYOverride;}

    double getAutoscaleMinY() const {return manualAutoscaleMinY;}
    void setAutoscaleMinY(double const newAutoscaleMinY) {manualAutoscaleMinY = newAutoscaleMinY;}

    QString getTitlePlot() const {return thisTitle;}
    void setTitlePlot(QString const &title);

    QString getTitleX() const {return thisTitleX;}
    void setTitleX(QString const &title);

    QString getTitleY() const {return thisTitleY;}
    void setTitleY(QString const &title);

    QString getPVS() const {return thisPVS.join(";");}
    void setPVS(QString const &newPV) {thisPVS = newPV.split(";");}
    QStringList getPVSList() const {return thisPVS;}
    void setPVSList(QStringList list) {thisPVS = list; updatePropertyEditorItem(this, "channels");}

    units getUnits() const {return thisUnits;}
    void setUnits(units const &newU) {thisUnits = newU; defineXaxis(thisUnits, thisPeriod);}

    double getPeriod() const { return thisPeriod; }
    void setPeriod(double const &newP) {thisPeriod = newP; defineXaxis(thisUnits, thisPeriod);}

    xAxisType getXaxisType() const {return thisXaxisType;}
    void setXaxisType(xAxisType s) {thisXaxisType=s; defineXaxis(thisUnits, thisPeriod);}

    yAxisType getYaxisType() const {return thisYaxisType;}
    void setYaxisType(yAxisType s);

    yAxisScaling getYaxisScaling() const {return thisYaxisScaling;}
    void setYaxisScaling(yAxisScaling s) {thisYaxisScaling = s;}

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

    cpuUsage getUsageCPU() const {return thisUsageCPU;}
    void setUsageCPU(cpuUsage usage) {thisUsageCPU = usage; }

    void setYaxisLimitsMax(int i, double const &newY) {if(i>= MAXCURVES) return; else thisYaxisLimitsMax[i] = newY;}
    void setYaxisLimitsMin(int i, double const &newY) {if(i>= MAXCURVES) return; else thisYaxisLimitsMin[i] = newY;}

    void setYscalingMin(int i, axisScaling s) {if(i>= MAXCURVES) return; else setYscalingMin(s, i); }
    void setYscalingMax(int i, axisScaling s) {if(i>= MAXCURVES) return; else setYscalingMax(s, i); }

    bool getSeleticeAutoScaleCurves(int i) {if(i>= MAXCURVES) return false; else return sAutoScaleCurves[i];}
    void setSelectiveAutoScaleCurves(int i, bool enable = true) {if(i>= MAXCURVES) return; else sAutoScaleCurves[i] = enable;}

    void showCurve(int number, bool on);
    void addText(double x, double y, char* text, QColor c, int fontsize);
    void startPlot();
    void UpdateScaling();

    void setLegendAttribute(QColor c, QFont f, LegendAtttribute sw);

    void setTicksResizeFactor(float factX, float factY);

public slots:
    void animation(QRect p) {
#include "animationcode.h"
    }

    void hideObject(bool hideit) {
#include "hideobjectcode.h"
    }

    void stopPlot();
    void resumePlot();
    void restartPlot();
    void pausePlot(bool pausePlot);

    void selectFixedYAxis(int newYAxisIndex);

    void setPlotPickerMode(int mode);

    void setIterableCurves(bool itCurvs) {thisIterableCurves = itCurvs;};
    void setSelectableCurves(bool selectCurvs) {thisSelectableCurves = selectCurvs;};

protected:
    void resizeEvent ( QResizeEvent * event);

signals:
    void ShowContextMenu(const QPoint&);
    void update();
    void timerThreadStop();

private slots:
     void TimeOut();
     void TimeOutThread();
     void onSelected(const QPointF& point);

private:
    int HISTORY;
    int timerID;
    double INTERVAL;
    int NumberOfCurves;
    QTimer *Timer;
    struct timeb  timeNow;
    struct timeb  timeStart;
    struct timeb plotStart;
    bool RestartPlot1, RestartPlot2;
    bool plotIsPaused;

    bool eventFilter(QObject *obj, QEvent *event);
    void setXaxis(double interval, double period);
    void defineXaxis(units unit, double period);
    void RescaleCurves(int width, units unit, double period);
    void RescaleAxis();
    void TimersStart();
    void selectYAxis(quint8 newYAxisIndex);
    void remapCurve(double newMin, double newMax, quint8 curvIndex, bool isNewLog);

    // curve only used to define nicely the legend
    QwtPlotCurve *curve[MAXCURVES];
    // curve for plotting line/point from minimum to maximum value
    QwtPlotIntervalCurveNaN *errorcurve[MAXCURVES];
    QwtPlotCurveNaN *fillcurve[MAXCURVES];

    // y data for error curve
    QVector<QwtIntervalSample> base;
    QVector<QwtIntervalSample> rangeData[MAXCURVES];
    QVector<QPointF> fillData[MAXCURVES];

    // original, raw y data for conversions
    QVector<QwtIntervalSample> rangeDataRaw[MAXCURVES];
    QVector<QPointF> fillDataRaw[MAXCURVES];

    DynamicPlotPicker * plotPicker;

    double timeData;
    int dataCount;

    cpuUsage  thisUsageCPU;

    bool thisXshow, thisYshow, thisLegendshow, thisGrid;
    bool thisIterableCurves, thisSelectableCurves;
    xAxisType thisXaxisType;
    yAxisType thisYaxisType;
    yAxisScaling thisYaxisScaling;
    PlotPicker thisPlotPicker;

    QString thisTitle, thisTitleX, thisTitleY;
    units thisUnits;
    QStringList	 thisPVS;
    double thisYaxisLimitsMax[MAXCURVES], thisYaxisLimitsMin[MAXCURVES];
    curvStyle thisStyle[MAXCURVES];
    QColor thisForeColor;
    QColor thisBackColor;
    QColor thisScaleColor;
    QColor thisLineColor[MAXCURVES], thisGridColor;
    double AutoscaleMaxY;
    double AutoscaleMinY;
    double manualAutoscaleMinY;
    bool sAutoScaleCurves[MAXCURVES];
    bool autoscaleMinYOverride;

    axisScaling thisYscalingMax[MAXCURVES], thisYscalingMin[MAXCURVES];

    double thisPeriod;

    QwtPlotGrid *plotGrid;
    QPen penGrid;

    QwtScaleWidget *scaleWidget;

    int onInit;
    double timeInterval;

    void ReplaceTrailingZerosByBlancs(char *asc);

    double maxVal[MAXCURVES], minVal[MAXCURVES], actVal[MAXCURVES];
    double realVal[MAXCURVES], realMax[MAXCURVES], realMin[MAXCURVES];
    struct timeb realTim[MAXCURVES];

    QStringList originalTitles;
    QStringList savedTitles;
    QString legendText(int i);

    stripplotthread *timerThread;

    QMutex mutex;

    bool initCurves;

    int thisXaxisSyncGroup;
    int thisXticks;
    float ResizeFactorX, ResizeFactorY;
    float oldResizeFactorX, oldResizeFactorY;

    quint8 YAxisIndex;
    float xAxisToleranceFactor;
};
#endif
