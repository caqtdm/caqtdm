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
#include "themecolors.h"
#include <QTimer>

#if defined(_MSC_VER)
    #ifndef snprintf
     #define snprintf _snprintf
    #endif
#endif

Q_LOGGING_CATEGORY(caTableLog, "caqtdm.widgets.catable")

// colour set of caTable (level 2), values come from themecolors.h
namespace caTableColor {
    constexpr const ThemeColor &AlarmNoAlarm  = ThemeColors::AlarmNoAlarm;
    constexpr const ThemeColor &AlarmMinor    = ThemeColors::AlarmMinor;
    constexpr const ThemeColor &AlarmMajor    = ThemeColors::AlarmMajor;
    constexpr const ThemeColor &AlarmDefault  = ThemeColors::AlarmDefault;
    constexpr const ThemeColor &NotConnected  = ThemeColors::AlarmInvalid;
    constexpr const ThemeColor &AlternateBase = ThemeColors::TableAlternateBase;
    constexpr const ThemeColor &Header        = ThemeColors::TableHeader;
    constexpr const ThemeColor &Grid          = ThemeColors::TableGrid;
}

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
    refreshPending = false;
    applyingColors = false;
    builtInStyleEnabled = false;

    setColorMode(Static);
    setAlternatingRowColors(true);
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    setEditTriggers(QTableWidget::NoEditTriggers);
    verticalHeader()->setDefaultSectionSize(20);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    horizontalHeader()->setResizeMode(QHeaderView::Interactive);
#else
    horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    //Iterate over every parent QWidget and check if any styles have been applied to it in designer --> If at any point any styles have been applied, do not overwrite them, else set the style so it looks like before the update.
    bool canSetStyle = true;
    const QWidget *parentWidgetPtr = this->parentWidget();
    const QString parentName = parentWidgetPtr ? parentWidgetPtr->objectName() : QStringLiteral("<no-parent>");
    for(QWidget *checkWidget = this;checkWidget->parentWidget();checkWidget = checkWidget->parentWidget()){
        if (!(checkWidget->styleSheet().isEmpty())){
            qCDebug(caTableLog).noquote() << QString("Style for a child widget of %1 is NOT set by object, preferring Style from designer").arg(parentName);
            canSetStyle = false;
            break;
        }
    }
    if (canSetStyle) qCDebug(caTableLog).noquote() << QString("Style for a child widget of %1 is set by object").arg(parentName);
    builtInStyleEnabled = canSetStyle;   // a panel stylesheet applied later must not drop it
    applyBuiltInStyle(true);
#endif
    createActions();

    connect(this, SIGNAL( cellDoubleClicked (int, int) ), this, SLOT(celldoubleclicked( int, int ) ) );
    setFocusPolicy(Qt::ClickFocus);
}

// Qt6 only: header and alternate-row colours of the built-in look, by scheme
// (whether the built-in look applies at all is decided once in the constructor)
void caTable::applyBuiltInStyle(bool initial)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if(!builtInStyleEnabled) return;
    const ThemeColors::Scheme s = ThemeColors::schemeOf(this);
    applyingColors = true;
    // keep what external stylesheets put into the palette: replace AlternateBase
    // only while it still holds our own value (either scheme)
    QPalette p = palette();
    const QColor current = p.color(QPalette::AlternateBase);
    const QColor target = ThemeColors::color(caTableColor::AlternateBase, s);
    const bool ours = current == ThemeColors::color(caTableColor::AlternateBase, ThemeColors::Light)
            || current == ThemeColors::color(caTableColor::AlternateBase, ThemeColors::Dark);
    if((initial || ours) && current != target) {
        p.setColor(QPalette::AlternateBase, target);
        setPalette(p);
    }
    qCDebug(caTableLog) << "built-in style" << (s == ThemeColors::Dark ? "dark" : "light") << this->objectName();
    const QString grid = ThemeColors::css(caTableColor::Grid, s);
    const QString header = ThemeColors::css(caTableColor::Header, s);
    const QString style = QString(
            "QHeaderView::section{border-right:1px solid %1;border-bottom: 1px solid %1;background-color:%2;}"
            "QTableCornerButton::section{border-right:1px solid %1;border-bottom: 1px solid %1;background-color:%2;}"
            "QScrollBar{border-right:1px solid %1;border-bottom:1px solid %1}").arg(grid, header);
    if(styleSheet() != style) setStyleSheet(style);
    ThemeColors::markSelfStyled(this);               // the matrix leaves our sheet alone
    applyingColors = false;
#endif
}

// the panel palette changed (theme switch): regenerate the colours, deferred
void caTable::changeEvent(QEvent *e)
{
    QTableWidget::changeEvent(e);
    if(applyingColors || refreshPending) return;
    if(e->type() == QEvent::PaletteChange || e->type() == QEvent::StyleChange) {
        refreshPending = true;
        QTimer::singleShot(0, this, SLOT(refreshTheme()));
    }
}

void caTable::refreshTheme()
{
    refreshPending = false;
    applyBuiltInStyle(false);
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
            qCDebug(caTableLog) << "no rows were selected";
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
            tableItem[row][col]->setForeground(ThemeColors::color(caTableColor::AlarmNoAlarm));
            break;
        case MINOR_ALARM:
            tableItem[row][col]->setForeground(ThemeColors::color(caTableColor::AlarmMinor));
            break;
        case MAJOR_ALARM:
            tableItem[row][col]->setForeground(ThemeColors::color(caTableColor::AlarmMajor));
            break;
        case INVALID_ALARM:
        case NOTCONNECTED:
            tableItem[row][col]->setForeground(ThemeColors::color(caTableColor::NotConnected));
            break;
        default:
            tableItem[row][col]->setForeground(ThemeColors::color(caTableColor::AlarmDefault));
            break;
        }
    }   else {
        tableItem[row][col]->setForeground(QBrush());     // palette text, follows the theme
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
