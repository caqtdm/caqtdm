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
#include <QComboBox>
#include <QGridLayout>
#include <qtcontrols_global.h>
#include <imagewidget.h>
#include <calabel.h>
#include "knobDefines.h"


#include <qwt_color_map.h>
#include <qwt_scale_widget.h>
#if QWT_VERSION >= 0x060100
#include <qwt_scale_div.h>
#endif
#include <time.h>

#ifndef MOBILE_ANDROID
#include <sys/timeb.h>
#else
#include <QtControls>
#include <androidtimeb.h>
#endif

#include <stdint.h>

#include "colormaps.h"
#include "caPropHandleDefs.h"

struct SyncMinMax{
    uint Max[2];
    uint Min[2];
    QMutex * MinMaxLock;
    QMutex * imageLock;
};

class QTCON_EXPORT caCamera : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QString channelData READ getPV_Data WRITE setPV_Data)
    Q_PROPERTY(QString channelWidth READ getPV_Width WRITE setPV_Width)
    Q_PROPERTY(QString channelHeight READ getPV_Height WRITE setPV_Height)

    Q_PROPERTY(bool simpleZoomedView READ getSimpleView WRITE setSimpleView)
    Q_PROPERTY(zoom Zoom READ getFitToSize WRITE setFitToSize)

    Q_PROPERTY(bool automaticLevels READ getInitialAutomatic WRITE setInitialAutomatic)
    Q_PROPERTY(QString minLevel READ getMinLevel WRITE setMinLevel)
    Q_PROPERTY(QString maxLevel READ getMaxLevel WRITE setMaxLevel)

    Q_PROPERTY(colormode colorMode READ getColormode WRITE setColormode)
    Q_PROPERTY(QString colorModeOverwriteChannel READ getPV_ColormodeChannel WRITE setPV_ColormodeChannel)
    Q_PROPERTY(packingmode packMode READ getPackmode WRITE setPackmode)
    Q_PROPERTY(QString packingModeOverwriteChannel READ getPV_PackingmodeChannel WRITE setPV_PackingmodeChannel)

    Q_PROPERTY(bool showComboBoxes READ getShowComboBoxes WRITE setShowComboBoxes)

    Q_PROPERTY(colormap ColorMap READ getColormap WRITE setColormap)
    Q_PROPERTY(QString customColorMap READ getCustomMap WRITE setCustomMap  DESIGNABLE isPropertyVisible(customcolormap))
    Q_PROPERTY(bool discreteCustomColorMap READ getDiscreteCustomMap WRITE setDiscreteCustomMap DESIGNABLE isPropertyVisible(discretecolormap))

    Q_PROPERTY(QStringList ROI_readChannelsList READ getROIChannelsReadList WRITE setROIChannelsReadList STORED false)
    Q_PROPERTY(QString ROI_readChannels READ getROIChannelsRead WRITE setROIChannelsRead DESIGNABLE inactiveButVisible())
    Q_PROPERTY(ROI_markertype ROI_readmarkerType READ getROIreadmarkerType WRITE setROIreadmarkerType)
    Q_PROPERTY(ROI_type ROI_readType READ getROIreadType WRITE setROIreadType)

    Q_PROPERTY(QStringList ROI_writeChannelsList READ getROIChannelsWriteList WRITE setROIChannelsWriteList STORED false)
    Q_PROPERTY(QString ROI_writeChannels READ getROIChannelsWrite WRITE setROIChannelsWrite DESIGNABLE inactiveButVisible())
    Q_PROPERTY(ROI_markertype ROI_writemarkerType READ getROIwritemarkerType WRITE setROIwritemarkerType)
    Q_PROPERTY(ROI_type ROI_writeType READ getROIwriteType WRITE setROIwriteType)

    Q_PROPERTY(QString channelXaverage  READ getPV_Xaverage WRITE setPV_Xaverage)
    Q_PROPERTY(QString channelYaverage  READ getPV_Yaverage WRITE setPV_Yaverage)

    // this will prevent user interference
    Q_PROPERTY(QString styleSheet READ styleSheet WRITE noStyle DESIGNABLE false)

    Q_ENUMS(zoom)
    Q_ENUMS(colormap)
    Q_ENUMS(colormode)
    Q_ENUMS(ROI_type)
    Q_ENUMS(ROI_markertype)
    Q_ENUMS(packingmode)

