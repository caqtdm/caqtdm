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
#include <QGridLayout>
#include <QtDebug>

caByte::caByte(QWidget *parent) : QWidget(parent)
{
    numRows = 32;

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    grid = new QGridLayout(this);
    grid->setMargin(0);
    grid->setSpacing(0);
    thisStartBit = 0;
    thisEndBit = 31;
    thisColorMode=Static;
    setDirection(Down);
    setTrueColor(Qt::blue);
    setFalseColor(Qt::gray);
    arrangeCells();

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
    foreach(rectangle *l, cells) {
        grid->removeWidget(l);
        l->hide();
        l->deleteLater();
    }
    cells.clear();

    for (int i = 0; i < numRows; i++) {
        rectangle* temp = new rectangle(this);
        temp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        if(thisDirection == Up || thisDirection == Down) {
          grid->addWidget(temp, i, 0);
        } else {
          grid->addWidget(temp, 0, i);
        }
        cells.push_back(temp);
        temp->show();
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
            //printf("1 numcells=%d treat cell %d\n", numRows, i);
            if(bitState(lvalue, i + thisStartBit)) {
                cells[i]->writeFG(trueColor);
            } else {
                cells[i]->writeFG(falseColor);
            }
        }
    } else {
        for(int i=0; i<= thisEndBit - thisStartBit; i++) {
            //printf("2 numcells=%d treat cell %d\n", numRows, thisEndBit - thisStartBit - i);
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
    arrangeCells();
}

void caByte::setFalseColor(QColor c)
{
    thisFalseColor = c;
    arrangeCells();
}

void caByte::setAlarmColors(short status)
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







