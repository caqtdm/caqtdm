#include <QtGui>
#include <math.h>
#include "cascan2d.h"

caScan2D::caScan2D(QWidget *parent) : QWidget(parent)
{
    m_forcemonochrome = false;
    m_init = true;

    m_codeDefined = false;
    m_bppDefined = false;
    m_widthDefined = false;
    m_heightDefined = false;
    m_xcptDefined = false;
	m_ycptDefined = false;
	m_xnewdataDefined = false;
	m_ynewdataDefined = false;
	m_savedata_pathDefined = false;
	m_savedata_subdirDefined = false;
	m_savedata_filenameDefined = false;
    thisColormap = Default;
    savedSize = 0;
    savedWidth = 0;
    savedHeight = 0;
    frameCount = 0;

    image = (QImage *) 0;

    for(int i=0; i<ColormapSize; i++) ColorMap[i] = qRgb(i,i,i);

    hbox = (QHBoxLayout*) 0;
    labelMin = (QLineEdit*) 0;
    labelMax = (QLineEdit*) 0;
    imageW = (ImageWidget*) 0;
    autoW = (QCheckBox *) 0;

    vbox = new QGridLayout(this);
    vbox->setMargin(0);
    vbox->setSpacing(0);
    setLayout(vbox);

    setup(false);

}

caScan2D::~caScan2D()
{
    if(image != (QImage *) 0) delete image;
    if(hbox != (QHBoxLayout*) 0) {
        delete labelMinText;
        delete labelMin;
        delete labelMaxText;
        delete labelMax;
        delete checkAutoText;
        delete autoW;

        delete hbox;
        delete window;
        delete imageW;
        delete vbox;
    }
}

void caScan2D::setup(bool interaction)
{
    if(hbox != (QHBoxLayout*) 0) {
        hbox->removeWidget(labelMaxText);
        hbox->removeWidget(labelMax);
        hbox->removeWidget(labelMinText);
        hbox->removeWidget(labelMin);
        hbox->removeWidget(checkAutoText);
        hbox->removeWidget(autoW);
        delete labelMinText;
        delete labelMin;
        delete labelMaxText;
        delete labelMax;
        delete checkAutoText;
        delete autoW;
        autoW = (QCheckBox *) 0;

        delete hbox;
        hbox = (QHBoxLayout*) 0;

        vbox->removeWidget(window);
        delete window;
    }

    if(imageW != (ImageWidget*) 0) {
        vbox->removeWidget(imageW);
        delete imageW;
    }

    if(interaction) {

        hbox = new QHBoxLayout();
        hbox->setMargin(0);
        hbox->setSpacing(2);

        window = new QWidget;

        labelMaxText = new QLabel(this);
        labelMaxText->setText("Max:");
        labelMinText = new QLabel(this);
        labelMinText->setText("Min:");
        checkAutoText = new QLabel(this);
        checkAutoText->setText("Auto:");

        labelMax = new QLineEdit(this);
        labelMin = new QLineEdit(this);
        imageW   = new ImageWidget(this);

        labelMax->setFixedWidth(30);
        labelMin->setFixedWidth(30);
        labelMaxText->setFixedWidth(40);
        labelMinText->setFixedWidth(40);
        checkAutoText->setFixedWidth(40);

        autoW = new QCheckBox(this);
        autoW->setChecked(true);
        hbox->addWidget(labelMinText, Qt::AlignLeft);
        hbox->addWidget(labelMin, Qt::AlignLeft);
        hbox->addWidget(labelMaxText, Qt::AlignLeft);
        hbox->addWidget(labelMax, Qt::AlignLeft);
        hbox->addWidget(checkAutoText, Qt::AlignLeft);
        hbox->addWidget(autoW,    Qt::AlignLeft);
        window->setLayout(hbox);
        window->show();

        vbox->addWidget(window, 0, 0);
        vbox->addWidget(imageW, 1, 0);

    } else {
        imageW = new ImageWidget(this);
        vbox->addWidget(imageW, 0, 0);
    }

    for(int i=0; i<4; i++) valuesPresent[i] = false;

    QResizeEvent *re = new QResizeEvent(size(), size());
    resizeEvent(re);
    delete re;
}

bool caScan2D::getInitialAutomatic()
{
    if(autoW == (QCheckBox *) 0) return false;
    return autoW->isChecked();
}

