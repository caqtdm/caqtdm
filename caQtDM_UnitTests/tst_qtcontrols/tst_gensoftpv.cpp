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
#include "tst_gensoftpv.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

static QJsonObject parseConfig(const QString &json)
{
    QJsonParseError error;
    QJsonDocument document = QJsonDocument::fromJson(json.toUtf8(), &error);
    if(error.error != QJsonParseError::NoError) return QJsonObject();
    return document.object();
}

void TestGenSoftPV::defaultConfigIsMinimal()
{
    genSoftPV widget;
    QJsonObject object = parseConfig(widget.buildConfigJSON());

    // only the type appears, everything else keeps the plugin defaults
    QCOMPARE(object.value("type").toString(), QString("double"));
    QCOMPARE(object.contains("mode"), false);
    QCOMPARE(object.contains("val"), false);
    QCOMPARE(object.contains("step"), false);
    QCOMPARE(object.contains("period"), false);
    QCOMPARE(object.contains("drvl"), false);
    QCOMPARE(object.contains("drvh"), false);
    QCOMPARE(object.contains("hopr"), false);
    QCOMPARE(object.contains("lopr"), false);
    QCOMPARE(object.contains("low"), false);
    QCOMPARE(object.contains("lolo"), false);
    QCOMPARE(object.contains("high"), false);
    QCOMPARE(object.contains("hihi"), false);
    QCOMPARE(object.contains("overflow"), false);
    QCOMPARE(object.contains("persistent"), false);
    QCOMPARE(object.contains("nelm"), false);
    QCOMPARE(object.contains("nord"), false);
    QCOMPARE(object.contains("units"), false);
    QCOMPARE(object.contains("prec"), false);
    QCOMPARE(object.contains("enums"), false);
    QCOMPARE(object.contains("regex"), false);
}

void TestGenSoftPV::allFieldsAppearInConfig()
{
    genSoftPV widget;
    widget.setVariable("RAMP");
    widget.setDataType(genSoftPV::Float);
    widget.setMode(genSoftPV::Counter);
    widget.setValue("5");
    widget.setStep(0.5);
    widget.setPeriod(200);
    widget.setDrvl("0");
    widget.setDrvh("100");
    widget.setLopr("10");
    widget.setHopr("90");
    widget.setLow("20");
    widget.setLolo("10");
    widget.setHigh("80");
    widget.setHihi("90");
    widget.setoverflow(false);
    widget.setPersistent(true);
    widget.setNelm(8);
    widget.setNord(3);
    widget.setUnits("V");
    widget.setPrecision(2);

    QJsonObject object = parseConfig(widget.buildConfigJSON());
    QCOMPARE(object.value("type").toString(), QString("float"));
    QCOMPARE(object.value("mode").toString(), QString("counter"));
    QCOMPARE(object.value("val").toDouble(), 5.0);
    QCOMPARE(object.value("step").toDouble(), 0.5);
    QCOMPARE(object.value("period").toInt(), 200);
    QCOMPARE(object.value("drvl").toDouble(), 0.0);
    QCOMPARE(object.value("drvh").toDouble(), 100.0);
    QCOMPARE(object.value("lopr").toDouble(), 10.0);
    QCOMPARE(object.value("hopr").toDouble(), 90.0);
    QCOMPARE(object.value("low").toDouble(), 20.0);
    QCOMPARE(object.value("lolo").toDouble(), 10.0);
    QCOMPARE(object.value("high").toDouble(), 80.0);
    QCOMPARE(object.value("hihi").toDouble(), 90.0);
    QCOMPARE(object.value("overflow").toBool(true), false);
    QCOMPARE(object.value("persistent").toBool(false), true);
    QCOMPARE(object.value("nelm").toInt(), 8);
    QCOMPARE(object.value("nord").toInt(), 3);
    QCOMPARE(object.value("units").toString(), QString("V"));
    QCOMPARE(object.value("prec").toInt(), 2);
}

