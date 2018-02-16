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

    rgb = (uint*) 0;

    thisSimpleView = false;
    thisFitToSize = No;
    savedSize = 0;
    savedWidth = 0;
    savedHeight = 0;
    selectionInProgress = false;

    thisPV_Xaverage = "";
    thisPV_Yaverage = "";
    thisPV_Mode = "";
    thisPV_Packing = "";

    savedData = (char*) 0;
    initWidgets();

    Xpos = Ypos = 0;

    scrollArea = (QScrollArea *) 0;

    mainLayout = new QGridLayout(this);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);
    setLayout(mainLayout);
    setup();

    setColormode(Mono);
    setPackmode(packNo);
    setColormodeStrings();
    setPackingModeStrings();

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

    startTimer(1000);

    // __itt_thread_set_name("My worker thread");

    writeTimer = new QTimer(this);
    connect(writeTimer, SIGNAL(timeout()), this, SLOT(updateChannels()));
}

void caCamera::setColormodeStrings()
{
   colorModeString <<  "Mono" << "RGB1" << "RGB2" << "RGB3" << "BayerRG_8" << "BayerGB_8" << "BayerGR_8" << "BayerBG_8" <<
                    "BayerRG_12" << "BayerGB_12" << "BayerGR_12" << "BayerBG_12" <<
                    "YUV444" << "YUV422" << "YUV421";
}

void caCamera::setPackingModeStrings()
{
    packingModeString <<  "packNo" << "MSB12Bit" <<  "LSB12Bit";
}

void caCamera::setDecodemodeStr(QString mode) {
    if(mode.length() == 0) return;
    printf("colormodeset with %s\n", qasc(mode));
    for(int i = 0; i< colorModeString.count(); i++) {
        if(mode == colorModeString.at(i)) {
            thisColormode = (colormode) i;
        }
    }
}

bool caCamera::testDecodemodeStr(QString mode) {
    if(mode.length() == 0) return false;
    printf("colormodeset with %s\n", qasc(mode));
    for(int i = 0; i< colorModeString.count(); i++) {
        if(mode == colorModeString.at(i)) {
            return true;
        }
    }
    return false;
}

void caCamera::setPackingmodeStr(QString mode) {
    if(mode.length() == 0) return;
    printf("colormodeset with %s\n", qasc(mode));
    for(int i = 0; i< packingModeString.count(); i++) {
        if(mode == packingModeString.at(i)) {
            thisPackingmode = (packingmode) i;
        }
    }
}

bool caCamera::testPackingmodeStr(QString mode) {
    if(mode.length() == 0) return false;
    printf("colormodeset with %s\n", qasc(mode));
    for(int i = 0; i< packingModeString.count(); i++) {
        if(mode == packingModeString.at(i)) {
            return true;
        }
    }
    return false;
}

void caCamera::setDecodemodeNum(int mode) {
    printf("colormodeset with %d\n", mode);
    thisColormode = (colormode) mode;
}

void caCamera::setDecodemodeNum(double mode) {
    printf("colormodeset with %d\n", (int) mode);
    int intermed = (int)mode;
    thisColormode = (colormode) intermed; // direct not allowed on Windows (C2440)
}

void caCamera::setPackingmodeNum(int mode) {
    printf("packingmodeset with %d\n", mode);
    thisPackingmode = (packingmode) mode;
}

