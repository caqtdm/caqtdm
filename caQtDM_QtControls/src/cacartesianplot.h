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

#ifndef CACARTESIANPLOT_H
#define CACARTESIANPLOT_H
#include "plotHelperClasses.h"
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_canvas.h>
#include <qwt_symbol.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_marker.h>
#include <qwt_scale_engine.h>
#include <qwt_legend.h>
#include <QMouseEvent>
#include <QVarLengthArray>
#include <qtcontrols_global.h>

#ifdef QWT_USE_OPENGL
#include <qevent.h>
#include <qgl.h>
#include <qwt_plot_glcanvas.h>
#endif

#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>

#if QWT_VERSION >= 0x060100
  #include <qwt_legend_label.h>
  #include <qwt_date_scale_engine.h>
  #include <qwt_date_scale_draw.h>
#else
  #include <qwt_legend_item.h>
#endif

#include <stdint.h>
#include <limits>
#include "caPropHandleDefs.h"

class QTCON_EXPORT caCartesianPlot : public QwtPlot
{
    Q_OBJECT

#if QWT_VERSION >= 0x060100
    // suppress these properties for the designer
    Q_PROPERTY( QBrush canvasBackground READ canvasBackground WRITE setCanvasBackground DESIGNABLE false)
    Q_PROPERTY( bool autoReplot READ autoReplot WRITE setAutoReplot DESIGNABLE false)
#endif

    Q_ENUMS(curvSymbol)
    Q_ENUMS(curvStyle)
    Q_ENUMS(axisScaling)
    Q_ENUMS(plMode)
    Q_ENUMS(eraMode)
    Q_ENUMS(axisType)

    Q_PROPERTY(QString Title READ getTitlePlot WRITE setTitlePlot)
    Q_PROPERTY(QString TitleX READ getTitleX WRITE setTitleX)
    Q_PROPERTY(QString TitleY READ getTitleY WRITE setTitleY)

    Q_PROPERTY(QStringList channelList_1 READ getPV1List WRITE setPV1List STORED false)
    Q_PROPERTY(QString channels_1 READ getPV_1 WRITE setPV_1 DESIGNABLE false)

    Q_PROPERTY(curvStyle Style_1 READ getStyle_1 WRITE setStyle_1)
    Q_PROPERTY(curvSymbol symbol_1 READ getSymbol_1 WRITE setSymbol_1)
    Q_PROPERTY(QColor color_1 READ getColor_1 WRITE setColor_1)

    Q_PROPERTY(QStringList channelList_2 READ getPV2List WRITE setPV2List STORED false)
    Q_PROPERTY(QString channels_2 READ getPV_2 WRITE setPV_2  DESIGNABLE false)

    Q_PROPERTY(curvStyle Style_2 READ getStyle_2 WRITE setStyle_2)
    Q_PROPERTY(curvSymbol symbol_2 READ getSymbol_2 WRITE setSymbol_2)
    Q_PROPERTY(QColor color_2 READ getColor_2 WRITE setColor_2)

    Q_PROPERTY(QStringList channelList_3 READ getPV3List WRITE setPV3List STORED false)
    Q_PROPERTY(QString channels_3 READ getPV_3 WRITE setPV_3 DESIGNABLE false)

    Q_PROPERTY(curvStyle Style_3 READ getStyle_3 WRITE setStyle_3)
    Q_PROPERTY(curvSymbol symbol_3 READ getSymbol_3 WRITE setSymbol_3)
    Q_PROPERTY(QColor color_3 READ getColor_3 WRITE setColor_3)

    Q_PROPERTY(QStringList channelList_4 READ getPV4List WRITE setPV4List STORED false)
    Q_PROPERTY(QString channels_4 READ getPV_4 WRITE setPV_4  DESIGNABLE false)

    Q_PROPERTY(curvStyle Style_4 READ getStyle_4 WRITE setStyle_4)
    Q_PROPERTY(curvSymbol symbol_4 READ getSymbol_4 WRITE setSymbol_4)
    Q_PROPERTY(QColor color_4 READ getColor_4 WRITE setColor_4)

