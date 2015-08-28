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

#include "cachoice.h"
#include "alarmdefs.h"
#include <QApplication>
#include <QGridLayout>
#include <QMouseEvent>
#include <math.h>

caChoice::caChoice(QWidget *parent) : QWidget(parent)
{
    numCells = 2;
    labels << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9" << "10" << "11" << "12" << "13" << "14" << "15" << "16";
    texts << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9" << "10" << "11" << "12" << "13" << "14" << "15" << "16";
    signalMapper = new QSignalMapper(this);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    grid = new QGridLayout(this);
    grid->setMargin(0);
    grid->setSpacing(2);

    thisStacking = Row;
    thisStartBit = 0;
    thisEndBit = 15;

    thisColorMode = Default;
    thisForeColor = QColor(0,0,0);
    thisBackColor = QColor(155,255,255);
    thisBorderColor = QColor(0,0,176);
    thisAlignment = center;
    thisScaleMode = EPushButton::WidthAndHeight;

    setAccessW(true);
    installEventFilter(this);

    populateCells(labels, -1);
    setColorMode(thisColorMode);

    setElevation(on_top);
}

QString caChoice::getPV() const
{
    return thisPV;
}

void caChoice::setPV(QString const &newPV)
{
    thisPV = newPV;
}

void caChoice::arrangeCells(QStringList list, int indx)
{
    double dSqrt = ceil(sqrt((double) numCells)); // preferentially vertical orientation
    int nbLines = qMax(2, (int) dSqrt);
    int column = 0;
    int row = 0;

    foreach(EPushButton *l, cells) {
        grid->removeWidget(l);
        l->hide();
        l->deleteLater();
    }

    cells.clear();

    // signalmapper will map signals from all buttons to one signal
    delete signalMapper;
    signalMapper = new QSignalMapper(this);

    // create all buttons
    for (int i = 0; i < numCells ;i++) {
        EPushButton* temp;
        //printf("numCells=%d start=%d end=%d\n", numCells, thisStartBit, thisEndBit);
        if((i + thisStartBit) > numCells) break;
        if((i + thisStartBit) > thisEndBit) break;
        if((i + thisStartBit) >= list.count()) break;

        if(list.at(i + thisStartBit).trimmed().size() > 0) {
          temp = new EPushButton(list.at(i + thisStartBit), this);
        } else {
            QString s = ""; // QString::number(i + thisStartBit);
            temp = new EPushButton(s, this);
        }
        temp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        if(thisScaleMode == EPushButton::None) {  // in this case we may use font
            temp->setFont(this->font());
        } else {
            temp->setFontScaleMode(thisScaleMode);
        }
        temp->setMinimumSize(2,2); //important for resizing as small as possible
        temp->setBotTopBorderWidth(0);
        temp->setObjectName(texts[i + thisStartBit]);

        QString style("");
        temp->setCheckable(true);
        // mark activ button
        if(i==indx) {
            style.append("* {border-style: solid; border-width: 1px 3px 1px 3px; padding:0px 1px 0px 1px; margin:0px;}");
            temp->setStyleSheet(style);
            temp->setChecked(true);
        } else {
            style.append("* {border-style: solid; border-width: 0px; padding:1px 4px 1px 4px; margin:0px;}");
            temp->setStyleSheet(style);
            temp->setChecked(false);
        }
        // take care of stacking
        if(thisStacking == Row) {
          grid->addWidget(temp, i, 0);
        } else if(thisStacking == Column) {
          grid->addWidget(temp, 0, i);
        } else {
            if(row >= nbLines) {
                row=0;
                column++;
            }
            grid->addWidget(temp, row, column);
            row++;
        }
        cells.push_back(temp);

        temp->show();

        signalMapper->setMapping(temp, list.at(i + thisStartBit));
        connect(temp, SIGNAL(clicked()), signalMapper, SLOT(map()));
    }

    lastValue = indx;
    connect(signalMapper, SIGNAL(mapped(QString)),this, SIGNAL(clicked(QString)));
}

void caChoice::setAlignment(alignmentHor alignment) {
    thisAlignment = alignment;
    setColors(thisBackColor, thisForeColor, thisBorderColor, thisAlignment);
}

