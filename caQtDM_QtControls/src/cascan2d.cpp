/*
 *  This file is part of the caQtDM Framework, was developed  by Tim Mooney from Argonne,
 *  based on cacamera by Anton Chr. Mezger at the Paul Scherrer Institut,
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
 *    Tim Mooney (Argonne), Anton Mezger (PSI)
 *  Contact details:
 *    anton.mezger@psi.ch
 */

#include <QtGui>
#include <QApplication>
#include <math.h>
#include "cascan2d.h"
#include "cacamera.h"

caScan2D::caScan2D(QWidget *parent) : QWidget(parent)
{
    m_init = true;

    m_widthDefined = false;
    m_heightDefined = false;

    m_xcptDefined = false;
    m_ycptDefined = false;
    m_xnewdataDefined = false;
    m_ynewdataDefined = false;
    m_savedata_pathDefined = false;
    m_savedata_subdirDefined = false;
    m_savedata_filenameDefined = false;

    thisSimpleView = false;
    savedSize = 0;
    savedWidth = 0;
    savedHeight = 0;
    frameCount = 0;
    selectionStarted=false;

    savedData = (float*) 0;

    initWidgets();

    Xpos = Ypos = 0;

    scrollArea = (QScrollArea *) 0;

    mainLayout = new QGridLayout(this);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);
    setLayout(mainLayout);

    setup();

    buttonPressed = false;
    ROIdetected = false;
    thisROItype = centerxy_width_height;

    setColormap(spectrum_wavelength);
    setCustomMap("");
    setDiscreteCustomMap(false);

    setAccessW(true);
    installEventFilter(this);

    scaleFactor = 1.0;

    UpdatesPerSecond = 0;
    startTimer(1000);
}

bool caScan2D::isPropertyVisible(Properties property)
{
    return designerVisible[property];
}

void caScan2D::setPropertyVisible(Properties property, bool visible)
{
    designerVisible[property] = visible;
}


void caScan2D::deleteWidgets()
{
    if(image != (QImage *) 0)                    delete image;

    if(valuesLayout != (QHBoxLayout *) 0)        delete valuesLayout;
    if(labelMaxText != (caLabel *) 0)            delete labelMaxText;
    if(labelMinText != (caLabel *) 0)            delete labelMinText;
    if(labelMin != (caLineEdit *) 0)             delete labelMin;
    if(labelMax != (caLineEdit *) 0)             delete labelMax;
    if(checkAutoText != (caLabel *) 0)           delete checkAutoText;
    if(autoW != (QCheckBox *) 0)                 delete autoW;
    if(intensity != (caLabel *) 0)               delete intensity;
    if(intensityText != (caLabel *) 0)           delete intensityText;
    if(nbUpdatesText != (caLabel *) 0)           delete nbUpdatesText;

    if(zoomSliderLayout != ( QGridLayout *) 0)   delete zoomSliderLayout;
    if(zoomSlider != (QSlider *) 0)              delete zoomSlider;
    if(zoomValue != (caLabel *) 0)               delete zoomValue;
    if(zoomInIcon != (QToolButton *) 0)          delete zoomInIcon;
    if(zoomOutIcon != (QToolButton *) 0)         delete zoomOutIcon;

    if(imageW != (ImageWidget *) 0)              delete imageW;
    if(valuesWidget != (QWidget *) 0)            delete valuesWidget;
    if(scrollArea != (QScrollArea *) 0)          delete scrollArea;
    if(colormapWidget != (QwtScaleWidget *) 0)   delete colormapWidget;
    if(zoomWidget != (QWidget *) 0)              delete zoomWidget;
}

void caScan2D::initWidgets()
{
    image = (QImage *) 0;
    labelMin = (caLineEdit *) 0;
    labelMax = (caLineEdit *) 0;
    intensity = (caLabel *) 0;
    imageW = (ImageWidget *) 0;
    autoW = (QCheckBox *) 0;
    labelMaxText = (caLabel *) 0;
    labelMinText = (caLabel *) 0;
    intensityText = (caLabel *) 0;
    checkAutoText = (caLabel *) 0;
    nbUpdatesText = (caLabel *) 0;
    scrollArea = (QScrollArea *) 0;
    valuesWidget = (QWidget *) 0;
    zoomWidget = (QWidget *) 0;
    zoomSlider = (QSlider *) 0;
    zoomValue = (caLabel *) 0;
    zoomInIcon = (QToolButton *) 0;
    zoomOutIcon = (QToolButton *) 0;

    valuesLayout = (QHBoxLayout *) 0;
    zoomSliderLayout = ( QGridLayout *) 0;
    colormapWidget = (QwtScaleWidget *) 0;
}

