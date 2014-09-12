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
#if defined(_MSC_VER)
#define NOMINMAX
#include <windows.h>
#define QWT_DLL
#endif

#include <stdio.h>
#include <QHeaderView>
#include <QApplication>
#include <QClipboard>
#include "cawavetable.h"
#include "alarmdefs.h"

caWaveTable::caWaveTable(QWidget *parent) : QTableWidget(parent)

{
    setPrecisionMode(Channel);
    setPrecision(0);

    for(int i=0; i< MaxItems; i++) {
        tableItem[i] = (QTableWidgetItem*)0;
        setFormat(0);
    }

    thisItemFont = this->font();

    setAlternatingRowColors(true);
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    setEditTriggers(QTableWidget::NoEditTriggers);
    setSelectionMode(QAbstractItemView::NoSelection);
    verticalHeader()->setDefaultSectionSize(40);
    setColumnSize(40);
    setOrientation(Horizontal);
}

void caWaveTable::setOrientation(Orientation orientation)
{
    thisOrientation = orientation;
    if(orientation == Horizontal) {
      setColumnCount(10);
      setRowCount(1);

    } else {
        setColumnCount(1);
        setRowCount(10);
    }
}

void caWaveTable::setColumnSize(int newSize)
{
    thisColumnSize = newSize;
    horizontalHeader()->setDefaultSectionSize(newSize);
    horizontalHeader()->setVisible(false);
    horizontalHeader()->setVisible(true);
}

void caWaveTable::setFormat(int prec)
{
    int precision;
    if(thisPrecMode == User) {
        precision = getPrecision();
    } else {
        precision = prec;
    }
    if(precision > 17) precision = 17;
    if(precision >= 0) {
        sprintf(thisFormat, "%s.%dlf", "%", precision);
    } else {
        sprintf(thisFormat, "%s.%dle", "%", -precision);
    }
}

void caWaveTable::displayText(int index, QString const &text)
{
    if(index >= MaxItems) return;
    if(keepText[index] == text) return;
    if(index >= columnCount() && thisOrientation == Horizontal) return;
    if(index >= rowCount() && thisOrientation == Vertical) return;
    keepText[index] = text;

    if(tableItem[index] != (QTableWidgetItem*) 0) {
        tableItem[index]->setText(text);
    } else {
        tableItem[index] = new QTableWidgetItem(text);
        if(thisOrientation == Horizontal) {
          setItem(0, index, tableItem[index]);
        } else {
          setItem(index, 0, tableItem[index]);
        }
        tableItem[index]->setFont(thisItemFont);
        setColumnWidth(index, thisColumnSize);
    }
}

void caWaveTable::setValueFont(QFont font)
{
   thisItemFont = font;
}

QString caWaveTable::getPV() const
    {
       return thisPV;
    }

void caWaveTable::setPV(QString const &newPV)
    {
        thisPV = newPV;
    }

void caWaveTable::setData(double *array, int size)
{
     fillData(array, size);
}

void caWaveTable::setData(float *array, int size)
{
    fillData(array, size);
}

void caWaveTable::setData(int16_t *array, int size)
{
    fillData(array, size);
}

void caWaveTable::setData(int32_t *array, int size)
{
    fillData(array, size);
}

template <typename pureData>
void caWaveTable::fillData(pureData *array, int size)
{
   string40 text;
   int maxSize = MaxItems;
   for(int i=0; i< qMin(size, maxSize); i++) {
       sprintf(text, thisFormat, array[i]);
       displayText(i, text);
   }
}



