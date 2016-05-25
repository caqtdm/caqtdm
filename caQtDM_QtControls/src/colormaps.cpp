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
#if defined(_WIN32) || defined(_WIN64)
#define QWT_DLL
#endif

#include "colormaps.h"

colorMaps::colorMaps()
{
}

void colorMaps::showColorMap(QwtScaleWidget *colormapWidget, bool hide, QwtLinearColorMap *colormap)
{
    if(colormapWidget != ( QwtScaleWidget *) 0) {
        colormapWidget->setHidden(hide);
        colormapWidget->setColorMap(QwtInterval(0,1), colormap);
    }
}

void colorMaps::getColormap(colormaps map, bool discrete, QStringList CustomMap, int ColormapSize, uint colorMap[], QwtScaleWidget *colormapWidget)
{

    switch (map) {
    case grey:
    {
        ColorMap_Grey * colormap =  new ColorMap_Grey();
        for (int i = 0; i < ColormapSize; ++i) colorMap[i] = colormap->rgb(QwtInterval(0, ColormapSize-1), i);
        // hide the colormap bar
        showColorMap(colormapWidget, true, (QwtLinearColorMap *) colormap);
    }
        break;

    case spectrum_wavelength:
    {
        ColorMap_Wavelength * colormap =  new ColorMap_Wavelength();
        for (int i = 0; i < ColormapSize; ++i) colorMap[i] = colormap->rgb(QwtInterval(0, ColormapSize-1), i);
        // show the colormap bar
        showColorMap(colormapWidget, false, (QwtLinearColorMap *) colormap);
    }
        break;

    case spectrum_hot:
    {
        ColorMap_Hot * colormap =  new ColorMap_Hot();
        for (int i = 0; i < ColormapSize; ++i) colorMap[i] = colormap->rgb(QwtInterval(0, ColormapSize-1), i);
        // show the colormap bar
        showColorMap(colormapWidget, false, (QwtLinearColorMap *) colormap);
    }
        break;

    case spectrum_heat:
    {
        ColorMap_Heat * colormap =  new ColorMap_Heat();
        for (int i = 0; i < ColormapSize; ++i) colorMap[i] = colormap->rgb(QwtInterval(0, ColormapSize-1), i);
        // show the colormap bar
        showColorMap(colormapWidget, false, (QwtLinearColorMap *) colormap);
    }
        break;

    case spectrum_jet:
    {
        ColorMap_Jet * colormap =  new ColorMap_Jet();
        for (int i = 0; i < ColormapSize; ++i) colorMap[i] = colormap->rgb(QwtInterval(0, ColormapSize-1), i);
        // show the colormap bar
        showColorMap(colormapWidget, false, (QwtLinearColorMap *) colormap);
    }
        break;

    case spectrum_custom: {

        // user has the possibility to input its own colormap with discrete QtColors from 2 t0 18
        // when nothing given, fallback to default colormap
        int *colorIndexes=NULL;

        if(CustomMap.count() > 2) {
            colorIndexes=(int *) malloc(CustomMap.count()*sizeof(int));

            // get the discrete colors
            for(int i=0; i< CustomMap.count(); i++) {
                bool ok;
                int index = CustomMap.at(i).toInt(&ok);
                if(ok) colorIndexes[i] = index; else colorIndexes[i] = 2; // black
            }

            // create colormap
            ColorMap_Custom * colormap =  new ColorMap_Custom();
            colormap->initColormap(colorIndexes, CustomMap.count(), discrete);
            for (int i = 0; i < ColormapSize; ++i) colorMap[i] = colormap->rgb(QwtInterval(0, ColormapSize-1), i);
            free(colorIndexes);
            // show the colormap bar
            showColorMap(colormapWidget, false, (QwtLinearColorMap *) colormap);

        } else {
            ColorMap_Wavelength * colormap =  new ColorMap_Wavelength();
            for (int i = 0; i < ColormapSize; ++i) colorMap[i] = colormap->rgb(QwtInterval(0, ColormapSize-1), i);
            // show the colormap bar
            showColorMap(colormapWidget, false, (QwtLinearColorMap *) colormap);
        }

    }
        break;
    default:
    {
        ColorMap_Grey * colormap =  new ColorMap_Grey();
        for (int i = 0; i < ColormapSize; ++i) colorMap[i] = colormap->rgb(QwtInterval(0, ColormapSize-1), i);
        // hide the colormap bar
        showColorMap(colormapWidget, true, (QwtLinearColorMap *) colormap);
    }
        break;
    }
}
