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
 *
 * camera color modes tested with the following hardware:
 * Basler acA4600-10uc
 * Basler acA1300-30gc
 * Prosilica GC1660C
 *
 *
 *
 */
#define QWT_DLL
#include <stdint.h>
#include <QtGui>
#include<QApplication>
#include <math.h>
#ifndef QT_NO_CONCURRENT
#include <qtconcurrentrun.h>
#include <QFutureSynchronizer>
#endif
#include "cacamera.h"

#define ZLIB_BYTE Bytef
#define ZLIB_ULONG uLongf

#if defined(_MSC_VER)
#include <QtZlib/zconf.h>
#include <QtZlib/zlib.h>
#endif

#if defined(linux)|| defined TARGET_OS_MAC
#include <zconf.h>
#include <zlib.h>
#endif

// Clamp out of range values
#define CLAMP(t) (((t)>255)?255:(((t)<0)?0:(t)))

//#include "ittnotify.h"

char caTypeStr[7][20] = {"caSTRING", "caINT", "caFLOAT", "caENUM", "caCHAR", "caLONG", "caDOUBLE"};

caCamera::caCamera(QWidget *parent) : QWidget(parent)
{
    m_init = true;
    m_widthDefined = false;
    m_heightDefined = false;
    m_datatype = -1;
    m_MinLevel = -1;
    m_MaxLevel = -1;
    m_zoom_value = 52; //value see widget init
    m_verticalScroll = -1;
    m_horizontalScroll = -1;

    rgb = (uint*)Q_NULLPTR;

    thisSimpleView = false;
    thisShowBoxes = false;
    thisFitToSize = No;
    savedSize = 0;
    savedWidth = 0;
    savedHeight = 0;
    selectionInProgress = false;

    thisPV_Xaverage = "";
    thisPV_Yaverage = "";
    thisPV_Mode = "";
    thisPV_Packing = "";

    savedData = (char*)Q_NULLPTR;
    initWidgets();

    Xpos = Ypos = 0;

    scrollArea = (QScrollArea *)Q_NULLPTR;

    mainLayout = new QGridLayout(this);
    SETMARGIN_QT456(mainLayout,0);
    mainLayout->setSpacing(0);
    setLayout(mainLayout);

    setColormodeStrings();
    setPackingModeStrings();
    setCompressionModeStrings();
    setup();

    setColormode(Mono);
    setPackmode(packNo);

    setColormap(as_is);
    setCustomMap("");
    setDiscreteCustomMap(false);

    setROIChannelsRead("");
    setROIChannelsWrite("");

    setROIreadType(none);
    setROIwriteType(none);

    setROIwritemarkerType(box);
    setROIreadmarkerType(box);

    setAccessW(true);
    installEventFilter(this);

    scaleFactor = 1.0;

    UpdatesPerSecond = 0;

    thisRedCoefficient = 1.0;
    thisGreenCoefficient = 1.0;
    thisBlueCoefficient = 1.0;

    thisCompressionmode=non;
    startTimer(1000);

    // __itt_thread_set_name("My worker thread");

    writeTimer = new QTimer(this);
    connect(writeTimer, SIGNAL(timeout()), this, SLOT(updateChannels()));
}

void caCamera::setColormodeStrings()
{
    colorModeString <<  "Mono"<< "Mono12p" << "Mono10p" << "Mono10Packed"<< "Mono8" << "RGB1_CA" << "RGB2_CA" << "RGB3_CA" << "BayerRG_8" << "BayerGB_8" << "BayerGR_8" << "BayerBG_8" <<
                        "BayerRG_12" << "BayerGB_12" << "BayerGR_12" << "BayerBG_12" <<
                        "RGB_8" << "BGR_8" << "RGBA_8" << "BGRA_8" <<
                        "YUV444" << "YUV422"<< "YUV411" << "YUV421";
}

void caCamera::setPackingModeStrings()
{
    packingModeString <<  "packNo" << "MSB12Bit" <<  "LSB12Bit" << "Reversed";
}

void caCamera::setCompressionModeStrings()
{
#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
    compressionModeString <<  "non" << "Zlib";
#else
    compressionModeString <<  "non" << "Zlib" << "JPG";
#endif
}

void caCamera::setDecodemodeStr(QString mode)
{
    if(mode.length() == 0) return;
    //printf("colormodeset with %s\n", qasc(mode));
    for(int i = 0; i< colorModeString.count(); i++) {
        if(mode == colorModeString.at(i)) {
            thisColormode = (colormode) i;
            if(colormodeCombo != (QComboBox*)Q_NULLPTR) colormodeCombo->setCurrentIndex(thisColormode);
            m_init = true;
        }
    }
}

bool caCamera::testDecodemodeStr(QString mode)
{
    if(mode.length() == 0) return false;
    //printf("colormodeset with %s\n", qasc(mode));
    for(int i = 0; i< colorModeString.count(); i++) {
        if(mode == colorModeString.at(i)) {
            return true;
        }
    }
    return false;
}

void caCamera::setPackingmodeStr(QString mode)
{
    if(mode.length() == 0) return;
    //printf("Packingmodeset with %s\n", qasc(mode));
    for(int i = 0; i< packingModeString.count(); i++) {
        if(mode == packingModeString.at(i)) {
            thisPackingmode = (packingmode) i;
            if(packingmodeCombo != (QComboBox*)Q_NULLPTR) packingmodeCombo->setCurrentIndex(thisPackingmode);
        }
    }
}

bool caCamera::testPackingmodeStr(QString mode)
{
    if(mode.length() == 0) return false;
    //printf("Packingmodeset with %s\n", qasc(mode));
    for(int i = 0; i< packingModeString.count(); i++) {
        if(mode == packingModeString.at(i)) {
            return true;
        }
    }
    return false;
}

void caCamera::setCompressionmodeStr(QString mode)
{
    if(mode.length() == 0) return;
    //printf("Packingmodeset with %s\n", qasc(mode));
    for(int i = 0; i< compressionModeString.count(); i++) {
        if(mode == compressionModeString.at(i)) {
            thisCompressionmode = (compressionmode) i;
            if(compressionmodeCombo != (QComboBox*)Q_NULLPTR) compressionmodeCombo->setCurrentIndex(thisCompressionmode);
        }
    }
}

bool caCamera::testCompressionmodeStr(QString mode)
{
    if(mode.length() == 0) return false;
    //printf("Packingmodeset with %s\n", qasc(mode));
    for(int i = 0; i< compressionModeString.count(); i++) {
        if(mode == compressionModeString.at(i)) {
            return true;
        }
    }
    return false;
}

void caCamera::setDecodemodeNum(int mode)
{
    //printf("colormodeset with %d\n", mode);
    m_init = true;
    thisColormode = (colormode) mode;
    if(colormodeCombo != (QComboBox*)Q_NULLPTR) colormodeCombo->setCurrentIndex(thisColormode);
}

void caCamera::setDecodemodeNum(double mode)
{
    //printf("colormodeset with %d\n", (int) mode);
    int intermed = (int)mode;
    m_init = true;
    thisColormode = (colormode) intermed; // direct not allowed on Windows (C2440)
    if(colormodeCombo != (QComboBox*)Q_NULLPTR)
        if (colormodeCombo->count()<thisColormode)
            colormodeCombo->setCurrentIndex(thisColormode);
}

void caCamera::setPackingmodeNum(int mode)
{
    //printf("packingmodeset with %d\n", mode);
    thisPackingmode = (packingmode) mode;
    if(packingmodeCombo != (QComboBox*)Q_NULLPTR) packingmodeCombo->setCurrentIndex(thisPackingmode);
}

void caCamera::setPackingmodeNum(double mode)
{
    //printf("packingmodeset with %d\n", (int) mode);
    int intermed = (int)mode;
    thisPackingmode = (packingmode) intermed;// direct not allowed on Windows (C2440)
    if(packingmodeCombo != (QComboBox*)Q_NULLPTR)
        if (packingmodeCombo->count()<thisPackingmode)
            packingmodeCombo->setCurrentIndex(thisPackingmode);
}

void caCamera::setCompressionmodeNum(int mode)
{
    //printf("compressionmodeset with %d\n", mode);
    thisCompressionmode = (compressionmode) mode;
    if(compressionmodeCombo != (QComboBox*)Q_NULLPTR)
        if (compressionmodeCombo->count()<thisCompressionmode)
            compressionmodeCombo->setCurrentIndex(thisCompressionmode);
}

void caCamera::setCompressionmodeNum(double mode)
{
    //printf("compressionmodeset with %d\n", (int) mode);
    int intermed = (int)mode;
    thisCompressionmode = (compressionmode) intermed;
    if(compressionmodeCombo != (QComboBox*)Q_NULLPTR)
       if (compressionmodeCombo->count()<thisCompressionmode)
            compressionmodeCombo->setCurrentIndex(thisCompressionmode);
}



bool caCamera::isPropertyVisible(Properties property)
{
    return designerVisible[property];
}

void caCamera::setPropertyVisible(Properties property, bool visible)
{
    designerVisible[property] = visible;
}

