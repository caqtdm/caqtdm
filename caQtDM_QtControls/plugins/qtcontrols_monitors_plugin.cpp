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

#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <QtControls>
#include <qtcontrols_monitors_plugin.h>
#include <qglobal.h>

#ifndef MOBILE
#include <QtDesigner/QtDesigner>
#endif

#include <QtPlugin>
#include <QDebug>

#include "designerPluginTexts.h"

typedef char strng[40];
typedef char longtext[500];

static QString XmlFunc(const char *clss, const char *name, int x, int y, int w, int h,
                strng *propertyname, strng* propertytype, longtext *propertytext, int nb)
{
#ifndef DESIGNER_TOOLTIP_DESCRIPTIONS
    Q_UNUSED(propertytext);
#endif
    QString mess = "";
    QString strng1 = "";
    QString strng2 = "";

    mess = "<ui language=\"c++\"><widget class=\"%1\" name=\"%2\">\
            <property name=\"geometry\">\
            <rect>\
            <x>%3</x>\
            <y>%4</y>\
            <width>%5</width>\
            <height>%6</height>\
            </rect>\
            </property>\
            </widget>";

     mess = mess.arg(clss).arg(name).arg(x).arg(y).arg(w).arg(h);

    if(nb > 0) {
        strng1 = " <customwidgets><customwidget><class>%1</class><propertyspecifications>";
        strng1 = strng1.arg(clss);
        for(int i=0; i<nb; i++) {
#ifdef DESIGNER_TOOLTIP_DESCRIPTIONS
            QString strng3 = "<tooltip name=\"%1\">%2</tooltip>";
            strng3 = strng3.arg(propertyname[i]).arg(propertytext[i]);
            strng1.append(strng3);
#endif
            if(strstr(propertytype[i], "multiline") != (char*) 0) {
                strng2 = " <stringpropertyspecification name=\"%1\" notr=\"true\" type=\"%2\"/>";
                strng2 = strng2.arg(propertyname[i]).arg(propertytype[i]);
            }
            strng1.append(strng2);
        }
        strng1.append(" </propertyspecifications></customwidget></customwidgets>");

    }
    mess.append(strng1);
    mess.append("</ui>");

  //control output in formatted xml format
/*
  QString formattedOutput;
  QDomDocument doc;
  doc.setContent(mess, false);
  QTextStream writer(&formattedOutput);
  doc.save(writer, 4);
  qDebug() << formattedOutput;
*/
    return mess;
}

CustomWidgetInterface_Monitors::CustomWidgetInterface_Monitors(QObject *parent): QObject(parent), d_isInitialized(false)
{
}

void CustomWidgetInterface_Monitors::initialize(QDesignerFormEditorInterface *)
{
    if (d_isInitialized) return;
    d_isInitialized = true;
}

