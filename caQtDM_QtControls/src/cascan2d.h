//******************************************************************************
// Copyright (c) 2012 Paul Scherrer Institut PSI), Villigen, Switzerland
// Disclaimer: neither  PSI, nor any of their employees makes any warranty
// or assumes any legal liability or responsibility for the use of this software
//******************************************************************************
//******************************************************************************
//
//     Author : Tim Mooney, based on cacamera by Anton Chr. Mezger
//
//******************************************************************************

#ifndef Scan2DWidget_H
#define Scan2DWidget_H

#include <QPixmap>
#include <QWidget>
#include <QSize>
#include <QLabel>
#include <QCheckBox>
#include <QFormLayout>
#include <qtcontrols_global.h>
#include <imagewidget.h>
#include <time.h>
#include <sys/timeb.h>
#include "mdaReader.h"

#define XMAXPTS 1000
#define YMAXPTS 1000

class QTCON_EXPORT caScan2D : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QString channelData READ getPV_Data WRITE setPV_Data)
    Q_PROPERTY(QString channelWidth READ getPV_Width WRITE setPV_Width)
    Q_PROPERTY(QString channelHeight READ getPV_Height WRITE setPV_Height)
    Q_PROPERTY(QString channelCode READ getPV_Code WRITE setPV_Code)
    Q_PROPERTY(QString channelBPP READ getPV_BPP WRITE setPV_BPP)

    Q_PROPERTY(QString channelXCPT READ getPV_XCPT WRITE setPV_XCPT)
    Q_PROPERTY(QString channelYCPT READ getPV_YCPT WRITE setPV_YCPT)
    Q_PROPERTY(QString channelXNEWDATA READ getPV_XNEWDATA WRITE setPV_XNEWDATA)
    Q_PROPERTY(QString channelYNEWDATA READ getPV_YNEWDATA WRITE setPV_YNEWDATA)
    Q_PROPERTY(QString channelSAVEDATA_PATH READ getPV_SAVEDATA_PATH WRITE setPV_SAVEDATA_PATH)
    Q_PROPERTY(QString channelSAVEDATA_SUBDIR READ getPV_SAVEDATA_SUBDIR WRITE setPV_SAVEDATA_SUBDIR)
    Q_PROPERTY(QString channelSAVEDATA_FILENAME READ getPV_SAVEDATA_FILENAME WRITE setPV_SAVEDATA_FILENAME)

    Q_PROPERTY(zoom Zoom READ getZoom WRITE setZoom)
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

    caScan2D(QWidget *parent = 0);
    ~caScan2D();

    void updateImage(const QImage &image, bool valuesPresent[], int values[]);
    void newArray(int numDataBytes, float *data);
    void showImage(int numXDataValues, int numYDataValues);
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

	//sscanRecord PVs
    QString getPV_XCPT() const {return thisPV_XCPT;}
    void setPV_XCPT(QString const &newPV) {thisPV_XCPT = newPV;}
    QString getPV_YCPT() const {return thisPV_YCPT;}
    void setPV_YCPT(QString const &newPV) {thisPV_YCPT = newPV;}
    QString getPV_XNEWDATA() const {return thisPV_XNEWDATA;}
    void setPV_XNEWDATA(QString const &newPV) {thisPV_XNEWDATA = newPV;}
    QString getPV_YNEWDATA() const {return thisPV_YNEWDATA;}
    void setPV_YNEWDATA(QString const &newPV) {thisPV_YNEWDATA = newPV;}
    QString getPV_SAVEDATA_PATH() const {return thisPV_SAVEDATA_PATH;}
    void setPV_SAVEDATA_PATH(QString const &newPV) {thisPV_SAVEDATA_PATH = newPV;}
    QString getPV_SAVEDATA_SUBDIR() const {return thisPV_SAVEDATA_SUBDIR;}
    void setPV_SAVEDATA_SUBDIR(QString const &newPV) {thisPV_SAVEDATA_SUBDIR = newPV;}
    QString getPV_SAVEDATA_FILENAME() const {return thisPV_SAVEDATA_FILENAME;}
    void setPV_SAVEDATA_FILENAME(QString const &newPV) {thisPV_SAVEDATA_FILENAME = newPV;}

    QString getDataProcChannels() const {return thisDataProcPV.join(";");}
    void setDataProcChannels(QString const &newPV) {thisDataProcPV = newPV.split(";");}

    colormap getColormap() const {return thisColormap;}
    void setColormap(colormap const &map);

    zoom getZoom () const {return thisZoom;}
    void setZoom(zoom const &z) {thisZoom = z;}

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

    void setXCPT(int xcpt);
    void setYCPT(int ycpt);
    void setXNEWDATA(int xnewdata);
    void setYNEWDATA(int ynewdata);
    void setSAVEDATA_PATH(const QString &savedata_path);
    void setSAVEDATA_SUBDIR(const QString &savedata_subdir);
    void setSAVEDATA_FILENAME(const QString &savedata_filename);
	void attemptInitialPlot();

    void updateMax(int max);
    void updateMin(int min);
    int getMin();
    int getMax();
    bool getAutomateChecked();
    void setup(bool interaction);
    void dataProcessing(int value, int id);

protected:
    void resizeEvent(QResizeEvent *event);

private:

    bool m_zoom, m_forcemonochrome;
    QString thisPV_Data, thisPV_Width, thisPV_Height, thisPV_Code, thisPV_BPP;
    QString thisPV_XCPT, thisPV_YCPT, thisPV_XNEWDATA, thisPV_YNEWDATA;
	QString thisPV_SAVEDATA_PATH, thisPV_SAVEDATA_SUBDIR, thisPV_SAVEDATA_FILENAME;
    QStringList thisDataProcPV;
    QString thisMinLevel, thisMaxLevel;
    colormap thisColormap;
    zoom thisZoom;
    QImage *image;
    bool m_init;
    enum { ColormapSize = 256 };
    uint ColorMap[ColormapSize];

    bool m_codeDefined;
    bool m_bppDefined;
    bool m_widthDefined;
    bool m_heightDefined;
    int m_code, m_bpp, m_width, m_height;

	// sscanRecord
    bool m_xcptDefined, m_ycptDefined, m_xnewdataDefined, m_ynewdataDefined;
	bool m_savedata_pathDefined, m_savedata_subdirDefined, m_savedata_filenameDefined;
	int m_xcpt, m_ycpt, m_xnewdata, m_ynewdata;
	QString m_savedata_path, m_savedata_subdir, m_savedata_filename;

    int frameCount;
    struct timeb timeRef, timeR;
    int savedSize;
    int savedWidth;
    int savedHeight;

    QHBoxLayout  *hbox;
    QGridLayout  *vbox;
    QLineEdit *labelMin;
    QLineEdit *labelMax;
    ImageWidget *imageW;
    QCheckBox *autoW;
    QLabel *labelMaxText;
    QLabel *labelMinText;
    QLabel *checkAutoText;
    QWidget *window;
    bool valuesPresent[4];
    int  values[4];

	// tmm:later we'll do this right
	int haveY[YMAXPTS];
	float xdata[YMAXPTS*XMAXPTS];
};

#endif