void caCamera::deleteWidgets()
{
    if(image != (QImage *)Q_NULLPTR)                    delete image;

    if(valuesLayout != (QHBoxLayout *)Q_NULLPTR)        delete valuesLayout;
    if(colormodeLayout != (QHBoxLayout *)Q_NULLPTR)     delete colormodeLayout;
    if(labelMaxText != (caLabel *)Q_NULLPTR)            delete labelMaxText;
    if(labelMinText != (caLabel *)Q_NULLPTR)            delete labelMinText;
    if(labelMin != (caLineEdit *)Q_NULLPTR)             delete labelMin;
    if(labelMax != (caLineEdit *)Q_NULLPTR)             delete labelMax;
    if(labelColormodeText != (caLabel *)Q_NULLPTR)      delete labelColormodeText;
    if(labelPackingmodeText != (caLabel *)Q_NULLPTR)    delete labelPackingmodeText;
    if(labelCompressionmodeText != (caLabel *)Q_NULLPTR)    delete labelCompressionmodeText;


    if(colormodeCombo != (QComboBox *)Q_NULLPTR)        delete colormodeCombo;
    if(packingmodeCombo != (QComboBox *)Q_NULLPTR)      delete packingmodeCombo;
    if(compressionmodeCombo != (QComboBox *)Q_NULLPTR)  delete compressionmodeCombo;


    if(checkAutoText != (caLabel *)Q_NULLPTR)           delete checkAutoText;
    if(autoW != (QCheckBox *)Q_NULLPTR)                 delete autoW;
    if(intensity != (caLabel *)Q_NULLPTR)               delete intensity;
    if(intensityText != (caLabel *)Q_NULLPTR)           delete intensityText;
    if(nbUpdatesText != (caLabel *)Q_NULLPTR)           delete nbUpdatesText;

    if(zoomSliderLayout != ( QGridLayout *)Q_NULLPTR)   delete zoomSliderLayout;
    if(zoomSlider != (QSlider *)Q_NULLPTR)              delete zoomSlider;
    if(zoomValue != (caLabel *)Q_NULLPTR)               delete zoomValue;
    if(zoomInIcon != (QToolButton *)Q_NULLPTR)          delete zoomInIcon;
    if(zoomOutIcon != (QToolButton *)Q_NULLPTR)         delete zoomOutIcon;

    if(imageW != (ImageWidget *)Q_NULLPTR)              delete imageW;
    if(valuesWidget != (QWidget *)Q_NULLPTR)            delete valuesWidget;
    if(colormodesWidget != (QWidget *)Q_NULLPTR)        delete colormodesWidget;
    if(scrollArea != (QScrollArea *)Q_NULLPTR)          delete scrollArea;
    if(colormapWidget != (QwtScaleWidget *)Q_NULLPTR)   delete colormapWidget;
    if(zoomWidget != (QWidget *)Q_NULLPTR)              delete zoomWidget;
}

void caCamera::initWidgets()
{
    image = (QImage *)Q_NULLPTR;
    labelMin = (caLineEdit *)Q_NULLPTR;
    labelMax = (caLineEdit *)Q_NULLPTR;
    intensity = (caLabel *)Q_NULLPTR;
    imageW = (ImageWidget *)Q_NULLPTR;
    autoW = (QCheckBox *)Q_NULLPTR;
    labelMaxText = (caLabel *)Q_NULLPTR;
    labelMinText = (caLabel *)Q_NULLPTR;
    labelColormodeText = (caLabel *)Q_NULLPTR;
    labelPackingmodeText = (caLabel *)Q_NULLPTR;
    labelCompressionmodeText = (caLabel *)Q_NULLPTR;
    colormodeCombo = (QComboBox *)Q_NULLPTR;
    packingmodeCombo = (QComboBox *)Q_NULLPTR;
    compressionmodeCombo = (QComboBox *)Q_NULLPTR;

    intensityText = (caLabel *)Q_NULLPTR;
    checkAutoText = (caLabel *)Q_NULLPTR;
    nbUpdatesText = (caLabel *)Q_NULLPTR;
    scrollArea = (QScrollArea *)Q_NULLPTR;
    valuesWidget = (QWidget *)Q_NULLPTR;
    colormodesWidget = (QWidget *)Q_NULLPTR;
    zoomWidget = (QWidget *)Q_NULLPTR;
    zoomSlider = (QSlider *)Q_NULLPTR;
    zoomValue = (caLabel *)Q_NULLPTR;
    zoomInIcon = (QToolButton *)Q_NULLPTR;
    zoomOutIcon = (QToolButton *)Q_NULLPTR;

    valuesLayout = (QHBoxLayout *)Q_NULLPTR;
    colormodeLayout = (QHBoxLayout *)Q_NULLPTR;
    zoomSliderLayout = ( QGridLayout *)Q_NULLPTR;
    colormapWidget = (QwtScaleWidget *)Q_NULLPTR;
}

caCamera::~caCamera()
{
    deleteWidgets();
    initWidgets();
}

void caCamera::timerEvent(QTimerEvent *)
{
    QString text= "%1 U/s (%2,%3)";
    if(m_datatype >=0) text = text.arg(UpdatesPerSecond).arg(colorModeString.at(thisColormode)).arg(caTypeStr[m_datatype]);
    else  text = text.arg(UpdatesPerSecond).arg(colorModeString.at(thisColormode)).arg("");
    if(nbUpdatesText != (caLabel*)Q_NULLPTR) nbUpdatesText->setText(text);
    UpdatesPerSecond = 0;
}

void caCamera::getROI(QPointF &p1, QPointF &p2)
{
    p1 = P1;
    p2 = P2;
}

void caCamera::Coordinates(int posX, int posY, double &newX, double &newY, double &maxX, double &maxY)
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

void caCamera::updateChannels()
{
    if((P1 != P1_old) || (P2 != P2_old)) {
        P1_old = P1;
        P2_old = P2;
        emit WriteDetectedValuesSignal(this);
    }
}

template <typename pureData> int caCamera::zValueImage(pureData *ptr, colormode mode, double xnew, double ynew,
                                                       double xmax, double ymax, int datasize, bool &validIntensity)
{
    int Zvalue = 0;
    validIntensity = true;
    if(mode == Mono) {
        int index = ((int) ynew * savedWidth + (int) xnew);
        if((xnew >=0) && (ynew >=0) && (xnew < xmax) && (ynew < ymax) && (index < datasize)) {
            Zvalue = ptr[index];
        } else {
            validIntensity = false;
        }
    } else {
        Zvalue = -1;
    }
    return Zvalue;
}

bool caCamera::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);

    if(thisSimpleView) return false;
    buttonPressed = false;

    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if(mouseEvent->button() == Qt::LeftButton) {
            double Xnew, Ynew, Xmax, Ymax;
            int x1, y1;
            buttonPressed = true;
            selectionInProgress = true;
            imageW->initSelectionBox(scaleFactor);

            writeTimer->start(200);

            Xpos = mouseEvent->pos().x();
            Ypos = mouseEvent->pos().y();
            Ypos = Ypos - valuesWidget->height();

            QApplication::setOverrideCursor(QCursor(Qt::CrossCursor));

            Coordinates(Xpos, Ypos,  Xnew, Ynew, Xmax, Ymax);
            P3 = P1 = QPointF(Xnew, Ynew);
            P1_old = QPointF(-1, -1);
            P2_old = QPointF(-1, -1);

            // for gray selection rectangle on image
            QPoint mouseOffset = mouseEvent->pos() ;
            x1 = mouseOffset.x() + scrollArea->horizontalScrollBar()->value();
            y1 = mouseOffset.y() - valuesWidget->height() + scrollArea->verticalScrollBar()->value();
            selectionPoints[0] = selectionPoints[1] = QPoint(x1,y1);
        }
    }

    else if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if(mouseEvent->button() == Qt::LeftButton) {
            buttonPressed = false;
            selectionInProgress = false;
            QApplication::restoreOverrideCursor();
            imageW->updateSelectionBox(selectionPoints, selectionInProgress);
        }
    }

    else if (event->type() == QEvent::MouseMove) {
        double Xnew, Ynew, Xmax, Ymax;
        int x1, y1;
        buttonPressed = true;
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        // for further calculation of x/y/z display
        Xpos = mouseEvent->pos().x();
        Ypos = mouseEvent->pos().y();
        Ypos = Ypos - valuesWidget->height();

        Coordinates(Xpos, Ypos,  Xnew, Ynew, Xmax, Ymax);

        if(getROIwriteType() != xy_only) {
            P2 = QPointF(Xnew, Ynew);
            P3 = (P1 + P2)/2;

            // for gray selection rectangle
            QPoint mouseOffset = mouseEvent->pos() ;
            x1 = mouseOffset.x() + scrollArea->horizontalScrollBar()->value();
            y1 = mouseOffset.y() - valuesWidget->height() + scrollArea->verticalScrollBar()->value();
            selectionPoints[1] = QPoint(x1, y1);
        } else {
            P3 = P1 = P2 = QPointF(Xnew, Ynew);
        }
    }

    if(buttonPressed) imageW->updateSelectionBox(selectionPoints, selectionInProgress);

    if(buttonPressed && (savedData != (char*)Q_NULLPTR)) {
        double Xnew, Ynew, Xmax, Ymax;
        bool validIntensity = true;
        int Zvalue = 0;

        Coordinates(Xpos, Ypos, Xnew, Ynew, Xmax, Ymax);

        // find intensity
        switch (m_datatype) {
        case caCHAR:
            Zvalue = zValueImage((uchar*) savedData, thisColormode, Xnew, Ynew, Xmax, Ymax, savedSizeNew, validIntensity);
            break;
        case caINT:
            Zvalue = zValueImage((ushort*) savedData, thisColormode, Xnew, Ynew, Xmax, Ymax, savedSizeNew/2, validIntensity);
            break;
        case caLONG:
            Zvalue = zValueImage((uint*) savedData, thisColormode, Xnew, Ynew, Xmax, Ymax, savedSizeNew/4, validIntensity);
            break;
        case caFLOAT:
            Zvalue = zValueImage((float*) savedData, thisColormode, Xnew, Ynew, Xmax, Ymax, savedSizeNew/4, validIntensity);
            break;
        case caDOUBLE:
            Zvalue = zValueImage((double*) savedData, thisColormode, Xnew, Ynew, Xmax, Ymax, savedSizeNew/8, validIntensity);
            break;
        default:
            break;
        }

        if(validIntensity) {
            if(Zvalue >=0) {
                QString strng = "(%1,%2,%3)";
                strng = strng.arg(int(Xnew)).arg(int(Ynew)).arg(Zvalue);
                updateIntensity(strng);
            } else {
                QString strng = "(%1,%2)";
                strng = strng.arg(int(Xnew)).arg(int(Ynew));
                updateIntensity(strng);
            }
        } else {
            updateIntensity("invalid");
        }
    }

    return false;
}

void caCamera::colormodeComboSlot(int num)
{
    setDecodemodeNum(num);
}

void caCamera::packingmodeComboSlot(int num) {
    setPackingmodeNum(num);
}

void caCamera::compressionmodeComboSlot(int num) {
    setCompressionmodeNum(num);
}