caScan2D::~caScan2D()
{
    deleteWidgets();
    initWidgets();
}

void caScan2D::timerEvent(QTimerEvent *)
{
    QString text= "%1 U/s";
    text = text.arg(UpdatesPerSecond);
    //if(nbUpdatesText != (caLabel*) 0) nbUpdatesText->setText(text);
    UpdatesPerSecond = 0;
}

bool caScan2D::getROI(int &x, int &y, int &w, int &h)
{
    x = ROIx;
    y = ROIy;
    w = ROIw;
    h = ROIh;

    printf("%d %d %d %d\n", x,y,w,h);

    // rectangle always from upper left corner
    if(ROIw < 0) {
        x = ROIx + ROIw;
        w = -ROIw;
    }
    if(ROIh < 0) {
        y = ROIy + ROIh;
        h = -ROIh;
    }

    return ROIdetected;
}

void caScan2D::Coordinates(int posX, int posY, double &newX, double &newY, double &maxX, double &maxY)
{
    // calculate true x, y values and limits of picture
    int width, height;
    imageW->getImageDimensions(width, height);
    double Xcorr = (double) width / (double) savedWidth;
    double Ycorr = (double) height / (double) savedHeight;

    double Correction = qMin(Xcorr, Ycorr); // aspect ratio
    if(scaleFactor < 1.0) Correction =  scaleFactor;

    maxX = width / Correction;
    maxY = height / Correction;
    maxX = qMin(savedWidth,  (int) maxX);
    maxY = qMin(savedHeight,  (int) maxY);
    newX = (posX  + scrollArea->horizontalScrollBar()->value()) / Correction;
    newY = (posY  + scrollArea->verticalScrollBar()->value()) / Correction;
}

bool caScan2D::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);

    if(thisSimpleView) return false;

    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if(mouseEvent->button() == Qt::LeftButton) {
            buttonPressed = true;
            ROIdetected = false;
            Xpos = mouseEvent->pos().x();
            Ypos = mouseEvent->pos().y();
            Ypos = Ypos - valuesWidget->height();

            QApplication::setOverrideCursor(QCursor(Qt::CrossCursor));

            selectionStarted=true;
            QPoint mouseOffset = mouseEvent->pos() ;
            mouseOffset.setY(mouseOffset.y() - valuesWidget->height() + scrollArea->verticalScrollBar()->value());
            mouseOffset.setX(mouseOffset.x() + scrollArea->horizontalScrollBar()->value());

            selectionRect.setTopLeft(mouseOffset);
            selectionRect.setBottomRight(mouseOffset);
        }
    }
    if (event->type() == QEvent::MouseButtonRelease) {
        double Xnew1, Ynew1, Xnew2, Ynew2, Xmax, Ymax;
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if(mouseEvent->button() == Qt::LeftButton) {
            buttonPressed = false;
            QApplication::restoreOverrideCursor();

            if(selectionStarted) {
                selectionStarted=false;
                if(qAbs(selectionRect.width()) > 5 &&  qAbs(selectionRect.height()) > 5) {
                    int x1, y1, x2, y2;
                    selectionRect.getCoords(&x1, &y1, &x2, &y2);
                    x1 = x1 - scrollArea->horizontalScrollBar()->value();
                    y1 = y1 - scrollArea->verticalScrollBar()->value();
                    x2 = x2 - scrollArea->horizontalScrollBar()->value();
                    y2 = y2 - scrollArea->verticalScrollBar()->value();
                    selectionRect.setCoords(x1, y1, x2, y2);
                    Coordinates( selectionRect.x(), selectionRect.y(),  Xnew1, Ynew1, Xmax, Ymax);
                    Coordinates( selectionRect.x() + selectionRect.width(), selectionRect.y() + selectionRect.height(),  Xnew2, Ynew2, Xmax, Ymax);
                    ROIx = (int) (Xnew1+0.5);
                    ROIy = (int) (Ynew1+0.5);
                    ROIw = (int) (Xnew2 - Xnew1);
                    ROIh = (int) (Ynew2 - Ynew1);
                    ROIdetected = true;
                }
                refreshImage();
            }
        }
    }
    if (event->type() == QEvent::MouseMove) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        Xpos = mouseEvent->pos().x();
        Ypos = mouseEvent->pos().y();
        Ypos = Ypos - valuesWidget->height();

        QPoint mouseOffset = mouseEvent->pos() ;
        mouseOffset.setY(mouseOffset.y() - valuesWidget->height() + scrollArea->verticalScrollBar()->value());
        mouseOffset.setX(mouseOffset.x() + scrollArea->horizontalScrollBar()->value());

        selectionRect.setBottomRight(mouseOffset);
        refreshImage();
    }

    if(buttonPressed && (savedData != (float*) 0)) {
        double Xnew, Ynew, Xmax, Ymax;
        validIntensity = true;

        Coordinates(Xpos, Ypos, Xnew, Ynew, Xmax, Ymax);

        float *ptr = (float*)  savedData;
        int index = (int) ((int) Ynew * savedWidth + (int) Xnew);
        if((Xnew >=0) && (Ynew >=0)  && (Xnew < Xmax) && (Ynew < Ymax) && (index < savedSize)) {
            Zvalue=ptr[index];
        } else {
            validIntensity = false;
        }

        QString strng = "%1, %2, %3";
        if(validIntensity) {
            strng = strng.arg((int) Xnew).arg((int) Ynew).arg(Zvalue);
            updateIntensity(strng);
        } else {
            updateIntensity("invalid");
        }
    }
    return false;
}

