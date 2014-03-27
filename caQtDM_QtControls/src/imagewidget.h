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
    void updateImage(bool fitToSize, const QImage &image, bool valuesPresent[], int values[], const double &scaleFactor);
    QImage scaleImage(const QImage &image, const double &scaleFactor, bool const &FitToSize);

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

private slots:

private:

    QImage imageNew;
    QPoint imageOffset;
    bool m_zoom;
    QGridLayout  *grid;
    caLineEdit *labelMin;
    caLineEdit *labelMax;
    QWidget *imageW;
    bool drawValues[4];
    int geoValues[4];
};

#endif