void caCamera::setup()
{
    deleteWidgets();
    initWidgets();
    // labels and texts for horizontal layout containing information of the image
    // image inside a scrollarea
    // zoom utilities

    if(!thisSimpleView) {
        labelMaxText = new caLabel(this);
        labelMaxText->setText(" Max: ");
        labelMinText = new caLabel(this);
        labelMinText->setText(" Min: ");
        checkAutoText = new caLabel(this);
        checkAutoText->setText(" Auto: ");
        intensityText = new caLabel(this);
        intensityText->setText(" x/y/z: ");
        labelColormodeText = new caLabel(this);
        labelColormodeText->setText("Color: ");
        labelPackingmodeText = new caLabel(this);
        labelPackingmodeText->setText("Packing: ");
        labelCompressionmodeText = new caLabel(this);
        labelCompressionmodeText->setText("Compression: ");

        colormodeCombo = new QComboBox(this);
        packingmodeCombo = new QComboBox(this);
        compressionmodeCombo = new QComboBox(this);


        for(int i=0; i<colorModeString.count(); i++) colormodeCombo->addItem(colorModeString.at(i));
        for(int i=0; i<packingModeString.count(); i++) packingmodeCombo->addItem(packingModeString.at(i));
        for(int i=0; i<compressionModeString.count(); i++) compressionmodeCombo->addItem(compressionModeString.at(i));

        connect(colormodeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(colormodeComboSlot(int)));
        connect(packingmodeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(packingmodeComboSlot(int)));
        connect(compressionmodeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(compressionmodeComboSlot(int)));

        compressionmodeCombo->setCurrentIndex(thisCompressionmode);

        nbUpdatesText = new caLabel(this);

        labelMax = new caLineEdit(this);
        labelMin = new caLineEdit(this);
        labelMax->newFocusPolicy(Qt::ClickFocus);
        labelMin->newFocusPolicy(Qt::ClickFocus);

        intensity = new caLabel(this);
        intensity->setAlignment(Qt::AlignVCenter | Qt::AlignLeft );
        labelMaxText->setScaleMode(caLabel::None);
        labelMinText->setScaleMode(caLabel::None);
        labelColormodeText->setScaleMode(caLabel::None);
        labelPackingmodeText->setScaleMode(caLabel::None);
        labelCompressionmodeText->setScaleMode(caLabel::None);

        checkAutoText->setScaleMode(caLabel::None);
        intensity->setScaleMode(caLabel::None);
        intensityText->setScaleMode(caLabel::None);
        nbUpdatesText->setScaleMode(caLabel::None);
        QFont font = labelMaxText->font();
        font.setPointSize(10);
        labelMaxText->setFont(font);
        labelMinText->setFont(font);
        labelColormodeText->setFont(font);
        labelPackingmodeText->setFont(font);
        labelCompressionmodeText->setFont(font);
        colormodeCombo->setFont(font);
        packingmodeCombo->setFont(font);
        compressionmodeCombo->setFont(font);

        checkAutoText->setFont(font);
        intensity->setFont(font);
        intensityText->setFont(font);
        nbUpdatesText->setFont(font);
        labelMaxText->setBackground(QColor(0,0,0,0));
        labelMinText->setBackground(QColor(0,0,0,0));
        labelColormodeText->setBackground(QColor(0,0,0,0));
        labelPackingmodeText->setBackground(QColor(0,0,0,0));
        labelCompressionmodeText->setBackground(QColor(0,0,0,0));


        checkAutoText->setBackground(QColor(0,0,0,0));
        intensity->setBackground(QColor(0,0,0,0));
        intensityText->setBackground(QColor(0,0,0,0));
        nbUpdatesText->setBackground(QColor(0,0,0,0));

        // checkbox
        autoW = new QCheckBox(this);
        autoW->setChecked(true);

        // add everything to layout
        valuesLayout = new QHBoxLayout();
        SETMARGIN_QT456(valuesLayout,0);
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

        colormodeLayout = new QHBoxLayout();
        SETMARGIN_QT456(colormodeLayout,0);
        colormodeLayout->setSpacing(2);
        colormodeLayout->addWidget(labelColormodeText, Qt::AlignLeft);
        colormodeLayout->addWidget(colormodeCombo, Qt::AlignLeft);
        colormodeLayout->addWidget(labelPackingmodeText, Qt::AlignLeft);
        colormodeLayout->addWidget(packingmodeCombo, Qt::AlignLeft);
        colormodeLayout->addWidget(labelCompressionmodeText, Qt::AlignLeft);
        colormodeLayout->addWidget(compressionmodeCombo, Qt::AlignLeft);

        colormodeLayout->addStretch(2);

        colormodesWidget = new QWidget;
        colormodesWidget->setLayout(colormodeLayout);

        // image inside a scrollarea
        imageW   = new ImageWidget();
        scrollArea = new QScrollArea;
        scrollArea->setBackgroundRole(QPalette::Dark);
        scrollArea->setWidget(imageW);
        scrollArea->setWidgetResizable(true);

        connect (scrollArea->verticalScrollBar(), SIGNAL(valueChanged (int)), this, SLOT(scrollAreaMoved(int)));
        connect (scrollArea->horizontalScrollBar(), SIGNAL(valueChanged (int)), this, SLOT(scrollAreaMoved(int)));

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
        //connect(zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(zoomNow()));
        connect(zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(setZoomSlider(int)));


        // add everything to main layout
        mainLayout->addWidget(valuesWidget, 0, 0);
        mainLayout->addWidget(scrollArea, 1, 0);
        mainLayout->addWidget(zoomWidget, 1, 2);
        mainLayout->addWidget(colormodesWidget, 2, 0);
        if(thisShowBoxes) colormodesWidget->show();
        else colormodesWidget->hide();

        for(int i=0; i<4; i++) readvaluesPresent[i] = false;

        updateMin(0);
        updateMax(0);
    } else {
        imageW   = new ImageWidget();
        // add to main layout
        mainLayout->addWidget(imageW, 0, 0);
        thisFitToSize = Yes;
    }
}

void caCamera::scrollAreaMoved(int)
{
    if(image != (QImage *)Q_NULLPTR)  imageW->update();
}


void caCamera::setAutoLevel(bool enable){

    if (thisInitialAutomatic!=enable){
        thisInitialAutomatic=enable;
        if(autoW != (QCheckBox *) Q_NULLPTR ) {
            if(enable) {autoW->setCheckState(Qt::Checked);}
            else {autoW->setCheckState(Qt::Unchecked);}
        }
    }
}



void caCamera::setZoomSlider(int zoom){
   if (zoom!=m_zoom_value){
    m_zoom_value=zoom;
    printf("setZoomSlider(int zoom)");
    fflush(stdout);
    if(zoomSlider != (QSlider *) Q_NULLPTR ) {
        if (zoomSlider->value()!=m_zoom_value) {
            zoomSlider->setValue(zoom);}
    }
    zoomNow();
   }
}

void caCamera::setverticalScrollBar(int pos){

    if (m_verticalScroll!=pos){
        m_verticalScroll = pos;
        if(scrollArea != (QScrollArea *) Q_NULLPTR ){
            scrollArea->verticalScrollBar()->setValue(pos);
        }
    }
}

void caCamera::sethorizontalScrollBar(int pos){
    if (m_horizontalScroll!=pos){
        m_horizontalScroll = pos;
        if(scrollArea != (QScrollArea *) Q_NULLPTR ){
            scrollArea->horizontalScrollBar()->setValue(pos);
        }
    }
}

void caCamera::zoomNow()
{
    if(zoomSlider != (QSlider *) Q_NULLPTR ) {
     m_zoom_value=zoomSlider->value();
    }


    double scale = qPow(2.0, ((double) m_zoom_value - 52.0) / 13.0);
    if(scale > 32) scale = 32;
    if(zoomValue != (QLabel *) Q_NULLPTR ) {
        zoomValue->setText(QString::number(scale, 'f', 3));
    }
    scaleFactor = scale;
    setFitToSize(No);

    // keep centered on last pick
    int posX =  P3.x() * scaleFactor;
    int posY =  P3.y() * scaleFactor;
    if(scrollArea != (QScrollArea *) Q_NULLPTR ){
        scrollArea->horizontalScrollBar()->setValue(posX - scrollArea->horizontalScrollBar()->pageStep()/2);
        scrollArea->verticalScrollBar()->setValue(posY - scrollArea->verticalScrollBar()->pageStep()/2);
    }
}

void caCamera::zoomIn(int level)
{
    m_zoom_value=zoomSlider->value() + level;
    zoomSlider->setValue(m_zoom_value);
}

void caCamera::zoomOut(int level)
{
    m_zoom_value=zoomSlider->value() - level;
    zoomSlider->setValue(m_zoom_value);
}

void caCamera::setFitToSize(zoom const &z)
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

bool caCamera::getInitialAutomatic()
{
    if(thisSimpleView) return thisInitialAutomatic;
    if(autoW == (QCheckBox *) Q_NULLPTR) return false;
    return autoW->isChecked();
}

