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
    thisFormatC[0] = '\0';
    thisFormat[0] = '\0';

    setPrecisionMode(Channel);
    setFormatType(decimal);
    setPrecision(0);
    setActualPrecision(0);

    colSaved = rowSaved = colcount = rowcount = 1;
    sizeSaved = -1;
    dataPresent = false;
    thisItemFont = this->font();

    setAlternatingRowColors(true);
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    verticalHeader()->setDefaultSectionSize(20);
    verticalHeader()->setSortIndicatorShown(false);
    horizontalHeader()->setResizeMode(QHeaderView::Stretch);

    setColumnSize(80);
    setAttribute(Qt::WA_Hover);

    clearFocus();
    setAccessW(true);

    connect(this, SIGNAL(cellDoubleClicked (int, int) ), this, SLOT(cellDoubleclicked( int, int ) ) );
    connect(this, SIGNAL(cellChanged(int, int)), this, SLOT(dataInput(int, int)));
    connect(this, SIGNAL(cellClicked(int, int)), this, SLOT(cellClicked(int, int)));

    connect(this, SIGNAL(currentCellChanged(int, int, int, int)), this,  SLOT(cellChange(int,int, int, int)));

    createActions();
    addAction(copyAct);

    defaultForeColor = palette().foreground().color();

    blockIndex = -1;

    installEventFilter(this);

    thisAlignment = Left;
    setNumberOfRows(1);
    setNumberOfColumns(1);
}

void caWaveTable::RedefineRowColumns(int xsav, int ysav, int z, int &x, int &y)
{
    /* expand to 1 column and size rows */
    if((xsav == 0) && (ysav == 0)) {
        y = 1;
        x = z;
        setupItems(x, y);
        /* expand to (size/colcount) rows */
    } else if((ysav > 0) && (xsav == 0)) {
        x = qRound((float) z / (float) y);
        setupItems(x, y);
        /* expand to (size/rowcount) rows */
    } else if((xsav > 0) && (ysav == 0)) {
        y = qRound((float) z / (float) x);
        setupItems(x, y);
    } else {
       setupItems(x, y);
    }
}

void caWaveTable::setNumberOfRows(int nbRows)
{
    if(nbRows <=0) rowSaved = rowcount = 0;
    else rowSaved = rowcount = nbRows;
    setupItems(rowcount, colcount);
}

void caWaveTable::setNumberOfColumns(int nbCols) {
    if(nbCols <=0) colSaved = colcount = 0;
    else colSaved = colcount = nbCols;
    setupItems(rowcount, colcount);
}

void caWaveTable::setupItems(int nbRows, int nbCols)
{
    // get rid of old items and clear table
    for(int i=0; i<rowCount(); i++) {
        for(int j=0; j<columnCount(); j++) {
            QTableWidgetItem *Item = item(i,j);
            if(Item != (QTableWidgetItem *) 0) {
                delete Item;
            }
        }
    }
    clear();

    // setup table with alignment of items
    setColumnCount(nbCols);
    setRowCount(nbRows);
    for(int i=0; i<nbRows; i++) {
        for(int j=0; j<nbCols; j++) {

            setItem(i,j, new QTableWidgetItem());
            item(i,j)->setFont(thisItemFont);
            switch (thisAlignment) {
            case Left:
                item(i,j)->setTextAlignment(Qt::AlignLeft);
                break;
            case Center:
                item(i,j)->setTextAlignment(Qt::AlignCenter);
                break;
            case Right:
            default:
                item(i,j)->setTextAlignment(Qt::AlignRight);
                break;
            }
        }
    }

    keepText.clear();
    keepText.resize(rowcount*colcount+1);
}

