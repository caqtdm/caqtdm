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
#include <time.h>
#include <sys/timeb.h>

class QTCON_EXPORT caCamera : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QString channelData READ getPV_Data WRITE setPV_Data)
    Q_PROPERTY(QString channelWidth READ getPV_Width WRITE setPV_Width)
    Q_PROPERTY(QString channelHeight READ getPV_Height WRITE setPV_Height)
    Q_PROPERTY(QString channelCode READ getPV_Code WRITE setPV_Code)
    Q_PROPERTY(QString channelBPP READ getPV_BPP WRITE setPV_BPP)
    Q_PROPERTY(zoom Zoom READ getFitToSize WRITE setFitToSize)
    Q_PROPERTY(colormap ColorMap READ getColormap WRITE setColormap)
    Q_PROPERTY(bool automaticLevels READ getInitialAutomatic WRITE setInitialAutomatic)
    Q_PROPERTY(QString minLevel READ getMinLevel WRITE setMinLevel)
    Q_PROPERTY(QString maxLevel READ getMaxLevel WRITE setMaxLevel)

    Q_PROPERTY(QString dataProcChannels READ getDataProcChannels WRITE setDataProcChannels)

    Q_ENUMS(zoom)
    Q_ENUMS(colormap)

public:
    enum zoom {No=0, Yes};
    enum colormap { Default=0, grey, spectrum};

    caCamera(QWidget *parent = 0);
    ~caCamera();

    void updateImage(const QImage &image, bool valuesPresent[], int values[], const double &scaleFactor);
    QImage * showImageCalc(int datasize, char *data);
    void showImage(int datasize, char *data);
    uint rgbFromWaveLength(double wave);

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

    QString getDataProcChannels() const {return thisDataProcPV.join(";");}
    void setDataProcChannels(QString const &newPV) {thisDataProcPV = newPV.split(";");}

    colormap getColormap() const {return thisColormap;}
    void setColormap(colormap const &map);

    zoom getFitToSize () const {return thisFitToSize;}
    void setFitToSize(zoom const &z);

    bool getInitialAutomatic();
    void setInitialAutomatic(bool automatic);

    QString getMinLevel() const {return thisMinLevel;}
    bool isAlphaMinLevel();
    void setMinLevel(QString const &level);
    QString getMaxLevel() const {return thisMaxLevel;}
    bool isAlphaMaxLevel();
    void setMaxLevel(QString const &level);

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
    void dataProcessing(int value, int id);

private slots:
    void zoomIn(int level = 1);
    void zoomOut(int level = 1);
    void zoomNow();

protected:
    void resizeEvent(QResizeEvent *event);
    void timerEvent(QTimerEvent *);

private:

    bool eventFilter(QObject *obj, QEvent *event);
    bool buttonPressed, validIntensity;
    bool forcemonochrome;
    QString thisPV_Data, thisPV_Width, thisPV_Height, thisPV_Code, thisPV_BPP;
    QStringList thisDataProcPV;
    QString thisMinLevel, thisMaxLevel;
    colormap thisColormap;
    zoom thisFitToSize;
    QImage *image;

    int Xpos, Ypos, Xnew, Ynew, Zvalue;
    bool m_init;
    enum { ColormapSize = 256 };
    uint ColorMap[ColormapSize];

    bool m_codeDefined;
    bool m_bppDefined;
    bool m_widthDefined;
    bool m_heightDefined;
    int  m_code, m_bpp, m_width, m_height;

    int frameCount;
    struct timeb timeRef, timeR;
    int savedSize;
    int savedWidth;
    int savedHeight;
    char *savedData;

    uint minvalue, maxvalue;

    QHBoxLayout  *valuesLayout;
    QGridLayout  *mainLayout;
    QVBoxLayout  *zoomSliderLayout;

    QLineEdit *labelMin;
    QLineEdit *labelMax;
    caLabel *intensity;
    ImageWidget *imageW;
    QCheckBox *autoW;
    caLabel *labelMaxText;
    caLabel *labelMinText;
    caLabel *intensityText;
    caLabel *checkAutoText;
    caLabel *nbUpdatesText;
    bool valuesPresent[4];
    int  values[4];

    QScrollArea *scrollArea;
    QWidget *valuesWidget;
    QWidget *zoomWidget;
    QSlider *zoomSlider;
    QLabel *zoomValue;
    QToolButton *zoomInIcon;
    QToolButton *zoomOutIcon;

    double scaleFactor;

    int UpdatesPerSecond;

};

#endif