public:
#include "caPropHandle.h"

    void noStyle(QString style) {Q_UNUSED(style);}

    enum  ChannelType {CH_X=0, CH_Y};

    enum ROI_type {none=0, xy_only, xy1_xy2, xyUpleft_xyLowright, xycenter_width_height};
    enum ROI_markertype {box=0, box_crosshairs, line, arrow};

    enum zoom {No=0, Yes};

    enum colormap {as_is = 0, color_to_mono, mono_to_wavelength, mono_to_hot, mono_to_heat, mono_to_jet, mono_to_custom};

    enum colormode {Mono,Mono12p,Mono10p,Mono10Packed,Mono8, RGB1_CA, RGB2_CA, RGB3_CA, BayerRG_8, BayerGB_8, BayerGR_8, BayerBG_8, BayerRG_12, BayerGB_12, BayerGR_12, BayerBG_12, RGB_8 ,BGR_8 ,RGBA_8 ,BGRA_8 , YUV444, YUV422, YUV411, YUV421};

    enum packingmode {packNo, MSB12Bit, LSB12Bit, Reversed};

    enum compressionmode {non, Zlib, JPG };

    enum Properties { customcolormap = 0, discretecolormap};

    caCamera(QWidget *parent = 0);
    ~caCamera();

    void updateImage(const QImage &image, bool valuesPresent[], double values[], double scaleFactor,
                     QVarLengthArray<double> X, QVarLengthArray<double> Y);
    void getROI(QPointF &P1, QPointF &P2);
    QImage * showImageCalc(int datasize, char *data, short datatype);
    void showImage(int datasize, char *data, short datatype);

    colormode getColormode() const {return thisColormode;}
    void setColormode(colormode const &mode) {thisColormode = mode; if(colormodeCombo != (QComboBox*)Q_NULLPTR) colormodeCombo->setCurrentIndex(mode);}

    void setPackmode(packingmode mode) {thisPackingmode = mode; if(packingmodeCombo != (QComboBox*)Q_NULLPTR) packingmodeCombo->setCurrentIndex(mode);}
    packingmode getPackmode() {return thisPackingmode;}

    void setCompressionmode(compressionmode mode) {thisCompressionmode = mode; if(compressionmodeCombo != (QComboBox*)Q_NULLPTR) compressionmodeCombo->setCurrentIndex(mode);}
    compressionmode getCompressionmode() {return thisCompressionmode;}

    void setShowComboBoxes(bool show) {if(colormodesWidget == (QWidget *)Q_NULLPTR)return; thisShowBoxes = show;  if(thisShowBoxes) colormodesWidget->show(); else colormodesWidget->hide();}
    bool getShowComboBoxes() const {return thisShowBoxes;}

    void setData(double *vector, int size, int curvIndex, int curvType, int curvXY);
    void setData(float *vector, int size, int curvIndex, int curvType, int curvXY);
    void setData(int16_t *vector, int size, int curvIndex, int curvType, int curvXY);
    void setData(int32_t* vector, int size, int curvIndex, int curvType, int curvXY);
    void setData(int8_t* vector, int size, int curvIndex, int curvType, int curvXY);

    bool getAccessW() const {return _AccessW;}
    void setAccessW(bool access);

    QString getPV_ColormodeChannel()  const {return thisPV_Mode;}
    void setPV_ColormodeChannel(QString const &modePV) {thisPV_Mode = modePV;}

    QString getPV_PackingmodeChannel()  const {return thisPV_Packing;}
    void setPV_PackingmodeChannel(QString const &packingPV) {thisPV_Packing = packingPV;}

    QString getPV_Data() const {return thisPV_Data;}
    void setPV_Data(QString const &newPV) {thisPV_Data = newPV;}
    QString getPV_Width() const {return thisPV_Width;}
    void setPV_Width(QString const &newPV) {thisPV_Width = newPV;}
    QString getPV_Height() const {return thisPV_Height;}
    void setPV_Height(QString const &newPV) {thisPV_Height = newPV;}

    QString getPV_Xaverage() const {return thisPV_Xaverage;}
    void setPV_Xaverage(QString const &newPV) {thisPV_Xaverage = newPV;}

    QString getPV_Yaverage() const {return thisPV_Yaverage;}
    void setPV_Yaverage(QString const &newPV) {thisPV_Yaverage = newPV;}

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
    QStringList getROIChannelsWriteList() const {return thisPV_ROI_Write;}
    void setROIChannelsWriteList(QStringList list) {thisPV_ROI_Write = list; updatePropertyEditorItem(this, "ROI_writeChannels");}

    QString getROIChannelsRead() const {return thisPV_ROI_Read.join(";");}
    void setROIChannelsRead(QString const &newPV) {thisPV_ROI_Read = newPV.split(";");}
    QStringList getROIChannelsReadList() const {return thisPV_ROI_Read;}
    void setROIChannelsReadList(QStringList list) { thisPV_ROI_Read = list; updatePropertyEditorItem(this, "ROI_readChannels");}

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

    QString getMaxLevel() const {return thisMaxLevel;}
    bool isAlphaMaxLevel();


    bool isPropertyVisible(Properties property);
    void setPropertyVisible(Properties property, bool visible);

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

    bool testDecodemodeStr(QString mode);
    bool testPackingmodeStr(QString mode);
    bool testCompressionmodeStr(QString mode);

