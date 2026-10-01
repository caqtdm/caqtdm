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
 *  Copyright (c) 2010 - 2025
 *
 *  Author:
 *    Anton Mezger
 *  Contact details:
 *    anton.mezger@psi.ch
 */


#include "qscrollbar.h"
#if defined(_MSC_VER)
#define NOMINMAX
#include <windows.h>
#define QWT_DLL
#endif

#include <stdio.h>
#include <QHeaderView>
#include <QApplication>
#include <QGuiApplication>
#include <QClipboard>
#include <qnumeric.h>
#include "cawavetable.h"
#include "cawavetablemodel.h"
#include "alarmdefs.h"

#define DEFAULT_CSV_SEPARATOR ','

Q_DECLARE_METATYPE(QtMsgType)

#if defined(_MSC_VER)
    #ifndef snprintf
     #define snprintf _snprintf
    #endif
#endif

Q_LOGGING_CATEGORY(caWaveTableLog, "caqtdm.widgets.cawavetable")

caWaveTable::caWaveTable(QWidget *parent) : QTableWidget(parent)
{
    thisFormatC[0] = '\0';
    thisFormat[0] = '\0';
    thisUnsigned = false;
    thisColorMode = Static;

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

    verticalOffset=1;
    horizontalOffset=1;
    verticalHeader()->setDefaultSectionSize(20);
    verticalHeader()->setSortIndicatorShown(false);
    caWaveTableModel* d=new caWaveTableModel(0,0,this);
    verticalHeader()->setModel(d);
    horizontalHeader()->setModel(d);

    QString csvSeparatorEnv = qgetenv("CAQTDM_CSV_SEPARATOR");
    if (!csvSeparatorEnv.isEmpty()) {
        csvSeparator = csvSeparatorEnv[0];
    } else {
        csvSeparator = DEFAULT_CSV_SEPARATOR;
    }


 #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    horizontalHeader()->setResizeMode(QHeaderView::Stretch);
#else
    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    //Iterate over every parent QWidget and check if any styles have been applied to it in designer --> If at any point any styles have been applied, do not overwrite them, else set the style so it looks like before the update.
    bool canSetStyle = true;
    const QWidget *parentWidgetPtr = this->parentWidget();
    const QString parentName = parentWidgetPtr ? parentWidgetPtr->objectName() : QStringLiteral("<no-parent>");
    for(QWidget *checkWidget = this;checkWidget->parentWidget();checkWidget = checkWidget->parentWidget()){
        if (!(checkWidget->styleSheet().isEmpty())){
            qCDebug(caWaveTableLog).noquote() << QString("Style for a child widget of %1 is NOT set by object, preferring Style from designer").arg(parentName);
            canSetStyle = false;
            break;
        }
    }
    if (canSetStyle){
        qCDebug(caWaveTableLog).noquote() << QString("Style for a child widget of %1 is set by object").arg(parentName);
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
    setColumnSize(80);
    setAttribute(Qt::WA_Hover);

    clearFocus();
    setAccessW(true);

    connect(this, SIGNAL(cellDoubleClicked (int, int) ), this, SLOT(cellDoubleclicked( int, int ) ) );
    connect(this, SIGNAL(cellChanged(int, int)), this, SLOT(dataInput(int, int)));
    connect(this, SIGNAL(cellClicked(int, int)), this, SLOT(cellClicked(int, int)));

    connect(this, SIGNAL(currentCellChanged(int, int, int, int)), this,  SLOT(cellChange(int,int, int, int)));

    connect(this->verticalScrollBar(),SIGNAL(valueChanged(int)),this, SLOT(vscrollbarInput(int)));
    connect(this->horizontalScrollBar(),SIGNAL(valueChanged(int)),this, SLOT(hscrollbarInput(int)));

    // find parent and connect slot
    QWidget *currentParent = parent;
    while (currentParent != Q_NULLPTR) {
        if (currentParent->metaObject() != Q_NULLPTR && qstrncmp(currentParent->metaObject()->className(), "CaQtDM_Lib", qstrlen("CaQtDM_Lib")) == 0) {
            connect(this, SIGNAL(messageWindowOutput(QtMsgType,QString)), currentParent, SLOT(messageWindowOutput(QtMsgType,QString)));
            break;
        }
        currentParent = currentParent->parentWidget();
    }

    builtInStyleSheet = styleSheet();
    builtInPalette = palette();

    blockIndex = -1;

    installEventFilter(this);

    thisAlignment = Left;
    setNumberOfRows(1);
    setNumberOfColumns(1);
    setFocusPolicy(Qt::ClickFocus);
}

void caWaveTable::vscrollbarInput(int scrollvalue)
{
    emit verticalScrollbarChanged(scrollvalue);
}
void caWaveTable::hscrollbarInput(int scrollvalue)
{
    emit verticalScrollbarChanged(scrollvalue);
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

void caWaveTable::noStyle(QString stylesheet){
    this->setStyleSheet(stylesheet);
}

void caWaveTable::setPanelThemePalette(bool legacyLight)
{
    QWidget *view = viewport();
    if (!viewportThemeSaved) {
        savedViewportPalette = view->palette();
        savedViewportHasPalette = view->testAttribute(Qt::WA_SetPalette);
        savedViewportStyleSheet = view->styleSheet();
        savedViewportAutoFill = view->autoFillBackground();
        const QPalette currentPalette = palette();
        const QPalette::ColorRole roles[] = {
            QPalette::Base, QPalette::AlternateBase, QPalette::Text, QPalette::Window
        };
        for (QPalette::ColorGroup group : {QPalette::Active, QPalette::Inactive, QPalette::Disabled}) {
            for (QPalette::ColorRole role : roles) {
                if (currentPalette.isBrushSet(group, role) &&
                    (!builtInPalette.isBrushSet(group, role) ||
                     currentPalette.brush(group, role) != builtInPalette.brush(group, role)))
                    hasAuthoredPalette = true;
            }
        }
        viewportThemeSaved = true;
    }

    if (!legacyLight || hasAuthoredPalette || styleSheet() != builtInStyleSheet ||
        !savedViewportStyleSheet.isEmpty()) {
        view->setStyleSheet(savedViewportStyleSheet);
        view->setPalette(savedViewportHasPalette ? savedViewportPalette : QPalette());
        view->setAutoFillBackground(savedViewportAutoFill);
        return;
    }

    QPalette viewPalette = view->palette();
    for (QPalette::ColorGroup group : {QPalette::Active, QPalette::Inactive, QPalette::Disabled}) {
        viewPalette.setColor(group, QPalette::Base, Qt::white);
        viewPalette.setColor(group, QPalette::AlternateBase, QColor(245, 245, 245));
        viewPalette.setColor(group, QPalette::Text, Qt::black);
    }
    view->setPalette(viewPalette);
    view->setAutoFillBackground(true);
    view->setStyleSheet("background-color: white; alternate-background-color: rgb(245, 245, 245); color: black;");
}
int caWaveTable::getVerticalOffset() const
{
    return verticalOffset;
}

void caWaveTable::setVerticalOffset(int newVerticalOffset)
{
    if (verticalOffset == newVerticalOffset)
        return;
    verticalOffset = newVerticalOffset;
    setupItems(rowcount, colcount);
    emit verticalOffsetChanged();
}

void caWaveTable::vscrollbarControl(int scrollvalue)
{
    verticalScrollBar()->setValue(scrollvalue);
}

void caWaveTable::hscrollbarControl(int scrollvalue)
{
    horizontalScrollBar()->setValue(scrollvalue);
}

int caWaveTable::getHorizontalOffset() const
{
    return horizontalOffset;
}

QString caWaveTable::getHeaderCSV()
{
    QString header = getHorizontalString();
    if (!header.isEmpty()) {
        int numTotalValues = qMin(sizeSaved, keepData.size());
        int numHeaderValues = header.count(';') + 1;

        // Sanitize header
        QStringList headerValues = header.split(';');
        for (auto& value: headerValues) {
            QString newValue = value;
            if (!value.startsWith('"')) {
                newValue = '"' + newValue;
            }
            if (!value.endsWith('"')) {
                newValue = newValue + '"';
            }
            value = newValue;
        }
        QString csvHeader = headerValues.join(csvSeparator);

        // Calculate how many empty header columns have to be inserted such that each following column has a header field -> better tool compatiblity
        int maxFilledColumns = qMin(colcount, numTotalValues);
        int missingHeaderValues = maxFilledColumns - numHeaderValues;
        if (missingHeaderValues > 0) {
            csvHeader = csvHeader.leftJustified(csvHeader.size() + missingHeaderValues, csvSeparator);
        }

        return csvHeader;
    }

    return "";
}

void caWaveTable::copyDataCSV()
{
    if (rowcount == 0 || colcount == 0 || sizeSaved == 0 || keepData.size() == 0) return;

    QVector<double> rawData = keepData;

    // Get the string representation of the data as displayed to the user
    QStringList stringData;
    stringData.reserve(rawData.size());
    int numTotalValues = qMin(sizeSaved, rawData.size());
    for (int i = 0; i < numTotalValues; i++) {
        stringData.push_back(setValue(rawData[i], keepDatatype));
    }

    // Create a csv string representing the tabular data encoded in the 1-Dimensional array
    // Structure: All elements of the first row, then all elements of the second row etc.
    // Reverse engineered from:
    // row = index / colcount; -> 0 = colcount < index
    // column = index - row * colcount;
    QString text = "";
    // For each row
    for (int i = 0; i < rowcount; i++) {
        bool gotAtLeastOne = false;
        // Go over all elements in it, based on the offset which is calculated using the previous number of columns
        for (int j = i * colcount; j < (i + 1) * colcount; j++) {
            if (j >= stringData.size()) break; // Shouldn't happen

            text += stringData[j] + csvSeparator;
            gotAtLeastOne = true;
        }
         if (gotAtLeastOne) {
            text[text.length() - 1] = '\n';
        } else {
            text.append('\n');
        }
    }
    // If the text was filled, remove the trailing separator/newline
    if (text.size() > 0) {
        text.remove(text.size() - 1, 1);
    }

    // Add header, if specified
    QString header = getHeaderCSV();
    if (!header.isEmpty()) {
        text = header + "\n" + text;
    }

    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(text);
}

void caWaveTable::pasteDataCSV()
{
    if (rowcount == 0 || colcount == 0) return;

    QClipboard *clipboard = QGuiApplication::clipboard();
    QString text = clipboard->text();

    if (text.size() == 0) return;

    // Figure out if the pasted data contains a header and remove it if so.
    // Pasting of header values is not allowed / headers should be ignored.
    QStringList lines = text.split("\n", Qt::SkipEmptyParts);
    bool removeHeader = false;
    if (lines.size() > 1) {
        // If the current header corresponds to the first pasted line, it is a header and should be ignored
        if (getHeaderCSV() == lines[0]) {
            removeHeader = true;
        } else {
            // Analyze the first possible header value to figure out if it can be converted into a value, if not it is a header
            QStringList headerParts = lines[0].split(csvSeparator, Qt::SkipEmptyParts);
            QString possibleHeaderValue;
            if (!headerParts.isEmpty()) {
                possibleHeaderValue = headerParts.first();
            }
            bool canConvert;
            switch (keepDatatype) {
            case doubles:
                if (thisFormatType == octal) {
                    possibleHeaderValue.replace('O', '0');
                }
                // In case its an integer format, propagate to long handling
                if (thisFormatType != octal && thisFormatType != hexadecimal) {
                    possibleHeaderValue.toDouble(&canConvert);
                    if (!canConvert) {
                        removeHeader = true;
                    }
                    break;
                }
                // propagates to long handling
            case longs:
                if (thisFormatType == octal) {
                    // For octal format to be automatically detected, use 0 instead of caQtDM-Style O.
                    possibleHeaderValue.replace('O', '0');
                }
                // base 0 makes it automatically detect format
                possibleHeaderValue.toInt(&canConvert, 0);
                if (!canConvert) {
                    removeHeader = true;
                }
                break;
            case characters:
                if (possibleHeaderValue.length() > 1) {
                    removeHeader = true;
                }
                break;
            case strings:
            default:
                // In this case we cannot distinguish.
                // Since it is not equal to the copied header, we can only assume it to be data.
                break;
            }
        }

        if (removeHeader) {
            emit messageWindowOutput(QtInfoMsg, "caWaveTable: When pasting user input, a header row was detected and removed");
            lines.removeFirst();
            text = lines.join('\n');
        }
    }

    if (!text.contains(csvSeparator) && colcount + rowcount > 1) {
        emit messageWindowOutput(QtCriticalMsg, "caWaveTable: When pasting user input, no CSV separators were found, aborting. CSV separator is: <" + QString(csvSeparator) + "> and can be defined via: CAQTDM_CSV_SEPARATOR env");
        return;
    }

    // Serialize it into one row, as internally it is stored as such
    text = text.replace("\n", csvSeparator);

    // Now we can separate it into an array of values, congruent with the stored data
    QStringList stringData;
    stringData = text.split(csvSeparator);

    // Emulate the user changing each cell manually, to get the same format parsing etc.
    int currentIndex = 0;
    for (int i = 0; i < rowcount; i++) {
        for (int j = 0; j < colcount; j++) {
            if (currentIndex >= stringData.size()) {
                break;
            }
            item(i, j)->setText(stringData[currentIndex]);
            currentIndex++;
            // blockIndex specifies that this item should be written back to the control system. will be reset after the write
            blockIndex = toIndex(i, j);
            dataInput(i, j);
        }
    }
}

void caWaveTable::setHorizontalOffset(int newHorizontalOffset)
{
    if (horizontalOffset == newHorizontalOffset)
        return;
    horizontalOffset = newHorizontalOffset;
    setupItems(rowcount, colcount);
    emit horizontalOffsetChanged();
}

QString caWaveTable::getVerticalString() const
{
    return verticalString;
}

void caWaveTable::setVerticalString(const QString &newVerticalString)
{
    if (verticalString == newVerticalString)
        return;
    verticalString = newVerticalString;
    setupItems(rowcount, colcount);
    emit verticalStringChanged();
}

QString caWaveTable::getHorizontalString() const
{
    return horizontalString;
}

void caWaveTable::setHorizontalString(const QString &newHorizontalString)
{
    if (horizontalString == newHorizontalString)
        return;
    horizontalString = newHorizontalString;
    setupItems(rowcount, colcount);
    emit horizontalStringChanged();
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
            if(Item != (QTableWidgetItem *) Q_NULLPTR) {
                delete Item;
            }
        }
    }
    clear();


    // setup table with alignment of items
    setColumnCount(nbCols);
    setRowCount(nbRows);
    QAbstractItemModel* temp_data=verticalHeader()->model();

    caWaveTableModel* dt=new caWaveTableModel(nbRows,nbCols,this);
    dt->setHorizontalOffset(this->horizontalOffset);
    dt->setVerticalOffset(this->verticalOffset);
    dt->setHorizontalString(this->horizontalString);
    dt->setVerticalString(this->verticalString);
    verticalHeader()->setModel(dt);
    horizontalHeader()->setModel(dt);

    verticalHeader()->update();
    horizontalHeader()->update();

    if (temp_data) delete(temp_data);

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
    keepData.clear();
    keepText.resize(rowcount*colcount+1);
    keepData.resize(rowcount*colcount+1);

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
        QString valueText = item(row, col)->text();

        clearSelection();

        // set the value back (dataInput is now blocked again)
        if(item(row,col) != (QTableWidgetItem*) Q_NULLPTR) {
            item(row,col)->setText(keepText[index]);
        }

        // and write it to the control system
        emit WaveEntryChanged(valueText, index);
    }
}

