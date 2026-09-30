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

#include "gensoftpv.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

genSoftPV::genSoftPV(QWidget *parent) : ESimpleLabel(parent)
{
    // to start with, clear the stylesheet, so that playing around is not possible
    setStyleSheet("");

    thisVariable = "";
    thisDataType = Double;
    thisMode = Constant;
    thisValue = "";
    thisStep = 1.0;
    thisPeriod = 1000;
    thisoverflow = true;
    thisPersistent = false;
    thisNelm = 1;
    thisNord = -1;
    thisMatrixRows = 0;
    thisMatrixColumns = 0;
    thisUnits = "";
    thisPrecision = -1;
    thisRegex = "";

    setFontScaleMode(WidthAndHeight);
    updateLabel();
}

void genSoftPV::updateLabel()
{
    setText(QString("internal://%1").arg(thisVariable.isEmpty() ? "?" : thisVariable));
}

QString genSoftPV::dataTypeString(DataType datatype)
{
    switch(datatype) {
    case Float:  return "float";
    case Int:    return "int";
    case Long:   return "long";
    case Enum:   return "enum";
    case String: return "string";
    case Char:   return "char";
    case Double:
    default:     return "double";
    }
}

// a value string is either a number, a text or a ';' separated list (waveform)
static QJsonValue valueToJson(const QString &value, bool isStringType)
{
    if(value.contains(';')) {
        QJsonArray array;
        foreach(const QString &item, value.split(';')) {
            bool numeric = false;
            double number = item.trimmed().toDouble(&numeric);
            if(numeric && !isStringType) array.append(number);
            else                         array.append(item.trimmed());
        }
        return array;
    }

    bool numeric = false;
    double number = value.trimmed().toDouble(&numeric);
    if(numeric && !isStringType) return QJsonValue(number);
    return QJsonValue(value);
}

// adds an optional limit only when the property is not empty
static void addLimit(QJsonObject &object, const char *key, const QString &limit)
{
    if(limit.trimmed().isEmpty()) return;
    bool numeric = false;
    double number = limit.trimmed().toDouble(&numeric);
    if(numeric) object[key] = number;
}

// assembles the JSON configuration for the internal plugin, unset fields are left out
QString genSoftPV::buildConfigJSON() const
{
    QJsonObject object;

    object["type"] = dataTypeString(thisDataType);
    if(thisMode == Counter) object["mode"] = QString("counter");

    if(!thisValue.trimmed().isEmpty()) object["val"] = valueToJson(thisValue, thisDataType == String);
    if(thisStep != 1.0) object["step"] = thisStep;
    if(thisPeriod != 1000) object["period"] = thisPeriod;

    addLimit(object, "drvl", thisDrvl);
    addLimit(object, "drvh", thisDrvh);
    addLimit(object, "hopr", thisHopr);
    addLimit(object, "lopr", thisLopr);
    addLimit(object, "low", thisLow);
    addLimit(object, "lolo", thisLolo);
    addLimit(object, "high", thisHigh);
    addLimit(object, "hihi", thisHihi);

    if(!thisoverflow) object["overflow"] = false;
    if(thisPersistent) object["persistent"] = true;

    // a matrix has a fixed element count rows*cols and no enums/regex
    bool matrix = isMatrix() && thisDataType != Enum && thisDataType != String;
    if(matrix) {
        QJsonArray dim;
        dim.append(thisMatrixRows);
        dim.append(thisMatrixColumns);
        object["dim"] = dim;
    } else {
        if(thisNelm > 1) object["nelm"] = thisNelm;
        if(thisNord >= 0) object["nord"] = thisNord;
    }

    if(!thisUnits.isEmpty()) object["units"] = thisUnits;
    if(thisPrecision >= 0) object["prec"] = thisPrecision;

    if(!thisEnumStrings.isEmpty() && !matrix) {
        QJsonArray array;
        foreach(const QString &item, thisEnumStrings) array.append(item);
        object["enums"] = array;
    }
    if(!thisRegex.isEmpty() && !matrix) object["regex"] = thisRegex;

    return QString::fromUtf8(QJsonDocument(object).toJson(QJsonDocument::Compact));
}