    Q_PROPERTY(QStringList channelList_5 READ getPV5List WRITE setPV5List STORED false)
    Q_PROPERTY(QString channels_5 READ getPV_5 WRITE setPV_5  DESIGNABLE false)

    Q_PROPERTY(curvStyle Style_5 READ getStyle_5 WRITE setStyle_5)
    Q_PROPERTY(curvSymbol symbol_5 READ getSymbol_5 WRITE setSymbol_5)
    Q_PROPERTY(QColor color_5 READ getColor_5 WRITE setColor_5)

    Q_PROPERTY(QStringList channelList_6 READ getPV6List WRITE setPV6List STORED false)
    Q_PROPERTY(QString channels_6 READ getPV_6 WRITE setPV_6  DESIGNABLE false)

    Q_PROPERTY(curvStyle Style_6 READ getStyle_6 WRITE setStyle_6)
    Q_PROPERTY(curvSymbol symbol_6 READ getSymbol_6 WRITE setSymbol_6)
    Q_PROPERTY(QColor color_6 READ getColor_6 WRITE setColor_6)

    Q_PROPERTY(double symbolResizeFactor READ getSymbolsizeFactor WRITE setSymbolsizeFactor)

    Q_PROPERTY(plMode plotMode READ getPlotMode WRITE setPlotMode)

    Q_PROPERTY(QString countNumOrChannel READ getCountPV WRITE setCountPV)
    Q_PROPERTY(QString triggerChannel READ getTriggerPV WRITE setTriggerPV)
    Q_PROPERTY(QString eraseChannel READ getErasePV WRITE setErasePV)

    Q_PROPERTY(eraMode eraseMode READ getEraseMode WRITE setEraseMode)

    Q_PROPERTY(QColor foreground READ getForeground WRITE setForeground)
    Q_PROPERTY(QColor background READ getBackground WRITE setBackground)
    Q_PROPERTY(QColor scaleColor READ getScaleColor WRITE setScaleColor)

    Q_PROPERTY(bool grid READ getGrid WRITE setGrid)
    Q_PROPERTY(QColor gridColor READ getGridColor WRITE setGridColor)

    Q_PROPERTY(axisScaling XaxisScaling READ getXscaling WRITE setXscaling)
    Q_PROPERTY(axisScaling YaxisScaling READ getYscaling WRITE setYscaling)

    Q_PROPERTY(QString XaxisLimits  READ getXaxisLimits WRITE setXaxisLimits)
    Q_PROPERTY(QString YaxisLimits  READ getYaxisLimits WRITE setYaxisLimits)

    Q_PROPERTY(bool XaxisEnabled READ getXaxisEnabled WRITE setXaxisEnabled)
    Q_PROPERTY(bool YaxisEnabled READ getYaxisEnabled WRITE setYaxisEnabled)
    Q_PROPERTY(bool LegendEnabled READ getLegendEnabled WRITE setLegendEnabled)

    Q_PROPERTY(axisType XaxisType READ getXaxisType WRITE setXaxisType)
    Q_PROPERTY(int numberOfXticks READ getXticks WRITE setXticks)
    Q_PROPERTY(axisType YAxisType READ getYaxisType WRITE setYaxisType)

    Q_PROPERTY(int XaxisSyncGroup READ getXaxisSyncGroup WRITE setXaxisSyncGroup)

    // this will prevent user interference
    Q_PROPERTY(QString styleSheet READ styleSheet WRITE noStyle DESIGNABLE false)

public:
#include "caPropHandle.h"

    double getSymbolsizeFactor() {return symbolSizeFactor;}
    void setSymbolsizeFactor(double value) {symbolSizeFactor=value;}

    void noStyle(QString style) {Q_UNUSED(style);}

    bool getXaxisEnabled() const { return thisXshow; }
    void setXaxisEnabled(bool thisXshow);
    bool getYaxisEnabled() const { return thisYshow; }
    void setYaxisEnabled(bool thisYshow);
    bool getLegendEnabled() const { return thisLegendshow; }
    void setLegendEnabled(bool show) {thisLegendshow = show;}

