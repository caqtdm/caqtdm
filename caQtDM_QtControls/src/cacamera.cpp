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
#include <QtGui>
#include<QApplication>
#include <math.h>
#ifndef QT_NO_CONCURRENT
#include <qtconcurrentrun.h>
#endif
#include "cacamera.h"

caCamera::caCamera(QWidget *parent) : QWidget(parent)
{
    m_init = true;

    m_codeDefined = false;
    m_bppDefined = false;
    m_widthDefined = false;
    m_heightDefined = false;

    thisSimpleView = false;
    savedSize = 0;
    savedWidth = 0;
    savedHeight = 0;
    selectionInProgress = false;

    savedData = (char*) 0;

    initWidgets();

    Xpos = Ypos = 0;

    scrollArea = (QScrollArea *) 0;

    mainLayout = new QGridLayout(this);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);
    setLayout(mainLayout);
    setup();

    setColormap(spectrum_wavelength);
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
    startTimer(1000);

    writeTimer = new QTimer(this);
    connect(writeTimer, SIGNAL(timeout()), this, SLOT(updateChannels()));
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
    QString text= "%1 U/s";
    text = text.arg(UpdatesPerSecond);
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
            P1 = QPointF(Xnew, Ynew);
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

            // for gray selection rectangle
            QPoint mouseOffset = mouseEvent->pos() ;
            x1 = mouseOffset.x() + scrollArea->horizontalScrollBar()->value();
            y1 = mouseOffset.y() - valuesWidget->height() + scrollArea->verticalScrollBar()->value();
            selectionPoints[1] = QPoint(x1, y1);
        } else {
            P1 = P2 = QPointF(Xnew, Ynew);
        }
    }

    if(buttonPressed) imageW->updateSelectionBox(selectionPoints, selectionInProgress);

    if(buttonPressed && (savedData != (char*) 0)) {
        double Xnew, Ynew, Xmax, Ymax;
        validIntensity = true;

        Coordinates(Xpos, Ypos, Xnew, Ynew, Xmax, Ymax);

        // find intensity
        switch (m_code) {

        // monochrome image
        case 1:{

            switch (m_bpp) {

            case 1:  {  // monochrome 1 bpp  (Damir camera)
                uchar *ptr = (uchar*)  savedData;
                int index = ((int) Ynew * savedWidth + (int) Xnew);
                if((Xnew >=0) && (Ynew >=0)  && (Xnew < Xmax) && (Ynew < Ymax) && (index < savedSize)) {
                    Zvalue=ptr[index];

                } else {
                    validIntensity = false;
                }
            }
                break;

            case 2: {   // monochrome 2 bpp  (Damir camera)
                uchar *ptr = (uchar*)  savedData;
                int index = ((int) Ynew * savedWidth * 2 + 2 * (int) Xnew);
                if((Xnew >=0) && (Ynew >=0)  &&  (Xnew < Xmax) && (Ynew < Ymax) && (index < savedSize)) {
                    if(thisColormap == grey) Zvalue=ptr[index];
                    else Zvalue=ptr[index] * 256 + ptr[index+1];

                } else {
                    validIntensity = false;
                }
            }
                break;

            case 3: {   // monochrome 2 bpp, but used only 12 bits  (Helge cameras)
                QSize resultSize;
                resultSize.setWidth(savedWidth);
                resultSize.setHeight(savedHeight);
                ushort *ptr = (ushort*) savedData;
                int index = ((int) Ynew * savedWidth + (int) Xnew);
                if((Xnew >=0) && (Ynew >=0) &&  (Xnew < Xmax) && (Ynew < Ymax) && ((index+resultSize.width())*2 < savedSize))
                    Zvalue = ptr[index];
                else
                    validIntensity = false;
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
            {
                QSize resultSize;
                resultSize.setWidth(savedWidth);
                resultSize.setHeight(savedHeight);
                uchar *ptr = (uchar*) savedData;
                int index = ((int) Ynew * savedWidth*3 + 3 * (int) Xnew);
                if((Xnew >=0) && (Ynew >=0) &&  (Xnew < Xmax) && (Ynew < Ymax) && ((index+2) < savedSize)) {
                    if(thisColormap != grey)
                        Zvalue =(int) (2.2 * ( 0.2989 * ptr[index] +  0.5870 * ptr[index+1] + 0.1140 * ptr[index+2]));
                    else
                        Zvalue = (int) (0.2989 * ptr[index] +  0.5870 * ptr[index+1] + 0.1140 * ptr[index+2]);
                } else {
                    validIntensity = false;
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

        QString strng = "%1, %2, %3";
        if(validIntensity) {
            strng = strng.arg(int(Xnew)).arg(int(Ynew)).arg(Zvalue);
            updateIntensity(strng);
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

void caCamera::zoomNow()
{
    double scale = qPow(2.0, ((double) zoomSlider->value() - 52.0) / 13.0);
    if(scale > 32) scale = 32;
    zoomValue->setText(QString::number(scale, 'f', 3));
    scaleFactor = scale;
    setFitToSize(No);
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
        }
    }
    if(image != (QImage *) 0)  imageW->rescaleSelectionBox(scaleFactor);
}

void caCamera::updateImage(const QImage &image, bool valuesPresent[], double values[], double scaleFactor)
{
    imageW->updateImage(thisFitToSize, image, valuesPresent, values, scaleFactor, thisSimpleView,
                        (short) getROIreadmarkerType(), (short) getROIreadType(),
                        (short) getROIwritemarkerType(), (short) getROIwriteType());
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

    resultSize.setWidth(m_width);
    resultSize.setHeight(m_height);

    // first time get image
    if(m_init || datasize != savedSize || m_width != savedWidth || m_height != savedHeight) {
        savedSize = datasize;
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

        // force resize
        QResizeEvent *re = new QResizeEvent(size(), size());
        resizeEvent(re);
    }

    savedData = data;

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

            if(thisColormap != grey) {

                for (int y = 0; y < resultSize.height(); ++y) {
                    uint *scanLine = reinterpret_cast<uint *>(image->scanLine(y));

                    for (int x = 0; x < resultSize.width(); ++x) {
                        indx = ptr[i]; i+=m_bpp;
                        indx1 = (indx - minvalue) * (ColormapSize-1) / (maxvalue - minvalue);
                        if(indx1 >= ColormapSize) indx1=ColormapSize -1;

                        *scanLine++ = ColorMap[indx1];

                        Max[(indx > Max[1])] = indx;
                        Min[(indx < Min[1])] = indx;
                        if(i >= datasize) break;
                    }
                    if(i >= datasize) break;
                }

            } else {
                for (int y = 0; y < resultSize.height(); ++y) {
                    uint *scanLine = reinterpret_cast<uint *>(image->scanLine(y));

                    for (int x = 0; x < resultSize.width(); ++x) {
                        indx = ptr[i]; i+=m_bpp;
                        *scanLine++ = ColorMap[indx];

                        Max[(indx > Max[1])] = indx;
                        Min[(indx < Min[1])] = indx;
                        if(i >= datasize) break;
                    }
                    if(i >= datasize) break;
                }
            }
        }
            break;

        case 2: {  // monochrome 2 bpp, but use only first byte of words (Damir cameras)

            uchar *ptr = (uchar*)  data;
            if(ptr == (void*) 0) return (QImage *) 0 ;

            if(thisColormap != grey) {

                for (int y = 0; y < resultSize.height(); ++y) {
                    uint *scanLine = reinterpret_cast<uint *>(image->scanLine(y));

                    for (int x = 0; x < resultSize.width(); ++x) {
                        indx = ptr[i]*256 + ptr[i+1]; i+=m_bpp;
                        indx1 = (indx - minvalue) * (ColormapSize-1) / (maxvalue - minvalue);
                        if(indx1 >= ColormapSize) indx1=ColormapSize -1;

                        *scanLine++ = ColorMap[indx1];

                        Max[(indx > Max[1])] = indx;
                        Min[(indx < Min[1])] = indx;
                        if(i >= datasize) break;
                    }
                    if(i >= datasize) break;
                }
            } else {
                for (int y = 0; y < resultSize.height(); ++y) {
                    uint *scanLine = reinterpret_cast<uint *>(image->scanLine(y));

                    for (int x = 0; x < resultSize.width(); ++x) {
                        indx = ptr[i]; i+=m_bpp;
                        *scanLine++ = ColorMap[indx];

                        Max[(indx > Max[1])] = indx;
                        Min[(indx < Min[1])] = indx;
                        if(i >= datasize) break;
                    }
                    if(i >= datasize) break;
                }
            }
        }
            break;

        case 3: {   // monochrome 2 bpp, but used only 12 bits  (Helge cameras)

            ushort *ptr = (ushort*) data;

            if(ptr == (void*) 0) return (QImage *) 0;

            if(thisColormap != grey) {

                for (int y = 0; y < resultSize.height(); ++y) {
                    uint *scanLine = reinterpret_cast<uint *>(image->scanLine(y));

                    for (int x = 0; x < resultSize.width(); ++x) {
                        indx = ptr[i++];
                        indx1 = (indx - minvalue) * (ColormapSize-1) / (maxvalue - minvalue);
                        if(indx1 >= ColormapSize) indx1=ColormapSize -1;

                        *scanLine++ = ColorMap[indx1];

                        Max[(indx > Max[1])] = indx;
                        Min[(indx < Min[1])] = indx;
						if ((i * 2) >= datasize) break;
                    }
                    if((i*2) >= datasize) break;
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
						if((i*2) >= datasize) break;
                    }
					if ((i * 2) >= datasize) break;
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
            if(thisColormap != grey) {
                uint intensity;
                uchar *ptr = (uchar*)  data;
                if(ptr == (void*) 0) return (QImage *) 0;

                for (int y = 0; y < resultSize.height(); ++y) {
                    uint *scanLine = reinterpret_cast<uint *>(image->scanLine(y));
                    for (int x = 0; x < resultSize.width(); ++x) {
                        intensity = (int) (2.2 * (0.2989 * ptr[i] +  0.5870 * ptr[i+1] + 0.1140 * ptr[i+2]));
                        *scanLine = qRgb(ptr[i],ptr[i+1],ptr[i+2]);
                        i+=3;

                        Max[(intensity > Max[1])] = intensity;
                        Min[(intensity < Min[1])] = intensity;
                        //*scanLine++;
                    }
                    if(i+2 >= datasize) break;
                }

            } else {  //convert color to gray scale
                uint average;
                uchar *ptr = (uchar*)  data;
                if(ptr == (void*) 0) return (QImage *) 0;

                for (int y = 0; y < resultSize.height(); ++y) {
                    uint *scanLine = reinterpret_cast<uint *>(image->scanLine(y));
                    for (int x = 0; x < resultSize.width(); ++x) {
                        average =(int) (0.2989 * ptr[i] + 0.5870 * ptr[i+1] +  + 0.1140 * ptr[i+2]);
                        *scanLine++ = qRgb(average, average, average);
                        i+=3;

                        Max[(average > Max[1])] = average;
                        Min[(average < Min[1])] = average;
                    }
                    if(i+2 >= datasize) break;
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
        if(maxvalue > 0xFFFFFFFE) maxvalue = 0xFFFFFFFE;
    }

    return image;
}

void caCamera::showImage(int datasize, char *data)
{
#ifndef QT_NO_CONCURRENT
    QFuture<QImage *> future = QtConcurrent::run(this, &caCamera::showImageCalc, datasize, data);
    image = future.result();
#else
    image = showImageCalc(datasize, data);
#endif

    if(image != (QImage *) 0) updateImage(*image, readvaluesPresent, readvalues, scaleFactor);

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


void caCamera::setAccessW(bool access)
{
    _AccessW = access;
}