void caWaveTable::cellClicked(int row, int col)
{
    Q_UNUSED(row)
    Q_UNUSED(col)
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
                    qCDebug(caWaveTableLog) << "keyPressEvent ignore";
                    event->ignore();
                }
                else {
                    qCDebug(caWaveTableLog) << "keyPressEvent accept";
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
        qCDebug(caWaveTableLog) << "focus out";
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

        case user_defined_format:
            qstrncpy(thisFormat,thisFormatUserString.toLatin1().data(),MAX_STRING_LENGTH);
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
        case user_defined_format:
            qstrncpy(thisFormat,thisFormatUserString.toLatin1().data(),MAX_STRING_LENGTH);
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
        case user_defined_format:
            qstrncpy(thisFormat,thisFormatUserString.toLatin1().data(),MAX_STRING_LENGTH);
            break;
        }
    }

}

QString caWaveTable::setValue(double value, DataType dataType)
{
    char asc[MAX_STRING_LENGTH];

    if(dataType == doubles) {
        if(thisFormatType == compact) {
            if ((value < 1.e4 && value > 1.e-4) || (value > -1.e4 && value < -1.e-4) || value == 0.0) {
                snprintf(asc, MAX_STRING_LENGTH, thisFormatC, value);
            } else {
                snprintf(asc, MAX_STRING_LENGTH, thisFormat, value);
            }
        } else {
            switch (thisFormatType) {
             case hexadecimal:
                snprintf(asc, MAX_STRING_LENGTH, thisFormat, (int64_t)value);
                break;
            case octal:
                snprintf(asc, MAX_STRING_LENGTH, thisFormat, (int64_t)value);
                break;
            default:
                snprintf(asc, MAX_STRING_LENGTH, thisFormat, value);
            }
        }
    } else if(dataType == longs) {
        if(thisUnsigned) snprintf(asc, MAX_STRING_LENGTH, thisFormat, (uint) value);
        else snprintf(asc, MAX_STRING_LENGTH, thisFormat, (int) value);
    } else if(dataType == characters) {
        if(thisUnsigned) snprintf(asc, MAX_STRING_LENGTH, thisFormat, (uchar) value);
        else snprintf(asc, MAX_STRING_LENGTH, thisFormat, (char) value);
    }

    if(qIsNaN(value)){
      snprintf(asc, MAX_STRING_LENGTH,  "nan");
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
            this->item(row, column)->setForeground(QBrush());
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
    keepDatatype = strings;
    keepDatasize = qMin(size, maxSize);
    keepStatus = status;
}

void caWaveTable::setData(double *array, short status, int size)
{
    if(size != sizeSaved) RedefineRowColumns(rowSaved, colSaved, size, rowcount, colcount);
    int maxSize = rowcount * colcount;
    sizeSaved = size;

    setFormat(doubles);
    for(int i=0; i< qMin(size, maxSize); i++) {
        displayText(i, status, setValue(array[i], doubles));
        keepData[i] = array[i];
    }
    dataPresent = true;
    keepDatatype = doubles;
    keepDatasize = qMin(size, maxSize);
    keepStatus = status;
}

void caWaveTable::setData(float *array, short status, int size)
{
    if(size != sizeSaved) RedefineRowColumns(rowSaved, colSaved, size, rowcount, colcount);
    int maxSize = rowcount * colcount;
    sizeSaved = size;

    setFormat(doubles);
    for(int i=0; i< qMin(size, maxSize); i++) {
        displayText(i, status, setValue(array[i], doubles));
        keepData[i] = (double) array[i];
    }
    dataPresent = true;
    keepDatatype = doubles;
    keepDatasize = qMin(size, maxSize);
    keepStatus = status;
}

void caWaveTable::setData(int16_t *array, short status, int size)
{
    if(size != sizeSaved) RedefineRowColumns(rowSaved, colSaved, size, rowcount, colcount);
    int maxSize = rowcount * colcount;
    sizeSaved = size;

    setFormat(longs);
    for(int i=0; i< qMin(size, maxSize); i++) {
        displayText(i, status, setValue(array[i], longs));
        keepData[i] = (double) array[i];
    }
    dataPresent = true;
    keepDatatype = longs;
    keepDatasize = qMin(size, maxSize);
    keepStatus = status;
}

void caWaveTable::setData(int32_t *array, short status, int size)
{
    if(size != sizeSaved) RedefineRowColumns(rowSaved, colSaved, size, rowcount, colcount);
    int maxSize = rowcount * colcount;
    sizeSaved = size;

    setFormat(longs);
    for(int i=0; i< qMin(size, maxSize); i++) {
        displayText(i, status, setValue(array[i], longs));
        keepData[i] = (double) array[i];
    }
    dataPresent = true;
    keepDatatype = longs;
    keepDatasize = qMin(size, maxSize);
    keepStatus = status;
}

void caWaveTable::setData(char *array, short status, int size)
{
    if(size != sizeSaved) RedefineRowColumns(rowSaved, colSaved, size, rowcount, colcount);
    int maxSize = rowcount * colcount;
    sizeSaved = size;

    setFormat(characters);
    for(int i=0; i< qMin(size, maxSize); i++) {
        displayText(i, status, setValue((double) ((int) array[i]), characters));
        keepData[i] =  (double) ((int) array[i]);
    }
    dataPresent = true;
    keepDatatype = characters;
    keepDatasize = qMin(size, maxSize);
    keepStatus = status;
}

void caWaveTable::setDataType(QString const &datatype)
{
    if(datatype.contains("U")) thisUnsigned = true;
    else thisUnsigned = false;

    if(keepDatatype == strings) return;
    if(dataPresent) {
        for(int i=0; i< keepDatasize; i++) {
            displayText(i, keepStatus, setValue(keepData[i], keepDatatype));
        }
    }
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
            qCDebug(caWaveTableLog) << "no rows were selected";
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

}
