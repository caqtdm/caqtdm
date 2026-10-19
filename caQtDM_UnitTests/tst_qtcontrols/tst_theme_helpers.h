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

#ifndef TST_THEME_HELPERS_H
#define TST_THEME_HELPERS_H

#include <QCoreApplication>
#include <QTest>
#include <QString>
#include <QWidget>
#include "themecolors.h"

// fake panel roots as the PanelThemeApplier would create them
namespace ThemeTestHelpers {

inline QWidget *themeRoot(const char *scheme)
{
    QWidget *root = new QWidget;
    root->setProperty(ThemeColors::ThemeRootProperty, true);
    root->setProperty(ThemeColors::ThemeSchemeProperty, scheme);
    root->setPalette(ThemeColors::palette(QString(scheme) == "dark" ? ThemeColors::Dark
                                                                     : ThemeColors::Light));
    return root;
}

inline QWidget *darkRoot() { return themeRoot("dark"); }

// like PanelThemeApplier::apply: reset, then seed, so that children created in
// between resolve their palette (Qt defers that with stylesheet propagation on)
inline void switchRoot(QWidget *root, const char *scheme)
{
    root->setProperty(ThemeColors::ThemeSchemeProperty, scheme);
    root->setPalette(QPalette());
    root->setPalette(ThemeColors::palette(QString(scheme) == "dark" ? ThemeColors::Dark
                                                                     : ThemeColors::Light));
    QTest::qWait(20);                 // widgets refresh deferred (zero timer)
}

// seed the current scheme again after the widgets were created
inline void settleRoot(QWidget *root)
{
    switchRoot(root, root->property(ThemeColors::ThemeSchemeProperty).toString() == "dark" ? "dark" : "light");
}

}

#endif
