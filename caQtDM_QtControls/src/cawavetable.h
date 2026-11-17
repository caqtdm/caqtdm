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

#ifndef CAWAVETABLE_H
#define CAWAVETABLE_H

#include <QTableWidget>
#include <QAction>
#include <QFont>
#include <QEvent>
#include <QKeyEvent>
#include <QTimer>
#include <stdint.h>
#include <qtcontrols_global.h>

typedef char string40[40];

class QTCON_EXPORT caWaveTable : public QTableWidget
{
    Q_OBJECT
    Q_PROPERTY(int rowCount READ rowCount WRITE setRowCount DESIGNABLE false)
    Q_PROPERTY(int columnCount READ columnCount WRITE setColumnCount DESIGNABLE false)

    Q_PROPERTY(QString channel READ getPV WRITE setPV)
    Q_PROPERTY(int numberOfRows READ getNumberOfRows WRITE setNumberOfRows)
    Q_PROPERTY(int numberOfColumns READ getNumberOfColumns WRITE setNumberOfColumns)
    Q_PROPERTY(int columnSize READ getColumnSize WRITE setColumnSize)

    Q_PROPERTY(Alignment alignment READ getAlignment WRITE setAlignment)

    Q_PROPERTY(colMode colorMode READ getColorMode WRITE setColorMode)

    Q_PROPERTY(int precision READ getPrecision WRITE setPrecision)
    Q_PROPERTY(SourceMode precisionMode READ getPrecisionMode WRITE setPrecisionMode)

    Q_PROPERTY(FormatType formatType READ getFormatType WRITE setFormatType)
    Q_PROPERTY(QString formatString READ getFormatString WRITE setFormatString)

    Q_PROPERTY(QString styleSheet READ styleSheet WRITE noStyle)


    Q_PROPERTY(QString horizontalString READ getHorizontalString WRITE setHorizontalString )
    Q_PROPERTY(QString verticalString READ getVerticalString WRITE setVerticalString)
    Q_PROPERTY(int horizontalOffset READ getHorizontalOffset WRITE setHorizontalOffset)
    Q_PROPERTY(int verticalOffset READ getVerticalOffset WRITE setVerticalOffset)

public:
    //void noStyle(QString style) {Q_UNUSED(style);}

    caWaveTable(QWidget *parent);

    enum FormatType {decimal, exponential, compact, hexadecimal, octal, string ,user_defined_format};
    Q_ENUM(FormatType)
    enum DataType {doubles, longs, characters, strings};
    enum Alignment {Center, Left, Right};
    Q_ENUM(Alignment)

    enum SourceMode {Channel = 0, User};
    Q_ENUM(SourceMode)
    SourceMode getPrecisionMode() const { return thisPrecMode; }
    void setPrecisionMode(SourceMode precmode) {thisPrecMode = precmode; setActualPrecision(thisPrecision);}

    int getPrecision() const {return thisPrecision;}
    void setPrecision(int prec) {thisPrecision = prec; setActualPrecision(prec);}

    enum colMode {Static=0, Alarm};
    Q_ENUM(colMode)
    colMode getColorMode() const { return thisColorMode; }
    void setColorMode(colMode colormode) {thisColorMode = colormode;}

    int getColumnSize() const {return thisColumnSize;}
    void setColumnSize(int newSize);

    QString getPV() const;
    void setPV(QString const &newPV);

    void setDataType(QString const &datatype);

    void setActualPrecision(int prec);
    void displayText(int index, short status, QString const &text);

    void setValueFont(QFont font);

    void setData(double *vector, short status, int size);
    void setData(float *vector, short status, int size);
    void setData(int16_t *vector, short status, int size);
    void setData(int32_t* vector, short status, int size);
    void setData(char* vector, short status, int size);
    void setStringList(QStringList, short status, int size);

    bool getAccessW() const {return _AccessW;}
    void setAccessW(bool access);