void caCamera::setInitialAutomatic(bool automatic)
{
    if(thisSimpleView) thisInitialAutomatic = automatic;
    if(autoW == (QCheckBox *) Q_NULLPTR) return;
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
    colorMaps colormaps;
    setPropertyVisible(customcolormap, false);
    setPropertyVisible(discretecolormap, false);

    switch (map) {

    case color_to_mono:
        colormaps.getColormap(colorMaps::grey, false, thisCustomMap, ColormapSize, ColorMap, colormapWidget);
        break;
    case mono_to_wavelength:
        colormaps.getColormap(colorMaps::spectrum_wavelength, false, thisCustomMap, ColormapSize, ColorMap, colormapWidget);
        break;
    case mono_to_hot:
        colormaps.getColormap(colorMaps::spectrum_hot, false, thisCustomMap, ColormapSize, ColorMap, colormapWidget);
        break;
    case mono_to_heat:
        colormaps.getColormap(colorMaps::spectrum_heat, false, thisCustomMap, ColormapSize, ColorMap, colormapWidget);
        break;
    case mono_to_jet:
        colormaps.getColormap(colorMaps::spectrum_jet, false, thisCustomMap, ColormapSize, ColorMap, colormapWidget);
        break;
    case mono_to_custom:
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
    if(zoomWidget != (QWidget*) Q_NULLPTR) zoomWidget->adjustSize();
    QResizeEvent *re = new QResizeEvent(size(), size());
    resizeEvent(re);
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
    if(thisSimpleView) return;

    if(m_widthDefined && m_heightDefined) {
        if(!thisFitToSize) {
            imageW->setMinimumSize((int) (m_width * scaleFactor), (int) (m_height * scaleFactor));

        } else if((zoomWidget != (QWidget*) Q_NULLPTR) && (valuesWidget != (QWidget*) Q_NULLPTR)) {
            double Xcorr = (double) (e->size().width() - zoomWidget->width()-4) / (double) savedWidth;
            double Ycorr = (double) (e->size().height()- valuesWidget->height()-4) / (double) savedHeight;
            double scale = qMin(Xcorr, Ycorr); // aspect ratio
            // disconnect signal to prevent firing now
            disconnect(zoomSlider, SIGNAL(valueChanged (int)), Q_NULLPTR, Q_NULLPTR);
            zoomSlider->setValue((int)(13.0*log(scale)/log(2.0)+52.0));
            zoomValue->setText(QString::number(scale, 'f', 3));
            connect(zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(zoomNow()));
            imageW->setFixedWidth(e->size().width() - zoomWidget->width()-4);
            imageW->setFixedHeight(e->size().height()- valuesWidget->height()-4);
            scaleFactor = scale;

            // define the middle of the image for zooming on center of the image
            double Xnew, Ynew, Xmax, Ymax;
            Coordinates((int) (m_width * scaleFactor)/2,  (int) (m_height * scaleFactor)/2,  Xnew, Ynew, Xmax, Ymax);
            P3 = QPointF(Xnew, Ynew);
        }
    }
    if(image != (QImage *) Q_NULLPTR)  imageW->rescaleSelectionBox(scaleFactor);
}

void caCamera::updateImage(const QImage &image, bool valuesPresent[], double values[], double scaleFactor,
                           QVarLengthArray<double> X,  QVarLengthArray<double> Y)
{
    imageW->updateImage(thisFitToSize, image, valuesPresent, values, scaleFactor, thisSimpleView,
                        (short) getROIreadmarkerType(), (short) getROIreadType(),
                        (short) getROIwritemarkerType(), (short) getROIwriteType(), X, Y);
}

void caCamera::showDisconnected()
{
    imageW->updateDisconnected();
}

bool caCamera::getAutomateChecked()
{
    if(thisSimpleView) {
        return thisInitialAutomatic;
    }
    if(autoW == (QCheckBox *) Q_NULLPTR) return false;
    return autoW->isChecked();
}

void caCamera::updateMax(int max)
{
    if(labelMax == (caLineEdit*) Q_NULLPTR) return;
    labelMax->setText(QString::number(max));
}

void caCamera::updateMin(int min)
{
    if(labelMin == (caLineEdit*) Q_NULLPTR) return;
    labelMin->setText(QString::number(min));
}

void caCamera::updateIntensity(QString strng)
{
    if(intensity == (caLabel*) Q_NULLPTR) return;
    intensity->setText(strng);
}

int caCamera::getMin()
{
    bool ok;
    if(thisSimpleView) return  thisMinLevel.toInt(&ok);
    if(labelMin == (caLineEdit*) Q_NULLPTR) return 0;
    return labelMin->text().toInt();
}
int caCamera::getMax()
{
    bool ok;
    if(thisSimpleView) return  thisMaxLevel.toInt(&ok);
    if(labelMax == (caLineEdit*) Q_NULLPTR) return 65535;
    return labelMax->text().toInt();
}

void caCamera::dataProcessing(double value, int id)
{
    if(id < 0 || id > 3) return;
    readvaluesPresent[id] = true;
    readvalues[id] = value;
}

void caCamera::MinMaxLock(SyncMinMax* MinMax, uint Max[2], uint Min[2])
{
    MinMax->MinMaxLock->lock();
    MinMax->Max[(Max[1] > MinMax->Max[1])] = Max[1];
    MinMax->Min[(Min[1] < MinMax->Min[1])] = Min[1];
    MinMax->MinMaxLock->unlock();
}

void caCamera::MinMaxImageLock(QVector<uint> LineData, int y, QSize resultSize, SyncMinMax* MinMax)
{
    MinMax->imageLock->lock();
    if(image != (QImage *) Q_NULLPTR) {
        if (image->height()>y){
            uint *scanLine = reinterpret_cast<uint *>(image->scanLine(y));
            if (scanLine){
                memcpy(scanLine, LineData.constData(), resultSize.width() * sizeof(uint));
            }
        }
    }
    MinMax->imageLock->unlock();
}

void caCamera::MinMaxImageLockBlock(uint *LineData, int ystart, int yend, QSize resultSize, SyncMinMax* MinMax)
{
    MinMax->imageLock->lock();
    if(image != (QImage *) Q_NULLPTR) {
        for(int i=ystart; i<yend; ++i) {
            if (i<image->height()){
                uint *scanLine = reinterpret_cast<uint *>(image->scanLine(i));
                if (scanLine) {
                    memcpy(scanLine, &LineData[(i-ystart) * resultSize.width()], resultSize.width() * sizeof(uint));
                }
            }
        }
    }
    MinMax->imageLock->unlock();
}

void caCamera::InitLoopdata(int &ystart, int &yend, long &i, int increment, int sector, int sectorcount, QSize resultSize, uint Max[2], uint Min[2])
{
    Max[1] = 0;
    Min[1] = 65535;

    ystart = sector * resultSize.height() / sectorcount;
    yend = ((sector + 1) * resultSize.height()) / sectorcount;
    // start of block to treat
    i = resultSize.width() * ystart * increment;
}

void caCamera::reallocate_central_image()
{

    imageMutex.lock();
    if(image != (QImage *)Q_NULLPTR){
        if ((image->width()!=m_width)||(image->height()!=m_height)){
            delete image;
            image = (QImage *)Q_NULLPTR;
        }
    }
    if (image == (QImage *)Q_NULLPTR)
        image = new QImage(m_width,m_height,QImage::Format_RGB32);
    imageMutex.unlock();
}

// I leave the code now as it was, while here we use uint and in calcimage QVector<uint>. I can merge later on.
template <typename pureData>
void caCamera::calcImageMono (pureData *ptr,  uint *LineData, long &i, int &ystart, int &yend, float correction, int datasize, QSize resultSize,
                              uint Max[2], uint Min[2])
{
    if(ptr &&(i > datasize)) return;
    if(thisColormap == as_is || thisColormap == color_to_mono) {
        if(i < datasize) {
            for(int k=0; k<(yend-ystart)*resultSize.width(); ++k) {
                Max[(ptr[i] > Max[1])] = ptr[i];
                Min[(ptr[i] < Min[1])] = ptr[i];

                int indx1 = ptr[i] * correction;
                if(indx1 > 255) indx1 = 255;

                LineData[k] =  qRgb(indx1,indx1,indx1);
                ++i;
                if(i >= datasize) break;
            }
        }
        // use colormap
    } else {
        if(i < datasize) {
            for(int k=0; k<(yend-ystart)*resultSize.width(); ++k) {
                Max[(ptr[i] > Max[1])] = ptr[i];
                Min[(ptr[i] < Min[1])] = ptr[i];

                int indx1 = (ptr[i] - minvalue) * correction;
                if(indx1 < 0) indx1 = 0;
                if(indx1 >= ColormapSize) indx1=ColormapSize -1;

                LineData[k] =  ColorMap[indx1];
                ++i;
                if(i >= datasize) break;
            }
        }
    }
}

template <typename pureData> void caCamera::calcImage (pureData *ptr,  colormode mode,  QVector<uint> &LineData, long &i, int &ystart, int &yend,
                                                       float correction, int datasize, QSize resultSize, SyncMinMax *MinMax, uint Max[2], uint Min[2])
{
    int offset1 = 1;            // pixel
    int offset2 = 2;
    int offset3 = 0;
    int  dataAdvance;

    if(mode == RGB3_CA) {          // blop red, blob green, blob blue
        offset1 = savedHeight * savedWidth;
        offset2 = 2 * offset1;
        dataAdvance = 1;
    } else if(mode == RGB2_CA) {   // row red, row green row blue
        offset1 = savedWidth;
        offset2 = 2 * offset1;
        offset3 = savedWidth * 2;
        dataAdvance = 1;
    } else {                   // elements red, green, blue
        dataAdvance = 3;
    }

    if((i + offset2 + offset3) > datasize) return;

    // normal rgb display
    float redcoeff = correction * thisRedCoefficient;
    float greencoeff = correction * thisGreenCoefficient;
    float bluecoeff = correction * thisBlueCoefficient;

    //printf("width=%d height=%d datasize=%d\n", resultSize.width(), yend, datasize);

    if(thisColormap == as_is || thisColormap > color_to_mono) {
        for (int y = ystart; y < yend; ++y) {
            for (int x = 0; x < resultSize.width(); ++x) {
                uint intensity = qMax(qMax(ptr[i], ptr[i+offset1]), ptr[i+offset2]);
                LineData[x] =  qRgb((int) (ptr[i] * redcoeff), (int) (ptr[i+offset1] * greencoeff), (int) (ptr[i+offset2] * bluecoeff));
                i += dataAdvance;
                Max[(intensity > Max[1])] = intensity;
                Min[(intensity < Min[1])] = intensity;
                if ((i + offset2 + offset3) >= datasize) break;
            }
            i += offset3;
            if((i + offset2 + offset3) >= datasize) break;
            MinMaxImageLock(LineData, y, resultSize, MinMax);
        }
        // convert to mono
    } else {
        for (int y = ystart; y < yend; ++y) {
            for (int x = 0; x < resultSize.width(); ++x) {
                uint intensity = qMax(qMax(ptr[i], ptr[i+offset1]), ptr[i+offset2] );
                int average =(int) 2.2 * (0.2989 * ptr[i] * correction + 0.5870 * ptr[i+offset1] * correction + 0.1140 * ptr[i+offset2] * correction);
                LineData[x] =  qRgb(average, average, average);
                i += dataAdvance;
                Max[(intensity > Max[1])] = intensity;
                Min[(intensity < Min[1])] = intensity;
                if((i + offset2 + offset3) >= datasize) break;
            }
            i += offset3;
            if ((i + offset2 + offset3) >= datasize) break;
            MinMaxImageLock(LineData, y, resultSize, MinMax);
        }
    }
}

void caCamera::CameraDataConvert(int sector, int sectorcount, SyncMinMax* MinMax, QSize resultSize, int datasize)
{
    uint Max[2], Min[2];
    int ystart, yend;
    long i;

    int elementSize = 1;
    float correction = 1.0;

    if (savedData==Q_NULLPTR) return;

    if(m_datatype == caINT) elementSize = 2;
    else if(m_datatype == caLONG || m_datatype == caFLOAT) elementSize = 4;
    else if(m_datatype == caDOUBLE) elementSize = 8;

    if(thisColormode == Mono) {

        uint *LineData;
        int elementAdvance = 1;
        InitLoopdata(ystart, yend, i, elementAdvance, sector, sectorcount, resultSize, Max, Min);
        LineData = (uint *) malloc(resultSize.width() * sizeof(uint) * (yend-ystart));

        // instead of testing in the big loop, subtract 10 lines when sizes do not fit
        bool notOK = true;
        bool writeIt = true;
        while (notOK) {
            long SizeToTreat = (yend-ystart) * resultSize.width() * elementSize;
            if(SizeToTreat > datasize) {
                yend -= 10;
                if(yend < ystart) {
                    printf("caCamera -- something really wrong between datasize and image width and height\n");
                    free (LineData);
                    return;
                }
                if(writeIt) {
                    printf("caCamera -- something wrong between datasize=%d and image width=%d and height=%d, trying to match\n", datasize, resultSize.width(), resultSize.height());
                    writeIt = false;
                    fflush(stdout);
                }
            } else {
                notOK = false;
            }
        }

        if(thisColormap == as_is || thisColormap == color_to_mono) {
            correction =  (float) 255 / (float) (maxvalue - minvalue);
        } else {
            correction =  (float)(ColormapSize-1) / (float) (maxvalue - minvalue);
        }

        switch (m_datatype) {
        case caCHAR:
            if((ulong) i*sizeof(uchar) >= (uint) datasize){
                free(LineData);
                return;
            }
            calcImageMono ((uchar*) savedData, LineData, i, ystart, yend, correction, datasize, resultSize, Max, Min);
            break;
        case caINT:
            if((ulong) i*sizeof(ushort) >= (uint) datasize) {
                free(LineData);
                return;
            }
            calcImageMono ((ushort*) savedData, LineData, i, ystart, yend, correction, datasize/elementSize, resultSize, Max, Min);
            break;
        case caLONG:
            if((ulong) i*sizeof(uint) >= (uint) datasize) {
                free(LineData);
                return;
            }
            calcImageMono ((uint*) savedData, LineData, i, ystart, yend, correction, datasize/elementSize, resultSize, Max, Min);
            break;
        case caFLOAT:
            if((ulong) i*sizeof(float) >= (uint) datasize) {
                free(LineData);
                return;
            }
            calcImageMono ((float*) savedData,  LineData, i, ystart, yend, correction, datasize/elementSize, resultSize, Max, Min);
            break;
        case caDOUBLE:
            if((ulong) i*sizeof(double) >= (uint) datasize) {
                free(LineData);
                return;
            }
            calcImageMono ((double*) savedData, LineData, i, ystart, yend, correction, datasize/elementSize, resultSize, Max, Min);
            break;
        default:
            printf("caCamera -- data format not supported\n");
        }

        MinMaxImageLockBlock(LineData, ystart, yend, resultSize, MinMax);
        MinMaxLock(MinMax, Max, Min);
        free(LineData);
    } else  {
        QVector<uint> LineData;

        if(maxvalue != 0) correction = 255.0 / (float) maxvalue;

        int increment = 1;
        if(thisColormode == RGB1_CA) increment = 3; // 3 elements RGB
        if(thisColormode == RGB2_CA) increment = 3; // 3 Lines RGB
        LineData.resize(resultSize.width());
        InitLoopdata(ystart, yend, i, increment, sector, sectorcount, resultSize, Max, Min);
        switch (m_datatype) {
        case caCHAR:
            calcImage ((uchar*) savedData, thisColormode, LineData, i, ystart, yend, correction, datasize, resultSize, MinMax, Max, Min);
            break;
        case caINT:
            calcImage ((ushort*) savedData, thisColormode, LineData, i, ystart, yend, correction, datasize/elementSize, resultSize, MinMax, Max, Min);
            break;
        case caLONG:
            calcImage ((uint*) savedData, thisColormode, LineData, i, ystart, yend, correction, datasize/elementSize, resultSize, MinMax, Max, Min);
            break;
        case caFLOAT:
            calcImage ((float*) savedData, thisColormode, LineData, i, ystart, yend, correction, datasize/elementSize, resultSize, MinMax, Max, Min);
            break;
        case caDOUBLE:
            calcImage ((double*) savedData, thisColormode, LineData, i, ystart, yend, correction, datasize/elementSize, resultSize, MinMax, Max, Min);
            break;
        default:
            printf("caCamera -- data format not supported\n");
        }

        MinMaxLock(MinMax, Max, Min);
    }
}

/*
 * 1394-Based Digital Camera Control Library
 *
 * Bayer pattern decoding functions
 *
 * Written by Damien Douxchamps and Frederic Devernay
 * The original VNG and AHD Bayer decoding are from Dave Coffin's DCR
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 */

template <typename pureData> void caCamera::FilterBayer(pureData *bayer, uint *rgb, int sx, int sy, int tile,int datasize)
{
    const int bayerStep = sx;
    const int rgbStep = 3 * sx;
    uchar *rgbStart=(uchar *)rgb;
    uchar *bayerStart=(uchar *)bayer;

    int width = sx;
    int height = sy;
    int blue = tile == BAYER_COLORFILTER_BGGR || tile == BAYER_COLORFILTER_GBRG ? -1 : 1;
    int start_with_green = tile == BAYER_COLORFILTER_GBRG|| tile == BAYER_COLORFILTER_GRBG;
    int i, iinc, imax;

    if ((tile > BAYER_COLORFILTER_MAX) || (tile < BAYER_COLORFILTER_MIN)) {
        printf("caCamera -- bayer, invalid filter\n");
        return;
    }

    /* add black border */
    imax = sx * sy * 3;
    for (i = sx * (sy - 1) * 3; i < imax; i++) {
        rgb[i] = 0;
    }
    iinc = (sx - 1) * 3;
    for (i = (sx - 1) * 3; i < imax; i += iinc) {
        rgb[i++] = 0;
        rgb[i++] = 0;
        rgb[i++] = 0;
    }

    rgb += 1;
    height -= 1;
    width -= 1;
    for (; height--; bayer += bayerStep, rgb += rgbStep) {
        pureData *bayerEnd = bayer + width;
        if (((uchar *)(rgb+rgbStep)<((uchar *)rgbStart+3*m_width*m_height*sizeof(uint)))&&((uchar *)(bayer+bayerStep)<((uchar *)bayerStart+datasize))){
            if (start_with_green) {
                rgb[-blue] = bayer[1];
                rgb[0] = bayer[bayerStep + 1];
                rgb[blue] = bayer[bayerStep];
                bayer++;
                rgb += 3;
            }

            if (blue > 0) {
                for (; bayer <= bayerEnd - 2; bayer += 2, rgb += 6) {
                    rgb[-1] = bayer[0];
                    rgb[0] = bayer[1];
                    rgb[1] = bayer[bayerStep + 1];

                    rgb[2] = bayer[2];
                    rgb[3] = bayer[bayerStep + 2];
                    rgb[4] = bayer[bayerStep + 1];
                }
            } else {
                for (; bayer <= bayerEnd - 2; bayer += 2, rgb += 6) {

                    rgb[1] = bayer[0];
                    rgb[0] = bayer[1];
                    rgb[-1] = bayer[bayerStep + 1];

                    rgb[4] = bayer[2];
                    rgb[3] = bayer[bayerStep + 2];
                    rgb[2] = bayer[bayerStep + 1];

                }
            }

            if (bayer < bayerEnd) {
                rgb[-blue] = bayer[0];
                rgb[0] = bayer[1];
                rgb[blue] = bayer[bayerStep + 1];
                bayer++;
                rgb += 3;
            }

            bayer -= width;
            rgb -= width * 3;

            blue = -blue;
            start_with_green = !start_with_green;
        }
    }

    return;
}

//https://en.wikipedia.org/wiki/Chroma_subsampling
//https://en.wikipedia.org/wiki/YCbCr
#define GET_R_FROM_YUV(y,cb,cr) 298.082*y/256 +                      408.583 * cr / 256 - 222.291 ;
#define GET_G_FROM_YUV(y,cb,cr) 298.082*y/256 - 100.291 * cb / 256 - 208.120 * cr / 256 + 135.576 ;
#define GET_B_FROM_YUV(y,cb,cr) 298.082*y/256 + 561.412 * cb / 256                      - 276.836 ;

//#define GET_R_FROM_YUV(y,u,v) y + 1.370705 * (v-128);
//#define GET_G_FROM_YUV(y,u,v) y - 0.698001 * (v-128) - 0.337633 * (u -128);
//#define GET_B_FROM_YUV(y,u,v) y + 1.732446 * (u-128);

void caCamera::PROC_YUYV422(uchar *YUV, uint *rgb, int sx, int sy, int datasize)  // 4 bytes for 2 pixels
{
    long max_data=(long)YUV + datasize;
    if ((sx==0)||(sy==0)) return;
    for (long i = 0; i < (sx) * sy / 2; ++i) {
        int Y1, Cr, Y2, Cb;
        long r,g,b;
        long min=0;
        // Extract YCbCr components
        Y1 = YUV[0];
        Cb = YUV[1];
        Y2 = YUV[2];
        Cr = YUV[3];

        YUV += 4;

        r=GET_R_FROM_YUV(Y1,Cb,Cr);
        g=GET_G_FROM_YUV(Y1,Cb,Cr);
        b=GET_B_FROM_YUV(Y1,Cb,Cr);

        rgb[0]=qMax(min,r);
        rgb[1]=qMax(min,g);
        rgb[2]=qMax(min,b);

        //        if (i<9){
        //            printf("(%i)Y1: %x Y2: %x Cr: %x Cb: %x\n",i,Y1,Y2,Cr,Cb);
        //            printf("\t R: %x G: %x B: %x ",rgb[0],rgb[1],rgb[2]);
        //        }

        rgb += 3;
        r=GET_R_FROM_YUV(Y2,Cb,Cr);
        g=GET_G_FROM_YUV(Y2,Cb,Cr);
        b=GET_B_FROM_YUV(Y2,Cb,Cr);
        rgb[0]=qMax(min,r);
        rgb[1]=qMax(min,g);
        rgb[2]=qMax(min,b);

        rgb += 3;
        //        if (i<9){
        //            printf("R2: %x G2: %x B2: %x \n",rgb[0],rgb[1],rgb[2]);
        //        }
        if (max_data<(long)YUV) break;
    }
}

void caCamera::PROC_UYVY422(uchar *YUV, uint *rgb, int sx, int sy, int datasize)  // 4 bytes for 4 pixels
{
    long max_data=(long)YUV + datasize;
    if ((sx==0)||(sy==0)) return;
    for (int i = 0; i < sx * sy / 4; ++i) {
        int y0, u0, y1, v0, y2, u2, y3, v2;

        // Extract yuv components
        u0 = YUV[0];
        y0 = YUV[1];
        v0 = YUV[2];
        y1 = YUV[3];
        u2 = YUV[4];
        y2 = YUV[5];
        v2 = YUV[6];
        y3 = YUV[7];
        YUV += 8;

        rgb[0] = GET_R_FROM_YUV(y0,u0,v0);
        rgb[1] = GET_G_FROM_YUV(y0,u0,v0);
        rgb[2] = GET_B_FROM_YUV(y0,u0,v0);
        rgb += 3;
        rgb[0] = GET_R_FROM_YUV(y1,u0,v0);
        rgb[1] = GET_G_FROM_YUV(y1,u0,v0);
        rgb[2] = GET_B_FROM_YUV(y1,u0,v0);
        rgb += 3;
        rgb[0] = GET_R_FROM_YUV(y2,u2,v2);
        rgb[1] = GET_G_FROM_YUV(y2,u2,v2);
        rgb[2] = GET_B_FROM_YUV(y2,u2,v2);
        rgb += 3;
        rgb[0] = GET_R_FROM_YUV(y3,u2,v2);
        rgb[1] = GET_G_FROM_YUV(y3,u2,v2);
        rgb[2] = GET_B_FROM_YUV(y3,u2,v2);
        rgb += 3;
        if (max_data<(long)YUV) break;
    }
}

void caCamera::PROC_YYUYYV411(uchar *YUV, uint *rgb, int sx, int sy, int datasize)  // 6 bytes for 4 pixels
{
    long max_data=(long)YUV + datasize;
    if ((sx==0)||(sy==0)) return;
    for (long i = 0; i < (sx) * sy / 4; ++i) {
        int Y1, U, Y2, V, Y3, Y4;

        long r,g,b;
        long min=0;
        // Extract YCbCr components
        Y1 = YUV[0];
        Y2 = YUV[1];
        U  = YUV[2];
        Y3 = YUV[3];
        Y4 = YUV[4];
        V  = YUV[5];

        YUV += 6;

        r=GET_R_FROM_YUV(Y1,U,V);
        g=GET_G_FROM_YUV(Y1,U,V);
        b=GET_B_FROM_YUV(Y1,U,V);
        rgb[0]=qMax(min,r);
        rgb[1]=qMax(min,g);
        rgb[2]=qMax(min,b);
        rgb += 3;

        r=GET_R_FROM_YUV(Y2,U,V);
        g=GET_G_FROM_YUV(Y2,U,V);
        b=GET_B_FROM_YUV(Y2,U,V);
        rgb[0]=qMax(min,r);
        rgb[1]=qMax(min,g);
        rgb[2]=qMax(min,b);
        rgb += 3;

        r=GET_R_FROM_YUV(Y3,U,V);
        g=GET_G_FROM_YUV(Y3,U,V);
        b=GET_B_FROM_YUV(Y3,U,V);
        rgb[0]=qMax(min,r);
        rgb[1]=qMax(min,g);
        rgb[2]=qMax(min,b);
        rgb += 3;

        r=GET_R_FROM_YUV(Y4,U,V);
        g=GET_G_FROM_YUV(Y4,U,V);
        b=GET_B_FROM_YUV(Y4,U,V);
        rgb[0]=qMax(min,r);
        rgb[1]=qMax(min,g);
        rgb[2]=qMax(min,b);
        rgb += 3;

        if (max_data<(long)YUV) break;
    }
}

void caCamera::PROC_UYYVYY411(uchar *YUV, uint *rgb, int sx, int sy, int datasize)  // 6 bytes for 4 pixels
{
    long max_data=(long)YUV + datasize;
    if ((sx==0)||(sy==0)) return;
    for (long i = 0; i < (sx) * sy / 4; ++i) {
        int Y1, U, Y2, V, Y3, Y4;
        long r,g,b;

        long min=0;
        // Extract YCbCr components
        U  = YUV[0];
        Y1 = YUV[1];
        Y2 = YUV[2];
        V  = YUV[3];
        Y3 = YUV[4];
        Y4 = YUV[5];
        YUV += 6;

        r=GET_R_FROM_YUV(Y1,U,V);
        g=GET_G_FROM_YUV(Y1,U,V);
        b=GET_B_FROM_YUV(Y1,U,V);
        rgb[0]=qMax(min,r);
        rgb[1]=qMax(min,g);
        rgb[2]=qMax(min,b);
        rgb += 3;

        r=GET_R_FROM_YUV(Y2,U,V);
        g=GET_G_FROM_YUV(Y2,U,V);
        b=GET_B_FROM_YUV(Y2,U,V);
        rgb[0]=qMax(min,r);
        rgb[1]=qMax(min,g);
        rgb[2]=qMax(min,b);
        rgb += 3;

        r=GET_R_FROM_YUV(Y3,U,V);
        g=GET_G_FROM_YUV(Y3,U,V);
        b=GET_B_FROM_YUV(Y3,U,V);
        rgb[0]=qMax(min,r);
        rgb[1]=qMax(min,g);
        rgb[2]=qMax(min,b);
        rgb += 3;

        r=GET_R_FROM_YUV(Y4,U,V);
        g=GET_G_FROM_YUV(Y4,U,V);
        b=GET_B_FROM_YUV(Y4,U,V);
        rgb[0]=qMax(min,r);
        rgb[1]=qMax(min,g);
        rgb[2]=qMax(min,b);
        rgb += 3;
        if (max_data<(long)YUV) break;

    }
}

void caCamera::PROC_YUV444(uchar *YUV, uint *rgb, int sx, int sy, int datasize)  // 3 bytes for 1 pixels
{
    long max_data=(long)YUV + datasize;
    if ((sx==0)||(sy==0)||(YUV==NULL)||(rgb==NULL)) return;
    for (long i = 0; i < (sx) * sy ; ++i) {
        int Y, U, V;
        long r,g,b;
        long min=0;
        Y  = YUV[0];
        U  = YUV[1];
        V  = YUV[2];

        YUV += 3;

        r=GET_R_FROM_YUV(Y,U,V);
        g=GET_G_FROM_YUV(Y,U,V);
        b=GET_B_FROM_YUV(Y,U,V);
        rgb[0]=qMax(min,r);
        rgb[1]=qMax(min,g);
        rgb[2]=qMax(min,b);
        rgb += 3;

        if (max_data<=(long)YUV) return;

    }
}

void caCamera::PROC_UVY444(uchar *YUV, uint *rgb, int sx, int sy, int datasize)  // 3 bytes for 1 pixels
{
    //printf("datatype=PROC_UVY444 colormode=%d %s (%x)(%x) %i %i\n", thisColormode, qasc(colorModeString.at(thisColormode)),(long)YUV,(long)rgb,sx, sy);
    long max_data=(long)YUV + datasize;

    if ((sx==0)||(sy==0)) return;
    for (long i = 0; i < (sx) * sy ; ++i) {
        int Y, U, V;
        long r,g,b;
        long min=0;
        U =  YUV[0];
        Y  = YUV[1];
        V  = YUV[2];

        YUV += 3;

        r=GET_R_FROM_YUV(Y,U,V);
        g=GET_G_FROM_YUV(Y,U,V);
        b=GET_B_FROM_YUV(Y,U,V);
        rgb[0]=qMax(min,r);
        rgb[1]=qMax(min,g);
        rgb[2]=qMax(min,b);
        rgb += 3;

        if (max_data<=(long)YUV) return;

    }
}

void caCamera::PROC_RGB8(uchar *RGB,rgb_interpretation rgb_type, uint *rgb, int sx, int sy, int datasize)  // 3 bytes for 1 pixels
{
    int rgb_matrix[5][3]={{0,1,2},  //COLOR_RGB
                          {2,1,0},  //COLOR_GBR
                          {1,0,2},  //COLOR_GRB
                          {2,1,0},  //COLOR_BGR
                          {1,2,0}   //COLOR_BRG
                         };
    long max_data=(long)RGB + datasize;

    if ((sx==0)||(sy==0)) return;
    for (long i = 0; i < (sx) * sy ; ++i) {
        int R, G, B;

        R =  RGB[rgb_matrix[rgb_type][0]];
        G  = RGB[rgb_matrix[rgb_type][1]];
        B  = RGB[rgb_matrix[rgb_type][2]];

        RGB += 3;

        rgb[0]=R;
        rgb[1]=G;
        rgb[2]=B;
        rgb += 3;

        if (max_data<=(long)RGB) return;

    }
}

void caCamera::PROC_RGBA8(uchar *RGBA,rgb_interpretation rgb_type, uint *rgb, int sx, int sy, int datasize)
{
    int rgb_matrix[5][4]={{0,1,2,3},  //COLOR_RGB
                          {2,1,0,3},  //COLOR_GBR
                          {1,0,2,3},  //COLOR_GRB
                          {2,1,0,3},  //COLOR_BGR
                          {1,2,0,3}   //COLOR_BRG
                         };

    long max_data=(long)RGBA + datasize;

    if ((sx==0)||(sy==0)) return;
    for (long i = 0; i < (sx) * sy ; ++i) {
        int R, G, B, A;

        R =  RGBA[rgb_matrix[rgb_type][0]];
        G  = RGBA[rgb_matrix[rgb_type][1]];
        B  = RGBA[rgb_matrix[rgb_type][2]];
        A  = RGBA[rgb_matrix[rgb_type][3]];

        RGBA += 4;

        rgb[0]=R;
        rgb[1]=G;
        rgb[2]=B;
        rgb += 3;

        if (max_data<=(long)RGBA) return;

    }
}



void caCamera::buf_unpack_12bitpacked_msb(void* target, void* source, size_t destcount, size_t targetcount)
{
    size_t x1, x2;
    unsigned char b0, b1, b2;
    for (x1 = 0, x2 = 0; x2 < (destcount / 2); x1 = x1 + 3, x2 = x2 + 2) {
        b0 = ((char*) source) [x1];
        b1 = ((char*) source) [x1 + 1];
        b2 = ((char*) source) [x1 + 2];
        ((unsigned short*) target) [x2] = (b1 & 0xf) + (b0 << 4);   // valid for Mono12Packet on IOC
        ((unsigned short*) target) [x2 + 1] = ((b1 & 0xf0) >> 4) + (b2 << 4);
        if (targetcount<x1+3) return;
    }
}

void caCamera::buf_unpack_12bitpacked_lsb(void* target, void* source, size_t destcount, size_t targetcount)
{
    size_t x1, x2;
    unsigned char b0, b1, b2;
    for (x1 = 0, x2 = 0; x2 < (destcount / 2); x1 = x1 + 3, x2 = x2 + 2) {
        b0 = ((char*) source) [x1];
        b1 = ((char*) source) [x1 + 1];
        b2 = ((char*) source) [x1 + 2];
        ((unsigned short*) target) [x2] = ((b1 & 0xf)<<8) + (b0);   // valid for our actual basler camera
        ((unsigned short*) target) [x2 + 1] = ((b1 & 0xf0) >> 4) + (b2 << 4);
        if (targetcount<x1+3) return;
    }
}

void caCamera::buf_unpack_10bitpacked(void* target, void* source, size_t destcount, size_t targetcount)
{
    size_t x1, x2;
    unsigned char b0, b1, b2;
    for (x1 = 0, x2 = 0; x2 < (destcount / 2); x1 = x1 + 3, x2 = x2 + 2) {
        b0 = ((char*) source) [x1];
        b1 = ((char*) source) [x1 + 1];
        b2 = ((char*) source) [x1 + 2];
        ((unsigned short*) target) [x2] = ((b1 & 0x30) >> 4 ) + (b0 << 2);
        ((unsigned short*) target) [x2 + 1] = ((b1 & 0x03) ) + (b2  << 2);
        if (targetcount<x1+3) return;
    }
}

void caCamera::buf_unpack_10bitp(void* target, void* source, size_t destcount, size_t targetcount)
{
    size_t x1, x2;
    unsigned char b0, b1, b2, b3, b4;
    for (x1 = 0, x2 = 0; x2 < (destcount / 2); x1 = x1 + 5, x2 = x2 + 4) {
        b0 = ((char*) source) [x1];
        b1 = ((char*) source) [x1 + 1];
        b2 = ((char*) source) [x1 + 2];
        b3 = ((char*) source) [x1 + 3];
        b4 = ((char*) source) [x1 + 4];

        //((unsigned short*) target) [x2] = ((b1 & 0xC0)>>6) + (b0<<2);   // valid for our actual basler camera
        //((unsigned short*) target) [x2 + 1] = ((b1 & 0x3F)<<4) + ((b2 & 0xF0) >> 4);
        //((unsigned short*) target) [x2 + 2] = ((b2 & 0x0F)<<6) + ((b3 & 0xFC) >> 2);
        //((unsigned short*) target) [x2 + 3] = ((b1 & 0x03)<<8) + (b4);

        ((unsigned short*) target) [x2] = ((b1 & 0x03)) + (b0);   // valid for our actual basler camera
        ((unsigned short*) target) [x2 + 1] = ((b1 & 0xFC)>>2) + ((b2 & 0x0F) << 6);
        ((unsigned short*) target) [x2 + 2] = ((b2 & 0xF0)>>4) + ((b3 & 0x1F) << 4);
        ((unsigned short*) target) [x2 + 3] = ((b3 & 0xC0)>>6) + (b4<<2);


        if (targetcount<x1+5) return;
    }
}






QImage *caCamera::showImageCalc(int datasize, char *data, short datatype)
{
    QSize resultSize;
    uint Max[2], Min[2];
    int tile = BAYER_COLORFILTER_BGGR;// bayer tile
    bool bayerMode = false;
    bool yuvMode = false;
    bool monoMode = false;

    m_datatype = datatype;

    //__itt_event mark_event;

    if(!m_heightDefined) return (QImage *) Q_NULLPTR;
    if(!(m_width > 0) || !(m_height > 0)) {
        savedWidth = m_width;
        savedHeight = m_height;
        return (QImage *) Q_NULLPTR;
    }

    resultSize.setWidth(m_width);
    resultSize.setHeight(m_height);

    switch (thisCompressionmode){
    case non:{
        decompressedData.resize(0);
        break;
    }
    case Zlib:{
        if(data && ( datasize-4)>0){
            uchar* data2=(uchar*)data;
            ulong expectedSize = uint((data2[0] << 24) | (data2[1] << 16) |
                    (data2[2] <<  8) | (data2[3]      ))+16384;
            data2=data2+4;
            decompressedData.resize(expectedSize);
            ZLIB_ULONG newsize=expectedSize;
            int error=uncompress((ZLIB_BYTE *)decompressedData.constData(),&newsize,(ZLIB_BYTE *)data2,datasize-4);

            //decompressedData=qUncompress((uchar*)data, datasize);
            //datasize=decompressedData->size();

            data=(char*)decompressedData.constData();
            savedData = data;
            //printf("datasize=%d:%d (%i)\n",datasize,decompressedData.size(),error);
            datasize=(int)newsize;//decompressedData.size();
        }else{
            datasize=0;
            return (QImage *) Q_NULLPTR;
        }
        break;
    }
    case JPG:{
#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
        printf("not yet supported colormode = JPG\n");
        return (QImage *) Q_NULLPTR;
#else
        QByteArray qdata=QByteArray((const char*)data,datasize);
        QBuffer databuffer(&qdata);
        QImageReader qimg;
        qimg.setDecideFormatFromContent(true);
        qimg.setDevice(&databuffer);
        if (qimg.canRead()){
            QImage* jimage= new QImage(qimg.read());
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            decompressedData.resize(jimage->byteCount());
            memcpy((void*)decompressedData.constData(),(void*)jimage->constBits(),jimage->byteCount());
            datasize=jimage->byteCount();
            //printf("datasize=%d:%d:%d (%i)\n",datasize,decompressedData.size(),jimage->byteCount(),jimage->format());
#else
            decompressedData.resize(jimage->sizeInBytes());
            memcpy((void*)decompressedData.constData(),(void*)jimage->constBits(),jimage->sizeInBytes());
            datasize=jimage->sizeInBytes();
#endif
            delete jimage;
            data=(char*)decompressedData.constData();
            savedData = data;
            savedSizeNew = savedSize = datasize;
            thisColormode = Mono8;
            m_datatype = caCHAR;


        }else{
            reallocate_central_image();
            image->fill(Qt::red);
            return image;

        }
#endif

    }
    }



    // first time get image
    if(m_init || datasize != savedSize || m_width != savedWidth || m_height != savedHeight) {
    //if(m_init || m_width != savedWidth || m_height != savedHeight) {
        savedSizeNew = savedSize = datasize;
        savedWidth = m_width;
        savedHeight = m_height;

        reallocate_central_image();

        //printf("datasize=%d\n",datasize);

        m_init = false;
        minvalue = 0;
        maxvalue = 0xFFFFFFFF;
        ftime(&timeRef);

        if(rgb != (uint*)Q_NULLPTR) free(rgb);
        ulong rgbsize = 3*m_width*m_height*sizeof(uint);
        rgb = (uint *) malloc(rgbsize);

        //printf("rgb(%x) size now define to %d uints => %d chars, received %d chars\n",rgb, 3*m_width*m_height, rgbsize, datasize);
        //fflush(stdout);

        // force resize
        QResizeEvent *re = new QResizeEvent(size(), size());
        resizeEvent(re);
    }


    Max[1] =  0;
    Min[1] = 65535;

    if(data == (void*)Q_NULLPTR) return (QImage *) Q_NULLPTR;
    if(datasize == 0) return (QImage *) Q_NULLPTR;
    if(rgb == (uint *)Q_NULLPTR) {
        printf("caCamera -- could not allocate rgb buffer\n");
        return (QImage *) Q_NULLPTR;
    }



    SyncMinMax MinMax;
    MinMax.Max[1] = 0;
    MinMax.Min[1] = 65535;
    MinMax.MinMaxLock=new QMutex();
    MinMax.imageLock=new QMutex();

    colormode auxMode = thisColormode;
    short auxDatatype = m_datatype;

    int sx = m_width; // resultSize.width();
    int sy = m_height;// resultSize.height();

    void (caCamera::*CameraDataConvert) (int sector, int sectorcount, SyncMinMax* MinMax, QSize resultSize, int datasize) = NULL;

    //printf("datatype=%d %s colormode=%d %s\n", datatype, caTypeStr[datatype], thisColormode, qasc(colorModeString.at(thisColormode)));
   // printf("thisColormode %i\n",thisColormode);
    fflush(stdout);

    switch (thisColormode) {
    case Mono:
        savedData = data;
        CameraDataConvert = &caCamera::CameraDataConvert;
        break;

    case Mono12p:{
        bitsPerElement = 16;
        monoMode=true;
        thisColormode = Mono;
        m_datatype = caINT;
        if(thisPackingmode == LSB12Bit) buf_unpack_12bitpacked_lsb(rgb, (uchar*) data, sx*sy*2,datasize);
        if(thisPackingmode == MSB12Bit) buf_unpack_12bitpacked_msb(rgb, (uchar*) data, sx*sy*2,datasize);
        if(thisPackingmode == packNo)   memcpy(rgb,data,datasize);
        savedData= (char *) rgb;
        savedSizeNew = 2*sx*sy*sizeof(uint);
        CameraDataConvert = &caCamera::CameraDataConvert;
        break;
        }
    case Mono10p:
        bitsPerElement = 10;
        monoMode=true;
        thisColormode = Mono;
        m_datatype = caINT;
        buf_unpack_10bitp(rgb, (uchar*) data, sx*sy*2,datasize);
        savedData= (char *) rgb;
        savedSizeNew = 2*sx*sy*sizeof(uint);
        CameraDataConvert = &caCamera::CameraDataConvert;
        break;
    case Mono10Packed:
        bitsPerElement = 10;
        monoMode=true;
        thisColormode = Mono;
        m_datatype = caINT;
        buf_unpack_10bitpacked(rgb, (uchar*) data, sx*sy*2,datasize);
        savedData= (char *) rgb;
        savedSizeNew = 2*sx*sy*sizeof(uint);
        CameraDataConvert = &caCamera::CameraDataConvert;
        break;

    case Mono8:
        bitsPerElement = 8;
        monoMode=true;
        thisColormode = Mono;
        savedData=data;

        m_datatype = caCHAR;
        CameraDataConvert = &caCamera::CameraDataConvert;
        break;

    case RGB1_CA:
    case RGB2_CA:
    case RGB3_CA:
        savedData = data;
        CameraDataConvert = &caCamera::CameraDataConvert;
        break;
    case BayerRG_8:
    case BayerGB_8:
    case BayerGR_8:
    case BayerBG_8:
    case BayerRG_12:
    case BayerGB_12:
    case BayerGR_12:
    case BayerBG_12:
        bayerMode = true;
        // which tile to use
        if     ((thisColormode == BayerRG_8) || (thisColormode == BayerRG_12)) tile = BAYER_COLORFILTER_RGGB;
        else if((thisColormode == BayerGB_8) || (thisColormode == BayerGB_12)) tile = BAYER_COLORFILTER_GBRG;
        else if((thisColormode == BayerGR_8) || (thisColormode == BayerGR_12)) tile = BAYER_COLORFILTER_GRBG;
        else if((thisColormode == BayerBG_8) || (thisColormode == BayerBG_12)) tile = BAYER_COLORFILTER_BGGR;
        // how many bits per element and packing
        if((thisColormode == BayerRG_8) || (thisColormode == BayerGB_8) || (thisColormode == BayerGR_8) || (thisColormode == BayerBG_8)) {
            bitsPerElement = 8;
        } else if((thisColormode == BayerRG_12) || (thisColormode == BayerGB_12) || (thisColormode == BayerGR_12) || (thisColormode == BayerBG_12)) {
            bitsPerElement = 12;
        }
        thisColormode = RGB1_CA;
        m_datatype = caLONG;

        //printf("bitsperlement=%d datasize=%d sx=%d sy=%d\n",bitsPerElement,  datasize, sx, sy);
        //fflush(stdout);
        if(bitsPerElement == 8) {
            FilterBayer((uchar *) data, rgb, sx, sy, tile,datasize);
        } else if((bitsPerElement == 12) && (thisPackingmode == packNo)) {
            FilterBayer((ushort *) data, rgb, sx, sy, tile,datasize);
        } else if((bitsPerElement == 12) && (thisPackingmode > packNo)) {
            int unpacked_datasize=2*sizeof(ushort) * datasize + 1;
            ushort *unpacked = (ushort *) malloc(unpacked_datasize);
            if(thisPackingmode == LSB12Bit) buf_unpack_12bitpacked_lsb(unpacked, (uchar*) data, sx*sy*2,datasize);
            else buf_unpack_12bitpacked_msb(unpacked, (uchar*) data, sx*sy*2,datasize);
            FilterBayer((ushort *) unpacked, rgb, sx, sy, tile, unpacked_datasize);
            free(unpacked);
        }

        savedData= (char *) rgb;
        savedSizeNew = 3*sx*sy*sizeof(uint);

        CameraDataConvert = &caCamera::CameraDataConvert;
        break;

    case RGB_8:
    case BGR_8:
    case RGBA_8:
    case BGRA_8:
        bayerMode = true;

        if(thisColormode == RGB_8) PROC_RGB8((uchar *) data, COLOR_RGB ,rgb, sx, sy,datasize);
        else if(thisColormode == BGR_8) PROC_RGB8((uchar *) data, COLOR_BGR ,rgb, sx, sy,datasize);
        else if(thisColormode == RGBA_8) PROC_RGBA8((uchar *) data, COLOR_RGB ,rgb, sx, sy,datasize);
        else if(thisColormode == BGRA_8) PROC_RGBA8((uchar *) data, COLOR_BGR ,rgb, sx, sy,datasize);

        thisColormode = RGB1_CA;
        m_datatype = caLONG;
        savedData= (char *) rgb;
        savedSizeNew = 3*sx*sy*sizeof(uint);
        CameraDataConvert = &caCamera::CameraDataConvert;
        break;

    case YUV411:
    case YUV422:
    case YUV444:
        yuvMode = true;

        if(thisColormode == YUV411) {

            if (thisPackingmode==Reversed){
                PROC_UYYVYY411((uchar *) data, rgb, sx, sy,datasize);
            } else{
                PROC_YYUYYV411((uchar *) data, rgb, sx, sy,datasize);
            }

        } else if(thisColormode == YUV422) {

            if (thisPackingmode==Reversed){
                PROC_UYVY422((uchar *) data, rgb, sx, sy,datasize);
            } else{
                PROC_YUYV422((uchar *) data, rgb, sx, sy,datasize);
            }

        } else if(thisColormode == YUV444) {

            if (thisPackingmode==Reversed){
                PROC_UVY444((uchar *) data, rgb, sx, sy,datasize);
            } else{
                PROC_YUV444((uchar *) data, rgb, sx, sy,datasize);
            }
        }

        thisColormode = RGB1_CA;
        m_datatype = caLONG;
        savedData= (char *) rgb;
        savedSizeNew = 3*sx*sy*sizeof(uint);
        CameraDataConvert = &caCamera::CameraDataConvert;
        break;

    case YUV421:
    default:
        savedData = data;
        //printf("not yet supported colormode = %s\n", qasc(colorModeString.at(thisColormode)));
        QPainter painter(image);
        QBrush brush(QColor(200,200,200,255), Qt::SolidPattern);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::black);
        painter.fillRect(rect(), brush);
        painter.setFont(QFont("Arial", width() / 30));
        int lineHeight = 1.2 * painter.fontMetrics().height();
        painter.drawText(5, 10 + lineHeight, "specified format not supported:"+colorModeString.at(thisColormode));
        painter.drawText(5, 10 + 2 * lineHeight, "only supported now:");
        painter.drawText(5, 10 + 3 * lineHeight, "mono");
        painter.drawText(5, 10 + 4 * lineHeight, "rgb1_ca, rgb1_ca, rgb3_ca");
        painter.drawText(5, 10 + 5 * lineHeight, "bayer8, bayer12 unpacked and packed");
        painter.drawText(5, 10 + 6 * lineHeight, "yuv formats and reversed except yuv421");
        painter.drawText(5, 10 + 7 * lineHeight, "HW Ref.:  Basler acA4600-10uc/acA1300-30gc  ");
        painter.drawText(5, 10 + 8 * lineHeight, "HW Ref.:  Prosilica GC1660C  ");

        delete MinMax.MinMaxLock;
        delete MinMax.imageLock;

        return image;
    }

#ifndef QT_NO_CONCURRENT

    //mark_event = __itt_event_create( "User Mark", 9 );
    //__itt_event_start( mark_event );

    int threadcounter=QThread::idealThreadCount()*2/3;  // seems to be a magic number
    if(threadcounter < 1) threadcounter = 1;

    QFutureSynchronizer<void> Sectors;
    for (int x=0;x<threadcounter;x++){
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        Sectors.addFuture(QtConcurrent::run(this, CameraDataConvert, x, threadcounter, &MinMax, resultSize, savedSizeNew));
#else
        Sectors.addFuture(QtConcurrent::run(CameraDataConvert,this,x, threadcounter, &MinMax, resultSize, savedSizeNew));
#endif


    }
    Sectors.waitForFinished();
    //__itt_event_end( mark_event );

#else

    (this->*CameraDataConvert)(0, 1, &MinMax, resultSize, savedSizeNew);
#endif

    delete MinMax.MinMaxLock;
    delete MinMax.imageLock;
    Max[1]=MinMax.Max[1];
    Min[1]=MinMax.Min[1];

    minvalue = Min[1];
    maxvalue= Max[1];

    if(maxvalue == minvalue) {
        maxvalue = maxvalue +1;
        minvalue = minvalue -1;
        if(maxvalue > 0xFFFFFFFE) maxvalue = 0xFFFFFFFE;
    }

    if(bayerMode || yuvMode || monoMode) {
        thisColormode = auxMode;
        m_datatype = auxDatatype;
    }
   // delete decompressedData;
    return image;
}

