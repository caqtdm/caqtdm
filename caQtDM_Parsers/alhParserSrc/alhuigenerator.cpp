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

#include "alhuigenerator.h"
#include <QBuffer>
#include <QFileInfo>
#include <QXmlStreamWriter>

static void stringProperty(QXmlStreamWriter &xml, const QString &name, const QString &value, bool custom)
{
    xml.writeStartElement("property");
    xml.writeAttribute("name", name);
    if(custom) xml.writeAttribute("stdset", "0");
    xml.writeStartElement("string");
    xml.writeAttribute("notr", "true");
    xml.writeCharacters(value);
    xml.writeEndElement();
    xml.writeEndElement();
}

static void boolProperty(QXmlStreamWriter &xml, const QString &name, bool value)
{
    xml.writeStartElement("property");
    xml.writeAttribute("name", name);
    xml.writeAttribute("stdset", "0");
    xml.writeTextElement("bool", value ? "true" : "false");
    xml.writeEndElement();
}

static void customWidget(QXmlStreamWriter &xml, const QString &klass, const QString &extends)
{
    xml.writeStartElement("customwidget");
    xml.writeTextElement("class", klass);
    xml.writeTextElement("extends", extends);
    xml.writeTextElement("header", klass);
    xml.writeEndElement();
}

QByteArray AlhUiGenerator::generate(const Params &p)
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    QXmlStreamWriter xml(&buffer);
    xml.setAutoFormatting(true);
    xml.setAutoFormattingIndent(1);

    const QString title = p.title.isEmpty() ? QFileInfo(p.configFile).fileName() : p.title;

    xml.writeStartDocument();
    xml.writeStartElement("ui");
    xml.writeAttribute("version", "4.0");
    xml.writeTextElement("class", "MainWindow");

    xml.writeStartElement("widget");
    xml.writeAttribute("class", "QMainWindow");
    xml.writeAttribute("name", "MainWindow");
    xml.writeStartElement("property");
    xml.writeAttribute("name", "geometry");
    xml.writeStartElement("rect");
    xml.writeTextElement("x", "0");
    xml.writeTextElement("y", "0");
    xml.writeTextElement("width", QString::number(p.width));
    xml.writeTextElement("height", QString::number(p.height));
    xml.writeEndElement();
    xml.writeEndElement();
    stringProperty(xml, "windowTitle", title, false);

    xml.writeStartElement("widget");
    xml.writeAttribute("class", "QWidget");
    xml.writeAttribute("name", "centralwidget");
    xml.writeStartElement("layout");
    xml.writeAttribute("class", "QVBoxLayout");
    xml.writeAttribute("name", "verticalLayout");
    xml.writeStartElement("item");

    xml.writeStartElement("widget");
    xml.writeAttribute("class", "QSplitter");
    xml.writeAttribute("name", "splitter");
    xml.writeStartElement("property");
    xml.writeAttribute("name", "orientation");
    xml.writeTextElement("enum", "Qt::Vertical");
    xml.writeEndElement();

    xml.writeStartElement("widget");
    xml.writeAttribute("class", "caAlarmTree");
    xml.writeAttribute("name", "alarmTree");
    stringProperty(xml, "configFile", p.configFile, true);
    if(!p.includeDir.isEmpty()) stringProperty(xml, "includeDir", p.includeDir, true);
    if(!p.macros.isEmpty()) stringProperty(xml, "macros", p.macros, true);
    if(p.withLog) stringProperty(xml, "logTarget", p.logTarget, true);
    boolProperty(xml, "alarmMode", p.alarmMode);
    xml.writeEndElement(); // caAlarmTree

    if(p.withLog) {
        xml.writeStartElement("widget");
        xml.writeAttribute("class", "caAlarmLog");
        xml.writeAttribute("name", p.logTarget);
        xml.writeEndElement();
    }

    xml.writeEndElement(); // splitter
    xml.writeEndElement(); // item
    xml.writeEndElement(); // layout
    xml.writeEndElement(); // centralwidget
    xml.writeEndElement(); // QMainWindow

    xml.writeStartElement("customwidgets");
    customWidget(xml, "caAlarmTree", "QWidget");
    if(p.withLog) customWidget(xml, "caAlarmLog", "QTableView");
    xml.writeEndElement();
    xml.writeEmptyElement("resources");
    xml.writeEmptyElement("connections");

    xml.writeEndElement(); // ui
    xml.writeEndDocument();
    buffer.close();
    return buffer.data();
}
