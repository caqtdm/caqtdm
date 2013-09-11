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
    }

    thisItemFont = this->font();

    setColorMode(Static);
    setAlternatingRowColors(true);
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    setSelectionMode(NoSelection);
    verticalHeader()->setDefaultSectionSize(20);

    defaultForeColor = palette().foreground().color();

    createActions();
    addAction(copyAct);
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
        QString out;
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
        str += "\n";
        clipboard->setText(str);
    }

    /*
    for(int i = 0; i < rowCount(); ++i)
    {
        for(int j = 0; j < columnCount(); ++j)
        {
            QTableWidgetItem* pWidget = item(i, j);
            out.append(pWidget->text());
            out.append("\t");
        }
        out.append("\n");
    }
    clipboard->setText(out);
*/
}

void caTable::setFormat(int row, int prec)
{
    if(row < 0 || row > MaxRows-1) return;
    if(thisPrecMode == User) {
        int precision = getPrecision();
        sprintf(thisFormat[row], "%s.%dlf", "%", precision);
    } else {
        sprintf(thisFormat[row], "%s.%dlf", "%", prec);
    }
}

void caTable::displayText(int row, int col, short status, QString const &text)
{
    if(row < 0 || row > MaxRows-1) return;
    if(col < 0 || col > MaxCols-1) return;

    if(keepText[row][col] == text) {  // accelerate things
        return;
    }
    if(row >= rowCount() || col >= columnCount()) return;

    QTableWidgetItem* item = new QTableWidgetItem(text);
    if(col==0) item->setTextAlignment(Qt::AlignAbsolute | Qt:: AlignLeft);
    else item->setTextAlignment(Qt::AlignAbsolute | Qt:: AlignRight);

    if(thisColorMode == Alarm) {

        switch (status) {
        case -1:
            break;
        case NO_ALARM:
            item->setForeground(AL_GREEN);
            break;
        case MINOR_ALARM:
            item->setForeground(AL_YELLOW);
            break;
        case MAJOR_ALARM:
            item->setForeground(AL_RED);
            break;
        case ALARM_INVALID:
        case NOTCONNECTED:
            item->setForeground(AL_WHITE);
            break;
        default:
            item->setForeground(AL_DEFAULT);
            break;
        }
    }   else {
        item->setForeground(defaultForeColor);
    }

    setItem(row, col, item);
    item->setFont(thisItemFont);

}

void caTable::setValueFont(QFont font)
{
   thisItemFont = font;
}


void caTable::setValue(int row, int col, short status, double value, QString const &unit)
{
    string40 text;
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