void TestGenSoftPV::valueListsAndTextsWork()
{
    // a ';' separated list becomes a JSON array of numbers
    genSoftPV wave;
    wave.setValue("1.5;2.5;3.5");
    QJsonObject object = parseConfig(wave.buildConfigJSON());
    QJsonArray array = object.value("val").toArray();
    QCOMPARE(array.size(), 3);
    QCOMPARE(array.at(0).toDouble(), 1.5);
    QCOMPARE(array.at(2).toDouble(), 3.5);

    // for a string channel the value stays text
    genSoftPV message;
    message.setDataType(genSoftPV::String);
    message.setValue("hello world");
    object = parseConfig(message.buildConfigJSON());
    QCOMPARE(object.value("val").toString(), QString("hello world"));

    // string lists become arrays of strings
    genSoftPV texts;
    texts.setDataType(genSoftPV::String);
    texts.setValue("one;two");
    object = parseConfig(texts.buildConfigJSON());
    array = object.value("val").toArray();
    QCOMPARE(array.at(0).toString(), QString("one"));
    QCOMPARE(array.at(1).toString(), QString("two"));

    // enum states and regex
    genSoftPV state;
    state.setDataType(genSoftPV::Enum);
    state.setEnumStrings(QStringList() << "OFF" << "ON");
    state.setRegex("STATE-[0-9]{2}");
    object = parseConfig(state.buildConfigJSON());
    QCOMPARE(object.value("type").toString(), QString("enum"));
    QCOMPARE(object.value("enums").toArray().size(), 2);
    QCOMPARE(object.value("regex").toString(), QString("STATE-[0-9]{2}"));
}

void TestGenSoftPV::matrixRowsColumnsBuildDimConfig()
{
    // rows and columns > 0 make the channel a matrix of its data type: dim
    // travels, nelm/nord/enums/regex never do
    genSoftPV pose;
    pose.setMatrixRows(4);
    pose.setMatrixColumns(4);
    pose.setValue("1;2;3;4;5;6;7;8;9;10;11;12;13;14;15;16");
    pose.setNelm(8);
    pose.setNord(2);
    pose.setEnumStrings(QStringList() << "OFF" << "ON");
    pose.setRegex("STATE-[0-9]{2}");
    QJsonObject object = parseConfig(pose.buildConfigJSON());
    QCOMPARE(object.value("type").toString(), QString("double"));
    QJsonArray dim = object.value("dim").toArray();
    QCOMPARE(dim.size(), 2);
    QCOMPARE(dim.at(0).toInt(), 4);
    QCOMPARE(dim.at(1).toInt(), 4);
    QCOMPARE(object.value("val").toArray().size(), 16);
    QCOMPARE(object.value("val").toArray().at(15).toDouble(), 16.0);
    QCOMPARE(object.contains("nelm"), false);
    QCOMPARE(object.contains("nord"), false);
    QCOMPARE(object.contains("enums"), false);
    QCOMPARE(object.contains("regex"), false);

    // the element type stays free
    genSoftPV grid;
    grid.setDataType(genSoftPV::Int);
    grid.setMatrixRows(2);
    grid.setMatrixColumns(3);
    object = parseConfig(grid.buildConfigJSON());
    QCOMPARE(object.value("type").toString(), QString("int"));
    QCOMPARE(object.value("dim").toArray().at(1).toInt(), 3);

    // without both dimensions the channel stays a plain waveform
    genSoftPV wave;
    wave.setMatrixRows(4);
    wave.setNelm(8);
    object = parseConfig(wave.buildConfigJSON());
    QCOMPARE(object.contains("dim"), false);
    QCOMPARE(object.value("nelm").toInt(), 8);

    // enum and string channels ignore the shape
    genSoftPV state;
    state.setDataType(genSoftPV::Enum);
    state.setMatrixRows(2);
    state.setMatrixColumns(2);
    QCOMPARE(parseConfig(state.buildConfigJSON()).contains("dim"), false);
}

void TestGenSoftPV::configIsReadableThroughGenericProperty()
{
    // the internal plugin reads the configuration without knowing the class,
    // through QObject::property() on the dispW pointer
    genSoftPV widget;
    widget.setDataType(genSoftPV::Long);
    widget.setValue("42");

    QObject *object = &widget;
    QString config = object->property("channelConfigJSON").toString();
    QJsonObject parsed = parseConfig(config);
    QCOMPARE(parsed.value("type").toString(), QString("long"));
    QCOMPARE(parsed.value("val").toDouble(), 42.0);
}