    enum LegendAtttribute { COLOR, FONT, TEXT};

    enum {curveCount = 6};

    enum axisScaling { Auto = 0, Channel, User};

    enum curvSymbol {  NoSymbol = -1,
                       Ellipse,
                       Rect,
                       Diamond,
                       Triangle,
                       DTriangle,
                       UTriangle,
                       LTriangle,
                       RTriangle,
                       Cross,
                       XCross,
                       HLine,
                       VLine,
                       Star1,
                       Star2,
                       Hexagon};

    enum  curvStyle { NoCurve = 0,
      Lines,
      Sticks,
      Steps,
      Dots,
      FillUnder,
      ThinLines,
      HorSticks,
      FatDots
    };

    enum axisType { linear=0, log10, time};

    enum plMode {
        PlotNPointsAndStop = 0,
        PlotLastNPoints
    };

    enum eraMode {
        ifnotzero = 0,
        ifzero
    };

    enum  CartesianChannelType { CH_X=0, CH_Y, CH_Trigger, CH_Count, CH_Erase, CH_Xscale, CH_Yscale};

    enum curvType {XY_nothing=0, X_only, Y_only, XY_both};

    QString getXaxisLimits() const {return thisXaxisLimits.join(";");}
    void setXaxisLimits(QString const &newX);

    QString getYaxisLimits() const {return thisYaxisLimits.join(";");}
    void setYaxisLimits(QString const &newY);

    QString getTitlePlot() const {return thisTitle;}
    void setTitlePlot(QString const &title);

    QString getTitleX() const {return thisTitleX;}
    void setTitleX(QString const &title);

    QString getTitleY() const {return thisTitleY;}
    void setTitleY(QString const &title);

    QString getPV_1() const {return thisPV[0].join(";");}
    void setPV_1(QString const &newPV) {thisPV[0] = newPV.split(";");}
    QStringList getPV1List() const {return thisPV[0];}
    void setPV1List(QStringList list) {thisPV[0] = list; updatePropertyEditorItem(this, "channels_1");}

    QString getPV_2() const {return thisPV[1].join(";");}
    void setPV_2(QString const &newPV) {thisPV[1] = newPV.split(";");}
    QStringList getPV2List() const {return thisPV[1];}
    void setPV2List(QStringList list) {thisPV[1] = list; updatePropertyEditorItem(this, "channels_2");}

    QString getPV_3() const {return thisPV[2].join(";");}
    void setPV_3(QString const &newPV)  {thisPV[2] = newPV.split(";");}
    QStringList getPV3List() const {return thisPV[2];}
    void setPV3List(QStringList list) {thisPV[2] = list; updatePropertyEditorItem(this, "channels_3");}

    QString getPV_4() const {return thisPV[3].join(";");}
    void setPV_4(QString const &newPV)  {thisPV[3] = newPV.split(";");}
    QStringList getPV4List() const {return thisPV[3];}
    void setPV4List(QStringList list) {thisPV[3] = list; updatePropertyEditorItem(this, "channels_4");}

    QString getPV_5() const {return thisPV[4].join(";");}
    void setPV_5(QString const &newPV)  {thisPV[4] = newPV.split(";");}
    QStringList getPV5List() const {return thisPV[4];}
    void setPV5List(QStringList list) {thisPV[4] = list; updatePropertyEditorItem(this, "channels_5");}

    QString getPV_6() const {return thisPV[5].join(";");}
    void setPV_6(QString const &newPV)  {thisPV[5] = newPV.split(";");}
    QStringList getPV6List() const {return thisPV[5];}
    void setPV6List(QStringList list) {thisPV[5] = list; updatePropertyEditorItem(this, "channels_6");}

    QString getTriggerPV() const {return thisTriggerPV;}
    void setTriggerPV(QString const &newPV);

    QString getErasePV() const {return thisErasePV;}
    void setErasePV(QString const &newPV)  {thisErasePV = newPV;}

    QString getCountPV() const {return thisCountPV;}
    void setCountPV(QString const &newPV);

