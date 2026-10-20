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

#ifndef TST_TEXTWIDGETS_THEME_H
#define TST_TEXTWIDGETS_THEME_H

#include <QObject>

// wave 1 of the panel theme: text widgets follow the inherited palette live
class TestTextWidgetsTheme : public QObject
{
    Q_OBJECT

private slots:
    void lineEditDefaultFollowsPalette();
    void lineEditStaticAndAlarmKeepColours();
    void multiLineStringDefaultFollowsPalette();
    void menuDefaultFollowsPalette();
    void defaultColoursStayOpaque();
    void replaceMacroDefaultFollowsPalette();
    void sliderDefaultFollowsPalette();
    void lineDrawDefaultFollowsPalette();
    void lineEditKeepsStateAcrossPaletteChange();
};

#endif