void caScan2D::setInitialAutomatic(bool automatic)
{
    if(autoW == (QCheckBox *) 0) return;
    autoW->setChecked(automatic);
}

void caScan2D::setMinLevel(QString const &level) {
    int minimum;
    bool ok;
    thisMinLevel = level;
    minimum = thisMinLevel.toInt(&ok);
    if(ok) {
        updateMin(minimum);
    } else {
        labelMin->setText("");
    }
}
void caScan2D::setMaxLevel(QString const &level) {
    int maximum;
    bool ok;
    thisMaxLevel = level;
    maximum = thisMaxLevel.toInt(&ok);
    if(ok) {
        updateMax(maximum);
    } else {
        labelMax->setText("");
    }
}

bool caScan2D::isAlphaMaxLevel()
{
    int maximum;
    bool ok;
    maximum = thisMaxLevel.toInt(&ok);
    return !ok;
}
bool caScan2D::isAlphaMinLevel()
{
    int minimum;
    bool ok;
    minimum = thisMinLevel.toInt(&ok);
    return !ok;
}

void caScan2D::setColormap(colormap const &map)
{

    thisColormap = map;
    switch (map) {
    case Default:
        for(int i=0; i<ColormapSize; i++) ColorMap[i] = qRgb(i,i,i);
        setup(false);
        break;
    case grey:
        for(int i=0; i<ColormapSize; i++) ColorMap[i] = qRgb(i,i,i);
        setup(false);
        break;
    case spectrum:
        for (int i = 0; i < ColormapSize; ++i) ColorMap[i] = rgbFromWaveLength(380.0 + (i * 400.0 / ColormapSize));
        //z scaling is broken
		//setup(true);
		setup(false);
        break;
    default:
        for (int i = 0; i < ColormapSize; ++i) ColorMap[i] = rgbFromWaveLength(380.0 + (i * 400.0 / ColormapSize));
        setup(false);
        break;
    }
}

void caScan2D::setCode(int code)
{
    m_code = code;
    m_codeDefined = true;
}
void caScan2D::setBPP(int bpp)
{
    m_bpp = bpp;
    m_bppDefined = true;
}

void caScan2D::setXCPT(int xcpt)
{
	//printf("caScan2D::setXCPT xpt=%d\n", xcpt);
    m_xcpt = xcpt;
    m_xcptDefined = true;
}

void caScan2D::setYCPT(int ycpt)
{
	//printf("caScan2D::setYCPT ypt=%d\n", ycpt);
    m_ycpt = ycpt;
    m_ycptDefined = true;
	if (m_init) attemptInitialPlot();
}

void caScan2D::setXNEWDATA(int xnewdata)
{
    m_xnewdata = xnewdata;
    m_xnewdataDefined = true;
}

void caScan2D::setYNEWDATA(int ynewdata)
{
	int i, status=0;

	// I get two calls per monitor event for some reason
	if (m_ynewdata == ynewdata) {
		//printf("caScan2D::setYNEWDATA for pv %s %d (ignored)\n", getPV_Data().toAscii().constData(), ynewdata);
		return;
	}
    m_ynewdata = ynewdata;
    m_ynewdataDefined = true;
	if (!m_widthDefined || !m_heightDefined)
		return;

	//printf("caScan2D::setYNEWDATA for pv %s %d\n", getPV_Data().toAscii().constData(), ynewdata);
	if (m_ynewdata == 0) {
		for (i=0; i<m_width*m_height; i++) xdata[i] = 0.;
		for (i=0; i<m_height; i++) haveY[i] = 0;
	} else {
		// Get all data from file
		if (m_savedata_pathDefined && m_savedata_subdirDefined && m_savedata_filenameDefined && m_ycptDefined) {
			QString dataFile = m_savedata_path + QString("/") + m_savedata_subdir + QString("/") + m_savedata_filename;
			status = mdaReader_gimmeYerData(dataFile, thisPV_Data, xdata, m_width, m_height, m_ycpt);
		}
	}
	// Don't call showImage() on m_init, because xdata may not have been initialized from the data file 
	if (status == 0 && !m_init) showImage(m_width, m_height);
}