    bool hasCountNumber(int *Number);
    void setCountNumber(int number);

    plMode getPlotMode() const {return thisPlotMode;}
    void setPlotMode(plMode mode) {thisPlotMode = mode;}

    eraMode getEraseMode() const {return thisEraseMode;}
    void setEraseMode(eraMode mode) {thisEraseMode = mode;}

    curvSymbol getSymbol_1() const {return thisSymbol[0];}
    void setSymbol_1(curvSymbol s);

    curvSymbol getSymbol_2() const {return thisSymbol[1];}
    void setSymbol_2(curvSymbol s);

    curvSymbol getSymbol_3() const {return thisSymbol[2];}
    void setSymbol_3(curvSymbol s);

    curvSymbol getSymbol_4() const {return thisSymbol[3];}
    void setSymbol_4(curvSymbol s);

    curvSymbol getSymbol_5() const {return thisSymbol[4];}
    void setSymbol_5(curvSymbol s);

    curvSymbol getSymbol_6() const {return thisSymbol[5];}
    void setSymbol_6(curvSymbol s);

    curvStyle getStyle_1() const {return thisStyle[0];}
    void setStyle_1(curvStyle s);

    curvStyle getStyle_2() const {return thisStyle[1];}
    void setStyle_2(curvStyle s);

    curvStyle getStyle_3() const {return thisStyle[2];}
    void setStyle_3(curvStyle s);

    curvStyle getStyle_4() const {return thisStyle[3];}
    void setStyle_4(curvStyle s);

    curvStyle getStyle_5() const {return thisStyle[4];}
    void setStyle_5(curvStyle s);

    curvStyle getStyle_6() const {return thisStyle[5];}
    void setStyle_6(curvStyle s);

    QColor getColor_1() const {return thisLineColor[0];}
    void setColor_1(QColor c);

    QColor getColor_2() const {return thisLineColor[1];}
    void setColor_2(QColor c);

    QColor getColor_3() const {return thisLineColor[2];}
    void setColor_3(QColor c);

    QColor getColor_4() const {return thisLineColor[3];}
    void setColor_4(QColor c);

    QColor getColor_5() const {return thisLineColor[4];}
    void setColor_5(QColor c);

    QColor getColor_6() const {return thisLineColor[5];}
    void setColor_6(QColor c);

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

    axisScaling getXscaling() const {return thisXscaling;}
    void setXscaling(axisScaling s);
    axisScaling getYscaling() const {return thisYscaling;}
    void setYscaling(axisScaling s);

    axisType getXaxisType() const {return thisXtype;}
    void setXaxisType(axisType s);
    axisType getYaxisType() const {return thisYtype;}
    void setYaxisType(axisType s);

    void setXticks( int nb ) {if(nb < 1) thisXticks = 1; else thisXticks = nb; setXaxisType(thisXtype);}
    int getXticks() {return thisXticks;}

    void setXaxisSyncGroup( int group ) {thisXaxisSyncGroup = group;}
    int getXaxisSyncGroup() {return thisXaxisSyncGroup;}

    caCartesianPlot(QWidget *parent);
    ~caCartesianPlot();

    QwtSymbol::Style myMarker(curvSymbol m);
    QwtPlotCurve::CurveStyle myStyle(curvStyle s);

    void setData(double *vector, int size, int curvIndex, int curvType, int curvXY);
    void setData(float *vector, int size, int curvIndex, int curvType, int curvXY);
    void setData(int16_t *vector, int size, int curvIndex, int curvType, int curvXY);
    void setData(int32_t* vector, int size, int curvIndex, int curvType, int curvXY);
    void setData(int8_t* vector, int size, int curvIndex, int curvType, int curvXY);
    void displayData(int curvIndex, int curvType);

    void setScaleX(double minX, double maxX);
    void setScaleY(double minY, double maxY);
    int getYLimits(double &minY, double &maxY);
    int getXLimits(double &minX, double &maxX);
    void setScaleXlimits(double value, int maxormin);
    void setScaleYlimits(double value, int maxormin);