void caCamera::setPackingmodeNum(double mode) {
    printf("packingmodeset with %d\n", (int) mode);
    int intermed = (int)mode;
    thisPackingmode = (packingmode) intermed;// direct not allowed on Windows (C2440)

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

void caCamera::initWidgets()
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
    if(nbUpdatesText != (caLabel*) 0) nbUpdatesText->setText(text);
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

    if(buttonPressed && (savedData != (char*) 0)) {
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

void caCamera::setup()
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
        connect(zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(zoomNow()));

        // add everything to main layout
        mainLayout->addWidget(valuesWidget, 0, 0);
        mainLayout->addWidget(scrollArea, 1, 0);
        mainLayout->addWidget(zoomWidget, 1, 2);

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
    if(image != (QImage *) 0)  imageW->update();
}

void caCamera::zoomNow()
{
    double scale = qPow(2.0, ((double) zoomSlider->value() - 52.0) / 13.0);
    if(scale > 32) scale = 32;
    zoomValue->setText(QString::number(scale, 'f', 3));
    scaleFactor = scale;
    setFitToSize(No);

    // keep centered on last pick
    int posX =  P3.x() * scaleFactor;
    int posY =  P3.y() * scaleFactor;
    scrollArea->horizontalScrollBar()->setValue(posX - scrollArea->horizontalScrollBar()->pageStep()/2);
    scrollArea->verticalScrollBar()->setValue(posY - scrollArea->verticalScrollBar()->pageStep()/2);
}

void caCamera::zoomIn(int level)
{
    zoomSlider->setValue(zoomSlider->value() + level);
}

void caCamera::zoomOut(int level)
{
    zoomSlider->setValue(zoomSlider->value() - level);
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
    if(autoW == (QCheckBox *) 0) return false;
    return autoW->isChecked();
}

void caCamera::setInitialAutomatic(bool automatic)
{
    if(thisSimpleView) thisInitialAutomatic = automatic;
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
    if(zoomWidget != (QWidget*) 0) zoomWidget->adjustSize();
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

        } else if((zoomWidget != (QWidget*) 0) && (valuesWidget != (QWidget*) 0)) {
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

            // define the middle of the image for zooming on center of the image
            double Xnew, Ynew, Xmax, Ymax;
            Coordinates((int) (m_width * scaleFactor)/2,  (int) (m_height * scaleFactor)/2,  Xnew, Ynew, Xmax, Ymax);
            P3 = QPointF(Xnew, Ynew);
        }
    }
    if(image != (QImage *) 0)  imageW->rescaleSelectionBox(scaleFactor);
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
    if(autoW == (QCheckBox *) 0) return false;
    return autoW->isChecked();
}

void caCamera::updateMax(int max)
{
    if(labelMax == (caLineEdit*) 0) return;
    labelMax->setText(QString::number(max));
}

void caCamera::updateMin(int min)
{
    if(labelMin == (caLineEdit*) 0) return;
    labelMin->setText(QString::number(min));
}

void caCamera::updateIntensity(QString strng)
{
    if(intensity == (caLabel*) 0) return;
    intensity->setText(strng);
}

