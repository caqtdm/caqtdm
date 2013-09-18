#include <QtGui>
#include <math.h>
#ifndef QT_NO_CONCURRENT
#include <qtconcurrentrun.h>
#endif
#include "cacamera.h"

caCamera::caCamera(QWidget *parent) : QWidget(parent)
{
    m_forcemonochrome = false;
    m_init = true;

    m_codeDefined = false;
    m_bppDefined = false;
    m_widthDefined = false;
    m_heightDefined = false;
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

caCamera::~caCamera()
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

void caCamera::setup(bool interaction)
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
        labelMaxText->setText("Maximum:");
        labelMinText = new QLabel(this);
        labelMinText->setText("Minimum:");
        checkAutoText = new QLabel(this);
        checkAutoText->setText("Auto:");

        labelMax = new QLineEdit(this);
        labelMin = new QLineEdit(this);
        imageW   = new ImageWidget(this);

        labelMax->setFixedWidth(65);
        labelMin->setFixedWidth(65);
        labelMaxText->setFixedWidth(70);
        labelMinText->setFixedWidth(70);
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

bool caCamera::getInitialAutomatic()
{
    if(autoW == (QCheckBox *) 0) return false;
    return autoW->isChecked();
}

void caCamera::setInitialAutomatic(bool automatic)
{
    if(autoW == (QCheckBox *) 0) return;
    autoW->setChecked(automatic);
}

void caCamera::setMinLevel(QString const &level) {
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
void caCamera::setMaxLevel(QString const &level) {
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

bool caCamera::isAlphaMaxLevel()
{
    bool ok;
    (void) thisMaxLevel.toInt(&ok);
    return !ok;
}
bool caCamera::isAlphaMinLevel()
{
    bool ok;
    (void) thisMinLevel.toInt(&ok);
    return !ok;
}

void caCamera::setColormap(colormap const &map)
{

    thisColormap = map;
    switch (map) {
    case Default:
        for(int i=0; i<ColormapSize; i++) ColorMap[i] = qRgb(i,i,i);
        setup(false);
        break;
    case grey:
        for(int i=0; i<ColormapSize; i++) ColorMap[i] = qRgb(i,i,i);
        //if (m_bpp==3){
		  setup(true);
		//}else{
        //  setup(false);
	    //}
        break;
    case spectrum:
        for (int i = 0; i < ColormapSize; ++i) ColorMap[i] = rgbFromWaveLength(380.0 + (i * 400.0 / ColormapSize));
        setup(true);
        break;
    default:
        for (int i = 0; i < ColormapSize; ++i) ColorMap[i] = rgbFromWaveLength(380.0 + (i * 400.0 / ColormapSize));
        setup(false);
        break;
    }
}

void caCamera::setCode(int code)
{
    m_code = code;
    m_codeDefined = true;
}
void caCamera::setBPP(int bpp)
{
    m_bpp = bpp;
    m_bppDefined = true;
}
void caCamera::setWidth(int width)
{
    m_width = width;
    m_widthDefined = true;
}
void caCamera::setHeight(int height)
{
    m_height = height;
    m_heightDefined = true;
}

void caCamera::resizeEvent(QResizeEvent *e)
{
    imageW->setFixedWidth(e->size().width());
    imageW->setFixedHeight(e->size().height());
}

void caCamera::updateImage(const QImage &image, bool valuesPresent[], int values[])
{
    imageW->updateImage(thisZoom, image, valuesPresent, values);
}

bool caCamera::getAutomateChecked()
{
    if(autoW == (QCheckBox *) 0) return false;
    return autoW->isChecked();
}

void caCamera::updateMax(int max)
{
    if(labelMax == (QLineEdit*) 0) return;
    labelMax->setText(QString::number(max));
}

void caCamera::updateMin(int min)
{
    if(labelMin == (QLineEdit*) 0) return;
    labelMin->setText(QString::number(min));
}

int caCamera::getMin()
{
    if(labelMin == (QLineEdit*) 0) return 0;
    return labelMin->text().toInt();
}
int caCamera::getMax()
{
    if(labelMax == (QLineEdit*) 0) return 65535;
    return labelMax->text().toInt();
}

void caCamera::dataProcessing(int value, int id)
{
    if(id < 0 || id > 3) return;
    valuesPresent[id] = true;
    values[id] = value;
}

QImage *caCamera::showImageCalc(int datasize, char *data)
{
    uint indx, indx1;
    long int i=0;
    QSize resultSize;
    uint Max[2], Min[2];

    if(!m_bppDefined) return (QImage *) 0;
    if(!m_widthDefined) return (QImage *) 0;
    if(!m_heightDefined) return (QImage *) 0;
    if(!m_codeDefined) return (QImage *) 0;

    resultSize.setWidth((int) m_width);
    resultSize.setHeight((int) m_height);

    // first time get image
    if(m_init || datasize != savedSize || m_width != savedWidth || m_height != savedHeight) {
        savedSize = datasize;
        savedWidth = m_width;
        savedHeight = m_height;
        if(image != (QImage *) 0) delete image;
        image = new QImage(resultSize, QImage::Format_RGB32);
        m_init = false;
        minvalue = 0;
        maxvalue = 65535;
        ftime(&timeRef);
    }

    Max[1] = 0;
    Min[1] = 65535;

    switch (m_code) {

    // monochrome image
    case 1:{

        // start bpp switch
        switch (m_bpp) {

        case 1:  {  // monochrome 1 bpp  (Damir camera)

            uchar *ptr = (uchar*)  data;
            if(ptr == (void*) 0) return (QImage *) 0;

            if(thisColormap == spectrum) {

                for (int y = 0; y < resultSize.height(); ++y) {
                    uint *scanLine = reinterpret_cast<uint *>(image->scanLine(y));

                    for (int x = 0; x < resultSize.width(); ++x) {
                        indx = ptr[i]; i+=m_bpp;
                        indx1 = (indx - minvalue) * (ColormapSize-1) / (maxvalue - minvalue);
                        if(indx1 >= ColormapSize) indx1=ColormapSize -1;

                        *scanLine++ = ColorMap[indx1];

                        Max[(indx > Max[1])] = indx;
                        Min[(indx < Min[1])] = indx;
                    }
                    if(i >= datasize) break;
                }

            } else {
                for (int y = 0; y < resultSize.height(); ++y) {
                    uint *scanLine = reinterpret_cast<uint *>(image->scanLine(y));

                    for (int x = 0; x < resultSize.width(); ++x) {
                        indx = ptr[i]; i+=m_bpp;
                        *scanLine++ = ColorMap[indx];
                    }
                    if(i >= datasize) break;
                }
            }
        }
        break;

        case 2: {  // monochrome 2 bpp, but use only first byte of words (Damir cameras)

            uchar *ptr = (uchar*)  data;
            if(ptr == (void*) 0) return (QImage *) 0 ;

            if(thisColormap == spectrum) {

                for (int y = 0; y < resultSize.height(); ++y) {
                    uint *scanLine = reinterpret_cast<uint *>(image->scanLine(y));

                    for (int x = 0; x < resultSize.width(); ++x) {
                        indx = ptr[i]*256 + ptr[i+1]; i+=m_bpp;
                        indx1 = (indx - minvalue) * (ColormapSize-1) / (maxvalue - minvalue);
                        if(indx1 >= ColormapSize) indx1=ColormapSize -1;

                        *scanLine++ = ColorMap[indx1];

                        Max[(indx > Max[1])] = indx;
                        Min[(indx < Min[1])] = indx;
                    }
                    if(i >= datasize) break;
                }
            } else {
                for (int y = 0; y < resultSize.height(); ++y) {
                    uint *scanLine = reinterpret_cast<uint *>(image->scanLine(y));

                    for (int x = 0; x < resultSize.width(); ++x) {
                        indx = ptr[i]; i+=m_bpp;
                        *scanLine++ = ColorMap[indx];
                    }
                    if(i >= datasize) break;
                }
            }
        }
        break;

        case 3: {   // monochrome 2 bpp, but used only 12 bits  (Helge cameras)

            ushort *ptr = (ushort*) data;

            if(ptr == (void*) 0) return (QImage *) 0;

            if(thisColormap == spectrum) {

                for (int y = 0; y < resultSize.height(); ++y) {
                    uint *scanLine = reinterpret_cast<uint *>(image->scanLine(y));

                    for (int x = 0; x < resultSize.width(); ++x) {
                        indx = ptr[i++];
                        indx1 = (indx - minvalue) * (ColormapSize-1) / (maxvalue - minvalue);
                        if(indx1 >= ColormapSize) indx1=ColormapSize -1;

                        *scanLine++ = ColorMap[indx1];

                        Max[(indx > Max[1])] = indx;
                        Min[(indx < Min[1])] = indx;
                    }
                    if(i >= datasize) break;
                }


            } else {

                for (int y = 0; y < resultSize.height(); ++y) {
                    uint *scanLine = reinterpret_cast<uint *>(image->scanLine(y));

                    for (int x = 0; x < resultSize.width(); ++x) {

                        indx=ptr[i++];
                        Max[(indx > Max[1])] = indx;
                        Min[(indx < Min[1])] = indx;
						indx1=indx * 255 /(maxvalue - minvalue);

						if(indx1 > 255) indx1 = 255;
                        *scanLine++ = qRgb(indx1,indx1,indx1);

                    }
                    if(i >= datasize) break;
                }
            }
        }
        break;

        default:
            break;
        } // end switch bpp

		break;
	}
        // color rgb image
    case 3:

        // start bpp switch
        switch (m_bpp) {

        case 3: // 3 bpp, each byte with r,g,b
            if(!m_forcemonochrome) {
                uchar *ptr = (uchar*)  data;
                if(ptr == (void*) 0) return (QImage *) 0;

                for (int y = 0; y < resultSize.height(); ++y) {
                    uint *scanLine = reinterpret_cast<uint *>(image->scanLine(y));
                    for (int x = 0; x < resultSize.width(); ++x) {
                        *scanLine++ = qRgb(ptr[i],ptr[i+1],ptr[i+2]);
                        i+=3;
                    }
                    if(i >= datasize) break;
                }
            } else {  //convert color to gray scale
                uchar *ptr = (uchar*)  data;
                if(ptr == (void*) 0) return (QImage *) 0;

                for (int y = 0; y < resultSize.height(); ++y) {
                    uint *scanLine = reinterpret_cast<uint *>(image->scanLine(y));
                    for (int x = 0; x < resultSize.width(); ++x) {
                        int average = (ptr[i] + ptr[i+1] + ptr[i+2]) / 3;
                        *scanLine++ = qRgb(average, average, average);
                        i+=3;
                    }
                    if(i >= datasize) break;
                }
            }
            break;

        default:
            break;

        } // end switch bpp

        break; // end code case 3

    default:
        break;
    } // end switch code

    minvalue = Min[1];
    maxvalue= Max[1];

    if(maxvalue == minvalue) {
        maxvalue = maxvalue +1;
        minvalue = minvalue -1;
        if(maxvalue > 65535) maxvalue = 65535;
        if(minvalue < 0) minvalue = 0;
    }

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
    return image;
}

void caCamera::showImage(int datasize, char *data)
{
/*
    struct timeb  timeNow;
    struct timeb  timeStart;
    double elapsedTime;
    ftime(&timeStart);
*/
    // up to now I did not really notice a difference with the routine in this thread
#ifndef QT_NO_CONCURRENT
    QFuture<QImage *> future = QtConcurrent::run(this, &caCamera::showImageCalc, datasize, data);
    image = future.result();
#else
    image = showImageCalc(datasize, data);
#endif
/*
    ftime(&timeNow);
    elapsedTime = ((double) timeNow.time + (double) timeNow.millitm / (double)1000) -
            ((double) timeStart.time + (double) timeStart.millitm / (double)1000);
    printf("elapsed %.3f\n", elapsedTime);
*/
    if(image != (QImage *) 0) updateImage(*image, valuesPresent, values);
}


uint caCamera::rgbFromWaveLength(double wave)
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


