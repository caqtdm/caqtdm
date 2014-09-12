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
#include <stdint.h>
#include <qtcontrols_global.h>

typedef char string40[40];

class QTCON_EXPORT caWaveTable : public QTableWidget
{

    Q_OBJECT

    Q_PROPERTY(QString channel READ getPV WRITE setPV)
    Q_PROPERTY(int columnSize READ getColumnSize WRITE setColumnSize)
    Q_PROPERTY(int precision READ getPrecision WRITE setPrecision)
    Q_PROPERTY(SourceMode precisionMode READ getPrecisionMode WRITE setPrecisionMode)
    Q_PROPERTY(Orientation orientation READ getOrientation WRITE setOrientation)
    Q_ENUMS(SourceMode)
    Q_ENUMS(Orientation)

public:

    caWaveTable(QWidget *parent);

    enum SourceMode {Channel = 0, User};
    SourceMode getPrecisionMode() const { return thisPrecMode; }
    void setPrecisionMode(SourceMode precmode) {thisPrecMode = precmode;}

    enum Orientation {Horizontal = 0, Vertical};
    Orientation getOrientation() const { return thisOrientation; }
    void setOrientation(Orientation orientation);
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

signals:

private slots:

protected:

private:

   template <typename pureData>
   void fillData(pureData *array, int size);

    enum { MaxItems = 500 };
    QString	thisPV;
    int	thisColumnSize;

    QString keepText[MaxItems];
    string40 thisFormat;

    int thisPrecision;
    SourceMode thisPrecMode;

    QFont thisItemFont;

    QAction *copyAct;

    QTableWidgetItem* tableItem[MaxItems];

    Orientation thisOrientation;
};

#endif
