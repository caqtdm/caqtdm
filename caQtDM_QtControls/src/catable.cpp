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

#if defined(_MSC_VER)
    #ifndef snprintf
     #define snprintf _snprintf
    #endif
#endif

caTable::caTable(QWidget *parent) : QTableWidget(parent)

{
    setPrecisionMode(Channel);
    setLimitsMode(Channel);
    setPrecision(0);
    setMinValue(0.0);
    setMaxValue(1.0);
    for(int i=0; i< MaxRows; i++) {
        setFormat(i, 1);
        for(int j=0; j< MaxCols; j++) tableItem[i][j] = (QTableWidgetItem*) Q_NULLPTR;
    }

    thisItemFont = this->font();

    setColorMode(Static);
    setAlternatingRowColors(true);
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    setEditTriggers(QTableWidget::NoEditTriggers);
    verticalHeader()->setDefaultSectionSize(20);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    horizontalHeader()->setResizeMode(QHeaderView::Interactive);
    defaultForeColor = palette().foreground().color();
#else
    horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    defaultForeColor = palette().windowText().color();

    //Iterate over every parent QWidget and check if any styles have been applied to it in designer --> If at any point any styles have been applied, do not overwrite them, else set the style so it looks like before the update.
    bool canSetStyle = true;
    for(QWidget *checkWidget = this;checkWidget->parentWidget();checkWidget = checkWidget->parentWidget()){
        if (!(checkWidget->styleSheet().isEmpty())){
            //qDebug().noquote()<<QString("Style for a child widget of %1 is NOT set by object, preferring Style from designer").arg(this->parentWidget()->objectName());
            canSetStyle = false;
            break;
        }
    }
    if (canSetStyle){
        //qDebug().noquote()<<QString("Style for a child widget of %1 is set by object").arg(this->parentWidget()->objectName());
        QPalette p = QPalette();
        p.setColor(QPalette::AlternateBase, QColor(233, 231, 227));
        setPalette(p);
        setStyleSheet(
            "QHeaderView::section{"
            "border-right:1px solid #D8D8D8;"
            "border-bottom: 1px solid #D8D8D8;"
            "background-color:#f0f0f0;"
            "}"
            "QTableCornerButton::section{"
            "border-right:1px solid #D8D8D8;"
            "border-bottom: 1px solid #D8D8D8;"
            "background-color:#f0f0f0;"
            "}"
            "QScrollBar{"
            "border-right:1px solid #D8D8D8;"
            "border-bottom:1px solid #D8D8D8"
            "}");
    }
#endif
    createActions();

    connect(this, SIGNAL( cellDoubleClicked (int, int) ), this, SLOT(celldoubleclicked( int, int ) ) );
    setFocusPolicy(Qt::ClickFocus);
}

void caTable::cellclicked(int row, int col)
{
    Q_UNUSED(row);
    Q_UNUSED(col);
}

void caTable::celldoubleclicked(int row, int col)
{
     if(col==1) emit TableDoubleClickedSignal(this->item(row, 0)->text());
     if(tableItem[row][col] != (QTableWidgetItem*) Q_NULLPTR)  this->item(row,col)->setSelected(false);
}

void caTable::createActions() {

}

void caTable::setColumnSizes(QString const &newSizes)
{
    if(newSizes.size() > 0) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        horizontalHeader()->setResizeMode(QHeaderView::Fixed);
#else
        horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
#endif
        thisColumnSizes = newSizes.split(";");
        for(int i=0; i< thisColumnSizes.count(); i++) {
            int colsize=thisColumnSizes.at(i).toInt();
            if(i < columnCount())  {
                setColumnWidth(i,colsize);
            }
        }
    } else {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        horizontalHeader()->setResizeMode(QHeaderView::Interactive);
#else
        horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
#endif

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

    if(tableItem[row][col] != (QTableWidgetItem*) Q_NULLPTR) {
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
    //sprintf(text, thisFormat[row], value);
    snprintf(text, 40, thisFormat[row], value);
    displayText(row, col, Alarm, text);
    displayText(row, col+1, Alarm, unit);
}


