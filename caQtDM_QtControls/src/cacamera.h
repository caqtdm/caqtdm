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

#ifndef CameraWidget_H
#define CameraWidget_H

#include <QPixmap>
#include <QWidget>
#include <QResizeEvent>
#include <QSize>
#include <caLabel>
#include <QCheckBox>
#include <QScrollArea>
#include <QFormLayout>
#include <QSlider>
#include <QToolButton>
#include <QScrollBar>
#include <qtcontrols_global.h>
#include <imagewidget.h>
#include <calabel.h>

#include <qwt_color_map.h>
#include <qwt_scale_widget.h>
#if QWT_VERSION >= 0x060100
#include <qwt_scale_div.h>
#endif
#include <time.h>
#include <sys/timeb.h>

#include "colormaps.h"

class QTCON_EXPORT caCamera : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QString channelData READ getPV_Data WRITE setPV_Data)
    Q_PROPERTY(QString channelWidth READ getPV_Width WRITE setPV_Width)
    Q_PROPERTY(QString channelHeight READ getPV_Height WRITE setPV_Height)
    Q_PROPERTY(QString channelCode READ getPV_Code WRITE setPV_Code)
    Q_PROPERTY(QString channelBPP READ getPV_BPP WRITE setPV_BPP)
    Q_PROPERTY(bool simpleZoomedView READ getSimpleView WRITE setSimpleView)
    Q_PROPERTY(zoom Zoom READ getFitToSize WRITE setFitToSize)

    Q_PROPERTY(bool automaticLevels READ getInitialAutomatic WRITE setInitialAutomatic)
    Q_PROPERTY(QString minLevel READ getMinLevel WRITE setMinLevel)
    Q_PROPERTY(QString maxLevel READ getMaxLevel WRITE setMaxLevel)

    Q_PROPERTY(colormap ColorMap READ getColormap WRITE setColormap)
    Q_PROPERTY(QString customColorMap READ getCustomMap WRITE setCustomMap  DESIGNABLE isPropertyVisible(customcolormap))
    Q_PROPERTY(bool discreteCustomColorMap READ getDiscreteCustomMap WRITE setDiscreteCustomMap DESIGNABLE isPropertyVisible(discretecolormap))

    Q_PROPERTY(ROI_markertype ROI_readmarkerType READ getROIreadmarkerType WRITE setROIreadmarkerType)
    Q_PROPERTY(ROI_type ROI_readType READ getROIreadType WRITE setROIreadType)
    Q_PROPERTY(QString ROI_readChannels READ getROIChannelsRead WRITE setROIChannelsRead)

    Q_PROPERTY(ROI_markertype ROI_writemarkerType READ getROIwritemarkerType WRITE setROIwritemarkerType)
    Q_PROPERTY(ROI_type ROI_writeType READ getROIwriteType WRITE setROIwriteType)
    Q_PROPERTY(QString ROI_writeChannels READ getROIChannelsWrite WRITE setROIChannelsWrite)

    Q_ENUMS(zoom)
    Q_ENUMS(colormap)
    Q_ENUMS(ROI_type)
    Q_ENUMS(ROI_markertype)