void caScan2D::setSAVEDATA_PATH(const QString &savedata_path)
{
	m_savedata_path = savedata_path;
	m_savedata_pathDefined = true;
	if (m_init) attemptInitialPlot();
}

void caScan2D::setSAVEDATA_SUBDIR(const QString &savedata_subdir)
{
	m_savedata_subdir = savedata_subdir;
	m_savedata_subdirDefined = true;
	if (m_init) attemptInitialPlot();
}

void caScan2D::setSAVEDATA_FILENAME(const QString &savedata_filename)
{
	m_savedata_filename = savedata_filename;
	m_savedata_filenameDefined = true;
	if (m_init) attemptInitialPlot();
}

void caScan2D::attemptInitialPlot() {
	if (m_init && m_widthDefined && m_heightDefined && m_savedata_pathDefined && m_savedata_subdirDefined && m_savedata_filenameDefined && m_ycptDefined) {
		QString dataFile = m_savedata_path + QString("/") + m_savedata_subdir + QString("/") + m_savedata_filename;
		mdaReader_gimmeYerData(dataFile, thisPV_Data, xdata, m_width, m_height, m_ycpt);
		showImage(m_width, m_height);
	}
}


void caScan2D::setWidth(int width)
{
    m_width = width;
    m_widthDefined = true;
	if (m_init) attemptInitialPlot();
}

void caScan2D::setHeight(int height)
{
    m_height = height;
    m_heightDefined = true;
	if (m_init) attemptInitialPlot();
}

void caScan2D::resizeEvent(QResizeEvent *e)
{
    imageW->setFixedWidth(e->size().width());
    imageW->setFixedHeight(e->size().height());
	showImage(m_width, m_height);
}

void caScan2D::updateImage(const QImage &image, bool valuesPresent[], int values[])
{
    //imageW->updateImage(thisZoom, image, valuesPresent, values);
    bool selectionStarted = false;
    QRect selectionRect;
	double scaleFactor = 1.0;
    imageW->updateImage(thisZoom, image, valuesPresent, values, scaleFactor, selectionStarted, selectionRect, false);

}

bool caScan2D::getAutomateChecked()
{
    if(autoW == (QCheckBox *) 0) return false;
    return autoW->isChecked();
}

void caScan2D::updateMax(int max)
{
    if(labelMax == (QLineEdit*) 0) return;
    labelMax->setText(QString::number(max));
}

void caScan2D::updateMin(int min)
{
    if(labelMin == (QLineEdit*) 0) return;
    labelMin->setText(QString::number(min));
}

int caScan2D::getMin()
{
    if(labelMin == (QLineEdit*) 0) return 0;
    return labelMin->text().toInt();
}
int caScan2D::getMax()
{
    if(labelMax == (QLineEdit*) 0) return 65535;
    return labelMax->text().toInt();
}

void caScan2D::dataProcessing(int value, int id)
{
    if(id < 0 || id > 3) return;
    valuesPresent[id] = true;
    values[id] = value;
}

void caScan2D::newArray(int numDataBytes, float *data) {
	int numDataPts = numDataBytes/4;
	int i, j;

    if(!m_widthDefined || !m_heightDefined ||  !m_ycptDefined) return;

	if (numDataPts > m_width) numDataPts = m_width;
	//printf("caScan2D::newArray ypt=%d\n", m_ycpt);
	// We're normally going to get new data before the outer scan has posted
	// the new CPT value, so it's useable directly as an array index even though
	// it really means the number of data points acquired.
	if (m_ycpt >= m_height) return;
	if (haveY[m_ycpt]) return;
	for (i=m_ycpt*m_width, j=0; j<numDataPts; i++, j++) {
		xdata[i] = data[j];
	}
	haveY[m_ycpt] = 1;
	showImage(m_width, m_height);
}

