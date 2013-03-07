//******************************************************************************
// Copyright (c) 2012 Paul Scherrer Institut PSI), Villigen, Switzerland
// Disclaimer: neither  PSI, nor any of their employees makes any warranty
// or assumes any legal liability or responsibility for the use of this software
//******************************************************************************
//******************************************************************************
//
//     Author : Anton Chr. Mezger
//
//******************************************************************************

#include "cabyte.h"
#include "alarmdefs.h"
#include <QGridLayout>
#include <QtDebug>

caByte::caByte(QWidget *parent) : QWidget(parent)
{
    numRows = 16;

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    grid = new QGridLayout(this);
    grid->setMargin(0);
    grid->setSpacing(0);
    thisStartBit = 0;
    thisEndBit = 15;
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

void caByte::setValue(int value)
{
    thisValue = (long)value;
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
    case ALARM_INVALID:
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
    } else if(numRows > 16) {
        thisStartBit=0;
        thisEndBit = 15;
        numRows = 16;
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
    } else if(numRows > 16) {
        thisStartBit=0;
        thisEndBit = 15;
        numRows = 16;
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







