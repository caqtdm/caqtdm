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

#include "calabel.h"
#include "alarmdefs.h"
#include <QEvent>

caLabel::caLabel(QWidget *parent) : ESimpleLabel(parent)
{
    setFontScaleMode(WidthAndHeight);
    thisBackColor = QColor(255,255,255,0);
    thisForeColor = Qt::black;
    thisBorderColor = Qt::black;
    thisColorMode=Static;

    setBorderWidth (0);
    setColorMode(Static);
    thisVisibility = StaticV;
}

void caLabel::setBorderColor(QColor c)
{
    thisBorderColor = c;
    setColors(thisBackColor, thisForeColor);
}


void caLabel::setColors(QColor bg, QColor fg)
{
    if(thisColorMode == Default) {
        setPalette(QPalette());
        setAutoFillBackground(false);
        if (!styleSheet().isEmpty()) setStyleSheet("");
        thisBackgroundStyle.clear();
        update();
        return;
    }

    QPalette labelPalette = palette();
    labelPalette.setColor(QPalette::Window, bg);
    labelPalette.setColor(QPalette::Base, bg);
    labelPalette.setColor(QPalette::WindowText, fg);
    labelPalette.setColor(QPalette::Text, fg);
    labelPalette.setColor(QPalette::ButtonText, fg);
    setPalette(labelPalette);
    setAutoFillBackground(bg.alpha() > 0);

    // Keep the designer's transparent background transparent. QLabel's
    // default base brush is otherwise painted white when a parent uses QSS.
    // Text deliberately remains palette-driven so alarm and contrast colors
    // do not need an inline stylesheet.
    const QString backgroundStyle =
        QStringLiteral("background-color: rgba(%1, %2, %3, %4); border: %5px solid rgba(%6, %7, %8, %9);")
            .arg(bg.red()).arg(bg.green()).arg(bg.blue()).arg(bg.alpha())
            .arg(thisBorderWidth)
            .arg(thisBorderColor.red()).arg(thisBorderColor.green())
            .arg(thisBorderColor.blue()).arg(thisBorderColor.alpha());
    if (backgroundStyle != thisBackgroundStyle) {
        setStyleSheet(backgroundStyle);
        thisBackgroundStyle = backgroundStyle;
    }

    update();
}

void caLabel::setBackground(QColor c)
{
    thisBackColor = c;
    setColors(thisBackColor, thisForeColor);
}

void caLabel::setForeground(QColor c)
{
    thisForeColor = c;
    setColors(thisBackColor, thisForeColor);
}

void caLabel::setAlarmColors(short status)
{
    QColor c;
    switch (status) {

    case NO_ALARM:
        c=AL_GREEN;
        break;
    case MINOR_ALARM:
        c=AL_YELLOW;
        break;
    case MAJOR_ALARM:
        c=AL_RED;
        break;
    case INVALID_ALARM:
    case NOTCONNECTED:
        c=AL_WHITE;
        break;
    default:
        c=AL_DEFAULT;
        break;
    }
    setColors(thisBackColor, c);
}