void caChoice::setColors(QColor back, QColor fore, QColor border, alignmentHor alignment)
{
    //set colors and styles

    // default get it from external or internal stylesheet or nothing, do not bother about alignment

    if(thisColorMode == Default) {
        oldColorMode = thisColorMode;

        if(border != oldBorderColor) {
            QString bordercolor = "border-color: rgba(%1, %2, %3, %4); ";
            bordercolor = bordercolor.arg(border.red()).arg(border.green()).arg(border.blue()).arg(border.alpha());
            QString style ="QPushButton { ";
            style.append(bordercolor);
            style.append("} ");
            setStyleSheet(style);
        } else {
           setStyleSheet("");
        }
        return;
    }

    if((back != oldBackColor) || fore != oldForeColor || border != oldBorderColor || alignment != oldAlignment || oldColorMode != thisColorMode)
    {
        QColor baseColor(back);
        QColor highlightColor(back);
        QColor shadowColor1(back);
        QColor shadowColor2(back);
        QColor activColor(baseColor);
        highlightColor.setHsv(baseColor.hue(), baseColor.saturation(), baseColor.value());
        shadowColor1.setHsv(baseColor.hue(), (int) (baseColor.saturation() * 0.6), baseColor.value());
        shadowColor2.setHsv((int) (baseColor.hue()*0.7), (int) (baseColor.saturation() * 0.7), (int) (baseColor.value() * 0.7));

        activColor.setHsv(baseColor.hue(), baseColor.saturation(), (int) qMin((int)(baseColor.value() * 1.1), 255) );

        QString background =  "";
        background.append("background: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, ");
        background.append("stop:0   rgba(%1, %2, %3, %4), ");
        background.append("stop:0.4 rgba(%5, %6, %7, %8), ");
        background.append("stop:0.6 rgba(%9, %10, %11, %12), ");
        background.append("stop:1   rgba(%13, %14, %15, %16)); ");

        background = background.arg(shadowColor1.red()).arg(shadowColor1.green()).arg(shadowColor1.blue()).arg(shadowColor1.alpha())
                .arg(baseColor.red()).arg(baseColor.green()).arg(baseColor.blue()).arg(baseColor.alpha())
                .arg(baseColor.red()).arg(baseColor.green()).arg(baseColor.blue()).arg(baseColor.alpha())
                .arg(shadowColor2.red()).arg(shadowColor2.green()).arg(shadowColor2.blue()).arg(shadowColor2.alpha());

        background.append("border-radius: 2px;padding: 3px; border-width: 1px;");

        QString foreground = "; color: rgba(%1, %2, %3, %4); ";
        foreground = foreground.arg(fore.red()).arg(fore.green()).arg(fore.blue()).arg(fore.alpha());

        QString bordercolor = "border-color: rgba(%1, %2, %3, %4); ";
        bordercolor = bordercolor.arg(border.red()).arg(border.green()).arg(border.blue()).arg(border.alpha());

        QString style ="QPushButton { ";
        style.append(bordercolor);
        style.append(background);
        style.append(foreground);
        switch(alignment) {
        case left:
            style.append("text-align: left; ");
            break;
        case right:
            style.append("text-align: right; ");
            break;
        case center:
            style.append("text-align: center; ");
            break;
        }

        style.append("} ");
        QString hover = "QPushButton:hover {background-color: rgba(%1, %2, %3, %4);}  QPushButton:pressed {background-color: rgba(%5, %6, %7, %8)};";
        hover = hover.arg(highlightColor.red()).arg(highlightColor.green()).arg(highlightColor.blue()).arg(highlightColor.alpha())
                .arg(thisBorderColor.red()).arg(thisBorderColor.green()).arg(thisBorderColor.blue()).arg(thisBorderColor.alpha());
        style.append(hover);

        QString activ = "QPushButton:checked {background-color: rgba(%1, %2, %3, %4);} ";
        activ = activ.arg(activColor.red()).arg(activColor.green()).arg(activColor.blue()).arg(activColor.alpha());
        style.append(activ);

        QString hoveractiv =  "QPushButton:checked:hover {background-color: rgba(%1, %2, %3, %4);} ";
        hoveractiv = hoveractiv.arg(highlightColor.red()).arg(highlightColor.green()).arg(highlightColor.blue()).arg(highlightColor.alpha());
        style.append(hoveractiv);

        QString disabled("QPushButton:disabled {background: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, ");
        disabled.append("stop:0   rgba(%1, %2, %3, %4), ");
        disabled.append("stop:0.4 rgba(%5, %6, %7, %8), ");
        disabled.append("stop:0.6 rgba(%9, %10, %11, %12), ");
        disabled.append("stop:1   rgba(%13, %14, %15, %16))");
        baseColor.setHsv(baseColor.hue(), baseColor.saturation(), (int) (baseColor.value() * 0.8));
        disabled = disabled.arg(shadowColor1.red()).arg(shadowColor1.green()).arg(shadowColor1.blue()).arg(shadowColor1.alpha())
                .arg(baseColor.red()).arg(baseColor.green()).arg(baseColor.blue()).arg(baseColor.alpha())
                .arg(baseColor.red()).arg(baseColor.green()).arg(baseColor.blue()).arg(baseColor.alpha())
                .arg(shadowColor2.red()).arg(shadowColor2.green()).arg(shadowColor2.blue()).arg(shadowColor2.alpha());

        disabled.append("; etch-disabled-text: true; color: grey;}");
        style.append(disabled);
        setStyleSheet(style);
        oldBorderColor = border;
        oldBackColor = back;
        oldForeColor = fore;
        oldAlignment = alignment;
        oldColorMode = thisColorMode;
    }
}

