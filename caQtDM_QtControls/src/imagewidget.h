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

#ifndef ImageWIDGET_H
#define ImageWIDGET_H

#include <QPixmap>
#include <QImage>
#include <QWidget>
#include <QGridLayout>
#include <caLineEdit>

class ImageWidget : public QWidget
{
    Q_OBJECT

public:

    ImageWidget(QWidget *parent = 0);
    ~ImageWidget(){}

    void updateImage(bool FitToSize, const QImage &image, bool readvaluesPresent[], double readvalues[],
                     double scaleFactor, bool selectSimpleView,
                     short readmarkerType, short readType, short writemarkerType, short writeType);

    void initSelectionBox(const double &scaleFactor);
    void rescaleSelectionBox(const double &scaleFactor);
    void updateSelectionBox(QPoint selectionPoints[], const bool &selectInProgress);
    void getImageDimensions(int &width, int &height);
    void updateDisconnected();

    // must match definitions in cacamera and cascan2d
    enum ROI_type {none=0, xy_only, xy1_xy2, xyUpleft_xyLowright, xycenter_width_height};
    enum ROI_markertype {box=0, box_crosshairs, line, arrow};

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

private slots:

private:
    void rescaleReadValues(const bool &fitToSize, const QImage &image, const double &scaleFactor,
                           bool readvaluesPresent[], double readvalues[]);

    QPolygonF getHead( QPointF p1, QPointF p2, int arrowSize);
    QImage imageNew;
    QPoint imageOffset;
    bool m_zoom;
    QGridLayout  *grid;
    caLineEdit *labelMin;
    caLineEdit *labelMax;
    QWidget *imageW;
    bool readValuesPresentL[4];
    double georeadValues[4];
    int geowriteValues[4];
    int readValuesL[4];

    bool selectStartedL;
    QPoint selectionPointsL[2];
    bool selectSimpleViewL;
    bool disconnected;
    short marker;
    ROI_markertype readmarkerTypeL;
    ROI_markertype writemarkerTypeL;
    ROI_type readTypeL;
    ROI_type writeTypeL;
    double selectionFirstFactor, scaleFactorL, imageFirstFactor;
    bool firstSelection;
    bool firstImage;
    bool selectionInProgress;
};

#endif
