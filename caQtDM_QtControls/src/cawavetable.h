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
    Q_PROPERTY(int precision READ getPrecision WRITE setPrecision)
    Q_PROPERTY(SourceMode precisionMode READ getPrecisionMode WRITE setPrecisionMode)
    Q_ENUMS(SourceMode)
    Q_ENUMS(Orientation)

public:

    caWaveTable(QWidget *parent);

    enum SourceMode {Channel = 0, User};
    SourceMode getPrecisionMode() const { return thisPrecMode; }
    void setPrecisionMode(SourceMode precmode) {thisPrecMode = precmode;}

    int getPrecision() const {return thisPrecision;}
    void setPrecision(int prec) {thisPrecision = prec; setFormat(prec);}

    int getColumnSize() const {return thisColumnSize;}
    void setColumnSize(int newSize);

    QString getPV() const;
    void setPV(QString const &newPV);

    void setFormat(int prec);
    void displayText(int index, QString const &text);

    void setValueFont(QFont font);

    void setData(double *vector, int size);
    void setData(float *vector, int size);
    void setData(int16_t *vector, int size);
    void setData(int32_t* vector, int size);
     void setData(char* vector, int size);

    int getAccessW() const {return _AccessW;}
    void setAccessW(int access);
    void updateText(const QString &text);

    int getNumberOfRows() const {return rowcount;}
    void setNumberOfRows(int nbRows);
    int getNumberOfColumns() const {return colcount;}
    void setNumberOfColumns(int nbCols);

private slots:
    void copy();
    void dataInput(int, int);
    void cellDoubleclicked(int, int);
    void cellClicked(int, int);
    void cellChange(int, int, int, int);

signals:
    void WaveEntryChanged(double value, int index);

private:
    bool eventFilter(QObject *obj, QEvent *event);
    void createActions();
    void enableEdit(QTableWidgetItem* pItem);
    void disableEdit(QTableWidgetItem* pItem);
    void setupItems(int nbRows, int nbCols);
    int toIndex(int row, int col);
    void fromIndex(int index, int &row, int &col);

    bool _AccessW;
    QString startText;
    bool emitted;

   template <typename pureData>
   void fillData(pureData *array, int size);

    QString	thisPV;
    int	thisColumnSize;
    string40 thisFormat;
    int thisPrecision;
    SourceMode thisPrecMode;
    QFont thisItemFont;
    QAction *copyAct;

    QVector<QString> keepText;
    QVector<double> keepValue;
    QVector<bool> blockItem;

    int timerID;
    int colcount;
    int rowcount;
    bool dataPresent;
    bool charsPresent;
};

#endif