public:

    enum ROI_type {none=0, xy_only, xy1_xy2, xyUpleft_xyLowright, xycenter_width_height};
    enum ROI_markertype {box=0, box_crosshairs, line, arrow};

    enum zoom {No=0, Yes};

    enum colormap {grey=0, spectrum_wavelength, spectrum_hot, spectrum_heat, spectrum_jet, spectrum_custom};

    enum Properties { customcolormap = 0, discretecolormap};

    caCamera(QWidget *parent = 0);
    ~caCamera();

    void updateImage(const QImage &image, bool valuesPresent[], double values[], double scaleFactor);
    void getROI(QPointF &P1, QPointF &P2);
    QImage * showImageCalc(int datasize, char *data);
    void showImage(int datasize, char *data);

    bool getAccessW() const {return _AccessW;}
    void setAccessW(bool access);

    QString getPV_Data() const {return thisPV_Data;}
    void setPV_Data(QString const &newPV) {thisPV_Data = newPV;}
    QString getPV_Width() const {return thisPV_Width;}
    void setPV_Width(QString const &newPV) {thisPV_Width = newPV;}
    QString getPV_Height() const {return thisPV_Height;}
    void setPV_Height(QString const &newPV) {thisPV_Height = newPV;}
    QString getPV_Code() const {return thisPV_Code;}
    void setPV_Code(QString const &newPV) {thisPV_Code = newPV;}
    QString getPV_BPP() const {return thisPV_BPP;}
    void setPV_BPP(QString const &newPV) {thisPV_BPP = newPV;}

    ROI_markertype getROIreadmarkerType() const {return thisROIreadmarkertype;}
    void setROIreadmarkerType(ROI_markertype const &roimarkertype) {thisROIreadmarkertype = roimarkertype;}

    ROI_markertype getROIwritemarkerType() const {return thisROIwritemarkertype;}
    void setROIwritemarkerType(ROI_markertype const &roimarkertype) {thisROIwritemarkertype = roimarkertype;}

    ROI_type getROIreadType() const {return thisROIreadtype;}
    void setROIreadType(ROI_type const &roireadtype) {thisROIreadtype = roireadtype;}

    ROI_type getROIwriteType() const {return thisROIwritetype;}
    void setROIwriteType(ROI_type const &roiwritetype) {thisROIwritetype = roiwritetype;}

    QString getROIChannelsWrite() const {return thisPV_ROI_Write.join(";");}
    void setROIChannelsWrite(QString const &newPV) {thisPV_ROI_Write = newPV.split(";");}

    QString getROIChannelsRead() const {return thisPV_ROI_Read.join(";");}
    void setROIChannelsRead(QString const &newPV) {thisPV_ROI_Read = newPV.split(";");}

    colormap getColormap() const {return thisColormap;}
    void setColormap(colormap const &map);

    QString getCustomMap() const {return thisCustomMap.join(";");}
    void setCustomMap(QString const &newmap) {thisCustomMap = newmap.split(";"); setColormap(thisColormap);}

    bool getDiscreteCustomMap() const {return thisDiscreteMap;}
    void setDiscreteCustomMap(bool discrete) {thisDiscreteMap = discrete; setColormap(thisColormap);}

    zoom getFitToSize () const {return thisFitToSize;}
    void setFitToSize(zoom const &z);

    bool getInitialAutomatic();
    void setInitialAutomatic(bool automatic);

    bool getSimpleView() { return thisSimpleView;}
    void setSimpleView(bool simpleV) {thisSimpleView = simpleV; setup();}

    QString getMinLevel() const {return thisMinLevel;}
    bool isAlphaMinLevel();
    void setMinLevel(QString const &level);
    QString getMaxLevel() const {return thisMaxLevel;}
    bool isAlphaMaxLevel();
    void setMaxLevel(QString const &level);

    bool isPropertyVisible(Properties property);
    void setPropertyVisible(Properties property, bool visible);

    void setCode(int code);
    void setBPP(int bpp);
    void setWidth(int width);
    void setHeight(int height);

    void updateMax(int max);
    void updateMin(int min);
    void updateIntensity(QString strng);
    int getMin();
    int getMax();
    bool getAutomateChecked();
    void setup();
    void dataProcessing(double value, int id);
    void showDisconnected();

signals:
   void WriteDetectedValuesSignal(QWidget*);

private slots:
    void zoomIn(int level = 1);
    void zoomOut(int level = 1);
    void zoomNow();
    void updateChannels();

protected:
    void resizeEvent(QResizeEvent *event);
    void timerEvent(QTimerEvent *);

private:

    bool eventFilter(QObject *obj, QEvent *event);
    void Coordinates(int posX, int posY, double &newX, double &newY, double &maxX, double &maxY);
    void deleteWidgets();
    void initWidgets();

    bool buttonPressed, validIntensity;
    QString thisPV_Data, thisPV_Width, thisPV_Height, thisPV_Code, thisPV_BPP;
    QStringList thisCustomMap;
    ROI_type thisROIreadtype, thisROIwritetype;
    ROI_markertype thisROIreadmarkertype, thisROIwritemarkertype;
    QStringList thisPV_ROI_Read, thisPV_ROI_Write;
    QString thisMinLevel, thisMaxLevel;
    colormap thisColormap;
    zoom thisFitToSize;
    QImage *image;

    int Xpos, Ypos, Zvalue;
    bool m_init;
    enum { ColormapSize = 256 };
    uint ColorMap[ColormapSize];

    bool m_codeDefined;
    bool m_bppDefined;
    bool m_widthDefined;
    bool m_heightDefined;
    int  m_code, m_bpp, m_width, m_height;
    struct timeb timeRef;
    int savedSize;
    int savedWidth;
    int savedHeight;
    char *savedData;

    uint minvalue, maxvalue;

    QHBoxLayout  *valuesLayout;
    QGridLayout  *mainLayout;
    QGridLayout  *zoomSliderLayout;

    caLineEdit *labelMin;
    caLineEdit *labelMax;
    caLabel *intensity;
    ImageWidget *imageW;
    QCheckBox *autoW;
    caLabel *labelMaxText;
    caLabel *labelMinText;
    caLabel *intensityText;
    caLabel *checkAutoText;
    caLabel *nbUpdatesText;
    bool readvaluesPresent[4];
    double  readvalues[4];
    int writevalues[4];

    QScrollArea *scrollArea;
    QWidget *valuesWidget;
    QWidget *zoomWidget;
    QSlider *zoomSlider;
    QLabel *zoomValue;
    QToolButton *zoomInIcon;
    QToolButton *zoomOutIcon;
    QwtScaleWidget *colormapWidget;

    double scaleFactor;
    int UpdatesPerSecond;
    QPoint selectionPoints[2];
    bool _AccessW;
    bool thisSimpleView;
    bool thisInitialAutomatic;
    bool thisDiscreteMap;
    bool designerVisible[10];

    QTimer *writeTimer;
    QPointF P1, P2, P1_old, P2_old;
    bool selectionInProgress;
};

#endif
