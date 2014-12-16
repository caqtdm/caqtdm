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
    colcount = rowcount = 1;
    dataPresent = false;
    charsPresent = false;

    thisItemFont = this->font();

    setAlternatingRowColors(true);
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    verticalHeader()->setDefaultSectionSize(20);
    verticalHeader()->setSortIndicatorShown(true);
    setColumnSize(80);
    setAttribute(Qt::WA_Hover);

    clearFocus();
    setAccessW(true);

    connect(this, SIGNAL(cellDoubleClicked (int, int) ), this, SLOT(cellDoubleclicked( int, int ) ) );
    connect(this, SIGNAL(cellChanged(int, int)), this, SLOT(dataInput(int, int)));
    connect(this, SIGNAL(cellClicked(int, int)), this, SLOT(cellClicked(int, int)));

    setNumberOfRows(1);
    setNumberOfColumns(10);

    connect(this, SIGNAL(currentCellChanged(int, int, int, int)), this,  SLOT(cellChange(int,int, int, int)));

    createActions();
    addAction(copyAct);

    installEventFilter(this);
}

void caWaveTable::setNumberOfRows(int nbRows)
{
    rowcount = nbRows;
    setRowCount(nbRows);
    setupItems(rowcount, colcount);
}

void caWaveTable::setNumberOfColumns(int nbCols) {
    colcount = nbCols;
    setColumnCount(nbCols);
    setupItems(rowcount, colcount);
}

void caWaveTable::setupItems(int nbRows, int nbCols)
{

    for(int i=0; i<nbRows; i++) {
        for(int j=0; j<nbCols; j++) {
            if(this->item(i,j) == 0) {
                QTableWidgetItem *tableItem = new QTableWidgetItem();
                tableItem->setFont(thisItemFont);
                setItem(i, j, tableItem);
            }
        }
    }
    keepText.resize(rowcount*colcount+1);
    keepValue.resize(rowcount*colcount+1);
    blockItem.resize(rowcount*colcount+1);
    blockItem.fill(false);
}

void caWaveTable::cellChange(int currentRow, int currentColumn, int previousRow, int previousColumn) {
    //printf("focus changed to another cell\n");
    int index = toIndex(previousRow, previousColumn);
    if( dataPresent) blockItem[index] = false;
}

void caWaveTable::enableEdit(QTableWidgetItem* pItem)
{
    Qt::ItemFlags eFlags = pItem->flags();
    eFlags |= Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    pItem->setFlags(eFlags);
}

void caWaveTable::disableEdit(QTableWidgetItem* pItem)
{
    Qt::ItemFlags eFlags = pItem->flags();
    eFlags &= ~Qt::ItemIsEditable;
    pItem->setFlags(eFlags);
}

void caWaveTable::dataInput(int row, int col)
{
    double value;
    int index = toIndex(row,col);
    if(!dataPresent) return;
    if(blockItem.at(index)) {
        bool ok=true;
        blockItem[index] = false;
        QString valueText =  item(row, col)->text();

        // in case of a char array, transmit the value as an ascii code in a double
        if(charsPresent) {
            value = (double) valueText.trimmed().begin()->toAscii();
        // otherwise always as double
        } else {
            value = valueText.trimmed().toDouble(&ok);
        }
        keepValue[index] = value;
        clearSelection();

        //printf("modify at index=%d with value=%lf\n", index, keepValue[index]);

        if(ok) emit WaveEntryChanged(keepValue[index], index);

        if(item(row,col) != (QTableWidgetItem*) 0) {
            item(row,col)->setText(keepText[index]);
        }
    }
}

void caWaveTable::cellClicked(int row, int col)
{
    Q_UNUSED(row);
    Q_UNUSED(col);
    disableEdit(item(row, col));
    QTimer::singleShot(2000, this, SLOT(clearSelection()));
}

void caWaveTable::cellDoubleclicked(int row, int col)
{
    //printf("set row=%d col=%d editable\n", row, col);
    enableEdit(item(row,col));

    // prevent monitoring change of this item until focus is lost again
    blockItem[toIndex(row, col)] = true;
}