void caScan2D::showImage(int numXDataValues, int numYDataValues)
{
    uint indx, indx1;
    long int i;
    QSize resultSize;
    uint Max[2], Min[2];
    static uint minvalue, maxvalue;
	float dataMin=1.e9, dataMax=-1.e9;
	float dataOffset, dataFactor;
	int numDataValues = numXDataValues * numYDataValues;

    if(!m_widthDefined) return;
    if(!m_heightDefined) return;
    // We don't actually use this now
	//if(!m_xcptDefined) return;
    //if(!m_ycptDefined) return;
    //if(!m_xnewdataDefined) return;
    //if(!m_ynewdataDefined) return;


    resultSize.setWidth((int) m_width);
    resultSize.setHeight((int) m_height);

    // first time get image
    if(m_init || numDataValues != savedSize || m_width != savedWidth || m_height != savedHeight) {
        savedSize = numDataValues;
        savedWidth = m_width;
        savedHeight = m_height;
        if(image != (QImage *) 0) delete image;
        image = new QImage(resultSize, QImage::Format_RGB32);
        m_init = false;
        minvalue = 0;
        maxvalue = 65535;

		//for (i=0; i<YMAXPTS; i++) haveY[i] = 0;
		//for (i=0; i<YMAXPTS*XMAXPTS; i++) xdata[i] = 0.;
		//mdaReader_RegisterPV(thisPV_Data);
        ftime(&timeRef);
    }

    Max[1] = 0;
    Min[1] = 65535;

	for (i=0; i<numDataValues; i++) {
		if (xdata[i] < dataMin) dataMin = xdata[i];
		if (xdata[i] > dataMax) dataMax = xdata[i];
	}
	dataOffset = dataMin;
	dataFactor = (maxvalue - minvalue)/(dataMax-dataMin);
	i = 0;
	if(thisColormap == spectrum) {

	    for (int y = 0; y < resultSize.height(); ++y) {
	        uint *scanLine = reinterpret_cast<uint *>(image->scanLine(y));

	        for (int x = 0; x < resultSize.width(); ++x) {
	            indx = (uint) ((xdata[i]-dataMin)*dataFactor);
				i++;
	            indx1 = (indx - minvalue) * (ColormapSize-1) / (maxvalue - minvalue);
	            if(indx1 >= ColormapSize) indx1 = ColormapSize -1;

	            *scanLine++ = ColorMap[indx1];

	            Max[(indx > Max[1])] = indx;
	            Min[(indx < Min[1])] = indx;
	        }
	        if(i >= numDataValues) break;
	    }

	} else {
	    for (int y = 0; y < resultSize.height(); ++y) {
	        uint *scanLine = reinterpret_cast<uint *>(image->scanLine(y));

	        for (int x = 0; x < resultSize.width(); ++x) {
	            indx = (uint) ((xdata[i]-dataMin)*dataFactor); i++;
	            *scanLine++ = ColorMap[indx];
	        }
	        if (i >= numDataValues) break;
	    }
	}


    minvalue = Min[1];
    maxvalue= Max[1];

    if(getAutomateChecked()) {
        updateMax(Max[1]);
        updateMin(Min[1]);
    } else {
        int minv = getMin();
        int maxv = getMax();
        if(maxv > minv) {
            maxvalue = maxv;
            minvalue = minv;
        }
    }

    updateImage(*image, valuesPresent, values);
}


uint caScan2D::rgbFromWaveLength(double wave)
{
    double r = 0.0;
    double g = 0.0;
    double b = 0.0;

    if (wave >= 380.0 && wave <= 440.0) {
        r = -1.0 * (wave - 440.0) / (440.0 - 380.0);
        b = 1.0;
    } else if (wave >= 440.0 && wave <= 490.0) {
        g = (wave - 440.0) / (490.0 - 440.0);
        b = 1.0;
    } else if (wave >= 490.0 && wave <= 510.0) {
        g = 1.0;
        b = -1.0 * (wave - 510.0) / (510.0 - 490.0);
    } else if (wave >= 510.0 && wave <= 580.0) {
        r = (wave - 510.0) / (580.0 - 510.0);
        g = 1.0;
    } else if (wave >= 580.0 && wave <= 645.0) {
        r = 1.0;
        g = -1.0 * (wave - 645.0) / (645.0 - 580.0);
    } else if (wave >= 645.0 && wave <= 780.0) {
        r = 1.0;
    }

    double s = 1.0;
    if (wave > 700.0)
        s = 0.3 + 0.7 * (780.0 - wave) / (780.0 - 700.0);
    else if (wave <  420.0)
        s = 0.3 + 0.7 * (wave - 380.0) / (420.0 - 380.0);

    r = pow(r * s, 0.8);
    g = pow(g * s, 0.8);
    b = pow(b * s, 0.8);
    return qRgb(int(r * 255), int(g * 255), int(b * 255));
}


