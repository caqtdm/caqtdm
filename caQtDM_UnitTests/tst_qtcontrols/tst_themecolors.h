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
 *  Copyright (c) 2026
 */

#ifndef TST_THEMECOLORS_H
#define TST_THEMECOLORS_H

#include <QObject>

// central colour file (level 1) and the pilot widgets caThermo / caCartesianPlot
class TestThemeColors : public QObject
{
    Q_OBJECT

private slots:
    void alarmColoursAreFixedAndAliased();
    void cssFormat();
    void schemeOfPalette();
    void schemeOfWidgetUsesNearestRoot();
    void darkSurfacesKeepContrastToAlarmColours();
    void thermoDefaultsUnchangedWithoutRoot();
    void thermoDefaultModeFollowsDarkRoot();
    void thermoStaticModeIgnoresDarkRoot();
    void cartesianPlotDefaultsFollowDarkRoot();
    void cartesianPlotFillUnderPenFollowsScale();
    void stripPlotScaleFollowsFrame();
    void adaptMatrixLoadsAndIsIdentityInLight();
    void adaptMatchesPythonVectors();
    void adaptKeepsAlarmColoursAndTransparency();
    void adaptKeepsPastelChromaAndTransparentText();
    void adaptStyleSheetKeepsSyntax();
    void thermoScaleTextFollowsOwnBackground();
};

#endif