bool caWaveTable::eventFilter(QObject *obj, QEvent *event)
{

    // repeat enter or return key are not really wanted
    QKeyEvent *ev = static_cast<QKeyEvent *>(event);
    if(ev != (QKeyEvent *) 0) {
        if( ev->key()==Qt::Key_Return || ev->key()==Qt::Key_Enter ) {
            if(ev->isAutoRepeat() ) {
                //printf("keyPressEvent ignore\n");
                event->ignore();
            } else {
                //printf("keyPressEvent accept\n");
                event->accept();
            }
        }
    }

    // treat mouse enter and leave as well as focus out
    if (event->type() == QEvent::Enter) {
        if(!_AccessW) {
            QApplication::setOverrideCursor(QCursor(Qt::ForbiddenCursor));
        } else {
            QApplication::restoreOverrideCursor();
        }
    } else if(event->type() == QEvent::Leave) {
        QApplication::restoreOverrideCursor();
        clearFocus();
    } else if(event->type() == QEvent::FocusOut) {
        //printf("focus out\n");
    }
    return QObject::eventFilter(obj, event);
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

// calcualte from the row and column indexes the array index
int caWaveTable::toIndex(int row, int col) {
    return row*colcount+col;
}

// calculate from index the row and column indexes
void caWaveTable::fromIndex(int index, int &row, int &col)
{
    row = index / colcount;
    col = index - row * colcount;
}

void caWaveTable::displayText(int index, QString const &text)
{
    int column =0;
    int row = 0;

    if(index > colcount * rowcount) return;
    if(keepText[index] == text) return;

    keepText[index] = text;

    if(blockItem.at(index)) return;

    row = index / colcount;
    column = index - row * colcount;
    if(this->item(row, column) != 0)  this->item(row,column)->setText(text);
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
    string40 text;
    int maxSize = rowcount * colcount;
    for(int i=0; i< qMin(size, maxSize); i++) {
        keepValue[i] = (double) array[i];
        sprintf(text, thisFormat, array[i]);
        displayText(i, text);
    }
    dataPresent = true;
}

void caWaveTable::setData(float *array, int size)
{
    string40 text;
    int maxSize = rowcount * colcount;
    for(int i=0; i< qMin(size, maxSize); i++) {
        keepValue[i] = (double) array[i];
        sprintf(text, thisFormat, array[i]);
        displayText(i, text);
    }
    dataPresent = true;
}

void caWaveTable::setData(int16_t *array, int size)
{
    string40 text;
    int maxSize = rowcount * colcount;
    for(int i=0; i< qMin(size, maxSize); i++) {
        keepValue[i] = (double) array[i];
        sprintf(text, "%d", array[i]);
        displayText(i, text);
    }
    dataPresent = true;
}

void caWaveTable::setData(int32_t *array, int size)
{
    string40 text;
    int maxSize = rowcount * colcount;
    for(int i=0; i< qMin(size, maxSize); i++) {
        keepValue[i] = (double) array[i];
        sprintf(text, "%d", array[i]);
        displayText(i, text);
    }
    dataPresent = true;
}

void caWaveTable::setData(char *array, int size)
{
    int maxSize = rowcount * colcount;
    for(int i=0; i< qMin(size, maxSize); i++) {
        keepValue[i] = (double) ((int) array[i]);
        QString str= QChar(array[i]);
        displayText(i, str);
    }
    dataPresent = true;
    charsPresent = true;
}

void caWaveTable::setAccessW(int access)
{
    _AccessW = access;
}


void caWaveTable::copy()
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

void caWaveTable::createActions() {

    copyAct = new QAction(this);
    copyAct->setShortcut(tr("Ctrl+C"));
    copyAct->setShortcutContext(Qt::WidgetShortcut);
    connect(copyAct, SIGNAL(triggered()), this, SLOT(copy()));
}