caBitnamesInterface::caBitnamesInterface(QObject *parent): CustomWidgetInterface_Monitors(parent)
{
    strng name[2], type[2] = {"", ""};
    longtext text[2] = {"", ""};

    strcpy(name[0], "channelEnum");
    strcpy(type[0], "multiline");
    strcpy(name[1], "channelValue");
    strcpy(type[1], "multiline");
    d_domXml = XmlFunc("caBitnames", "cabitnames", 0, 0, 160, 90, name, type, text, 2);
    d_toolTip = "[Enum table with state]";
    d_name = "caBitnames";
    d_include = "caBitnames";
    QPixmap qpixmap =  QPixmap(":pixmaps/eflag.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
}

QWidget *caBitnamesInterface::createWidget(QWidget *parent)
{
    return new caBitnames(parent);
}

caLedInterface::caLedInterface(QObject *parent): CustomWidgetInterface_Monitors(parent)
{
    strng name[4], type[4] = {"","","",""};
    longtext text[4] = {CHANNEL, COLORMODE, TRUEVALUE, FALSEVALUE};

    strcpy(name[0], "channel");
    strcpy(type[0], "multiline");
    strcpy(name[1], "colorMode");
    strcpy(name[2], "trueValue");
    strcpy(name[3], "falseValue");
    d_domXml = XmlFunc("caLed", "caled", 0, 0, 30, 30, name, type, text, 4);
    d_toolTip = "[LED for true or false bitstate or for severity states]";
    d_name = "caLed";
    d_include = "caLed";
    QPixmap qpixmap =  QPixmap(":pixmaps/eled.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
}

QWidget *caLedInterface::createWidget(QWidget *parent)
{
    return new caLed(parent);
}

caLinearGaugeInterface::caLinearGaugeInterface(QObject *parent): CustomWidgetInterface_Monitors(parent)
{
    strng name[3], type[3] = {"","",""};
    longtext text[3] = {CHANNEL, ORIENTATION, FILLMODE};

    strcpy(name[0], "channel");
    strcpy(type[0], "multiline");
    strcpy(name[1], "orientation");
    strcpy(name[2], "fillMode");
    d_domXml = XmlFunc("caLinearGauge", "calineargauge", 0, 0, 40, 100, name, type, text, 3);
    d_toolTip = "[Colorized bar monitor]";
    d_name = "caLinearGauge";
    d_include = "caGauge";
    QPixmap qpixmap =  QPixmap(":pixmaps/elineargauge.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
}

QWidget *caLinearGaugeInterface::createWidget(QWidget *parent)
{
    return new caLinearGauge(parent);
}

caCircularGaugeInterface::caCircularGaugeInterface(QObject *parent): CustomWidgetInterface_Monitors(parent)
{
    strng name[3], type[3] = {"","",""};
    longtext text[3] = {CHANNEL, VALUEDISPLAY, GAUGELABEL};

    strcpy(name[0], "channel");
    strcpy(type[0], "multiline");
    strcpy(name[1], "valueDisplayed");
    strcpy(name[2], "label");
    strcpy(type[2], "multiline");
    d_domXml = XmlFunc("caCircularGauge", "cacirculargauge", 0, 0, 100, 100, name, type, text, 3);
    d_toolTip = "[Colorized meter]";
    d_name = "caCircularGauge";
    d_include = "caGauge";
    QPixmap qpixmap =  QPixmap(":pixmaps/ecirculargauge.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
}

QWidget *caCircularGaugeInterface::createWidget(QWidget *parent)
{
    return new caCircularGauge(parent);
}

QWidget *caLineEditInterface::createWidget(QWidget* parent)
{
    return new caLineEdit(parent);
}

caLineEditInterface::caLineEditInterface(QObject* parent) : CustomWidgetInterface_Monitors(parent)
{
    strng name[16], type[16]={"","","","","","","","","","","","","","","",""};
    longtext text[16] = {CHANNEL, LFOREGROUND, LBACKGROUND, LCOLORMODE, FRAMEPRESENT, FRAMECOLOR, FRAMELINEWIDTH, ALARMHANDLING, LPRECISION,
                        PRECISIONMODE, LIMITSMODE, MAXVALUE, MINVALUE, FONTSCALEMODE, UNITSENABLED, FORMATTYPE};

    strcpy(name[0], "channel");
    strcpy(type[0], "multiline");
    strcpy(name[1], "foreground");
    strcpy(name[2], "background");
    strcpy(name[3], "colorMode");
    strcpy(name[4], "framePresent");
    strcpy(name[5], "frameColor");
    strcpy(name[6], "frameLineWidth");
    strcpy(name[7], "alarmHandling");
    strcpy(name[8], "precision");
    strcpy(name[9], "precisionMode");
    strcpy(name[10], "limitsMode");
    strcpy(name[11], "maxValue");
    strcpy(name[12], "minValue");
    strcpy(name[13], "fontScaleMode");
    strcpy(name[14], "unitsEnabled");
    strcpy(name[15], "formatType");
    d_domXml = XmlFunc("caLineEdit", "calineedit", 0, 0, 100, 20, name, type, text, 16);
    d_toolTip = "[Text Monitor]";
    d_name = "caLineEdit";
    d_include = "caLineEdit";
    QPixmap qpixmap =   QPixmap(":pixmaps/textmonitor.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
}

QWidget *caMultiLineStringInterface::createWidget(QWidget* parent)
{
    return new caMultiLineString(parent);
}

caMultiLineStringInterface::caMultiLineStringInterface(QObject* parent) : CustomWidgetInterface_Monitors(parent)
{
    strng name[6], type[6]={"","","","","",""};
    longtext text[6] = {CHANNEL, LFOREGROUND, LBACKGROUND, LCOLORMODE, ALARMHANDLING, FONTSCALEMODE};

    strcpy(name[0], "channel");
    strcpy(type[0], "multiline");
    strcpy(name[1], "foreground");
    strcpy(name[2], "background");
    strcpy(name[3], "colorMode");
    strcpy(name[4], "alarmHandling");
    strcpy(name[5], "fontScaleMode");

    d_domXml = XmlFunc("caMultiLineString", "camultilinestring", 0, 0, 100, 20, name, type, text, 6);
    d_toolTip = "[Multiline String Monitor]";
    d_name = "caMultiLineString";
    d_include = "caMultiLineString";
    QPixmap qpixmap =   QPixmap(":pixmaps/multilinemonitor.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
}

QWidget *caThermoInterface::createWidget(QWidget* parent)
{
    return new caThermo(parent);
}

caThermoInterface::caThermoInterface(QObject* parent) : CustomWidgetInterface_Monitors(parent)
{
    strng name[1], type[1] = {""};
    longtext text[1] = {""};

    strcpy(name[0], "channel");
    strcpy(type[0], "multiline");
    d_domXml = XmlFunc("caThermo", "cathermo", 0, 0, 100, 30, name, type, text, 1);
    d_toolTip = "[Scale & Bar Monitor]";
    d_name = "caThermo";
    d_include = "caThermo";
    QPixmap qpixmap =  QPixmap(":pixmaps/thermo.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
}

QWidget *caMeterInterface::createWidget(QWidget* parent)
{
    return new caMeter(parent);
}

caMeterInterface::caMeterInterface(QObject* parent) : CustomWidgetInterface_Monitors(parent)
{
    strng name[14], type[14] = {"","","","","","","","","","","","","",""};
    longtext text[14] = {CHANNEL, MAXVALUE, MINVALUE, LIMITSMODE, BASECOLOR, SCALEDEFAULTCOLOR, SCALECOLOR, COLORMODE, SCALEENABLED,
                        VALUEDISPLAYED, PRECISION, PRECISIONMODE, FORMATTYPE, UNITSENABLED};

    strcpy(name[0], "channel");
    strcpy(type[0], "multiline");
    strcpy(name[1], "maxValue");
    strcpy(name[2], "minValue");
    strcpy(name[3], "limitsMode");
    strcpy(name[4], "baseColor");
    strcpy(name[5], "scaleDefaultColor");
    strcpy(name[6], "scaleColor");
    strcpy(name[7], "colorMode");
    strcpy(name[8], "scaleEnabled");
    strcpy(name[9], "valueDisplayed");
    strcpy(name[10], "precision");
    strcpy(name[11], "precisionMode");
    strcpy(name[12], "formatType");
    strcpy(name[13], "unitsEnabled");
    d_domXml = XmlFunc("caMeter", "cameter", 0, 0, 75, 75, name, type, text, 14);
    d_toolTip = "[simple Meter Monitor]";
    d_name = "caMeter";
    d_include = "caMeter";
    QPixmap qpixmap =  QPixmap(":pixmaps/meter.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
}


QWidget *caCartesianPlotInterface::createWidget(QWidget* parent)
{
    return new caCartesianPlot(parent);
}

caCartesianPlotInterface::caCartesianPlotInterface(QObject* parent) : CustomWidgetInterface_Monitors(parent)
{
    strng name[18], type[18] = {"","","","","","","","","","","","","","","","","",""};
    longtext text[18]  = {CHANNELLISTC,CHANNELLISTC,CHANNELLISTC,CHANNELLISTC,CHANNELLISTC,CHANNELLISTC,"","","","","","",
                         STRINGFROMLIST, STRINGFROMLIST, STRINGFROMLIST, STRINGFROMLIST, STRINGFROMLIST, STRINGFROMLIST};

    strcpy(name[0], "channelList_1");
    strcpy(name[1], "channelList_2");
    strcpy(name[2], "channelList_3");
    strcpy(name[3], "channelList_4");
    strcpy(name[4], "channelList_5");
    strcpy(name[5], "channelList_6");
    strcpy(type[5], "multiline");
    strcpy(name[6], "countNumOrChannel");
    strcpy(type[6], "multiline");
    strcpy(name[7], "triggerChannel");
    strcpy(type[7], "multiline");
    strcpy(name[8], "eraseChannel");
    strcpy(type[8], "multiline");
    strcpy(name[9], "Title");
    strcpy(type[9], "multiline");
    strcpy(name[10], "TitleX");
    strcpy(type[10], "multiline");
    strcpy(name[11], "TitleY");
    strcpy(type[11], "multiline");
    strcpy(name[12], "channels_1");
    strcpy(type[12], "multiline");
    strcpy(name[13], "channels_2");
    strcpy(type[13], "multiline");
    strcpy(name[14], "channels_3");
    strcpy(type[14], "multiline");
    strcpy(name[15], "channels_4");
    strcpy(type[15], "multiline");
    strcpy(name[16], "channels_5");
    strcpy(type[16], "multiline");
    strcpy(name[17], "channels_6");
    strcpy(type[17], "multiline");
    d_domXml = XmlFunc("caCartesianPlot", "cacartesianplot", 0, 0, 200, 50, name, type, text, 18);
    d_toolTip = "[Cartesian Plot]";
    d_name = "caCartesianPlot";
    d_include = "caCartesianPlot";
    QPixmap qpixmap = QPixmap(":pixmaps/cartesian.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
}

QWidget *caStripPlotInterface::createWidget(QWidget* parent)
{
    return new caStripPlot(parent);
}

caStripPlotInterface::caStripPlotInterface(QObject* parent) : CustomWidgetInterface_Monitors(parent)
{
    strng name[5], type[5] = {"","","","",""};
    longtext text[5] = {CHANNELLIST,"","","",STRINGFROMLIST};

    strcpy(name[0], "channelsList");
    strcpy(name[1], "Title");
    strcpy(type[1], "multiline");
    strcpy(name[2], "TitleX");
    strcpy(type[2], "multiline");
    strcpy(name[3], "TitleY");
    strcpy(type[3], "multiline");
    strcpy(name[4], "channels");
    strcpy(type[4], "multiline");
    d_domXml = XmlFunc("caStripPlot", "castripplot", 0, 0, 170, 70, name, type, text, 5);
    d_toolTip = "[Strip Chart]";
    d_name = "caStripPlot";
    d_include = "caStripPlot";
    QPixmap qpixmap = QPixmap(":pixmaps/qwtplot.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
}

caByteInterface::caByteInterface(QObject *parent): CustomWidgetInterface_Monitors(parent)
{
    strng name[1], type[1] = {""};
    longtext text[1] = {""};

    strcpy(name[0], "channel");
    strcpy(type[0], "multiline");
    d_domXml = XmlFunc("caByte", "cabyte", 0, 0, 20, 120, name, type, text, 1);
    d_toolTip = "[Byte Monitor]";
    d_name = "caByte";
    d_include = "caByte";
    QPixmap qpixmap = QPixmap(":pixmaps/eflag.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
}

QWidget *caByteInterface::createWidget(QWidget *parent)
{
    return new caByte(parent);
}

caTableInterface::caTableInterface(QObject *parent): CustomWidgetInterface_Monitors(parent)
{
    strng name[5], type[5] = {"","","","",""};
    longtext text[5] = {CHANNELLIST,"","","",STRINGFROMLIST};

    strcpy(name[0], "channelsList");
    strcpy(name[1], "scriptCommand");
    strcpy(type[1], "multiline");
    strcpy(name[2], "scriptParameter");
    strcpy(type[2], "multiline");
    strcpy(name[3], "columnSizes");
    strcpy(type[3], "multiline");
    strcpy(name[4], "channels");
    strcpy(type[4], "multiline");
    d_domXml = XmlFunc("caTable", "catable", 0, 0, 120, 120, name, type, text, 5);
    d_toolTip = "[Table Monitor for a list of channels]";
    d_name = "caTable";
    d_include = "caTable";
    QPixmap qpixmap = QPixmap(":pixmaps/table.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
}

QWidget *caTableInterface::createWidget(QWidget *parent)
{
    return new caTable(parent);
}

caWaveTableInterface::caWaveTableInterface(QObject *parent): CustomWidgetInterface_Monitors(parent)
{
    strng name[1], type[1] = {""};
    longtext text[1] = {""};

    strcpy(name[0], "channel");
    strcpy(type[0], "multiline");
    d_domXml = XmlFunc("caWaveTable", "cawavetable", 0, 0, 120, 60, name, type, text, 1);
    d_toolTip = "[Table Monitor for waveform]";
    d_name = "caWaveTable";
    d_include = "caWaveTable";
    QPixmap qpixmap = QPixmap(":pixmaps/wavetable.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
}

QWidget *caWaveTableInterface::createWidget(QWidget *parent)
{
    return new caWaveTable(parent);
}

QWidget *caCameraInterface::createWidget(QWidget* parent)
{
    return new caCamera(parent);
}

caCameraInterface::caCameraInterface(QObject* parent) : CustomWidgetInterface_Monitors(parent)
{
    strng name[12], type[12] = {"","","","","","","","","","","",""};
    longtext text[12] = {"","","","","","","","",CHANNELLIST,CHANNELLIST,STRINGFROMLIST,STRINGFROMLIST};

    strcpy(name[0], "channelData");
    strcpy(type[0], "multiline");
    strcpy(name[1], "channelWidth");
    strcpy(type[1], "multiline");
    strcpy(name[2], "channelHeight");
    strcpy(type[2], "multiline");
    strcpy(name[3], "channelCode");
    strcpy(type[3], "multiline");
    strcpy(name[4], "channelBPP");
    strcpy(type[4], "multiline");
    strcpy(name[5], "minLevel");
    strcpy(type[5], "multiline");
    strcpy(name[6], "maxLevel");
    strcpy(type[6], "multiline");
    strcpy(name[7], "customColorMap");
    strcpy(type[7], "multiline");
    strcpy(name[8], "ROI_readChannelsList");
    strcpy(name[9], "ROI_writeChannelsList");
    strcpy(name[10], "ROI_readChannels");
    strcpy(type[10], "multiline");
    strcpy(name[11], "ROI_writeChannels");
    strcpy(type[11], "multiline");
    d_domXml = XmlFunc("caCamera", "cacamera", 0, 0, 200, 200, name, type, text, 12);
    d_toolTip = "[Image from camera]";
    d_name = "caCamera";
    d_include = "caCamera";
    QPixmap qpixmap = QPixmap(":pixmaps/camera.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
}

QWidget *caCalcInterface::createWidget(QWidget* parent)
{
    return new caCalc(parent);
}

caCalcInterface::caCalcInterface(QObject* parent) : CustomWidgetInterface_Monitors(parent)
{
    strng name[6], type[6]  = {"","","","","",""};
    longtext text[6] = {"","","","","",""};

    strcpy(name[0], "channel");
    strcpy(type[0], "multiline");
    strcpy(name[1], "channelB");
    strcpy(type[1], "multiline");
    strcpy(name[2], "channelC");
    strcpy(type[2], "multiline");
    strcpy(name[3], "channelD");
    strcpy(type[3], "multiline");
    strcpy(name[4], "calc");
    strcpy(type[4], "multiline");
    strcpy(name[5], "variable");
    strcpy(type[5], "multiline");

    d_domXml = XmlFunc("caCalc", "cacalc", 0, 0, 5, 5, name, type, text, 6);
    d_toolTip = "[define internal process variable]";
    d_name = "caCalc";
    d_include = "caCalc";
    QPixmap qpixmap = QPixmap(":pixmaps/calc.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
}

QWidget *caWaterfallPlotInterface::createWidget(QWidget* parent)
{
    return new caWaterfallPlot(parent);
}

caWaterfallPlotInterface::caWaterfallPlotInterface(QObject* parent) : CustomWidgetInterface_Monitors(parent)
{
    strng name[5], type[5] = {"","","","",""};
    longtext text[5] = {"","","","",""};

    strcpy(name[0], "channel");
    strcpy(type[0], "multiline");
    strcpy(name[1], "countNumOrChannel");
    strcpy(type[1], "multiline");
    strcpy(name[2], "Title");
    strcpy(type[2], "multiline");
    strcpy(name[3], "TitleX");
    strcpy(type[3], "multiline");
    strcpy(name[4], "TitleY");
    strcpy(type[4], "multiline");
    d_domXml = XmlFunc("caWaterfallPlot", "cawaterfallplot", 0, 0, 450, 200, name, type, text, 5);
    d_toolTip = "[Waterfall plot]";
    d_name = "caWaterfallPlot";
    d_include = "caWaterfall";
    QPixmap qpixmap = QPixmap(":pixmaps/waterfall.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
}

QWidget *caScan2DInterface::createWidget(QWidget* parent)
{
    return new caScan2D(parent);
}

#define NUM_SCAN2D_PARAMS 19
caScan2DInterface::caScan2DInterface(QObject* parent) : CustomWidgetInterface_Monitors(parent)
{
    strng name[NUM_SCAN2D_PARAMS], type[NUM_SCAN2D_PARAMS]  = {"","","","","","","","","","","","","","","","","","",""};
    longtext text[NUM_SCAN2D_PARAMS] = {"","","","","","","","","","","","","","","",CHANNELLIST,CHANNELLIST,STRINGFROMLIST,STRINGFROMLIST};

    strcpy(name[0], "channelData");
    strcpy(type[0], "multiline");
    strcpy(name[1], "channelWidth");
    strcpy(type[1], "multiline");
    strcpy(name[2], "channelHeight");
    strcpy(type[2], "multiline");
    strcpy(name[3], "channelCode");
    strcpy(type[3], "multiline");
    strcpy(name[4], "channelBPP");
    strcpy(type[4], "multiline");
    strcpy(name[5], "customColorMap"); // min level, max level, x center of mass, y c.o.m.
    strcpy(type[5], "multiline");
    strcpy(name[6], "minLevel");
    strcpy(type[6], "multiline");
    strcpy(name[7], "maxLevel");
    strcpy(type[7], "multiline");

    strcpy(name[8], "channelXCPT");
    strcpy(type[8], "multiline");
    strcpy(name[9], "channelYCPT");
    strcpy(type[9], "multiline");
    strcpy(name[10], "channelXNEWDATA");
    strcpy(type[10], "multiline");
    strcpy(name[11], "channelYNEWDATA");
    strcpy(type[11], "multiline");
    strcpy(name[12], "channelSAVEDATA_PATH");
    strcpy(type[12], "multiline");
    strcpy(name[13], "channelSAVEDATA_SUBDIR");
    strcpy(type[13], "multiline");
    strcpy(name[14], "channelSAVEDATA_FILENAME");
    strcpy(type[14], "multiline");
    strcpy(name[15], "ROI_readChannelsList");
    strcpy(name[16], "ROI_writeChannelsList");
    strcpy(name[17], "ROI_readChannels");
    strcpy(type[17], "multiline");
    strcpy(name[18], "ROI_writeChannels");
    strcpy(type[18], "multiline");

    d_domXml = XmlFunc("caScan2D", "cascan2d", 0, 0, 200, 200, name, type, text, NUM_SCAN2D_PARAMS);
    d_toolTip = "[Image from Scan2D]";
    d_name = "caScan2D";
    d_include = "caScan2D";
    d_icon = QPixmap(":pixmaps/scan2d.png");
}

CustomWidgetCollectionInterface_Monitors::CustomWidgetCollectionInterface_Monitors(QObject *parent): QObject(parent)
{
    d_plugins.append(new caLedInterface(this));
    d_plugins.append(new caLinearGaugeInterface(this));
    d_plugins.append(new caCircularGaugeInterface(this));
    d_plugins.append(new caMeterInterface(this));
    d_plugins.append(new caLineEditInterface(this));
    d_plugins.append(new caMultiLineStringInterface(this));
    d_plugins.append(new caThermoInterface(this));
    d_plugins.append(new caCartesianPlotInterface(this));
    d_plugins.append(new caStripPlotInterface(this));
    d_plugins.append(new caByteInterface(this));
    d_plugins.append(new caTableInterface(this));
    d_plugins.append(new caWaveTableInterface(this));
    d_plugins.append(new caBitnamesInterface(this));
    d_plugins.append(new caCameraInterface(this));
    d_plugins.append(new caCalcInterface(this));
    d_plugins.append(new caWaterfallPlotInterface(this));
    d_plugins.append(new caScan2DInterface(this));
}

QList<QDesignerCustomWidgetInterface*> CustomWidgetCollectionInterface_Monitors::customWidgets(void) const
{
    return d_plugins;
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0) 
#else
Q_EXPORT_PLUGIN2(QtControls, CustomWidgetCollectionInterface_Monitors)
#endif
