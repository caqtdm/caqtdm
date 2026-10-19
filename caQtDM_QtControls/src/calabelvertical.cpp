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

#include "calabelvertical.h"
#include "alarmdefs.h"
#include "themecolors.h"

#include <QPainter>

// colour set of caLabelVertical (level 2), values come from themecolors.h
namespace caLabelVerticalColor {
    constexpr const ThemeColor &AlarmNoAlarm = ThemeColors::AlarmNoAlarm;
    constexpr const ThemeColor &AlarmMinor   = ThemeColors::AlarmMinor;
    constexpr const ThemeColor &AlarmMajor   = ThemeColors::AlarmMajor;
    constexpr const ThemeColor &AlarmInvalid = ThemeColors::AlarmInvalid;
    constexpr const ThemeColor &AlarmDefault = ThemeColors::AlarmDefault;
    constexpr const ThemeColor &NotConnected = ThemeColors::AlarmInvalid;
}

caLabelVertical::caLabelVertical(QWidget *parent) : QWidget(parent), FontScalingWidget(this)
{
    setFontScaleMode(WidthAndHeight);
    thisBackColor = QColor(255,255,255,0);
    thisForeColor = Qt::black;
    thisColorMode=Static;
    thisBorderWidth = 0;
    thisBorderColor = Qt::black;
    setColorMode(Static);
    thisVisibility = StaticV;
    setDirection(Down);
    setAlignment(Center);
    setVerticalLabel(true);
    setText("");
}

void caLabelVertical::setDirection(const Direction &direction)
{
    switch (direction) {
        case Up:
           rotateText(270.0);
           break;
        case Down:
        default:
            rotateText(90.0);
            break;
    }
    thisDirection = direction;
    update();
}

void caLabelVertical::setAlignment(const Alignment &alignment)
{
    thisAlignment = alignment;
    update();
}

void caLabelVertical::setBackground(QColor c)
{
    thisBackColor = c;
    update();
}

void caLabelVertical::setForeground(QColor c)
{
    thisForeColor = c;
    update();
}

void caLabelVertical::setBorderColor(QColor c)
{
    thisBorderColor = c;
    update();
}

void caLabelVertical::setColors(QColor bg, QColor fg)
{
    thisBackColor = bg;
    thisForeColor = fg;
    update();
}

void caLabelVertical::setAlarmColors(short status)
{
    if(thisColorMode == Default) return;      // like caLabel: the palette paints the text
    QColor c;
    switch (status) {

    case NO_ALARM:
        c=ThemeColors::color(caLabelVerticalColor::AlarmNoAlarm);
        break;
    case MINOR_ALARM:
        c=ThemeColors::color(caLabelVerticalColor::AlarmMinor);
        break;
    case MAJOR_ALARM:
        c=ThemeColors::color(caLabelVerticalColor::AlarmMajor);
        break;
    case INVALID_ALARM:
    case NOTCONNECTED:
        c=ThemeColors::color(caLabelVerticalColor::NotConnected);
        break;
    default:
        c=ThemeColors::color(caLabelVerticalColor::AlarmDefault);
        break;
    }
    setColors(thisBackColor, c);
}

// the panel palette changed (theme switch): the Default text colour is read at paint time
void caLabelVertical::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    if(e->type() == QEvent::PaletteChange || e->type() == QEvent::StyleChange) update();
}

bool caLabelVertical::rotateText(float degrees)
{
    if (degrees >=0 && degrees <= 360) {
        rotation=degrees;
        update();
        return true;
    }
    return false;
}

void caLabelVertical::paintEvent(QPaintEvent *)
{
    QFontMetrics fm(font());
    int w = QMETRIC_QT456_FONT_WIDTH(fm,thisText);
    int h = fm.height();
    QPainter painter(this);
    // Default: no own fill, text from the panel palette (like caLabel)
    const bool byPalette = (thisColorMode == Default);
    QBrush brush = QBrush(byPalette ? QColor(0, 0, 0, 0) : thisBackColor);
    painter.setBackground(brush);
    painter.setBackgroundMode(Qt::OpaqueMode);
    painter.fillRect(0,0, width(), height(), brush);
    if(thisBorderWidth > 0) {
        painter.setPen( QPen( thisBorderColor, thisBorderWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ) );
        painter.drawRect(thisBorderWidth, thisBorderWidth, width()- 2*thisBorderWidth, height() - 2*thisBorderWidth);
    }
    painter.setPen(byPalette ? palette().color(QPalette::WindowText) : thisForeColor);
    painter.rotate(rotation);
    switch (thisDirection) {
    case Up:
        switch (thisAlignment) {
        case Left:
            painter.drawText(QPoint(-height(), width()/2 +h/2-fm.descent()), text());             // left align
            break;
        case Right:
            painter.drawText(QPoint(-w, width()/2 +h/2-fm.descent()), text());                    // right align
            break;
        case Center:
        default:
            painter.drawText(QPoint(-height()/2-w/2, width()/2 +h/2-fm.descent()), text());       // center align
            break;
        }
        break;
    case Down:
    default:
        switch (thisAlignment) {
        case Left:
            painter.drawText(QPoint(0, -width()/2 +h/2-fm.descent()), text());                    // left align
            break;
        case Right:
            painter.drawText(QPoint(height()-w, -width()/2 +h/2-fm.descent()), text());           // right align
            break;
        case Center:
        default:
            painter.drawText(QPoint(height()/2-w/2, -width()/2 +h/2-fm.descent()), text());       // center align
            break;
        }
    }
}

void caLabelVertical::setText(const QString &txt)
{
    FontScalingWidget::rescaleFont(txt, d_savedTextSpace);
    thisText = txt;
    update();
}

bool caLabelVertical::event(QEvent *e)
{
    if(e->type() == QEvent::Resize || e->type() == QEvent::Show) {
        FontScalingWidget::rescaleFont(text(), calculateTextSpace());
    }
    return QWidget::event(e);
}

QSize caLabelVertical::calculateTextSpace()
{
    d_savedTextSpace = contentsRect().size();
    d_savedTextSpace.setWidth(d_savedTextSpace.width());
    d_savedTextSpace.setHeight(d_savedTextSpace.height());
    return d_savedTextSpace;
}

QSize caLabelVertical::sizeHint() const
{
    if(!fontScaleEnabled())
    {
        return QWidget::sizeHint();
    }
    QFont f = font();
    f.setPointSize(4);
    QFontMetrics fm(f);
    int w = QMETRIC_QT456_FONT_WIDTH(fm,text());
    int h = fm.height();
    /* add some pixels... */
    w += 4;
    h += 4;
    QSize size(w, h);
    return size;
}

QSize caLabelVertical::minimumSizeHint() const
{
    QSize size;
    if(!fontScaleEnabled())
        size = QWidget::minimumSizeHint();
    else
        size = sizeHint();
    return size;
}