void caWaveTable::cellChange(int currentRow, int currentColumn, int previousRow, int previousColumn) {
    Q_UNUSED(currentRow);
    Q_UNUSED(currentColumn);
    Q_UNUSED(previousRow);
    Q_UNUSED(previousColumn);
    blockIndex = -1;
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
    int index = toIndex(row,col);
    if(!dataPresent) return;

    if(index == blockIndex) {
        blockIndex = -1;
        QString valueText =  item(row, col)->text();

        clearSelection();

        // set the value back (dataInput is now blocked again
        if(item(row,col) != (QTableWidgetItem*) 0) {
            item(row,col)->setText(keepText[index]);
        }

        // and write it to the control system
        emit WaveEntryChanged(valueText, index);
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
    enableEdit(item(row,col));

    // prevent monitoring change of this item until focus is lost again
    blockIndex = toIndex(row, col);
}

bool caWaveTable::eventFilter(QObject *obj, QEvent *event)
{

    // repeat enter or return key are not really wanted
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *ev = static_cast<QKeyEvent *>(event);
        if (ev != (QKeyEvent *)0) {
            if (ev->key() == Qt::Key_Return || ev->key() == Qt::Key_Enter) {
                if (ev->isAutoRepeat()) {
                    //printf("keyPressEvent ignore\n");
                    event->ignore();
                }
                else {
                    //printf("keyPressEvent accept\n");
                    event->accept();
                }
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

void caWaveTable::setActualPrecision(int prec)
{
    if(thisPrecMode == User) {
        actualPrecision = getPrecision();
    } else {
        channelPrecision = prec;
        actualPrecision = channelPrecision;
    }
    if(actualPrecision > 17) actualPrecision = 17;
}

void caWaveTable::setFormat(DataType dataType)
{
    if(dataType == doubles) {
        switch (thisFormatType) {
        case string:
        case decimal:
            if(actualPrecision >= 0) {
                sprintf(thisFormat, "%s.%dlf", "%", actualPrecision);
            } else {
                sprintf(thisFormat, "%s.%dle", "%", -actualPrecision);
            }
            break;
        case compact:
            sprintf(thisFormat, "%s.%dle", "%", qAbs(actualPrecision));
            sprintf(thisFormatC, "%s.%dlf", "%", qAbs(actualPrecision));
            break;
        case exponential:
            sprintf(thisFormat, "%s.%dle", "%", qAbs(actualPrecision));
            break;
        case hexadecimal:
            strcpy(thisFormat, "0x%x");
            break;
        case octal:
            strcpy(thisFormat, "O%o");
            break;
        }

    } else if (dataType == longs) {
        switch (thisFormatType) {
        case string:
        case decimal:
        case compact:
        case exponential:
            strcpy(thisFormat, "%d");
            strcpy(thisFormatC, "%d");
            break;
        case hexadecimal:
            strcpy(thisFormat, "0x%x");
            break;
        case octal:
            strcpy(thisFormat, "O%o");
            break;
        }
    } else if(dataType == characters) {
        switch (thisFormatType) {
        case string:
            strcpy(thisFormat, "%c");
            strcpy(thisFormatC, "%c");
            break;
        case decimal:
        case compact:
        case exponential:
            strcpy(thisFormat, "%d");
            strcpy(thisFormatC, "%d");
            break;
        case hexadecimal:
            strcpy(thisFormat, "0x%x");
            break;
        case octal:
            strcpy(thisFormat, "O%o");
            break;
        }
    }

}

QString caWaveTable::setValue(double value, DataType dataType)
{
    char asc[40];

    if(dataType == doubles) {
        if(thisFormatType == compact) {
            if ((value < 1.e4 && value > 1.e-4) || (value > -1.e4 && value < -1.e-4) || value == 0.0) {
                sprintf(asc, thisFormatC, value);
            } else {
                sprintf(asc, thisFormat, value);
            }
        } else {
            sprintf(asc, thisFormat, value);
        }
    } else if(dataType == longs) {
        sprintf(asc, thisFormat, (int) value);
    } else if(dataType == characters) {
        sprintf(asc, thisFormat, (int) value);
    }

    return QString(asc);
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

void caWaveTable::displayText(int index, short status, QString const &text)
{
    int column =0;
    int row = 0;

    if(index > colcount * rowcount) return;
    if(keepText[index] == text) return;

    keepText[index] = text;

    if(index == blockIndex) return;

    if(colcount == 0) row = 0;
    else row = index / colcount;
    column = index - row * colcount;
    if(column < 0) column=0;

    if(this->item(row, column) != 0)  {

        this->item(row,column)->setText(text);

        if(thisColorMode == Alarm) {

            switch (status) {
            case -1:
                break;
            case NO_ALARM:
                this->item(row, column)->setForeground(AL_GREEN);
                break;
            case MINOR_ALARM:
                this->item(row, column)->setForeground(AL_YELLOW);
                break;
            case MAJOR_ALARM:
                this->item(row, column)->setForeground(AL_RED);
                break;
            case INVALID_ALARM:
            case NOTCONNECTED:
                this->item(row, column)->setForeground(AL_WHITE);
                break;
            default:
                this->item(row, column)->setForeground(AL_DEFAULT);
                break;
            }
        }   else {
            this->item(row, column)->setForeground(defaultForeColor);
        }
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

void caWaveTable::setStringList(QStringList list, short status, int size)
{
    if(size != sizeSaved) RedefineRowColumns(rowSaved, colSaved, size, rowcount, colcount);
    int maxSize = rowcount * colcount;
    sizeSaved = size;

    for(int i=0; i< qMin(size, maxSize); i++) {
        displayText(i, status, list.at(i));
    }
    dataPresent = true;
}

void caWaveTable::setData(double *array, short status, int size)
{
    if(size != sizeSaved) RedefineRowColumns(rowSaved, colSaved, size, rowcount, colcount);
    int maxSize = rowcount * colcount;
    sizeSaved = size;

    setFormat(doubles);
    for(int i=0; i< qMin(size, maxSize); i++) {
        displayText(i, status, setValue(array[i], doubles));
    }
    dataPresent = true;
}

void caWaveTable::setData(float *array, short status, int size)
{
    if(size != sizeSaved) RedefineRowColumns(rowSaved, colSaved, size, rowcount, colcount);
    int maxSize = rowcount * colcount;
    sizeSaved = size;

    setFormat(doubles);
    for(int i=0; i< qMin(size, maxSize); i++) {
        displayText(i, status, setValue(array[i], doubles));
    }
    dataPresent = true;
}

void caWaveTable::setData(int16_t *array, short status, int size)
{
    if(size != sizeSaved) RedefineRowColumns(rowSaved, colSaved, size, rowcount, colcount);
    int maxSize = rowcount * colcount;
    sizeSaved = size;

    setFormat(longs);
    for(int i=0; i< qMin(size, maxSize); i++) {
        displayText(i, status, setValue(array[i], longs));
    }
    dataPresent = true;
}

void caWaveTable::setData(int32_t *array, short status, int size)
{
    if(size != sizeSaved) RedefineRowColumns(rowSaved, colSaved, size, rowcount, colcount);
    int maxSize = rowcount * colcount;
    sizeSaved = size;

    setFormat(longs);
    for(int i=0; i< qMin(size, maxSize); i++) {
        displayText(i, status, setValue(array[i], longs));
    }
    dataPresent = true;
}

void caWaveTable::setData(char *array, short status, int size)
{
    if(size != sizeSaved) RedefineRowColumns(rowSaved, colSaved, size, rowcount, colcount);
    int maxSize = rowcount * colcount;
    sizeSaved = size;

    setFormat(characters);
    for(int i=0; i< qMin(size, maxSize); i++) {
        displayText(i, status, setValue((double) ((int) array[i]), characters));
    }
    dataPresent = true;
}

void caWaveTable::setAccessW(bool access)
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


