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

#include <QtGui>
#include<QApplication>
#include <math.h>
#ifndef QT_NO_CONCURRENT
#include <qtconcurrentrun.h>
#endif
#include "cacamera.h"
#include <qwt_color_map.h>

class ColorMap_Hot : public  QwtLinearColorMap {
public:
    ColorMap_Hot() : QwtLinearColorMap(QColor(0,0,0), QColor(1,1,1)) {

        float r[] = { 0, 0.03968253968253968, 0.07936507936507936, 0.119047619047619, 0.1587301587301587, 0.1984126984126984, 0.2380952380952381, 0.2777777777777778, 0.3174603174603174, 0.3571428571428571, 0.3968253968253968, 0.4365079365079365, 0.4761904761904762, 0.5158730158730158, 0.5555555555555556, 0.5952380952380952, 0.6349206349206349, 0.6746031746031745, 0.7142857142857142, 0.753968253968254, 0.7936507936507936, 0.8333333333333333, 0.873015873015873, 0.9126984126984127, 0.9523809523809523, 0.992063492063492, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
        float g[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.03174603174603163, 0.0714285714285714, 0.1111111111111112, 0.1507936507936507, 0.1904761904761905, 0.23015873015873, 0.2698412698412698, 0.3095238095238093, 0.3492063492063491, 0.3888888888888888, 0.4285714285714284, 0.4682539682539679, 0.5079365079365079, 0.5476190476190477, 0.5873015873015872, 0.6269841269841268, 0.6666666666666665, 0.7063492063492065, 0.746031746031746, 0.7857142857142856, 0.8253968253968254, 0.8650793650793651, 0.9047619047619047, 0.9444444444444442, 0.984126984126984, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
        float b[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.04761904761904745, 0.1269841269841265, 0.2063492063492056, 0.2857142857142856, 0.3650793650793656, 0.4444444444444446, 0.5238095238095237, 0.6031746031746028, 0.6825396825396828, 0.7619047619047619, 0.8412698412698409, 0.92063492063492, 1};
        double pos;
        for(int i=0; i<256; i++) {
            pos= (double) i/ 255.0;
            int j = (int) (i+0.5) / 4.0;
            addColorStop(pos, QColor(255*r[j], 255*g[j], 255*b[j]));
        }
    }
};

class ColorMap_Custom : public  QwtLinearColorMap {
public:
    ColorMap_Custom() : QwtLinearColorMap() {
    }

    void initColormap(int *colorIndexes, int nbColors) {

        for ( int i = 0; i < nbColors; i++ ) {
            if(colorIndexes[i] < 2) colorIndexes[i] = 2;
            if(colorIndexes[i] > 18) colorIndexes[i] = 18;
        }

        //setMode( QwtLinearColorMap::FixedColors );

        setColorInterval( Qt::GlobalColor( colorIndexes[0] ), Qt::GlobalColor(colorIndexes[nbColors-1]) );

        for ( int i = 1; i < nbColors; i++ ) {
            addColorStop( i / (double) nbColors, Qt::GlobalColor(colorIndexes[i]) );
        }
    }
};

class ColorMap_Jet: public QwtLinearColorMap
{
public:
    ColorMap_Jet(): QwtLinearColorMap(QColor(0,0,189), QColor(132,0,0))
    {
        double pos;
        pos = 1.0/13.0*1.0; addColorStop(pos, QColor(0,0,255));
        pos = 1.0/13.0*2.0; addColorStop(pos, QColor(0,66,255));
        pos = 1.0/13.0*3.0; addColorStop(pos, QColor(0,132,255));
        pos = 1.0/13.0*4.0; addColorStop(pos, QColor(0,189,255));
        pos = 1.0/13.0*5.0; addColorStop(pos, QColor(0,255,255));
        pos = 1.0/13.0*6.0; addColorStop(pos, QColor(66,255,189));
        pos = 1.0/13.0*7.0; addColorStop(pos, QColor(132,255,132));
        pos = 1.0/13.0*8.0; addColorStop(pos, QColor(189,255,66));
        pos = 1.0/13.0*9.0; addColorStop(pos, QColor(255,255,0));
        pos = 1.0/13.0*10.0; addColorStop(pos, QColor(255,189,0));
        pos = 1.0/13.0*12.0; addColorStop(pos, QColor(255,66,0));
        pos = 1.0/13.0*13.0; addColorStop(pos, QColor(189,0,0));
    }
};


caCamera::caCamera(QWidget *parent) : QWidget(parent)
{
    m_init = true;

    m_codeDefined = false;
    m_bppDefined = false;
    m_widthDefined = false;
    m_heightDefined = false;

    thisSimpleView = false;
    thisColormap = Default;
    savedSize = 0;
    savedWidth = 0;
    savedHeight = 0;
    frameCount = 0;
    selectionStarted=false;

    savedData = (char*) 0;

    initWidgets();

    Xpos = Ypos = 0;

    for(int i=0; i<ColormapSize; i++) ColorMap[i] = qRgb(i,i,i);

    scrollArea = (QScrollArea *) 0;

    mainLayout = new QGridLayout(this);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);
    setLayout(mainLayout);
    setup();

    buttonPressed = false;
    ROIdetected = false;
    thisROItype = centerxy_width_height;

    setCustomMap("");

    setAccessW(true);
    installEventFilter(this);

    scaleFactor = 1.0;

    UpdatesPerSecond = 0;
    startTimer(1000);
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

    if(zoomSliderLayout != ( QVBoxLayout *) 0)   delete zoomSliderLayout;
    if(zoomSlider != (QSlider *) 0)              delete zoomSlider;
    if(zoomValue != (caLabel *) 0)               delete zoomValue;
    if(zoomInIcon != (QToolButton *) 0)          delete zoomInIcon;
    if(zoomOutIcon != (QToolButton *) 0)         delete zoomOutIcon;

    if(imageW != (ImageWidget *) 0)              delete imageW;
    if(valuesWidget != (QWidget *) 0)            delete valuesWidget;
    if(scrollArea != (QScrollArea *) 0)          delete scrollArea;
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
    zoomSliderLayout = ( QVBoxLayout *) 0;
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

bool caCamera::getROI(int &x, int &y, int &w, int &h)
{
    x = ROIx;
    y = ROIy;
    w = ROIw;
    h = ROIh;

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

bool caCamera::eventFilter(QObject *obj, QEvent *event)
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
                    ROIx = (int) Xnew1;
                    ROIy = (int) Ynew1;
                    ROIw = (int) (Xnew2 - Xnew1);
                    ROIh = (int) (Ynew2 - Ynew1);
                    ROIdetected = true;
                }
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
    }

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
                int index = (int) ((int) Ynew * savedWidth + (int) Xnew);
                if((Xnew >=0) && (Ynew >=0)  && (Xnew < Xmax) && (Ynew < Ymax) && (index < savedSize)) {
                    Zvalue=ptr[index];

                } else {
                    validIntensity = false;
                }
            }
                break;

            case 2: {   // monochrome 2 bpp  (Damir camera)
                uchar *ptr = (uchar*)  savedData;
                int index = (int) ((int) Ynew * savedWidth * 2 + 2 * (int) Xnew);
                if((Xnew >=0) && (Ynew >=0)  &&  (Xnew < Xmax) && (Ynew < Ymax) && (index < savedSize)) {
                    if(thisColormap != spectrum) Zvalue=ptr[index];
                    else Zvalue=ptr[index] * 256 + ptr[index+1];

                } else {
                    validIntensity = false;
                }
            }
                break;

            case 3: {   // monochrome 2 bpp, but used only 12 bits  (Helge cameras)
                QSize resultSize;
                resultSize.setWidth((int) savedWidth);
                resultSize.setHeight((int) savedHeight);
                ushort *ptr = (ushort*) savedData;
                int index = (int) ((int) Ynew * savedWidth + (int) Xnew);
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
                resultSize.setWidth((int) savedWidth);
                resultSize.setHeight((int) savedHeight);
                uchar *ptr = (uchar*) savedData;
                int index = (int) ((int) Ynew * savedWidth*3 + 3 * (int) Xnew);
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
            strng = strng.arg((int) Xnew).arg((int) Ynew).arg(Zvalue);
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
        zoomSlider->setMaximum(90); // do not exceed 6*
        zoomSlider->setValue(52);
        zoomSlider->setTickPosition(QSlider::NoTicks);

        zoomValue = new QLabel("");
        zoomValue->setFixedWidth(60);

        // add everything to layout
        zoomSliderLayout = new QVBoxLayout();
        zoomSliderLayout->addWidget(zoomInIcon);
        zoomSliderLayout->addWidget(zoomSlider);
        zoomSliderLayout->addWidget(zoomOutIcon);
        zoomSliderLayout->addWidget(zoomValue);

        zoomWidget = new QWidget;
        zoomWidget->setLayout(zoomSliderLayout);

        // connect buttons and slider
        connect(zoomInIcon, SIGNAL(clicked()), this, SLOT(zoomIn()));
        connect(zoomOutIcon, SIGNAL(clicked()), this, SLOT(zoomOut()));
        connect(zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(zoomNow()));

        // add everything to main layout
        mainLayout->addWidget(valuesWidget, 0, 0);
        mainLayout->addWidget(scrollArea, 1, 0);
        mainLayout->addWidget(zoomWidget, 1, 1);

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

void caCamera::zoomNow()
{
    double scale = qPow(2.0, ((double) zoomSlider->value() - 52.0) / 13.0);
    if(scale > 6) scale = 6;
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
        scaleFactor = 1.0;
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
    switch (map) {
    case Default:
        for(int i=0; i<ColormapSize; i++) ColorMap[i] = qRgb(i,i,i);
        break;
    case grey:
        for(int i=0; i<ColormapSize; i++) ColorMap[i] = qRgb(i,i,i);
        break;
    case spectrum: {
/*
        QwtLinearColorMap *heatColormap =  new ColorMap_Custom();
        for (int i = 0; i < ColormapSize; ++i) {
            ColorMap[i] = heatColormap->rgb(QwtInterval(0, ColormapSize-1), i);
        }
*/
        // user has the possibility to input its own colormap with discrete QtColors from 2 t0 18
        // when nothing given, fallback to default colormap
        if(thisCustomMap.count() > 2) {
           //int colorIndexes[17] = {2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17, 18};  // available colors
            int colorIndexes[thisCustomMap.count()];
            for(int i=0; i< thisCustomMap.count(); i++) {
                bool ok;
                int index = thisCustomMap.at(i).toInt(&ok);
                if(ok) colorIndexes[i] = index; else colorIndexes[i] = 2; // black
            }

           ColorMap_Custom * colormap =  new ColorMap_Custom();
           colormap->initColormap(colorIndexes, thisCustomMap.count());
           for (int i = 0; i < ColormapSize; ++i) ColorMap[i] = colormap->rgb(QwtInterval(0, ColormapSize-1), i);
        } else {
            //for (int i = 0; i < ColormapSize; ++i) ColorMap[i] = floatRGB((double) i, 0.0, ColormapSize -1);
            for (int i = 0; i < ColormapSize; ++i) ColorMap[i] = rgbFromWaveLength(380.0 + (i * 400.0 / ColormapSize));
        }

        break;
    }
    default:
        //for (int i = 0; i < ColormapSize; ++i) ColorMap[i] = floatRGB((double) i, 0.0, ColormapSize-1);
        for (int i = 0; i < ColormapSize; ++i) ColorMap[i] = rgbFromWaveLength(380.0 + (i * 400.0 / ColormapSize));
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
    if(thisSimpleView) return;

    if(m_widthDefined && m_heightDefined) {
        if(!thisFitToSize) {
            imageW->setMinimumSize((int) (m_width * scaleFactor), (int) (m_height * scaleFactor));
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
        }
    }
}

void caCamera::updateImage(const QImage &image, bool valuesPresent[], int values[], const double &scaleFactor)
{
    imageW->updateImage(thisFitToSize, image, valuesPresent, values, scaleFactor, selectionStarted, selectionRect, thisSimpleView);
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

                        Max[(indx > Max[1])] = indx;
                        Min[(indx < Min[1])] = indx;

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

                        Max[(indx > Max[1])] = indx;
                        Min[(indx < Min[1])] = indx;
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

                    }
                    if((i*2) >= datasize) break;
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

    if(image != (QImage *) 0) updateImage(*image, valuesPresent, values, scaleFactor);

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

// rainbow
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

// heat from blue to yellow
uint caCamera::floatRGB(double mag, double min, double max)
{
    double r = 0.0;
    double g = 0.0;
    double b = 0.0;

    double x = (mag-min) / (max-min);
    b = qMin(qMax(4*(0.75-x), 0.0), 1.0);
    r = qMin(qMax(4*(x-0.25), 0.0), 1.0);
    g = qMin(qMax(4*fabs(x-0.5)-1.0, 0.0), 1.0);

    printf("%f %f %f\n", r, g, b);

    return qRgb(int(r * 255), int(g * 255), int(b * 255));
}


void caCamera::setAccessW(int access)
{
    _AccessW = access;
}