void caScan2D::setup()
{
    deleteWidgets();
    initWidgets();
    // labels and texts for horizontal layout containing information of the image
    // image inside a scrollarea
    // zoom utilities

    // labels
    if(!thisSimpleView) {
        labelMaxText = new caLabel(this);
        labelMaxText->setText(" Max: ");
        labelMinText = new caLabel(this);
        labelMinText->setText(" Min: ");
        checkAutoText = new caLabel(this);
        checkAutoText->setText(" Auto: ");
        intensityText = new caLabel(this);
        intensityText->setText(" x/y/z: ");

        nbUpdatesText = new caLabel(this);

        // texts
        labelMax = new caLineEdit(this);
        labelMin = new caLineEdit(this);

        labelMax->newFocusPolicy(Qt::ClickFocus);
        labelMin->newFocusPolicy(Qt::ClickFocus);

        intensity = new caLabel(this);

        intensity->setAlignment(Qt::AlignVCenter | Qt::AlignLeft );
        labelMaxText->setScaleMode(caLabel::None);
        labelMinText->setScaleMode(caLabel::None);
        checkAutoText->setScaleMode(caLabel::None);
        intensity->setScaleMode(caLabel::None);
        intensityText->setScaleMode(caLabel::None);
        nbUpdatesText->setScaleMode(caLabel::None);
        QFont font = labelMaxText->font();
        font.setPointSize(10);
        labelMaxText->setFont(font);
        labelMinText->setFont(font);
        checkAutoText->setFont(font);
        intensity->setFont(font);
        intensityText->setFont(font);
        nbUpdatesText->setFont(font);
        labelMaxText->setBackground(QColor(0,0,0,0));
        labelMinText->setBackground(QColor(0,0,0,0));
        checkAutoText->setBackground(QColor(0,0,0,0));
        intensity->setBackground(QColor(0,0,0,0));
        intensityText->setBackground(QColor(0,0,0,0));
        nbUpdatesText->setBackground(QColor(0,0,0,0));

        // checkbox
        autoW = new QCheckBox(this);
        autoW->setChecked(true);

        // add everything to layout
        valuesLayout = new QHBoxLayout();
        valuesLayout->setMargin(0);
        valuesLayout->setSpacing(2);
        valuesLayout->addWidget(labelMinText, Qt::AlignLeft);
        valuesLayout->addWidget(labelMin, Qt::AlignLeft);
        valuesLayout->addWidget(labelMaxText, Qt::AlignLeft);
        valuesLayout->addWidget(labelMax, Qt::AlignLeft);
        valuesLayout->addWidget(checkAutoText, Qt::AlignLeft);
        valuesLayout->addWidget(autoW,    Qt::AlignLeft);
        valuesLayout->addWidget(intensityText, Qt::AlignLeft);
        valuesLayout->addWidget(intensity, Qt::AlignLeft);
        valuesLayout->addWidget(nbUpdatesText, Qt::AlignLeft);
        valuesLayout->addStretch(2);

        valuesWidget = new QWidget;
        valuesWidget->setLayout(valuesLayout);
        valuesWidget->show();

        // image inside a scrollarea
        imageW   = new ImageWidget();
        scrollArea = new QScrollArea;
        scrollArea->setBackgroundRole(QPalette::Dark);
        scrollArea->setWidget(imageW);
        scrollArea->setWidgetResizable(true);

        // add some zoom utilities to our widget
        int iconsize = style()->pixelMetric(QStyle::PM_ToolBarIconSize);
        QSize iconSize(iconsize, iconsize);

        zoomInIcon = new QToolButton;
        zoomInIcon->setIcon(QPixmap(":pixmaps/zoomin.png"));
        zoomInIcon->setIconSize(iconSize);

        zoomOutIcon = new QToolButton;
        zoomOutIcon->setIcon(QPixmap(":pixmaps/zoomout.png"));
        zoomOutIcon->setIconSize(iconSize);

        zoomSlider = new QSlider;
        zoomSlider->setMinimum(0);
        //zoomSlider->setMaximum(90); // do not exceed 6*
        zoomSlider->setMaximum(120); // do not exceed 6*
        zoomSlider->setValue(52);
        zoomSlider->setTickPosition(QSlider::NoTicks);

        zoomValue = new QLabel("");
        zoomValue->setFixedWidth(60);

        colormapWidget = new QwtScaleWidget();
        colormapWidget->setColorBarEnabled(true);
        colormapWidget->setHidden(true);
#if QWT_VERSION >= 0x060100
        QwtScaleDiv *div = new QwtScaleDiv(0.0, 1.0);
        colormapWidget->setScaleDiv(*div);
#endif

        // add everything to layout
        zoomSliderLayout = new QGridLayout();
        zoomSliderLayout->setSpacing(0);
        zoomSliderLayout->addWidget(zoomInIcon, 0, 1);
        zoomSliderLayout->addWidget(zoomSlider, 1, 1);
        zoomSliderLayout->addWidget(zoomOutIcon, 2, 1);
        zoomSliderLayout->addWidget(zoomValue, 3, 1);
        zoomSliderLayout->addWidget(colormapWidget,0,0,4,1);

        zoomWidget = new QWidget;
        zoomWidget->setLayout(zoomSliderLayout);

        // connect buttons and slider
        connect(zoomInIcon, SIGNAL(clicked()), this, SLOT(zoomIn()));
        connect(zoomOutIcon, SIGNAL(clicked()), this, SLOT(zoomOut()));
        connect(zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(zoomNow()));

        // add everything to main layout
        mainLayout->addWidget(valuesWidget, 0, 0);
        mainLayout->addWidget(scrollArea, 1, 0);
        mainLayout->addWidget(zoomWidget, 1, 2);

        for(int i=0; i<4; i++) valuesPresent[i] = false;

        updateMin(0);
        updateMax(0);
    } else {
        imageW   = new ImageWidget();
        // add to main layout
        mainLayout->addWidget(imageW, 0, 0);
        thisFitToSize = Yes;
    }
}