void caCamera::showImage(int datasize, char *data, short datatype)
{
    //QElapsedTimer timer;
    //timer.start();
    QImage *localimage = showImageCalc(datasize, data, datatype);
    //printf("Image timer 1 : %d (%x) milliseconds \n", (int) timer.elapsed(),image);
    //fflush(stdout);

    if(localimage != (QImage *)Q_NULLPTR) updateImage(*localimage, readvaluesPresent, readvalues, scaleFactor, X, Y);

    if(getAutomateChecked()) {
        updateMax(maxvalue);
        updateMin(minvalue);
    } else {
        int minv = getMin();
        int maxv = getMax();
        if(maxv >= minv) {
            maxvalue = maxv;
            minvalue = minv;
        } else {
            maxvalue = minv;
            minvalue = maxv;
        }
        if(maxvalue == minvalue) maxvalue = minvalue + 1000;
    }

    UpdatesPerSecond++;
}

void caCamera::setData(double *array, int size, int curvIndex, int curvType, int curvXY)
{
    fillData(array, size, curvIndex, curvType, curvXY);
}

void caCamera::setData(float *array, int size, int curvIndex, int curvType, int curvXY)
{
    fillData(array, size, curvIndex, curvType, curvXY);
}

void caCamera::setData(int16_t *array, int size, int curvIndex, int curvType, int curvXY)
{
    fillData(array, size, curvIndex, curvType, curvXY);
}

void caCamera::setData(int32_t *array, int size, int curvIndex, int curvType, int curvXY)
{
    fillData(array, size, curvIndex, curvType, curvXY);
}

void caCamera::setData(int8_t *array, int size, int curvIndex, int curvType, int curvXY)
{
    fillData(array, size, curvIndex, curvType, curvXY);
}

template <typename pureData>
void caCamera::fillData(pureData *array, int size, int curvIndex, int curvType, int curvXY)
{
    Q_UNUSED(curvIndex);
    Q_UNUSED(curvType);
    // keep data points
    if(curvXY == CH_X) {                       // X
        X.resize(size);
        double *data = X.data();
        for(int i=0; i<  size; i++) data[i] = array[i];
    } else {                                   // Y
        Y.resize(size);
        double *data = Y.data();
        for(int i=0; i<  size; i++) data[i] = array[i];
    }
}

void caCamera::setAccessW(bool access)
{
    _AccessW = access;
}
