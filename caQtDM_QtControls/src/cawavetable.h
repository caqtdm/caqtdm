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
    Q_ENUMS(Alignment)

    Q_PROPERTY(colMode colorMode READ getColorMode WRITE setColorMode)
    Q_ENUMS(colMode)

    Q_PROPERTY(int precision READ getPrecision WRITE setPrecision)
    Q_PROPERTY(SourceMode precisionMode READ getPrecisionMode WRITE setPrecisionMode)
    Q_ENUMS(SourceMode)

    Q_PROPERTY(FormatType formatType READ getFormatType WRITE setFormatType)

    // this will prevent user interference
    Q_PROPERTY(QString styleSheet READ styleSheet WRITE noStyle DESIGNABLE false)

    Q_ENUMS(FormatType)

public:
    void noStyle(QString style) {Q_UNUSED(style);}

    caWaveTable(QWidget *parent);

    enum FormatType {decimal, exponential, compact, hexadecimal, octal, string};
    enum DataType {doubles, longs, characters, strings};
    enum Alignment {Center, Left, Right};

    enum SourceMode {Channel = 0, User};
    SourceMode getPrecisionMode() const { return thisPrecMode; }
    void setPrecisionMode(SourceMode precmode) {thisPrecMode = precmode; setActualPrecision(thisPrecision);}

    int getPrecision() const {return thisPrecision;}
    void setPrecision(int prec) {thisPrecision = prec; setActualPrecision(prec);}

    enum colMode {Static=0, Alarm};
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

    void setAlignment(const Alignment &alignment) { thisAlignment = alignment;
                                                    if(colcount > 0 && rowcount > 0) setupItems(rowcount, colcount);
                                                  }
    Alignment getAlignment() const {return thisAlignment;}

                                                  void copy();

public slots:
    void animation(QRect p) {
#include "animationcode.h"
    }

    void hideObject(bool hideit) {
#include "hideobjectcode.h"
    }

private slots:
    void dataInput(int, int);
    void cellDoubleclicked(int, int);
    void cellClicked(int, int);
    void cellChange(int, int, int, int);

signals:
    void WaveEntryChanged(const QString &text, int index);

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
    char thisFormat[20];
    char thisFormatC[20];
    FormatType thisFormatType;
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
};

#endif