void caScan2D::zoomNow()
{
    double scale = qPow(2.0, ((double) zoomSlider->value() - 52.0) / 13.0);
    if(scale > 32) scale = 32;
    zoomValue->setText(QString::number(scale, 'f', 3));
    scaleFactor = scale;
    setFitToSize(No);
}

void caScan2D::zoomIn(int level)
{
    zoomSlider->setValue(zoomSlider->value() + level);
}

void caScan2D::zoomOut(int level)
{
    zoomSlider->setValue(zoomSlider->value() - level);
}

void caScan2D::setFitToSize(zoom const &z)
{
    if(thisSimpleView) return;
    thisFitToSize = z;

    if(!thisFitToSize) {
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    } else {
        //scaleFactor = 1.0;
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff );
        scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

    // force resize
    QResizeEvent *re = new QResizeEvent(size(), size());
    resizeEvent(re);
    delete re;
}

bool caScan2D::getInitialAutomatic()
{
    if(thisSimpleView) return thisInitialAutomatic;
    if(autoW == (QCheckBox *) 0) return false;
    return autoW->isChecked();
}

void caScan2D::setInitialAutomatic(bool automatic)
{
    if(thisSimpleView) thisInitialAutomatic = automatic;
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
    bool ok;
    (void) thisMaxLevel.toInt(&ok);
    return !ok;
}
bool caScan2D::isAlphaMinLevel()
{
    bool ok;
    (void) thisMinLevel.toInt(&ok);
    return !ok;
}

