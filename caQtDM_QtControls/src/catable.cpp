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

#include <stdio.h>
#include <QHeaderView>
#include <QApplication>
#include <QClipboard>
#include "catable.h"
#include "alarmdefs.h"

caTable::caTable(QWidget *parent) : QTableWidget(parent)

{
    setPrecisionMode(Channel);
    setLimitsMode(Channel);
    setPrecision(0);
    setMinValue(0.0);
    setMaxValue(1.0);
    for(int i=0; i< MaxRows; i++) {
        setFormat(i, 1);
        for(int j=0; j< MaxCols; j++) tableItem[i][j] = (QTableWidgetItem*) 0;
    }

    thisItemFont = this->font();

    setColorMode(Static);
    setAlternatingRowColors(true);
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    setEditTriggers(QTableWidget::NoEditTriggers);
    verticalHeader()->setDefaultSectionSize(20);
    horizontalHeader()->setResizeMode(QHeaderView::Stretch);

    defaultForeColor = palette().foreground().color();

    createActions();
    addAction(copyAct);

    connect(this, SIGNAL( cellDoubleClicked (int, int) ), this, SLOT(celldoubleclicked( int, int ) ) );
    //connect(this, SIGNAL( cellClicked (int, int) ), this, SLOT(cellclicked( int, int ) ) );
}

void caTable::cellclicked(int row, int col)
{
    Q_UNUSED(row);
    Q_UNUSED(col);
}

void caTable::celldoubleclicked(int row, int col)
{
     if(col==1) emit TableDoubleClickedSignal(this->item(row, 0)->text());
     this->item(row,col)->setSelected(false);
}

void caTable::createActions() {

    copyAct = new QAction(this);
    copyAct->setShortcut(tr("Ctrl+C"));
    connect(copyAct, SIGNAL(triggered()), this, SLOT(copy()));
}

void caTable::setColumnSizes(QString const &newSizes)
{
    thisColumnSizes = newSizes.split(";");
    for(int i=0; i< thisColumnSizes.count(); i++) {
        int colsize=thisColumnSizes.at(i).toInt();
        if(i < columnCount())  {
            setColumnWidth(i,colsize);
        }
    }
}

void caTable::copy()
{
    QItemSelectionModel *select = this->selectionModel();
    if( select->hasSelection()) {
        QClipboard *clipboard = QApplication::clipboard();
        QString str;

        QModelIndexList rows = select->selectedRows();
        int i=0;
        foreach (QModelIndex Row, rows) {
            if (i > 0) str += "\n";
            for(int j = 0; j < columnCount(); ++j) {
                if (j > 0) str += "\t";
                QTableWidgetItem* pWidget = item(Row.row(), j);
                str += pWidget->text();
            }
            i++;
        }

        if(i==0) {
            //printf("no rows were selected\n");
            QModelIndexList cols = select->selectedColumns();
            foreach (QModelIndex Col, cols) {
                if (i > 0) str += "\n";
                for(int j = 0; j < rowCount(); ++j) {
                    if (j > 0) str += "\t";
                    QTableWidgetItem* pWidget = item(j, Col.column());
                    str += pWidget->text();
                }
                i++;
            }
        }
        if(i> 0) {
           str += "\n";
           clipboard->setText(str);
        }
    }
}

void caTable::setFormat(int row, int prec)
{
    int precision;
    if(row < 0 || row > MaxRows-1) return;
    if(thisPrecMode == User) {
        precision = getPrecision();
    } else {
        precision = prec;
    }
    if(precision > 17) precision = 17;
    if(precision >= 0) {
        sprintf(thisFormat[row], "%s.%dlf", "%", precision);
    } else {
        sprintf(thisFormat[row], "%s.%dle", "%", -precision);
    }
}

void caTable::displayText(int row, int col, short status, QString const &text)
{
    if(row < 0 || row > MaxRows-1) return;
    if(col < 0 || col > MaxCols-1) return;

    if(row >= rowCount() || col >= columnCount()) return;

    if(tableItem[row][col] != (QTableWidgetItem*) 0) {
        tableItem[row][col]->setText(text);
    } else {
        tableItem[row][col] = new QTableWidgetItem(text);
        tableItem[row][col]->setFont(thisItemFont);
        if(col==0) tableItem[row][col]->setTextAlignment(Qt::AlignAbsolute | Qt:: AlignLeft);
        else tableItem[row][col]->setTextAlignment(Qt::AlignAbsolute | Qt:: AlignRight);
        setItem(row, col, tableItem[row][col]);
    }

    if(thisColorMode == Alarm) {

        switch (status) {
        case -1:
            break;
        case NO_ALARM:
            tableItem[row][col]->setForeground(AL_GREEN);
            break;
        case MINOR_ALARM:
            tableItem[row][col]->setForeground(AL_YELLOW);
            break;
        case MAJOR_ALARM:
            tableItem[row][col]->setForeground(AL_RED);
            break;
        case INVALID_ALARM:
        case NOTCONNECTED:
            tableItem[row][col]->setForeground(AL_WHITE);
            break;
        default:
            tableItem[row][col]->setForeground(AL_DEFAULT);
            break;
        }
    }   else {
        tableItem[row][col]->setForeground(defaultForeColor);
    }
}

void caTable::setValueFont(QFont font)
{
   thisItemFont = font;
}


void caTable::setValue(int row, int col, short status, double value, QString const &unit)
{
    string40 text = {'\0'};
    short Alarm = -1;

    if(row < 0 || row > MaxRows-1) return;
    if(col < 0 || col > MaxCols-1) return;

    if(thisLimitsMode == Channel) {
        Alarm = status;
    } else if(thisLimitsMode == User) {
        if(value > getMaxValue() || value < getMinValue()) {
            Alarm = MAJOR_ALARM;
        } else {
            Alarm = NO_ALARM;
        }
    }
    sprintf(text, thisFormat[row], value);
    displayText(row, col, Alarm, text);
    displayText(row, col+1, Alarm, unit);
}