int caCamera::getMin()
{
    bool ok;
    if(thisSimpleView) return  thisMinLevel.toInt(&ok);
    if(labelMin == (caLineEdit*) 0) return 0;
    return labelMin->text().toInt();
}
int caCamera::getMax()
{
    bool ok;
    if(thisSimpleView) return  thisMaxLevel.toInt(&ok);
    if(labelMax == (caLineEdit*) 0) return 65535;
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
    if(image != (QImage *) 0) {
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
    if(image != (QImage *) 0) {
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

void caCamera::InitLoopdata(int &ystart, int &yend, long &i, QVector<uint> &LineData, int increment, int sector, int sectorcount, QSize resultSize, uint Max[2], uint Min[2])
{
    LineData.resize(resultSize.width());

    Max[1] = 0;
    Min[1] = 65535;

    ystart = sector * resultSize.height() / sectorcount;
    yend = ((sector + 1) * resultSize.height()) / sectorcount;
    i = resultSize.width() * ystart * increment;
}

void caCamera::InitLoopdataNew(int &ystart, int &yend, long &i, int increment, int sector, int sectorcount, QSize resultSize, uint Max[2], uint Min[2])
{
    Max[1] = 0;
    Min[1] = 65535;

    ystart = sector * resultSize.height() / sectorcount;
    yend = ((sector + 1) * resultSize.height()) / sectorcount;
    i = resultSize.width() * ystart * increment;
}

// I leave the code now as it was, while here we use uint and in calcimage QVector<uint>. I can merge later on.
template <typename pureData>
void caCamera::calcImageMono (pureData *ptr,  uint *LineData, long &i, int &ystart, int &yend, float correction, int datasize, QSize resultSize,
                              uint Max[2], uint Min[2])
{
    if(i > datasize) return;
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
    int  increment;

    if(mode == RGB3) {          // interleaved
        offset1 = savedHeight * savedWidth;
        offset2 = 2 * offset1;
        increment = 1;
    } else if(mode == RGB2) {   // row
        offset1 = savedWidth;
        offset2 = 2 * offset1;
        offset3 = savedWidth * 2;
        increment = 1;
    } if(mode ==  YUV422){
        increment = 3;
    }else {
        increment = 3;
    }

    if((i + offset2 + offset3) > datasize) return;

        // normal rgb display
        float redcoeff = correction * thisRedCoefficient;
        float greencoeff = correction * thisGreenCoefficient;
        float bluecoeff = correction * thisBlueCoefficient;


        if(thisColormap == as_is || thisColormap > color_to_mono) {
            for (int y = ystart; y < yend; ++y) {
                for (int x = 0; x < resultSize.width(); ++x) {
                    uint intensity = qMax(qMax(ptr[i], ptr[i+offset1]), ptr[i+offset2]);
                    LineData[x] =  qRgb((int) (ptr[i] * redcoeff), (int) (ptr[i+offset1] * greencoeff), (int) (ptr[i+offset2] * bluecoeff));
                    i += increment;
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
                    i += increment;
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

    if(m_datatype == caINT) elementSize = 2;
    else if(m_datatype == caLONG || m_datatype == caFLOAT) elementSize = 4;
    else if(m_datatype == caDOUBLE) elementSize = 8;

    if(thisColormode == Mono) {

        uint *LineData;
        InitLoopdataNew(ystart, yend, i, 1, sector, sectorcount, resultSize, Max, Min);
        LineData = (uint *) malloc(resultSize.width() * sizeof(uint) * (yend-ystart));

        // instead of testing in the big loop, subtract a line when sizes do not fit
        bool notOK = true;
        while (notOK) {
            long SizeToTreat = (yend-ystart) * resultSize.width() * elementSize;
            if(SizeToTreat > datasize) {
                yend -= 1;
                if(yend < ystart) {
                    printf("something really wrong between datasize and image width and height\n");
                    free (LineData);
                    return;
                }
                printf("datasize=%d ystart=%d yend=%d width=%d\n", datasize, ystart, yend, resultSize.width());
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
            if((ulong) i*sizeof(uchar) >= (uint) datasize) return;
            calcImageMono ((uchar*) savedData, LineData, i, ystart, yend, correction, datasize, resultSize, Max, Min);
            break;
        case caINT:
            if((ulong) i*sizeof(ushort) >= (uint) datasize) return;
            calcImageMono ((ushort*) savedData, LineData, i, ystart, yend, correction, datasize/elementSize, resultSize, Max, Min);
            break;
        case caLONG:
            if((ulong) i*sizeof(uint) >= (uint) datasize) return;
            calcImageMono ((uint*) savedData, LineData, i, ystart, yend, correction, datasize/elementSize, resultSize, Max, Min);
            break;
        case caFLOAT:
            if((ulong) i*sizeof(float) >= (uint) datasize) return;
            calcImageMono ((float*) savedData,  LineData, i, ystart, yend, correction, datasize/elementSize, resultSize, Max, Min);
            break;
        case caDOUBLE:
            if((ulong) i*sizeof(double) >= (uint) datasize) return;
            calcImageMono ((double*) savedData, LineData, i, ystart, yend, correction, datasize/elementSize, resultSize, Max, Min);
            break;
        default:
            printf("CameraDataConvert -- data format not supported\n");
        }

        MinMaxImageLockBlock(LineData, ystart, yend, resultSize, MinMax);
        MinMaxLock(MinMax, Max, Min);
        free(LineData);
    } else  {
        QVector<uint> LineData;

        if(maxvalue != 0) correction = 255.0 / (float) maxvalue;

        int increment = 1;
        if(thisColormode == RGB1) increment = 3;
        if(thisColormode == RGB2) increment = 3;
        if(thisColormode == YUV422) increment = 3;

        InitLoopdata(ystart, yend, i, LineData, increment, sector, sectorcount, resultSize, Max, Min);
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
            printf("CameraDataConvert -- data format not supported\n");
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
template <typename pureData> void caCamera::FilterBayer(pureData *bayer, uint *rgb, int sx, int sy, int tile)
{
    const int bayerStep = sx;
    const int rgbStep = 3 * sx;

    int width = sx;
    int height = sy;
    int blue = tile == BAYER_COLORFILTER_BGGR || tile == BAYER_COLORFILTER_GBRG ? -1 : 1;
    int start_with_green = tile == BAYER_COLORFILTER_GBRG|| tile == BAYER_COLORFILTER_GRBG;
    int i, iinc, imax;

    if ((tile > BAYER_COLORFILTER_MAX) || (tile < BAYER_COLORFILTER_MIN)) {
        printf("bayer, invalid filter\n");
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

    return;
}
//https://en.wikipedia.org/wiki/Chroma_subsampling
//https://en.wikipedia.org/wiki/YCbCr
#define GET_R_FROM_YCbCr(y,cb,cr) 298.082*y/256 +                      408.583 * cr / 256 - 222.291 ;
#define GET_G_FROM_YCbCr(y,cb,cr) 298.082*y/256 - 100.291 * cb / 256 - 208.120 * cr / 256 + 135.576 ;
#define GET_B_FROM_YCbCr(y,cb,cr) 298.082*y/256 + 561.412 * cb / 256                      - 276.836 ;

//#define GET_R_FROM_YCbCr(y,cb,cr) (y + 1.40200 * (cr - 0x80));
//#define GET_G_FROM_YCbCr(y,cb,cr) (y - 0.34414 * (cb - 0x80) - 0.71414 * (cr - 0x80));
//#define GET_B_FROM_YCbCr(y,cb,cr) (y + 1.77200 * (cb - 0x80));


void caCamera::YCbCr422(uchar *YCbCr, uint *rgb, int sx, int sy)  // 4 bytes for 2 pixels
{
    for (long i = 0; i < (sx) * sy / 2; ++i) {
        int Y1, Cr, Y2, Cb;
        int r,g,b;


        // Extract YCbCr components
        Cb = YCbCr[1];
        Y1 = YCbCr[0];
        Cr = YCbCr[3];
        Y2 = YCbCr[2];

        YCbCr += 4;//4;

        rgb[0]=GET_R_FROM_YCbCr(Y1,Cb,Cr);
        rgb[1]=GET_G_FROM_YCbCr(Y1,Cb,Cr);
        rgb[2]=GET_B_FROM_YCbCr(Y1,Cb,Cr);

        //rgb[0] = qMax(0, qMin(255,r ));
        //rgb[1] = qMax(0, qMin(255,g ));
        //rgb[2] = qMax(0, qMin(255,b ));
//        if (i<9){
//            printf("(%i)Y1: %x Y2: %x Cr: %x Cb: %x\n",i,Y1,Y2,Cr,Cb);
//            printf("\t R: %x G: %x B: %x ",rgb[0],rgb[1],rgb[2]);

//        }

        rgb += 3;
        rgb[0]=GET_R_FROM_YCbCr(Y2,Cb,Cr);
        rgb[1]=GET_G_FROM_YCbCr(Y2,Cb,Cr);
        rgb[2]=GET_B_FROM_YCbCr(Y2,Cb,Cr);

        //rgb[0] = qMax(0, qMin(255,r ));
        //rgb[1] = qMax(0, qMin(255,g ));
        //rgb[2] = qMax(0, qMin(255,b ));
        rgb += 3;
//        if (i<9){
//            printf("R2: %x G2: %x B2: %x \n",rgb[0],rgb[1],rgb[2]);
//        }

    }
}


#define GET_R_FROM_YUV(y,u,v) y + 1.370705 * (v-128);
#define GET_G_FROM_YUV(y,u,v) y - 0.698001 * (v-128) - 0.337633 * (u -128);
#define GET_B_FROM_YUV(y,u,v) y + 1.732446 * (u-128);




void caCamera::yuv422(uchar *yuv, uint *rgb, int sx, int sy)  // 4 bytes for 2 pixels
{
    for (int i = 0; i < sx * sy / 4; ++i) {
        int y0, u0, y1, v0, y2, u2, y3, v2;

        // Extract yuv components
        u0 = yuv[0];
        y0 = yuv[1];
        v0 = yuv[2];
        y1 = yuv[3];
        u2 = yuv[4];
        y2 = yuv[5];
        v2 = yuv[6];
        y3 = yuv[7];
        yuv += 8;

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

    }
}

void caCamera::buf_unpack_12bitpacked_msb(void* target, void* source, size_t count)
{
    size_t x1, x2;
    unsigned char b0, b1, b2;
    for (x1 = 0, x2 = 0; x2 < (count / 2); x1 = x1 + 3, x2 = x2 + 2) {
        b0 = ((char*) source) [x1];
        b1 = ((char*) source) [x1 + 1];
        b2 = ((char*) source) [x1 + 2];
        ((unsigned short*) target) [x2] = (b1 & 0xf) + (b0 << 4);   // valid for Mono12Packet on IOC
        ((unsigned short*) target) [x2 + 1] = ((b1 & 0xf0) >> 4) + (b2 << 4);

    }
}

void caCamera::buf_unpack_12bitpacked_lsb(void* target, void* source, size_t count)
{
    size_t x1, x2;
    unsigned char b0, b1, b2;
    for (x1 = 0, x2 = 0; x2 < (count / 2); x1 = x1 + 3, x2 = x2 + 2) {
        b0 = ((char*) source) [x1];
        b1 = ((char*) source) [x1 + 1];
        b2 = ((char*) source) [x1 + 2];
        ((unsigned short*) target) [x2] = ((b1 & 0xf)<<8) + (b0);   // valid for our actual basler camera
        ((unsigned short*) target) [x2 + 1] = ((b1 & 0xf0) >> 4) + (b2 << 4);

    }
}

QImage *caCamera::showImageCalc(int datasize, char *data, short datatype)
{
    QSize resultSize;
    uint Max[2], Min[2];
    int tile = BAYER_COLORFILTER_BGGR;; // bayer tile
    bool bayerMode = false;

    m_datatype = datatype;

    //__itt_event mark_event;

    if(!m_heightDefined) return (QImage *) 0;
    if(!(m_width > 0) || !(m_height > 0)) {
        savedWidth = m_width;
        savedHeight = m_height;
        return (QImage *) 0;
    }

    resultSize.setWidth(m_width);
    resultSize.setHeight(m_height);

    // first time get image
    if(m_init || datasize != savedSize || m_width != savedWidth || m_height != savedHeight) {
        savedSizeNew = savedSize = datasize;
        savedWidth = m_width;
        savedHeight = m_height;

        if(image != (QImage *) 0) {
            delete image;
        }
        image = new QImage(resultSize, QImage::Format_RGB32);

        m_init = false;
        minvalue = 0;
        maxvalue = 0xFFFFFFFF;
        ftime(&timeRef);

        if(rgb != (uint*) 0) free(rgb);
        rgb = (uint *) malloc(3*m_width*m_height*sizeof(uint));

        // force resize
        QResizeEvent *re = new QResizeEvent(size(), size());
        resizeEvent(re);
    }

    Max[1] =  0;
    Min[1] = 65535;

    if(data == (void*) 0) return (QImage *) 0;

    SyncMinMax MinMax;
    MinMax.Max[1] = 0;
    MinMax.Min[1] = 65535;
    MinMax.MinMaxLock=new QMutex();
    MinMax.imageLock=new QMutex();

    colormode auxMode = thisColormode;
    short auxDatatype = m_datatype;

    int sx = resultSize.width();
    int sy = resultSize.height();

    void (caCamera::*CameraDataConvert) (int sector, int sectorcount, SyncMinMax* MinMax, QSize resultSize, int datasize) = NULL;

    //printf("datatype=%d %s colormode=%d %s\n", datatype, caTypeStr[datatype], thisColormode, qasc(colorModeString.at(thisColormode)));

    switch (thisColormode) {
    case Mono:
    case RGB1:
    case RGB2:
    case RGB3:
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
        thisColormode = RGB1;
        m_datatype = caLONG;

        //printf("bitsperlement=%d datasize=%d sx=%d sy=%d\n",bitsPerElement,  datasize, sx, sy);

        if(bitsPerElement == 8) {
            FilterBayer((uchar *) data, rgb, sx, sy, tile);
        } else if((bitsPerElement == 12) && (thisPackingmode == packNo)) {
            FilterBayer((ushort *) data, rgb, sx, sy, tile);
        } else if((bitsPerElement == 12) && (thisPackingmode > packNo)) {
            ushort *unpacked = (ushort *) malloc(16/12*sizeof(ushort) * datasize + 1);
            if(thisPackingmode == LSB12Bit) buf_unpack_12bitpacked_lsb(unpacked, (uchar*) data, datasize);
            else buf_unpack_12bitpacked_msb(unpacked, (uchar*) data, datasize);
            FilterBayer((ushort *) unpacked, rgb, sx, sy, tile);
            free(unpacked);
        }
        savedData= (char *) rgb;
        savedSizeNew = 3*sx*sy*sizeof(uint);
        CameraDataConvert = &caCamera::CameraDataConvert;
        break;

    case YUV422:
        //yuv422((uchar *) data, rgb, sx, sy);
        YCbCr422((uchar *) data, rgb, sx, sy);
        savedData= (char *) rgb;
        savedSizeNew = 3*sx*sy*sizeof(uint);
        m_datatype = caLONG;

        CameraDataConvert = &caCamera::CameraDataConvert;
        break;

    case YUV444:

        YCbCr422((uchar *) data, rgb, sx, sy);
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
        painter.drawText(5, 10 + 4 * lineHeight, "rgb1, rgb1, rgb3");
        painter.drawText(5, 10 + 5 * lineHeight, "bayer8, bayer12 unpacked and packed");
        painter.drawText(5, 10 + 6 * lineHeight, "yuv formats not tested or not implemented");
        return image;
    }

#ifndef QT_NO_CONCURRENT

    //mark_event = __itt_event_create( "User Mark", 9 );
    //__itt_event_start( mark_event );

    int threadcounter=QThread::idealThreadCount()*2/3;  // seems to be a magic number
    if(threadcounter < 1) threadcounter = 1;

    QFutureSynchronizer<void> Sectors;
    for (int x=0;x<threadcounter;x++){
        Sectors.addFuture(QtConcurrent::run(this, CameraDataConvert, x, threadcounter, &MinMax, resultSize, savedSizeNew));
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

    if(bayerMode) {
       thisColormode = auxMode;
       m_datatype = auxDatatype;
    }

    return image;
}

void caCamera::showImage(int datasize, char *data, short datatype)
{
    //QElapsedTimer timer;
    //timer.start();
    image = showImageCalc(datasize, data, datatype);
    //printf("Image timer 1 : %d (%x) milliseconds \n", (int) timer.elapsed(),image);

    fflush(stdout);
    if(image != (QImage *) 0) updateImage(*image, readvaluesPresent, readvalues, scaleFactor, X, Y);

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
