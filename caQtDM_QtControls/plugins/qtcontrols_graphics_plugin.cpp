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
#include <qtcontrols_graphics_plugin.h>
#include <qglobal.h>
#ifndef MOBILE
#include <QtDesigner/QtDesigner>
#endif
#include <QtPlugin>

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

    if(strstr(name, "cadoubletabwidget") != (char*) 0) {
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
            strng1 = " <customwidgets><customwidget><class>%1</class><addpagemethod>addPage</addpagemethod><propertyspecifications>";
            strng1 = strng1.arg(clss);
            for(int i=0; i<nb; i++) {
                strng2 = " <stringpropertyspecification name=\"%1\" notr=\"true\" type=\"%2\"/>";
                strng2 = strng2.arg(propertyname[i]).arg(propertytype[i]);
            }
            strng1.append(strng2);
            strng1.append(" </propertyspecifications></customwidget></customwidgets>");
        }
        mess.append(strng1);
        mess.append("</ui>");

    } else {
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
    }
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

CustomWidgetInterface_Graphics::CustomWidgetInterface_Graphics(QObject *parent): QObject(parent), d_isInitialized(false)
{
}

void CustomWidgetInterface_Graphics::initialize(QDesignerFormEditorInterface *formEditor)
{
    if (d_isInitialized)
        return;

#ifndef MOBILE
    // extension for PolyDraw
    if(d_name.contains("caPolyLine")) {
        QExtensionManager *manager = formEditor->extensionManager();
        Q_ASSERT(manager != 0);
        manager->registerExtensions(new caPolyLineTaskMenuFactory(manager),
                                    Q_TYPEID(QDesignerTaskMenuExtension));
    }
    if(d_name.contains("caDoubleTabWidget")) {
        QExtensionManager *manager = formEditor->extensionManager();
        Q_ASSERT(manager != 0);
        manager->registerExtensions(new caDoubleTabWidgetExtensionFactory(manager),
                                    Q_TYPEID(QDesignerContainerExtension));
    }
#else
    Q_UNUSED(formEditor);
#endif

    // set this property in order to find out later if we use our controls through the designer or otherwise
    qApp->setProperty("APP_SOURCE", QVariant(QString("DESIGNER")));

    d_isInitialized = true;
}