public slots:
    void animation(QRect p) {
#include "animationcode.h"
    }

    void hideObject(bool hideit) {
#include "hideobjectcode.h"
    }

    void setMinLevel(QString const &level);
    void setMaxLevel(QString const &level);
    void setAutoLevel(bool enable);

    void setZoomSlider(int zoom);
    void sethorizontalScrollBar(int pos);
    void setverticalScrollBar(int pos);

    void setDecodemodeStr(QString mode);
    void setDecodemodeNum(int mode);
    void setDecodemodeNum(double mode);

    void setPackingmodeStr(QString mode);
    void setPackingmodeNum(int mode);
    void setPackingmodeNum(double mode);

    void setCompressionmodeStr(QString mode);
    void setCompressionmodeNum(int mode);
    void setCompressionmodeNum(double mode);


/*
    void red_coeff(double coeff) { thisRedCoefficient = coeff;}
    void green_coeff(double coeff) { thisGreenCoefficient = coeff;}
    void blue_coeff(double coeff) { thisBlueCoefficient = coeff;}
*/
signals:
   void WriteDetectedValuesSignal(QWidget*);

private slots:
    void zoomIn(int level = 1);
    void zoomOut(int level = 1);
    void zoomNow();
    void updateChannels();
    void scrollAreaMoved(int);
    void colormodeComboSlot(int);
    void packingmodeComboSlot(int);
    void compressionmodeComboSlot(int);

protected:
    void resizeEvent(QResizeEvent *event);
    void timerEvent(QTimerEvent *);

private:

    typedef enum {
        BAYER_COLORFILTER_RGGB = 512,
        BAYER_COLORFILTER_GBRG,
        BAYER_COLORFILTER_GRBG,
        BAYER_COLORFILTER_BGGR
    } bayerFilter;