    int getNumberOfRows() const {return rowcount;}
    void setNumberOfRows(int nbRows);
    int getNumberOfColumns() const {return colcount;}
    void setNumberOfColumns(int nbCols);

    void setFormatType(FormatType m) { thisFormatType = m;}
    FormatType getFormatType() { return thisFormatType; }
    void setFormatString(const QString m) { thisFormatUserString = m; }
    QString getFormatString() {return thisFormatUserString;}


    void setAlignment(const Alignment &alignment) { thisAlignment = alignment;
                                                    if(colcount > 0 && rowcount > 0) setupItems(rowcount, colcount);
                                                  }
    Alignment getAlignment() const {return thisAlignment;}

                                                  void copy();

    void noStyle(QString stylesheet);
    QString getHorizontalString() const;
    void setHorizontalString(const QString &newHorizontalString);

    QString getVerticalString() const;
    void setVerticalString(const QString &newVerticalString);

    int getHorizontalOffset() const;
    void setHorizontalOffset(int newHorizontalOffset);

    int getVerticalOffset() const;
    void setVerticalOffset(int newVerticalOffset);

    QString getHeaderCSV();

public slots:
    void animation(QRect p) {
#include "animationcode.h"
    }

    void hideObject(bool hideit) {
#include "hideobjectcode.h"
    }
    void vscrollbarControl(int scrollvalue);
    void hscrollbarControl(int scrollvalue);

    /**
     * @brief copies the displayed tabular data to the clipboard in csv format, without headers. String values are ignored / don't work.
     * Uses american format with <csvSeparator> as separators and \n as newlines
     */
    void copyDataCSV();
    /**
     * @brief pastes data from the clipboard (in CSV format) into the table. Format should be exactly as the output from copyDataCSV, so <csvSeparator> as separators and \n as newlines.
     */
    void pasteDataCSV();

private slots:
    void dataInput(int, int);
    void cellDoubleclicked(int, int);
    void cellClicked(int, int);
    void cellChange(int, int, int, int);
    void vscrollbarInput(int scrollvalue);
    void hscrollbarInput(int scrollvalue);


signals:
    void WaveEntryChanged(const QString &text, int index);

    void horizontalStringChanged();

    void verticalStringChanged();

    void horizontalOffsetChanged();

    void verticalOffsetChanged();
    void verticalScrollbarChanged(int);
    void horizontalScrollbarChanged(int);

    void messageWindowOutput(const QtMsgType type, const QString &message);

private:
    bool eventFilter(QObject *obj, QEvent *event);
    void createActions();
    void enableEdit(QTableWidgetItem* pItem);
    void disableEdit(QTableWidgetItem* pItem);
    void setupItems(int nbRows, int nbCols);
    int toIndex(int row, int col);
    void fromIndex(int index, int &row, int &col);
    void setFormat(DataType dataType);
    QString setValue(double value, DataType dataType);
    void RedefineRowColumns(int xsav, int ysav, int z, int &x, int &y);

    bool _AccessW;

    QString	thisPV;
    int	thisColumnSize;
    char thisFormat[MAX_STRING_LENGTH];
    char thisFormatC[MAX_STRING_LENGTH];
    FormatType thisFormatType;
    QString thisFormatUserString;
    bool thisUnsigned;
    Alignment thisAlignment;
    int thisPrecision;
    SourceMode thisPrecMode;
    colMode thisColorMode;
    QFont thisItemFont;
    QAction *copyAct;

    QVector<QString> keepText;
    QVector<double> keepData;
    DataType keepDatatype;
    int keepDatasize;
    short keepStatus;

    int blockIndex;

    int colcount;
    int rowcount;
    bool dataPresent;

    QColor defaultForeColor;

    int channelPrecision;
    int actualPrecision;
    int colSaved, rowSaved, sizeSaved;
    QString horizontalString;
    QString verticalString;
    int horizontalOffset;
    int verticalOffset;

    QChar csvSeparator;
};

#endif