    void erasePlots();
    void setWhiteColors();
    void setAllProperties();
    void updateLegendsPV();

    void setSymbol(curvSymbol s, int indx);
    curvSymbol getSymbol(int indx) const {return thisSymbol[indx];}

    void setColor(QColor c, int indx);
    QColor getColor(int indx) const {return thisLineColor[indx];}

    void setStyle(curvStyle s, int indx);
    curvStyle getStyle(int indx) const {return thisStyle[indx];}

    QString getPV(int indx) const {return thisPV[indx].join(";");}
    void setPV(QString const &newPV, int indx)  {if(newPV.size() > 0) thisPV[indx] = newPV.split(";"); else thisPV[indx]=QStringList();}

    void setLegendAttribute(QColor c, QFont f, LegendAtttribute sw);

public slots:
    void animation(QRect p) {
#include "animationcode.h"
    }

    void hideObject(bool hideit) {
#include "hideobjectcode.h"
    }
    void setMinXResize(double value);
    void setMaxXResize(double value);
    void setMinYResize(double value);
    void setMaxYResize(double value);
    void triggerXAutoScale(){
       setXscaling(Auto);
    }
    void triggerYAutoScale(){
       setYscaling(Auto);
    }
    void resetZoom();
    void setZoom(const QRectF& newZoomRect);
    void zoomOnXAxis(const QRectF& newZoomRect);

private slots:
    void handleZoomedRect(const QRectF& zoomedRect);

signals:
    void ShowContextMenu(const QPoint&);
    void getAutoScaleXMin(double xmin);
    void getAutoScaleXMax(double xmax);
    void getAutoScaleYMin(double ymin);
    void getAutoScaleYMax(double ymax);
    void zoomedToRect(const QRectF& zoomedRect);
    void zoomHasReset();

protected:

    void resizeEvent ( QResizeEvent * event);

private:
    template <typename pureData>
    void fillData(pureData *array, int size, int curvIndex, int curvType, int curvXY);
    void AverageData(double *array, double *avg, int size, int ratio);

    QString thisTitle, thisTitleX, thisTitleY, thisTriggerPV, thisCountPV, thisErasePV;
    QStringList	 thisPV[curveCount], thisXaxisLimits, thisYaxisLimits;
    QColor thisForeColor;
    QColor thisBackColor;
    QColor thisScaleColor;
    QColor thisGridColor;
    QColor thisLineColor[curveCount];
    curvSymbol thisSymbol[curveCount];
    curvStyle thisStyle[curveCount];
    plMode thisPlotMode;
    eraMode thisEraseMode;
    int thisCountNumber;

    bool thisGrid;
    axisScaling thisXscaling, thisYscaling;
    axisType thisXtype, thisYtype;

    QwtPlotCurve curve[curveCount];

    QVarLengthArray<double> X[curveCount], XSAVE[curveCount];
    QVarLengthArray<double> Y[curveCount], YSAVE[curveCount];
    QVarLengthArray<double> XAUX[curveCount], YAUX[curveCount];

    QVarLengthArray<double> accumulX[curveCount];
    QVarLengthArray<double> accumulY[curveCount];

    QwtPlotGrid *plotGrid;
    QPen penGrid;

    bool thisXshow, thisYshow, thisToBeTriggered, thisLegendshow;

    void setBackgroundColor(QColor c);
    void setForegroundColor(QColor c);
    void setScalesColor(QColor c);
    void setGridsColor(QColor c);
    void setSamplesData(int index, double *x, double *y, int size, bool saveFlag);
    bool eventFilter(QObject *obj, QEvent *event);

    QwtPlotZoomer* zoomer;
    DynamicPlotPicker *plotPicker;

    bool thisTriggerNow;

    int thisXaxisSyncGroup;

    QwtLegend *lgd;

    int thisXticks;
    bool whiteColors;

    double symbolSizeFactor;
    double filter_MinY,filter_MaxY,filter_MinX,filter_MaxX;
    bool   ignorefirst_MinY,ignorefirst_MaxY,ignorefirst_MinX,ignorefirst_MaxX;
};

#endif