void caScan2D::setColormap(colormap const &map)
{
    thisColormap = map;
    colorMaps colormaps;
    setPropertyVisible(customcolormap, false);
    setPropertyVisible(discretecolormap, false);

    switch (map) {

    case grey:
        colormaps.getColormap(colorMaps::grey, false, thisCustomMap, ColormapSize, ColorMap, colormapWidget);
        break;
    case spectrum_wavelength:
        colormaps.getColormap(colorMaps::spectrum_wavelength, false, thisCustomMap, ColormapSize, ColorMap, colormapWidget);
        break;
    case spectrum_hot:
        colormaps.getColormap(colorMaps::spectrum_hot, false, thisCustomMap, ColormapSize, ColorMap, colormapWidget);
        break;
    case spectrum_heat:
        colormaps.getColormap(colorMaps::spectrum_heat, false, thisCustomMap, ColormapSize, ColorMap, colormapWidget);
        break;
    case spectrum_jet:
        colormaps.getColormap(colorMaps::spectrum_jet, false, thisCustomMap, ColormapSize, ColorMap, colormapWidget);
        break;
    case spectrum_custom:
        setPropertyVisible(customcolormap, true);
        setPropertyVisible(discretecolormap, true);
        // user has the possibility to input its own colormap with discrete QtColors from 2 t0 18
        // when nothing given, fallback to default colormap
        if(thisCustomMap.count() > 2) {
            colormaps.getColormap(colorMaps::spectrum_custom, thisDiscreteMap, thisCustomMap, ColormapSize, ColorMap, colormapWidget);
        } else {
            colormaps.getColormap(colorMaps::spectrum_wavelength, false, thisCustomMap, ColormapSize, ColorMap, colormapWidget);
        }
        break;
    default:
        colormaps.getColormap(colorMaps::grey, false, thisCustomMap, ColormapSize, ColorMap, colormapWidget);
        break;
    }
    // force resize
    if(zoomWidget != (QWidget*) 0) zoomWidget->adjustSize();
    QResizeEvent *re = new QResizeEvent(size(), size());
    resizeEvent(re);
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
    if(thisSimpleView) {
        if(m_widthDefined && m_heightDefined) {
            showImage(m_width, m_height);
        }
        return;
    }

    if(m_widthDefined && m_heightDefined) {
        if(!thisFitToSize) {
            imageW->setMinimumSize((int) (m_width * scaleFactor), (int) (m_height * scaleFactor));
            showImage((int) (m_width * scaleFactor), (int) (m_height * scaleFactor));
        } else {
            double Xcorr = (double) (e->size().width() - zoomWidget->width()-4) / (double) savedWidth;
            double Ycorr = (double) (e->size().height()- valuesWidget->height()-4) / (double) savedHeight;
            double scale = qMin(Xcorr, Ycorr); // aspect ratio
            // disconnect signal to prevent firing now
            disconnect(zoomSlider, SIGNAL(valueChanged (int)), 0, 0);
            zoomSlider->setValue((int)(13.0*log(scale)/log(2.0)+52.0));
            zoomValue->setText(QString::number(scale, 'f', 3));
            connect(zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(zoomNow()));
            imageW->setFixedWidth(e->size().width() - zoomWidget->width()-4);
            imageW->setFixedHeight(e->size().height()- valuesWidget->height()-4);
            scaleFactor = scale;
            showImage(m_width, m_height);
        }
    }
}

void caScan2D::refreshImage()
{
    if(m_widthDefined && m_heightDefined) {
        if(!thisFitToSize) {
            imageW->setMinimumSize((int) (m_width * scaleFactor), (int) (m_height * scaleFactor));
            showImage((int) (m_width * scaleFactor), (int) (m_height * scaleFactor));
        } else {
            showImage((int) (m_width), (int) (m_height));
        }
    }
}

void caScan2D::updateImage(const QImage &image, bool valuesPresent[], int values[], const double &scaleFactor)
{
    imageW->updateImage(thisFitToSize, image, valuesPresent, values, scaleFactor, selectionStarted, selectionRect, thisSimpleView);
}

void caScan2D::showDisconnected()
{
    imageW->updateDisconnected();
}


bool caScan2D::getAutomateChecked()
{
    if(thisSimpleView) {
        return thisInitialAutomatic;
    }
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

void caScan2D::updateIntensity(QString strng)
{
    if(intensity == (caLabel*) 0) return;
    intensity->setText(strng);
}

int caScan2D::getMin()
{
    bool ok;
    if(thisSimpleView) return  thisMinLevel.toInt(&ok);
    if(labelMin == (caLineEdit*) 0) return 0;
    return labelMin->text().toInt();
}
int caScan2D::getMax()
{
    bool ok;
    if(thisSimpleView) return  thisMaxLevel.toInt(&ok);
    if(labelMax == (caLineEdit*) 0) return 65535;
    return labelMax->text().toInt();
}

void caScan2D::dataProcessing(int value, int id)
{

    if(id < 0 || id > 3) return;
    valuesPresent[id] = true;
    values[id] = value;

    refreshImage();
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
    savedData = xdata;
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

    updateImage(*image, valuesPresent, values, scaleFactor);
}

void caScan2D::setAccessW(int access)
{
    _AccessW = access;
}



