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
#include <qtcontrols_utilities_plugin.h>
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

    //control output in formatted xml format */
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
CustomWidgetInterface_Utilities::CustomWidgetInterface_Utilities(QObject *parent): QObject(parent), d_isInitialized(false)
{
}

void CustomWidgetInterface_Utilities::initialize(QDesignerFormEditorInterface *)
{
    if (d_isInitialized)
        return;

    // set this property in order to find out later if we use our controls through the designer or otherwise
    qApp->setProperty("APP_SOURCE", QVariant(QString("DESIGNER")));

    d_isInitialized = true;
}

QWidget *wmSignalPropagatorInterface::createWidget(QWidget* parent)
{
    return new wmSignalPropagator(parent);
}

wmSignalPropagatorInterface::wmSignalPropagatorInterface(QObject* parent) : CustomWidgetInterface_Utilities(parent)
{
    strng name[4], type[4] = {""};
    longtext text[1] = {""};


    d_domXml = XmlFunc("wmSignalPropagator", "wmsignalpropagator", 0, 0, 70, 20, name, type, text, 0);
    d_name = "wmSignalPropagator";
    d_include = "wmSignalPropagator";

    QPixmap qpixmap =   QPixmap(":pixmaps/wmsignal.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    d_toolTip = "[this widget gives the possibility to propagate window management signals like close, ...]";
    d_whatsThis = "";
}

QWidget *replaceMacroInterface::createWidget(QWidget* parent)
{
    return new replaceMacro(parent);
}

replaceMacroInterface::replaceMacroInterface(QObject* parent) : CustomWidgetInterface_Utilities(parent)
{
    strng name[5], type[5] = {"", "", "", "",""};
    longtext text[5] = {"a list of values where the value for the specified macroKey can be defined during run-time", STRINGFROMLIST,
                        "name of macro for which value chosen at run-time will be replaced, in case of an unknown macro it will be added and initialized with macroValue",
                        "value that will be replaced for the key choosen in the associated combobox (filled during run-time)",
                        "channel that should provide a list of values for the specified macroKey for displayMode=Channel (instead of a fixed list when displayMode=List"};

    strcpy(name[0], "macroValuesList");
    strcpy(name[1], "macroValues");
    strcpy(type[1], "multiline");
    strcpy(name[2], "macroKey");
    strcpy(type[2], "multiline");
    strcpy(name[3], "macroValue");
    strcpy(type[3], "multiline");
    strcpy(name[4], "enumChannel");
    strcpy(type[4], "multiline");
    d_domXml = XmlFunc("replaceMacro", "replacemacro", 0, 0, 150, 50, name, type, text, 5);
    d_name = "replaceMacro";
    d_include = "replaceMacro";

    QPixmap qpixmap =   QPixmap(":pixmaps/replacemacro.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    d_toolTip = "[can change an existing macro for replacing the macros value for caRelatedDisplay or when reloading the window with ctrl-R]";
    d_whatsThis = "hello, i am a whatsthis string";
}

QWidget *caShellCommandInterface::createWidget(QWidget *parent)
{
    return new caShellCommand(parent);
}

caShellCommandInterface::caShellCommandInterface(QObject *parent): CustomWidgetInterface_Utilities(parent)
{
    strng name[4], type[4] = {"","","",""};
    longtext text[4] = {"","","",""};

    strcpy(name[0], "label");
    strcpy(type[0], "multiline");
    strcpy(name[1], "labelsList");
    strcpy(name[2], "filesList");
    strcpy(name[3], "argsList");
    strcpy(type[3], "multiline");
    d_domXml = XmlFunc("caShellCommand", "cashellcommand", 0, 0, 170, 70, name, type, text, 4);
    d_name = "caShellCommand";
    d_include = "caShellCommand";
    QPixmap qpixmap =   QPixmap(":pixmaps/exclamation.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    d_toolTip = "[menu or button for detached processes]";
    d_whatsThis = "hello, i am a whatsthis string";
}


QWidget *caScriptButtonInterface::createWidget(QWidget* parent)
{
    return new caScriptButton(parent);
}

caScriptButtonInterface::caScriptButtonInterface(QObject* parent) : CustomWidgetInterface_Utilities(parent)
{
    strng name[3], type[3] = {"","",""};
    longtext text[3] = {"","",""};

    strcpy(name[0], "label");
    strcpy(type[0], "multiline");
    strcpy(name[1], "scriptCommand");
    strcpy(type[1], "multiline");
    strcpy(name[2], "scriptParameter");
    strcpy(type[2], "multiline");

    d_domXml = XmlFunc("caScriptButton", "cascriptbutton", 0, 0, 100, 22, name, type, text, 3);
    d_name = "caScriptButton";
    d_include = "caScriptButton";
    QPixmap qpixmap = QPixmap(":pixmaps/process.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    d_toolTip = "[execute a script or image as detached process]";
    d_whatsThis = "hello, i am a whatsthis string";
}
caMimeDisplayInterface::caMimeDisplayInterface(QObject *parent): CustomWidgetInterface_Utilities(parent)
{
    strng name[3], type[3] = {"","",""};
    longtext text[3] = {"","","mime file will be looked up through absolute path or caQtDM_DISPLAY_PATH\nor caQTDM_MIME_PATH. Separate files with a semicolumn\n"};

    strcpy(name[0], "label");
    strcpy(type[0], "multiline");
    strcpy(name[1], "labelsList");
    strcpy(name[2], "filesList");
    strcpy(type[2], "multiline");

    d_domXml = XmlFunc("caMimeDisplay", "camimedisplay", 0, 0, 100, 22, name, type, text, 3);
    d_toolTip = "[Mime display]";
    d_name = "caMimeDisplay";
    d_include = "caMimeDisplay";
    QPixmap qpixmap =  QPixmap(":pixmaps/mime.png");
    d_icon = qpixmap.scaled(90, 90, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    d_toolTip = "[calls a mime application for file]";
}

QWidget *caMimeDisplayInterface::createWidget(QWidget *parent)
{
    return new caMimeDisplay(parent);
}


CustomWidgetCollectionInterface_Utilities::CustomWidgetCollectionInterface_Utilities(QObject *parent): QObject(parent)
{
    d_plugins.append(new replaceMacroInterface(this));
    d_plugins.append(new wmSignalPropagatorInterface(this));
    d_plugins.append(new caShellCommandInterface(this));
    d_plugins.append(new caScriptButtonInterface(this));
    d_plugins.append(new caMimeDisplayInterface(this));
}

QList<QDesignerCustomWidgetInterface*> CustomWidgetCollectionInterface_Utilities::customWidgets(void) const
{
    return d_plugins;
}
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0) 
#else
Q_EXPORT_PLUGIN2(QtControls, CustomWidgetCollectionInterface_Utilities)
#endif
