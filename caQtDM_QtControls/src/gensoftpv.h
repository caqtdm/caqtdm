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
 *  Copyright (c) 2010 - 2026
 *
 *  Author:
 *    Helge Brands
 *  Contact details:
 *    helge.brands@psi.ch
 */

#ifndef GENSOFTPV_H
#define GENSOFTPV_H

#include <QtCore>
#include <QWidget>
#include "esimplelabel.h"
#include <qtcontrols_global.h>

// defines a soft pv of any EPICS data type for the internal plugin,
// referenced by other widgets as internal://<variable>
class QTCON_EXPORT genSoftPV : public ESimpleLabel
{
    Q_OBJECT

    Q_PROPERTY(QString variable READ getVariable WRITE setVariable)
    Q_PROPERTY(DataType dataType READ getDataType WRITE setDataType)
    Q_PROPERTY(Mode mode READ getMode WRITE setMode)

    // VAL: initial value; scalar number, text (string type) or ';' separated list for waveforms
    Q_PROPERTY(QString value READ getValue WRITE setValue)
    Q_PROPERTY(double step READ getStep WRITE setStep)
    Q_PROPERTY(int period READ getPeriod WRITE setPeriod)

    // EPICS limits; empty string = not defined
    Q_PROPERTY(QString drvl READ getDrvl WRITE setDrvl)
    Q_PROPERTY(QString drvh READ getDrvh WRITE setDrvh)
    Q_PROPERTY(QString hopr READ getHopr WRITE setHopr)
    Q_PROPERTY(QString lopr READ getLopr WRITE setLopr)
    Q_PROPERTY(QString low READ getLow WRITE setLow)
    Q_PROPERTY(QString lolo READ getLolo WRITE setLolo)
    Q_PROPERTY(QString high READ getHigh WRITE setHigh)
    Q_PROPERTY(QString hihi READ getHihi WRITE setHihi)

    Q_PROPERTY(bool overflow READ getoverflow WRITE setoverflow)
    Q_PROPERTY(bool persistent READ getPersistent WRITE setPersistent)

    // NELM: maximum array size, NORD: used elements (-1 = automatic)
    Q_PROPERTY(int nelm READ getNelm WRITE setNelm)
    Q_PROPERTY(int nord READ getNord WRITE setNord)

    // matrix shape (dim = [rows, cols], row-major); 0 = plain scalar/waveform
    Q_PROPERTY(int matrixRows READ getMatrixRows WRITE setMatrixRows)
    Q_PROPERTY(int matrixColumns READ getMatrixColumns WRITE setMatrixColumns)

    Q_PROPERTY(QString units READ getUnits WRITE setUnits)
    Q_PROPERTY(int precision READ getPrecision WRITE setPrecision)
    Q_PROPERTY(QStringList enumStrings READ getEnumStrings WRITE setEnumStrings)
    Q_PROPERTY(QString regex READ getRegex WRITE setRegex)

    // assembled configuration, read by the internal plugin
    Q_PROPERTY(QString channelConfigJSON READ buildConfigJSON DESIGNABLE false STORED false)

    // this will prevent user interference
    Q_PROPERTY(QString styleSheet READ styleSheet WRITE noStyle DESIGNABLE false)

    Q_ENUMS(DataType)
    Q_ENUMS(Mode)

public:
    // append only, .ui files store the enum by name but the config dialog maps by index
    enum DataType { Double = 0, Float, Int, Long, Enum, String, Char };
    enum Mode { Constant = 0, Counter };

    genSoftPV(QWidget *parent = 0);

    void noStyle(QString style) {Q_UNUSED(style);}

    QString getVariable() const {return thisVariable;}
    void setVariable(QString const &var) {thisVariable = var; updateLabel();}

    DataType getDataType() const {return thisDataType;}
    void setDataType(DataType datatype) {thisDataType = datatype; updateLabel();}

    Mode getMode() const {return thisMode;}
    void setMode(Mode mode) {thisMode = mode;}

    QString getValue() const {return thisValue;}
    void setValue(QString const &value) {thisValue = value;}

    double getStep() const {return thisStep;}
    void setStep(double step) {thisStep = step;}

    int getPeriod() const {return thisPeriod;}
    void setPeriod(int period) {thisPeriod = period;}

    QString getDrvl() const {return thisDrvl;}
    void setDrvl(QString const &limit) {thisDrvl = limit;}
    QString getDrvh() const {return thisDrvh;}
    void setDrvh(QString const &limit) {thisDrvh = limit;}
    QString getHopr() const {return thisHopr;}
    void setHopr(QString const &limit) {thisHopr = limit;}
    QString getLopr() const {return thisLopr;}
    void setLopr(QString const &limit) {thisLopr = limit;}
    QString getLow() const {return thisLow;}
    void setLow(QString const &limit) {thisLow = limit;}
    QString getLolo() const {return thisLolo;}
    void setLolo(QString const &limit) {thisLolo = limit;}
    QString getHigh() const {return thisHigh;}
    void setHigh(QString const &limit) {thisHigh = limit;}
    QString getHihi() const {return thisHihi;}
    void setHihi(QString const &limit) {thisHihi = limit;}

    bool getoverflow() const {return thisoverflow;}
    void setoverflow(bool overflow) {thisoverflow = overflow;}
    bool getPersistent() const {return thisPersistent;}
    void setPersistent(bool persistent) {thisPersistent = persistent;}

    int getNelm() const {return thisNelm;}
    void setNelm(int nelm) {thisNelm = nelm;}
    int getNord() const {return thisNord;}
    void setNord(int nord) {thisNord = nord;}
    int getMatrixRows() const {return thisMatrixRows;}
    void setMatrixRows(int rows) {thisMatrixRows = rows;}
    int getMatrixColumns() const {return thisMatrixColumns;}
    void setMatrixColumns(int columns) {thisMatrixColumns = columns;}

    QString getUnits() const {return thisUnits;}
    void setUnits(QString const &units) {thisUnits = units;}
    int getPrecision() const {return thisPrecision;}
    void setPrecision(int precision) {thisPrecision = precision;}
    QStringList getEnumStrings() const {return thisEnumStrings;}
    void setEnumStrings(QStringList const &strings) {thisEnumStrings = strings;}
    QString getRegex() const {return thisRegex;}
    void setRegex(QString const &regex) {thisRegex = regex;}

    // matrix: fixed element count rows*cols, the plugin derives nelm/nord itself
    bool isMatrix() const {return thisMatrixRows > 0 && thisMatrixColumns > 0;}

    // the JSON configuration for the internal plugin, only set fields appear
    QString buildConfigJSON() const;

private:
    void updateLabel();
    static QString dataTypeString(DataType datatype);

    QString thisVariable;
    DataType thisDataType;
    Mode thisMode;
    QString thisValue;
    double thisStep;
    int thisPeriod;
    QString thisDrvl, thisDrvh, thisHopr, thisLopr, thisLow, thisLolo, thisHigh, thisHihi;
    bool thisoverflow;
    bool thisPersistent;
    int thisNelm;
    int thisNord;
    int thisMatrixRows;
    int thisMatrixColumns;
    QString thisUnits;
    int thisPrecision;
    QStringList thisEnumStrings;
    QString thisRegex;
};

#endif // GENSOFTPV_H