#define BAYER_COLORFILTER_MIN        BAYER_COLORFILTER_RGGB
#define BAYER_COLORFILTER_MAX        BAYER_COLORFILTER_BGGR
#define BAYER_COLORFILTER_NUM       (BAYER_COLORFILTER_MAX - BAYER_COLORFILTER_MIN + 1)


    typedef enum {
        COLOR_RGB,
        COLOR_GBR,
        COLOR_GRB,
        COLOR_BGR,
        COLOR_BRG
    } rgb_interpretation;

    template <typename pureData>
    void fillData(pureData *array, int size, int curvIndex, int curvType, int curvXY);
    QVarLengthArray<double> X;
    QVarLengthArray<double> Y;

    void buf_unpack_12bitpacked_lsb(void* target, void* source, size_t destcount, size_t targetcount);
    void buf_unpack_12bitpacked_msb(void* target, void* source, size_t destcount, size_t targetcount);
    void buf_unpack_10bitpacked(void* target, void* source, size_t destcount, size_t targetcount);
    void buf_unpack_10bitp(void* target, void* source, size_t destcount, size_t targetcount);

    template <typename pureData>
    void calcImage (pureData *ptr,  colormode mode, QVector<uint> &LineData, long &i, int &ystart, int &yend, float correction,
                    int datasize, QSize resultSize, SyncMinMax* MinMax, uint Max[2], uint Min[2]);

    template <typename pureData>
    void calcImageMono (pureData *ptr,  uint *LineData, long &i, int &ystart, int &yend, float correction, int datasize, QSize resultSize,
                        uint Max[2], uint Min[2]);

    template <typename pureData> void FilterBayer(pureData *bayer, uint *rgb, int sx, int sy, int tile,int datasize);

    template <typename pureData>
    int zValueImage(pureData *ptr, colormode mode, double xnew, double ynew, double xmax, double ymax, int datasize, bool &validIntensity);

    void PROC_UYVY422(uchar *YUV, uint *rgb, int sx, int sy, int datasize);
    void PROC_YUYV422(uchar *YUV, uint *rgb, int sx, int sy, int datasize);
    void PROC_YYUYYV411(uchar *YUV, uint *rgb, int sx, int sy, int datasize);
    void PROC_UYYVYY411(uchar *YUV, uint *rgb, int sx, int sy, int datasize);
    void PROC_YUV444(uchar *YUV, uint *rgb, int sx, int sy, int datasize);
    void PROC_UVY444(uchar *YUV, uint *rgb, int sx, int sy, int datasize);

    void PROC_RGB8(uchar *RGB,rgb_interpretation rgb_type, uint *rgb, int sx, int sy, int datasize);
    void PROC_RGBA8(uchar *RGBA,rgb_interpretation rgb_type, uint *rgb, int sx, int sy, int datasize);

    bool eventFilter(QObject *obj, QEvent *event);
    void Coordinates(int posX, int posY, double &newX, double &newY, double &maxX, double &maxY);
    void deleteWidgets();
    void initWidgets();
    void setColormodeStrings();
    void setPackingModeStrings();
    void setCompressionModeStrings();

    void CameraDataConvert(int sector, int sectorcount, SyncMinMax *MinMax, QSize resultSize, int datasize);
    void MinMaxLock(SyncMinMax* MinMax, uint Max[2], uint Min[2]);
    void MinMaxImageLock(QVector<uint> LineData, int y, QSize resultSize, SyncMinMax* MinMax);
    void MinMaxImageLockBlock(uint *LineData, int ystart, int yend, QSize resultSize, SyncMinMax* MinMax);
    void InitLoopdata(int &ystart, int &yend, long &i, int increment, int sector, int sectorcount,
                         QSize resultSize, uint Max[2], uint Min[2]);

    void reallocate_central_image();
    bool buttonPressed;
    QString  thisPV_Mode, thisPV_Packing;
    QString thisPV_Data, thisPV_Width, thisPV_Height;
    QString thisPV_Xaverage, thisPV_Yaverage;
    QStringList thisCustomMap;
    ROI_type thisROIreadtype, thisROIwritetype;
    ROI_markertype thisROIreadmarkertype, thisROIwritemarkertype;
    QStringList thisPV_ROI_Read, thisPV_ROI_Write;
    QString thisMinLevel, thisMaxLevel;
    int m_MinLevel,m_MaxLevel;
    int m_zoom_value;
    int m_verticalScroll,m_horizontalScroll;

    colormap thisColormap;
    zoom thisFitToSize;
    QImage *image;
    QMutex imageMutex;
    QByteArray decompressedData;

    int Xpos, Ypos;
    bool m_init;
    enum { ColormapSize = 256 };
    uint ColorMap[ColormapSize];

    bool m_widthDefined;
    bool m_heightDefined;
    short m_datatype;
    colormode thisColormode;
    int  m_width, m_height;
    struct timeb timeRef;
    int savedSize, savedSizeNew;
    int savedWidth;
    int savedHeight;
    char *savedData;
    int bitsPerElement;

    uint minvalue, maxvalue;

    QHBoxLayout  *valuesLayout;
    QHBoxLayout  *colormodeLayout;
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

    caLabel *labelColormodeText;
    caLabel *labelPackingmodeText;
    caLabel *labelCompressionmodeText;
    QComboBox *colormodeCombo;
    QComboBox *packingmodeCombo;
    QComboBox *compressionmodeCombo;

    bool readvaluesPresent[4];
    double  readvalues[4];
    int writevalues[4];

    QScrollArea *scrollArea;
    QWidget *valuesWidget;
    QWidget *colormodesWidget;
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
    bool thisShowBoxes;

    QTimer *writeTimer;
    QPointF P1, P2, P1_old, P2_old, P3;
    bool selectionInProgress;

    QStringList thisList;
    QStringList colorModeString;
    QStringList packingModeString;
    QStringList compressionModeString;

    packingmode thisPackingmode;
    compressionmode thisCompressionmode;
    float thisRedCoefficient;
    float thisGreenCoefficient;
    float thisBlueCoefficient;

     uint *rgb;
};

#endif