void  caChoice::setColorMode(colMode colormode)
{
    thisColorMode = colormode;
    setColors(thisBackColor, thisForeColor, thisBorderColor, thisAlignment);
}

void caChoice::setBordercolor(QColor c)
{
    thisBorderColor = c;
    setColors(thisBackColor, thisForeColor, thisBorderColor, thisAlignment);
}

void caChoice::setBackground(QColor c)
{
    thisBackColor = c;
    setColors(thisBackColor, thisForeColor, thisBorderColor, thisAlignment);
}

void caChoice::setForeground(QColor c)
{
    thisForeColor = c;
    setColors(thisBackColor, thisForeColor, thisBorderColor, thisAlignment);
}

void caChoice::setAlarmColors(short status)
{
        QColor bg, fg;
        fg = thisForeColor;
        switch (status) {

        case NO_ALARM:
            bg = AL_GREEN;
            break;
        case MINOR_ALARM:
            bg = AL_YELLOW;
            break;
        case MAJOR_ALARM:
            bg = AL_RED;
            break;
        case INVALID_ALARM:
        case NOTCONNECTED:
            bg = AL_WHITE;
            fg = bg;
            break;
        default:
            bg = AL_DEFAULT;
            fg = thisForeColor;
            break;
        }
        setColors(bg, fg, thisBorderColor, thisAlignment);
}

void caChoice::setNormalColors()
{
    setColors(thisBackColor, thisForeColor, thisBorderColor, thisAlignment);
}

void caChoice::setStacking(Stacking stacking)
{
    thisStacking = stacking;
    populateCells(labels, -1);
}

void caChoice::updateChoice()
{

    int i = 0;
    foreach(EPushButton *temp, cells) {
        QString style("");
        // mark activ button
        if(i==lastValue) {
            style.append("* {border-style: inset; border-width: 1px 3px 1px 3px; padding:0px 1px 0px 1px; margin:0px;}");
            temp->setStyleSheet(style);
            temp->setChecked(true);
        } else {
            style.append("* {border-style: solid; border-width: 0px; padding:1px 4px 1px 4px; margin:0px;}");
            temp->setStyleSheet(style);
            temp->setChecked(false);
        }
        i++;
     }
}

void caChoice::populateCells(QStringList list, int indx)
{
    numCells = list.size();
    if(numCells > 16) numCells=16;
    arrangeCells(list, indx);
}

void caChoice::setStartBit(int const &bit) {
    thisStartBit = bit;
    int numRows = thisEndBit - thisStartBit + 1;
    if(numRows < 1) {
        thisStartBit=0;
        thisEndBit = 0;
    } else if(numRows > 16) {
        thisStartBit=0;
        thisEndBit = 15;
    }
    populateCells(labels, -1);
}

void caChoice::setEndBit(int const &bit) {
    thisEndBit = bit;
    int numRows = thisEndBit - thisStartBit + 1;
    if(numRows < 1) {
        thisStartBit=0;
        thisEndBit = 0;
    } else if(numRows > 16) {
        thisStartBit=0;
        thisEndBit = 15;
    }
    populateCells(labels, -1);
}

void caChoice::setAccessW(int access)
{
     thisAccessW = access;
}

bool caChoice::eventFilter(QObject *obj, QEvent *event)
{
        if (event->type() == QEvent::Enter) {
            if(!thisAccessW) {
                QApplication::setOverrideCursor(QCursor(Qt::ForbiddenCursor));
                //setEnabled(false);
            } else {
                QApplication::restoreOverrideCursor();
            }
        } else if(event->type() == QEvent::Leave) {
            QApplication::restoreOverrideCursor();
            //setEnabled(true);
        }
        return QObject::eventFilter(obj, event);
}


void caChoice::resizeEvent(QResizeEvent *e)
{
     Q_UNUSED(e);
}

void caChoice::setFontScaleModeL(EPushButton::ScaleMode m)
{
   thisScaleMode = m;
   populateCells(labels, -1);
}

EPushButton::ScaleMode caChoice::fontScaleMode()
{
    populateCells(labels, -1);
    return thisScaleMode;
}