caLabelInterface::caLabelInterface(QObject* parent) : CustomWidgetInterface_Graphics(parent)
{
    strng name[7], type[7] = {"","","","","","",""};
    longtext text[7] = {COLORMODE, VISIBILITY, TEXTPVA, TEXTPVB, TEXTPVC, TEXTPVD, VISIBILITYCALC};

    strcpy(name[0], "colorMode");
    strcpy(name[1], "visibility");
    strcpy(name[2], "channel");
    strcpy(type[2], "multiline");
    strcpy(name[3], "channelB");
    strcpy(type[3], "multiline");
    strcpy(name[4], "channelC");
    strcpy(type[4], "multiline");
    strcpy(name[5], "channelD");
    strcpy(type[5], "multiline");
    strcpy(name[6], "visibilityCalc");
    strcpy(type[6], "multiline");
    d_domXml = XmlFunc("caLabel", "calabel", 0, 0, 90, 28, name, type, text, 7);
    d_name = "caLabel";
    d_include = "caLabel";
    QPixmap qpixmap = QPixmap(":pixmaps/elabel.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
}

QWidget* caLabelInterface::createWidget(QWidget* parent)
{
    return new caLabel(parent);
}

QWidget *caLabelVerticalInterface::createWidget(QWidget* parent)
{
    caLabelVertical *widget = new caLabelVertical(parent);
    return widget;
}

caLabelVerticalInterface::caLabelVerticalInterface(QObject* parent) : CustomWidgetInterface_Graphics(parent)
{
    strng name[9], type[9] = {"","","","","","","","",""};
    longtext text[9] = {COLORMODE, VISIBILITY, TEXTPVA, TEXTPVB, TEXTPVC, TEXTPVD, VISIBILITYCALC, DISPLAYTEXT, DIRECTIONTEXT};

    strcpy(name[0], "colorMode");
    strcpy(name[1], "visibility");
    strcpy(name[2], "channel");
    strcpy(type[2], "multiline");
    strcpy(name[3], "channelB");
    strcpy(type[3], "multiline");
    strcpy(name[4], "channelC");
    strcpy(type[4], "multiline");
    strcpy(name[5], "channelD");
    strcpy(type[5], "multiline");
    strcpy(name[6], "visibilityCalc");
    strcpy(type[6], "multiline");
    strcpy(name[7], "text");
    strcpy(type[7], "multiline");
    strcpy(name[8], "direction");
    d_domXml = XmlFunc("caLabelVertical", "calabelvertical", 0, 0, 28, 90, name, type, text, 9);
    d_name = "caLabelVertical";
    d_include = "caLabelVertical";
    QPixmap qpixmap = QPixmap(":pixmaps/elabelv.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
}


QWidget *caFrameInterface::createWidget(QWidget* parent)
{
    return new caFrame(parent);
}

caFrameInterface::caFrameInterface(QObject* parent) : CustomWidgetInterface_Graphics(parent)
{
    strng name[9], type[9] = {"","","","","","","","",""};
    longtext text[9] = {MACRO, BCKMODE, VISIBILITY, VISIBILITYMODE, TEXTPVA, TEXTPVB, TEXTPVC, TEXTPVD, VISIBILITYCALC};

    strcpy(name[0], "macro");
    strcpy(type[0], "multiline");
    strcpy(name[1], "backgroundMode");
    strcpy(name[2], "visibility");
    strcpy(name[3], "visibilityMode");
    strcpy(name[4], "channel");
    strcpy(type[4], "multiline");
    strcpy(name[5], "channelB");
    strcpy(type[5], "multiline");
    strcpy(name[6], "channelC");
    strcpy(type[6], "multiline");
    strcpy(name[7], "channelD");
    strcpy(type[7], "multiline");
    strcpy(name[8], "visibilityCalc");
    strcpy(type[8], "multiline");
    d_domXml = XmlFunc("caFrame", "caframe", 0, 0, 120, 120, name, type, text, 9);
    d_name = "caFrame";
    d_include = "caFrame";
    QPixmap qpixmap =  QPixmap(":pixmaps/frame.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
}

caImageInterface::caImageInterface(QObject *parent): CustomWidgetInterface_Graphics(parent)
{
    strng name[10], type[10] = {"","","","","","","","","",""};
    longtext text[10] = {IMAGEFILE, IMAGECALC, IMAGEFRAME, IMAGEDELAY, VISIBILITY, VISIBILITYCALC, TEXTPVA, TEXTPVB, TEXTPVC, TEXTPVD};

    strcpy(name[0], "filename");
    strcpy(type[0], "multiline");
    strcpy(name[1], "imageCalc");
    strcpy(type[1], "multiline");
    strcpy(name[2], "frame");
    strcpy(name[3], "delayMilliseconds");
    strcpy(name[4], "visibility");
    strcpy(name[5], "visibilityCalc");
    strcpy(type[5], "multiline");
    strcpy(name[6], "channel");
    strcpy(type[6], "multiline");
    strcpy(name[7], "channelB");
    strcpy(type[7], "multiline");
    strcpy(name[8], "channelC");
    strcpy(type[8], "multiline");
    strcpy(name[9], "channelD");
    strcpy(type[9], "multiline");

    d_domXml = XmlFunc("caImage", "caimage", 0, 0, 70, 70, name, type, text, 10);
    d_name = "caImage";
    d_include = "caImage";
    QPixmap qpixmap = QPixmap(":pixmaps/images.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
}

QWidget *caImageInterface::createWidget(QWidget *parent)
{
    return new caImage(parent);
}

QWidget *caGraphicsInterface::createWidget(QWidget* parent)
{
    return new caGraphics(parent);
}

caGraphicsInterface::caGraphicsInterface(QObject* parent) : CustomWidgetInterface_Graphics(parent)
{
    strng name[16], type[16] = { "","","","","","","","","","","","","","","",""};
    longtext text[16] = {LINESTYLE, LINESIZE, FILLSTYLE, COLORMODE, VISIBILITY, VISIBILITYCALC, TEXTPVA, TEXTPVB, TEXTPVC, TEXTPVD, GRAPHICFORM,
                         GRAPHICSPECIALS, GRAPHICSPECIALS, GRAPHICSPECIALS, GRAPHICSPECIALS, GRAPHICSPECIALS};

    strcpy(name[0], "linestyle");
    strcpy(name[1], "linesize");
    strcpy(name[2], "fillstyle");
    strcpy(name[3], "colorMode");
    strcpy(name[4], "visibility");
    strcpy(name[5], "visibilityCalc");
    strcpy(type[5], "multiline");
    strcpy(name[6], "channel");
    strcpy(type[6], "multiline");
    strcpy(name[7], "channelB");
    strcpy(type[7], "multiline");
    strcpy(name[8], "channelC");
    strcpy(type[8], "multiline");
    strcpy(name[9], "channelD");
    strcpy(type[9], "multiline");
    strcpy(name[10], "form");
    strcpy(name[11], "arrowSize");
    strcpy(name[12], "arrowMode");
    strcpy(name[13], "tiltAngle");
    strcpy(name[14], "startAngle");
    strcpy(name[15], "spanAngle");

    d_domXml = XmlFunc("caGraphics", "cagraphics", 0, 0, 100, 100, name, type, text, 16);
    d_name = "caGraphics";
    d_include = "caGraphics";
    QPixmap qpixmap = QPixmap(":pixmaps/gtool.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
}

QWidget *caPolyLineInterface::createWidget(QWidget *parent)
{
    return new caPolyLine(parent);
}

caPolyLineInterface::caPolyLineInterface(QObject *parent): CustomWidgetInterface_Graphics(parent)
{
    strng name[12], type[12] = {"","","","","","","","","","","",""};
    longtext text[12] = {LINESTYLE, LINESIZE, FILLSTYLE, COLORMODE, POLYSTYLE, VISIBILITY, VISIBILITYCALC, TEXTPVA, TEXTPVB, TEXTPVC, TEXTPVD, XYPAIRS};
    strcpy(name[0], "linestyle");
    strcpy(name[1], "lineSize");
    strcpy(name[2], "fillstyle");
    strcpy(name[3], "colorMode");
    strcpy(name[4], "polystyle");
    strcpy(name[5], "visibility");
    strcpy(name[6], "visibilityCalc");
    strcpy(type[6], "multiline");
    strcpy(name[7], "channel");
    strcpy(type[7], "multiline");
    strcpy(name[8], "channelB");
    strcpy(type[8], "multiline");
    strcpy(name[9], "channelC");
    strcpy(type[9], "multiline");
    strcpy(name[10], "channelD");
    strcpy(type[10], "multiline");
    strcpy(name[11], "xyPairs");
    strcpy(type[11], "multiline");
    d_domXml = XmlFunc("caPolyLine", "capolyline", 0, 0, 150, 150, name, type, text, 12);
    d_name = "caPolyLine";
    d_include = "caPolyLine";
    QPixmap qpixmap = QPixmap(":pixmaps/polyline.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
}


QWidget *caIncludeInterface::createWidget(QWidget* parent)
{
    return new caInclude(parent);
}

caIncludeInterface::caIncludeInterface(QObject* parent) : CustomWidgetInterface_Graphics(parent)
{
    strng name[12], type[12] = {"","","","","","","","","","","", ""};
    longtext text[12] = {INCLUDEMACRO, INCLUDEFILE, VISIBILITY, VISIBILITYCALC, TEXTPVA, TEXTPVB, TEXTPVC, TEXTPVD, INCLUDESTACKING,
                       INCLUDEMAXLINES, INCLUDENBITEMS, STRINGFROMLIST};

    strcpy(name[0], "macro");
    strcpy(type[0], "multiline");
    strcpy(name[1], "filename");
    strcpy(type[1], "multiline");
    strcpy(name[2], "visibility");
    strcpy(name[3], "visibilityCalc");
    strcpy(type[3], "multiline");
    strcpy(name[4], "channel");
    strcpy(type[4], "multiline");
    strcpy(name[5], "channelB");
    strcpy(type[5], "multiline");
    strcpy(name[6], "channelC");
    strcpy(type[6], "multiline");
    strcpy(name[7], "channelD");
    strcpy(type[7], "multiline");
    strcpy(name[8], "stacking");
    strcpy(name[9], "maximumLines");
    strcpy(name[10], "numberOfItems");
    strcpy(name[11], "macroList");

    d_domXml = XmlFunc("caInclude", "cainclude", 0, 0, 100, 100, name, type, text, 12);
    d_name = "caInclude";
    d_include = "caInclude";
    QPixmap qpixmap = QPixmap(":pixmaps/frame.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
}

QWidget *caDoubleTabWidgetInterface::createWidget(QWidget* parent)
{
    caDoubleTabWidget *widget = new caDoubleTabWidget(parent);
    return widget;
}

caDoubleTabWidgetInterface::caDoubleTabWidgetInterface(QObject* parent) : CustomWidgetInterface_Graphics(parent)
{
    strng name[2], type[2] = {"",""};
    longtext text[2] = {"",""};

    strcpy(name[0], "itemsHorizontal");
    strcpy(type[0], "multiline");
    strcpy(name[1], "itemsVertical");
    strcpy(type[1], "multiline");
    d_domXml = XmlFunc("caDoubleTabWidget", "cadoubletabwidget", 0, 0, 250, 250, name, type, text, 2);
    d_name = "caDoubleTabWidget";
    d_include = "caDoubleTabWidget";
    QPixmap qpixmap = QPixmap(":pixmaps/tabwidget.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
}

QWidget *caClockInterface::createWidget(QWidget* parent)
{
    caClock *widget = new caClock(parent);
    return widget;
}

caClockInterface::caClockInterface(QObject* parent) : CustomWidgetInterface_Graphics(parent)
{
    strng name[3], type[3] = {"","",""};
    longtext text[3] = {CHANNEL, TIMETYPE, COLORMODE};

    strcpy(name[0], "channel");
    strcpy(type[0], "multiline");
    strcpy(name[1], "timeType");
    strcpy(name[2], "colorMode");
    d_domXml = XmlFunc("caClock", "caclock", 0, 0, 125, 125, name, type, text, 3);
    d_name = "caClock";
    d_include = "caClock";
    QPixmap qpixmap = QPixmap(":pixmaps/clock.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
}

CustomWidgetCollectionInterface_Graphics::CustomWidgetCollectionInterface_Graphics(QObject *parent): QObject(parent)
{
    d_plugins.append(new caFrameInterface(this));
    d_plugins.append(new caLabelInterface(this) );
    d_plugins.append(new caLabelVerticalInterface(this) );
    d_plugins.append(new caGraphicsInterface(this));
    d_plugins.append(new caPolyLineInterface(this));
    d_plugins.append(new caImageInterface(this));
    d_plugins.append(new caIncludeInterface(this));
    d_plugins.append(new caDoubleTabWidgetInterface(this));
    d_plugins.append(new caClockInterface(this));
}

QList<QDesignerCustomWidgetInterface*> CustomWidgetCollectionInterface_Graphics::customWidgets(void) const
{
    return d_plugins;
}
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0) 
#else
Q_EXPORT_PLUGIN2(QtControls, CustomWidgetCollectionInterface_Graphics)
#endif
