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

#include "cabyte.h"
#include "alarmdefs.h"
#include "themecolors.h"
#include <QGridLayout>
#include <QtDebug>

Q_LOGGING_CATEGORY(caByteLog, "caqtdm.widgets.cabyte")

// colour set of caByte (level 2), values come from themecolors.h
namespace caByteColor {
    constexpr const ThemeColor &AlarmNoAlarm = ThemeColors::AlarmNoAlarm;
    constexpr const ThemeColor &AlarmMinor   = ThemeColors::AlarmMinor;
    constexpr const ThemeColor &AlarmMajor   = ThemeColors::AlarmMajor;
    constexpr const ThemeColor &AlarmDefault = ThemeColors::AlarmDefault;
    constexpr const ThemeColor &NotConnected = ThemeColors::AlarmInvalid;
}

caByte::caByte(QWidget *parent) : QWidget(parent)
{
    numRows = 32;

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    grid = new QGridLayout(this);
    SETMARGIN_QT456(grid,0);
    grid->setSpacing(0);
    thisStartBit = 0;
    thisEndBit = 31;
    thisColorMode=Static;
    thisValue = 0;

    // for performance reasons create already 32 rectangles

    for (int i = 0; i < 32; i++) {
        rectangle* temp = new rectangle(this);
        temp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        cells.push_back(temp);
    }

    setDirection(Down);
    setTrueColor(Qt::blue);
    setFalseColor(Qt::gray);

    installEventFilter(this);
}

QString caByte::getPV() const
{
    return thisPV;
}

void caByte::setPV(QString const &newPV)
{
    thisPV = newPV;
}

void caByte::arrangeCells()
{

    // remove cells from grid
    foreach(rectangle *l, cells) {
        grid->removeWidget(l);
        l->hide();
    }

    // add requested cells to grid
    for (int i = 0; i < numRows; i++) {
        if(thisDirection == Up || thisDirection == Down) {
            grid->addWidget(cells[i], i, 0);
        } else {
            grid->addWidget(cells[i], 0, i);
        }
        cells[i]->show();
    }
    setValue(0);
}

bool caByte::bitState(long value, int bitNr)
{
    return ((((int) value >> bitNr) & 1) == 1);
}

void caByte::setValue(long value)
{
    thisValue = value;
    drawByte(thisValue, thisTrueColor, thisFalseColor);
}

void caByte::drawByte(long lvalue, QColor trueColor, QColor falseColor)
{
    if(thisDirection == Down || thisDirection == Right)  {
        for(int i=0; i<= thisEndBit - thisStartBit; i++) {
            qCDebug(caByteLog) << "1 numcells=" << numRows << "treat cell" << i;
            if(bitState(lvalue, i + thisStartBit)) {
                cells[i]->writeFG(trueColor);
            } else {
                cells[i]->writeFG(falseColor);
            }
        }
    } else {
        for(int i=0; i<= thisEndBit - thisStartBit; i++) {
            qCDebug(caByteLog) << "2 numcells=" << numRows << "treat cell" << thisEndBit - thisStartBit - i;
            if(bitState(lvalue, i + thisStartBit)) {
                cells[thisEndBit - thisStartBit  - i]->writeFG(trueColor);
            } else {
                cells[thisEndBit - thisStartBit  - i]->writeFG(falseColor);
            }
        }
    }
}

void caByte::setTrueColor(QColor c)
{
    thisTrueColor = c;
    setValue(thisValue);
}

void caByte::setFalseColor(QColor c)
{
    thisFalseColor = c;
    setValue(thisValue);
}

void caByte::setAlarmColors(short status)
{
    QColor c;
    switch (status) {

    case NO_ALARM:
        c=ThemeColors::color(caByteColor::AlarmNoAlarm);
        break;
    case MINOR_ALARM:
        c=ThemeColors::color(caByteColor::AlarmMinor);
        break;
    case MAJOR_ALARM:
        c=ThemeColors::color(caByteColor::AlarmMajor);
        break;
    case INVALID_ALARM:
    case NOTCONNECTED:
        c=ThemeColors::color(caByteColor::NotConnected);
        break;
    default:
        c=ThemeColors::color(caByteColor::AlarmDefault);
        break;
    }

    if(status == NOTCONNECTED) {
      for(int i=0; i<= thisEndBit - thisStartBit; i++)  cells[i]->writeFG(c);
    } else {
        drawByte(thisValue, c, thisFalseColor);
    }

}

void caByte::setStartBit(int const &bit) {
    thisStartBit = bit;
    numRows = thisEndBit - thisStartBit + 1;
    if(numRows < 1) {
        thisStartBit=0;
        thisEndBit = 0;
        numRows = 1;
    } else if(numRows > 32) {
        thisStartBit=0;
        thisEndBit = 31;
        numRows = 32;
    }
    arrangeCells();
}

void caByte::setEndBit(int const &bit) {
    thisEndBit = bit;
    numRows = thisEndBit - thisStartBit + 1;
    if(numRows < 1) {
        thisStartBit=0;
        thisEndBit = 0;
        numRows = 1;
    } else if(numRows > 32) {
        thisStartBit=0;
        thisEndBit = 31;
        numRows = 32;
    }
    arrangeCells();
}


void caByte::setDirection(Direction dir)
{
    thisDirection = dir;
    arrangeCells();
}

void caByte::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e);
}







