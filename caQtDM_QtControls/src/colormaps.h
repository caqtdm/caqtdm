#ifndef COLORMAPS_H
#define COLORMAPS_H
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
#include <qwt_color_map.h>
#include <qwt_scale_widget.h>
#if QWT_VERSION >= 0x060100
#include <qwt_scale_div.h>
#endif
#include <math.h>

// heat from blue to yellow
class ColorMap_Grey : public  QwtLinearColorMap {
public:
    ColorMap_Grey() : QwtLinearColorMap(QColor(0,0,0), QColor(1,1,1)) {
        for (int i = 0; i < 256; ++i) {
            addColorStop((double)(i)/255.0, QColor(i, i, i));
        }
    }
};

class ColorMap_Hot : public  QwtLinearColorMap {
public:
    ColorMap_Hot() : QwtLinearColorMap(QColor(0,0,0), QColor(1,1,1)) {

        double r[] = { 0, 0.03968253968253968, 0.07936507936507936, 0.119047619047619, 0.1587301587301587, 0.1984126984126984, 0.2380952380952381, 0.2777777777777778, 0.3174603174603174, 0.3571428571428571, 0.3968253968253968, 0.4365079365079365, 0.4761904761904762, 0.5158730158730158, 0.5555555555555556, 0.5952380952380952, 0.6349206349206349, 0.6746031746031745, 0.7142857142857142, 0.753968253968254, 0.7936507936507936, 0.8333333333333333, 0.873015873015873, 0.9126984126984127, 0.9523809523809523, 0.992063492063492, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
        double g[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.03174603174603163, 0.0714285714285714, 0.1111111111111112, 0.1507936507936507, 0.1904761904761905, 0.23015873015873, 0.2698412698412698, 0.3095238095238093, 0.3492063492063491, 0.3888888888888888, 0.4285714285714284, 0.4682539682539679, 0.5079365079365079, 0.5476190476190477, 0.5873015873015872, 0.6269841269841268, 0.6666666666666665, 0.7063492063492065, 0.746031746031746, 0.7857142857142856, 0.8253968253968254, 0.8650793650793651, 0.9047619047619047, 0.9444444444444442, 0.984126984126984, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
        double b[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.04761904761904745, 0.1269841269841265, 0.2063492063492056, 0.2857142857142856, 0.3650793650793656, 0.4444444444444446, 0.5238095238095237, 0.6031746031746028, 0.6825396825396828, 0.7619047619047619, 0.8412698412698409, 0.92063492063492, 1};
        double pos;
        for(int i=0; i<256; i++) {
            pos= (double) i/ 255.0;
            int j = (int) ((i+0.5) / 4.0);
            addColorStop(pos, QColor((int) (255*r[j]), (int) (255*g[j]), (int) (255*b[j])));
        }
    }
};

// heat from blue to yellow
class ColorMap_Heat : public  QwtLinearColorMap {
public:
    ColorMap_Heat() : QwtLinearColorMap(QColor(0,0,0), QColor(1,1,1)) {
        double r,g,b;
        for (int i = 0; i < 256; ++i) {
            rgbHeat((double) i, r, g, b);
            addColorStop((double)(i)/255.0, QColor((int) r, (int) g, (int) b));
        }
    }
private:
    void rgbHeat(double wave, double &r, double &g, double &b) {
        double max = 255.0;
        double min = 0.0;
        double x = (wave-min) / (max-min);
        b = 255 * qMin(qMax(4*(0.75-x), 0.0), 1.0);
        r = 255 * qMin(qMax(4*(x-0.25), 0.0), 1.0);
        g = 255 * qMin(qMax(4*fabs(x-0.5)-1.0, 0.0), 1.0);
    }
};

class ColorMap_Jet : public QwtLinearColorMap
{
public:
    ColorMap_Jet() : QwtLinearColorMap(QColor(0,0,189), QColor(132,0,0)) {
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

class ColorMap_Wavelength : public QwtLinearColorMap
{
public:
    ColorMap_Wavelength() : QwtLinearColorMap(QColor(80,80,0), QColor(100,0,0))
    {
        double r,g,b;
        for (int i = 0; i < 256; ++i) {
            rgbFromWaveLength(380.0 + (i * 400.0 / 255), r, g, b);
            addColorStop((double)(i)/255.0, QColor((int) r, (int) g, (int) b));
        }
    }
private:
    void rgbFromWaveLength(double wave, double &r, double &g, double &b)
    {
        r = 0.0;
        g = 0.0;
        b = 0.0;

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

        r = pow(r * s, 0.8) * 255;
        g = pow(g * s, 0.8) * 255;
        b = pow(b * s, 0.8) * 255;
    }
};

class ColorMap_Custom : public  QwtLinearColorMap {
public:
    ColorMap_Custom() : QwtLinearColorMap() {
    }

    void initColormap(int *colorIndexes, int nbColors, bool discrete) {

        if(discrete) setMode( QwtLinearColorMap::FixedColors);
        setColorInterval( Qt::GlobalColor( colorIndexes[0] ), Qt::GlobalColor(colorIndexes[nbColors-1]) );

        for ( int i = 1; i < nbColors; i++ ) {
            addColorStop( i / (double) nbColors, Qt::GlobalColor(colorIndexes[i]) );
        }
    }
};

class colorMaps
{

public:
   colorMaps();
   ~colorMaps() {}

  enum colormaps {grey, spectrum_wavelength, spectrum_hot, spectrum_heat, spectrum_jet, spectrum_custom};

  void getColormap(colormaps map, bool discrete, QStringList CustomMap, int ColorMapSize, uint colorMap[], QwtScaleWidget *colormapWidget);

private:

    void showColorMap(QwtScaleWidget *colormapWidget, bool hide, QwtLinearColorMap *colormap);
};

#endif
